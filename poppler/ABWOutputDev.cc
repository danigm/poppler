//========================================================================
//
// ABWOutputDev.cc
//
// Jauco Noordzij
//
// Based somewhat on HtmlOutputDev.cc
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <math.h>
#include "goo/GooString.h"
#include "goo/GooList.h"
#include "UnicodeMap.h"
#include "goo/gmem.h"
#include "Error.h"
#include "GfxState.h"
#include "GlobalParams.h"
#include "ABWOutputDev.h"
#include "PDFDoc.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/debugXML.h>


// Inter-character space width which will cause addChar to start a new
// word.
#define minWordBreakSpace 0.1

// Maximum inter-word spacing, as a fraction of the font size.
#define maxWordSpacing 1.5

// Max distance between baselines of two lines within a block, as a
// fraction of the font size.
#define maxLineSpacingDelta 1.5

#define C_maxVCutValue 4
#define C_maxHCutValue 5
//------------------------------------------------------------------------
// ABWOutputDev
//------------------------------------------------------------------------

ABWOutputDev::ABWOutputDev(xmlDocPtr ext_doc)
{
  pdfdoc = NULL;
  N_page = N_style = N_text = N_styleset = N_Block = N_word = NULL;
  doc = ext_doc;
  N_root = xmlNewNode(NULL, BAD_CAST "abiword");
  xmlDocSetRootElement(doc, N_root);
  N_styleset = xmlNewChild(N_root, NULL, BAD_CAST "styles", NULL);
  N_content = xmlNewChild(N_root, NULL, BAD_CAST "content", NULL);
  uMap = globalParams->getTextEncoding();
  maxStyle = Style = 1;
}

ABWOutputDev::~ABWOutputDev() {
  xmlCleanupParser();
}

void ABWOutputDev::startPage(int pageNum, GfxState *state) {
  /*While reading a pdf page this node acts as a placeholder parent.
  when conversion is finished and the page is structured as we like it
  all text fragments are moved from N_page to N_content.*/
  N_page = xmlNewNode(NULL, BAD_CAST "page");
  G_pageNum = pageNum;
} 

/*Callback to denote that poppler reached the end of a page
here I insert most of the interesting processing stuff*/
void ABWOutputDev::endPage() {
  //make sure all words are closed
  endTextBlock();
  cleanUpNode(N_page, true);
  //xmlAddChild(N_content, N_page);
  //xmlSaveFormatFileEnc("pre-cut.xml", doc, "UTF-8", 1);
  //xmlUnlinkNode(N_page);
  //call the top down cutting mechanism
  recursiveXYC(N_page);
  //by stopping to worry about creating empty nodes I made the code quite a 
  //bit more robust. This function makes sure we have a nice'n'clean tree
  cleanUpNode(N_page, true);
  //xmlAddChild(N_content, N_page);
  //xmlSaveFormatFileEnc("raw.xml", doc, "UTF-8", 1);
  //xmlUnlinkNode(N_page);
  
  //Interpret the XY tree and infer text blocks and columns
  interpretXYTree();
  cleanUpNode(N_page, true);
  //xmlAddChild(N_content, N_page);
  //xmlSaveFormatFileEnc("interpreted.xml", doc, "UTF-8", 1);
  //xmlUnlinkNode(N_page);
  
  //I have blocks and columns, this function will turn that into paragraphs and
  //columns
  generateParagraphs();
  cleanUpNode(N_page, true);
  //xmlAddChild(N_content, N_page);
  //xmlSaveFormatFileEnc("paragraphs.xml", doc, "UTF-8", 1);
  
  xmlAddChild(N_content, N_page);
  //just for cleanliness
  N_page = NULL;
}

void ABWOutputDev::recursiveXYC(xmlNodePtr nodeset) {
  /*This function implements the recursive XY Cut. basically, it gets
  the largest piece of whitespace (using getBiggestSeperator()) and then
  splits the page using splitNodes on that whitespace. It calls itself again
  with both the halves*/
  float bhs, bvs, X1, X2, Y1, Y2;
  xmlNodePtr N_cur;

  bvs = getBiggestSeperator(nodeset, VERTICAL, &X1, &X2);
  bhs = getBiggestSeperator(nodeset, HORIZONTAL, &Y1, &Y2);
  
  //printf("***\nbetween %f and %f there is a vertical seperation of %f\n",X1,X2,bvs);
  //printf("between %f and %f there is a horizontal seperation of %f\n",Y1,Y2,bhs);
  
  
  if ((bvs == -1) and (bhs > -1)){
    //printf("Make a horizontal cut!\n");
    splitNodes(Y1, HORIZONTAL, nodeset, bhs);
  }
  else {
    if ((bvs > -1) and (bhs == -1)){
      //printf("Make a vertical cut!\n");
      splitNodes(X1, VERTICAL, nodeset, bvs);
    }
    else {
      if ((bvs > -1) and (bhs > -1)){
        if (bvs >= (bhs/1.7)){
          //When people read a text they prefer vertical cuts over horizontal 
          //ones. I'm not that sure about the 1.7 value, but it seems to work.
          //printf("Make a vertical cut!\n");
          splitNodes(X1, VERTICAL, nodeset, bvs);
        }
        else {
        //printf("Make a horizontal cut!\n");
          splitNodes(Y1, HORIZONTAL, nodeset, bhs);
        }
      }
    }
  }
  if (not((bvs == -1) and (bhs == -1))){
      recursiveXYC(nodeset->children);
      recursiveXYC(nodeset->children->next);
  }
}

void ABWOutputDev::splitNodes(float splitValue, unsigned int direction, xmlNodePtr N_parent, double extravalue){
  //This function takes a nodeset and splits it based on a cut value. It returns
  //the nodePtr with two childnodes, the both chunks.
  xmlNodePtr N_move, N_cur, N_newH, N_newL;
  char * propName;
  const char *nodeName;
  char buf[20];
  if (direction == HORIZONTAL) {propName = "Y1"; nodeName = "horizontal";}
  else { propName = "X1"; nodeName = "vertical";}
  N_newH = xmlNewNode(NULL, BAD_CAST nodeName);
  N_newL = xmlNewNode(NULL, BAD_CAST nodeName);
  sprintf(buf, "%f", extravalue); xmlNewProp(N_newH, BAD_CAST "diff", BAD_CAST buf);
  sprintf(buf, "%f", extravalue); xmlNewProp(N_newL, BAD_CAST "diff", BAD_CAST buf);
  N_cur = N_parent->children;
  //only do this if there are at least two child nodes
  if (direction == HORIZONTAL) {propName = "Y1";}
  else { propName = "X1"; }
  while (N_cur){
    N_move = N_cur->next;
    if (xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST propName)) > splitValue){
      xmlUnlinkNode(N_cur);
      xmlAddChild(N_newH, N_cur);
    }
    else {
      xmlUnlinkNode(N_cur);
      xmlAddChild(N_newL, N_cur);
    }
    N_cur = N_move;
  }
  xmlAddChild(N_parent, N_newL);
  xmlAddChild(N_parent, N_newH);
}

/*
This function gets the biggest whitespace in a portion of the document.
It does so by creating an array of begin and end points of non-whitespace
ex: 132,180,200,279,280,600
whitespace ^       ^
however, to get such an array working nicely required quite a bit of hackish:
"let's predict every possible eventuality and describe appropriate actions for 
it."
Better ideas for managing this information are welcomed.
*/
float ABWOutputDev::getBiggestSeperator(xmlNodePtr N_set, unsigned int direction, float * C1, float * C2)
{
  char * buf;
  float curC1, curC2, retVal;
  xmlNodePtr N_cur;
  unsigned int borders_size = 0;
  unsigned int j;
  float * borders;
  borders = new float[borders_size];
  
  for (N_cur = N_set->children; N_cur; N_cur = N_cur->next){
    if (direction == VERTICAL){
      curC1 = xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "X1"));
      curC2 = xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "X2"));
    }
    else {
      curC1 = xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "Y1"));
      curC2 = xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "Y2"));
    }
    /*printf("borders_size = %d, curC1 = %f, curC2 = %f\n",borders_size,curC1,curC2);
    for (unsigned int i = 0; i<borders_size; i++){
      printf("%f, ",borders[i]);
    }
    printf("\n");*/
    j=0;
    /*
    we skip to the array range by range. At any time borders[j] is the beginning
    of the range, borders[j+1] is the end. borders[j+2] is the beginning of the 
    next one etc.
    */
    while (j < borders_size) {
      if (curC1 < borders[j]) {
        //printf("  110:curC1 < borders[j] (%f)\n",borders[j]);
        if (curC2 < borders[j]) {
          //printf("    112:curC2 < borders[j] (%f)\n",borders[j]);
          //printf("***insert before %d\n",j);
          float * borders_new = new float[borders_size+2]; 
          for (unsigned int n = 0; n < borders_size; n++ ) {
            if (n <j){
              borders_new[n] = borders[n];
            }
            else {
              borders_new[n+2] = borders[n];
            }
          }
          delete [] borders;
          borders = borders_new;
          borders[j]   = curC1;
          borders[j+1] = curC2;
          borders_size = borders_size+2;
          break;
        }
        else {
          //printf("  130:curC1 >= borders[j] (%f)\n",borders[j]);
          if (curC2 <= borders[j+1]){
            //printf("***replace %f with %f\n",borders[j], curC1);
            //printf("    132:curC2 <= borders[j+1] (%f)\n",borders[j+1]);
            borders[j] = curC1;
            break;
          }
          else {
            if (curC2 >= borders[j+2]){
              //printf("***replace larger ones.\n");
              //printf("        155:curC2 >= borders[j+2](%f)\n",borders[j+2]);
              //At this point we have a range whose right coordinate is higher
              //then the left coordinate of the following range. ie. the new one
              //overlaps. This code removes all the elements it overlaps, and
              //inserts the new end-marker
              unsigned int c = 0;
              //first, look for how much array elements need to be removed
              //starting fro the right-hand coordinate of the current range.
              for (unsigned int n = j+2; n < borders_size; n++) {
                if (curC2 < borders[n]){
                  break;
                }
                c++;
              }
              //printf("end: c = %d\n",c);
              //Remove the intermediate ones. insert the curC2
              unsigned int newSize;
              if (c %2 ==0){
                newSize = borders_size-c;
              }
              else {
                newSize = borders_size-c-1;
              }
              float * borders_new = new float[newSize];
              for (unsigned int n = 0; n < borders_size; n++ ) {
                if (n <= j){
                  //printf("*copying: old: n = %d, new: n = %d, value = %f\n",n,n,borders[n]);
                  borders_new[n] = borders[n];
                }
                else {
                  if (c %2 ==0){
                    if (n == j+c+1){
                     //printf("*new value: new: n = %d, value = %f\n",n-c),curC2);
                     borders_new[n-c] = curC2;
                    }
                    else {
                      if (n>j+c+1) {
                        //printf("*copy after: old: n = %d, new: n = %d, value = %f\n",n,n-c,borders[n]);
                        borders_new[n-c] = borders[n];
                      }
                    }
                  }
                  else {
                    if (n>j+c+1) {
                    //printf("*copy after: old: n = %d, new: n = %d, value = %f\n",n,n-c,borders[n]);
                      borders_new[n-c-1] = borders[n];
                    }
                  }
                }
              }
              delete [] borders;
              borders = borders_new;
              borders_size = newSize;
              break;
            }
            else {
              //printf("    137:curC2 > borders[j+1] (%f)\n",borders[j+1]);
              //printf("***replace %f with %f and %f with %f\n",borders[j], curC1, borders[j+1], curC2);
              borders[j] = curC1;
              borders[j+1] = curC2;
              break;
            }
          }
        }
      }
      else {
        //printf("  145:curC1 >= borders[j] (%f)\n",borders[j]);
        if (curC1 <= borders[j+1]) {
          //printf("    147:curC1 < borders[j+1] (%f)\n",borders[j]);
          if (curC2 <= borders[j+1]) {
            //printf("***ignore.\n");
            //printf("      149:curC2 <= borders[j+1] (%f)\n",borders[j+1]);
            break;
          }
          else {
            //printf("      153:curC2 > borders[j+1] (%f)\n",borders[j+1]);
            if (curC2 >= borders[j+2]){
              //printf("***replace larger ones.\n");
              //printf("        155:curC2 >= borders[j+2](%f)\n",borders[j+2]);
              //At this point we have a range whose right coordinate is higher
              //then the left coordinate of the following range. ie. the new one
              //overlaps. This code removes all the elements it overlaps, and
              //inserts the new end-marker
              unsigned int c = 0;
              //first, look for how much array elements need to be removed
              //starting fro the right-hand coordinate of the current range.
              for (unsigned int n = j+2; n < borders_size; n++) {
                if (curC2 < borders[n]){
                  break;
                }
                c++;
              }
              //printf("end: c = %d\n",c);
              //Remove the intermediate ones. insert the curC2
              unsigned int newSize;
              if (c %2 ==0){
                newSize = borders_size-c;
              }
              else {
                newSize = borders_size-c-1;
              }
              float * borders_new = new float[newSize];
              for (unsigned int n = 0; n < borders_size; n++ ) {
                if (n <= j){
                  //printf("*copying: old: n = %d, new: n = %d, value = %f\n",n,n,borders[n]);
                  borders_new[n] = borders[n];
                }
                else {
                  if (c %2 ==0){
                    if (n == j+c+1){
                     //printf("*new value: new: n = %d, value = %f\n",n-c),curC2);
                     borders_new[n-c] = curC2;
                    }
                    else {
                      if (n>j+c+1) {
                        //printf("*copy after: old: n = %d, new: n = %d, value = %f\n",n,n-c,borders[n]);
                        borders_new[n-c] = borders[n];
                      }
                    }
                  }
                  else {
                    if (n>j+c+1) {
                    //printf("*copy after: old: n = %d, new: n = %d, value = %f\n",n,n-c,borders[n]);
                      borders_new[n-c-1] = borders[n];
                    }
                  }
                }
              }
              delete [] borders;
              borders = borders_new;
              borders_size = newSize;
              break;
            }
            else {
              //printf("***replace %f with %f\n", borders[j+1], curC2);
              borders[j+1] = curC2;
              break;
            }
          }
        }
        else {
          //printf("    159:curC1 >= borders[j+1] (%f)\n",borders[j]);
          j += 2;
        }
      }
    }
    if (j >= borders_size){
      //printf("    165:appending curC1 and curC2 to borders\n");
      float * borders_new = new float[borders_size+2]; 
      for (unsigned int n = 0; n < borders_size; n++ ) {
        borders_new[n] = borders[n];
      }
      delete [] borders;
      borders = borders_new;
      borders[borders_size]   = curC1;
      borders[borders_size+1] = curC2;
      borders_size = borders_size+2;
    }
  }
  retVal = -1;
  //printf("%f, ",borders[0]);
  for (unsigned int i = 2; i<borders_size; i+=2){
    //printf("%f, %f, ",borders[i-1],borders[i]);
    if (((borders[i]-borders[i-1]) - retVal) > 0.5){
      retVal = borders[i]-borders[i-1];
      *C1 = borders[i-1];
      *C2 = borders[i];
    }
    //printf("between %f and %f there is a seperation of %f\n",borders[i-1],borders[i],borders[i]-borders[i-1]);
  }
  //printf("\n");
  delete [] borders;
  //Arbitrary cut-of values
/*  if (
      (direction == HORIZONTAL and retVal < C_maxHCutValue) or 
      (direction == VERTICAL and retVal < C_maxVCutValue)
     ) 
  {
    retVal = -1;
  }*/
  return retVal;
}

void ABWOutputDev::updateFont(GfxState *state) {
  char buf[160];
  double x,y;
  xmlNodePtr N_cur;
  GfxFont *font;
  bool found = false;
  xmlChar val[8];
  bool isBold, isItalic, S_isBold, S_isItalic;
  isBold = isItalic = S_isBold =  S_isItalic = false;
  font = state->getFont();
  GooString *ftName;
  char *fnEnd, *fnName;
  int fnLength, fnStart, ftSize;
  //the first time this function is called there is no funt.
  //Fixme: find out if that isn'y a bug
  if (font){
    isBold = (font->isBold() or font->getWeight() >6 or (strstr(font->getOrigName()->getCString(), "Bold")-font->getOrigName()->getCString() == (font->getOrigName()->getLength()-4)));
    isItalic =  (font->isItalic() or (strstr(font->getOrigName()->getCString(), "Italic")-font->getOrigName()->getCString() == (font->getOrigName()->getLength()-6)));
    ftSize = int(state->getTransformedFontSize())-1;
    ftName = new GooString(font->getOrigName());
    fnStart = strcspn(ftName->getCString(), "+");
    if (fnStart < ftName->getLength())
      ftName->del(0,fnStart+1);
    fnEnd = strrchr(ftName->getCString(), 44);
    if (fnEnd == 0)
      fnEnd = strrchr(ftName->getCString(), 45);
    if (fnEnd != 0)
      ftName->del(fnEnd-ftName->getCString(),ftName->getLength()-1);
    
/*    fnName = ftName;
    if (isBold or isItalic){
      fnStart = strcspn(fnName, "+");
      if (fnStart == font->getOrigName()->getLength())
        fnStart = 0;
      else fnStart++;

      fnEnd = strstr(fnName, ",");
      if (fnEnd == 0)
        fnEnd = strstr(fnName, "-");
      if (fnEnd != 0)
        fnName[fnEnd-fnName] = 0;
//      char fntName[fnLength];
//      strncpy (fntName,fnName+fnStart+1,fnLength);
      fnName+=fnStart;
//      fnName = fntName;
    }
    else {*/
      fnName = ftName->getCString();
//    }
    for (N_cur = N_styleset->children; N_cur; N_cur = N_cur ->next){
      if (
       isBold == (xmlStrcasecmp(xmlGetProp(N_cur,BAD_CAST "bold"),BAD_CAST "bold;") == 0)
       and
       isItalic == (xmlStrcasecmp(xmlGetProp(N_cur,BAD_CAST "italic"),BAD_CAST "italic") == 0)
       and
       xmlStrcasecmp(xmlGetProp(N_cur,BAD_CAST "font"),BAD_CAST fnName) == 0
       and
       xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "size")) == ftSize
      ) {
        found = true;
        Style = int(xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "id")));
      }
    }
    if (!found){
      N_cur = xmlNewChild(N_styleset, NULL, BAD_CAST "s", NULL);
      xmlSetProp(N_cur, BAD_CAST "type", BAD_CAST "P");
      sprintf(buf, "%d", maxStyle++);
      xmlSetProp(N_cur, BAD_CAST "name", BAD_CAST buf);
      xmlSetProp(N_cur, BAD_CAST "id", BAD_CAST buf);
      Style = maxStyle;
      sprintf(buf, "%d", ftSize); xmlSetProp(N_cur, BAD_CAST "size", BAD_CAST buf);
      isBold   ? xmlSetProp(N_cur, BAD_CAST "bold", BAD_CAST "bold;")  : xmlSetProp(N_cur, BAD_CAST "bold", BAD_CAST "normal;");
      isItalic ? xmlSetProp(N_cur, BAD_CAST "italic", BAD_CAST "italic"): xmlSetProp(N_cur, BAD_CAST "italic", BAD_CAST "normal");
      xmlSetProp(N_cur, BAD_CAST "font", BAD_CAST fnName);
    }
  }
}

void ABWOutputDev::drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen)
{
  //I wouldn't know what size this should safely be. I guess 64 bytes should be
  //enough for any unicode character
  char buf[64];
  int charLen;
  x = dx;
  y = dy;
  //state->textTransformDelta(dx * state->getHorizScaling(), dy, &dx, &dy);
  //state->transformDelta(dx, dy, &dx, &dy);
  if (uLen == 1 && code == 0x20) {
    //If we break a text sequence on space, then the X1 should be increased
    //but the Y1 and Y2 should remain the same.
    beginWord(state,X2+dx,Y2);
  }
  else {
    X2    += dx;
    Y2    += dy;
    charLen = uMap->mapUnicode(*u,buf,sizeof(buf));
    //Getting Unicode to libxml is something I need to fix.
    //simply passing it using a bad-cast isn't working.
    //I assume that CharCode code it the U+value of the unicode character
    //But for a ligature code gives me DF which is the ringel-s, I guess
    //code should be two bytes wide?
    xmlNodeAddContentLen(N_word, BAD_CAST buf, charLen);
  }
}

void ABWOutputDev::beginString(GfxState *state, GooString *s) {
  double x,y;
  //state->textTransform(x, y, &x, &y);
  state->transform(state->getCurX(), state->getCurY(), &x, &y);
  if (N_word) {
    verDist = y-Y2;
    horDist = x-X2;
    //TEST:changed fabs(horDist) to horDist
    //FIXME: this if statement seems awkward to me.
    if (horDist > (state->getTransformedFontSize()*maxWordSpacing) or (fabs(verDist) > (state->getTransformedFontSize()/maxLineSpacingDelta))) {
      beginTextBlock(state,x,y);
    }
    else {
      if ((horDist > (state->getTransformedFontSize()*minWordBreakSpace)) or (fabs(verDist) > (state->getTransformedFontSize()/maxLineSpacingDelta))) {
        beginWord(state,x,y);
      }
    }
  }
  else {
  //This is the first word. Clear all values and call beginWord;
    X2 = x;
    Y2 = y;
    horDist = 0;
    verDist = 0;
    height  = 0;
    beginTextBlock(state,x,y);
  }
}

void ABWOutputDev::endString(GfxState *state) {

}

void ABWOutputDev::beginWord(GfxState *state, double x, double y){
  char buf[20];
//  printf("***BREAK!***\n");
  endWord();
  X1 = x;
  Y2 = y;

  horDist = X1-X2;
  verDist = Y1-Y2;

  X2 = X1;
  height = state->getFont()->getAscent() * state->getTransformedFontSize();
  Y1 = Y2-height;

  N_word = xmlNewChild(N_Block, NULL, BAD_CAST "word", NULL);
  sprintf(buf, "%f", X1); xmlNewProp(N_word, BAD_CAST "X1", BAD_CAST buf);
  sprintf(buf, "%f", Y1); xmlNewProp(N_word, BAD_CAST "Y1", BAD_CAST buf);
  sprintf(buf, "%d", Style); xmlNewProp(N_word, BAD_CAST "style", BAD_CAST buf);
}

void ABWOutputDev::endWord(){
  char buf[20];
  if (N_word) {
    sprintf(buf, "%f", X2);    xmlNewProp(N_word, BAD_CAST "X2", BAD_CAST buf);
    sprintf(buf, "%f", Y2);    xmlNewProp(N_word, BAD_CAST "Y2", BAD_CAST buf);
    sprintf(buf, "%f", X2-X1); xmlNewProp(N_word, BAD_CAST "width", BAD_CAST buf);
    sprintf(buf, "%f", Y2-Y1); xmlNewProp(N_word, BAD_CAST "height", BAD_CAST buf);
    N_word = NULL;
  }
}

void ABWOutputDev::beginTextBlock(GfxState *state, double x, double y){
  char buf[20];
  endTextBlock();
  N_Block = xmlNewChild(N_page, NULL, BAD_CAST "Textblock", NULL);
  beginWord(state,x,y);
}

void ABWOutputDev::endTextBlock(){
  char buf[20];
  if (N_Block) {
    endWord();
    N_Block = NULL;  
  }
}
/*
This will be a function to retrieve coherent text blocks from the chunk tree.*/
void ABWOutputDev::interpretXYTree(){
  xmlNodePtr N_oldPage;
  N_oldPage = N_page;
  N_page = xmlNewNode(NULL, BAD_CAST "page");
  N_column = N_page;
  //xmlAddChild(N_content, N_page);
  N_Block = xmlNewChild(N_column, NULL, BAD_CAST "chunk", NULL);
  ATP_recursive(N_oldPage);
}

void ABWOutputDev::ATP_recursive(xmlNodePtr N_parent){
  xmlNodePtr N_first, N_second, N_line, N_tempCol, N_tempColset;
  N_first  = N_parent->children;
  if (!N_first)
    return;

  N_second = N_first->next;
  char buf[20];
/*
  Possibilities: 
  there is one child node
    Because we cleaned up before the only case where we allow one childnode is 
    within Textblocks and textBlocks within 'vertical' nodes.
      basically one text node means: add it to the current block.
  There are two childnodes
    This can be two verticals, two horizontals or one horizontal and a text node.
    verticals:
      If the first is vertical, the second is as well.
      verticals mean: create a new Block, add a column per vertical make the
      vertical the block and recurse inside.
      then make the second vertical the block and recurse inside
      then finish the block (ie. create a new one)
    horizontal and or Textblocks
        if first is textnode
          add first to block
          if second is textnode
            at to block
          else
            call again
        else
          begin new block
            call again
          begin new block
          if second is text node
            add to block
          else
            call again
  there are more then two child nodes
    this can be a number of Textblocks and horizontals
    add the textNodes to the current Block
    if a horizontal is encountered enter it and generate a new block afterwards
  */
  //fprintf(stderr,"**********************************************************************\n");
  //xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
  switch (xmlLsCountNode(N_parent)) {
  case 1:
    //fprintf(stderr,"case 1\n");
    N_line = xmlNewChild(N_Block, NULL, BAD_CAST "line", NULL);
    xmlUnlinkNode(N_first);
    xmlAddChild(N_line, N_first);
    break;
  case 2:
    //fprintf(stderr,"case 2\n");
    if (xmlStrcasecmp(N_first->name,BAD_CAST "vertical") == 0){
      //store the column for the moment
      N_tempCol = N_column;
      /*If we have three columns they will turn up in the tree as:
      <vertical>
        <vertical/>
        <vertical/>
      </vertical>
      <vertical/>
      */
      //if the parent is a vertical as well, we can skip the colset generation 
      //thing here we can also remove the just added column and block, because 
      //these are going to replace them
      if (xmlStrcasecmp(N_parent->name,BAD_CAST "vertical") != 0){
        //fprintf(stderr,"first time column\n");
        N_tempColset = N_colset;
        N_colset = xmlNewChild(N_column, NULL, BAD_CAST "colset", NULL);
        N_column = xmlNewChild(N_colset, NULL, BAD_CAST "column", NULL);
        N_Block = xmlNewChild(N_column, NULL, BAD_CAST "chunk", NULL);
      }
      else {
        //fprintf(stderr,"second time column\n");
        xmlUnlinkNode(N_column);
        N_column = xmlNewChild(N_colset, NULL, BAD_CAST "column", NULL);
        N_Block = xmlNewChild(N_column, NULL, BAD_CAST "chunk", NULL);
      }
      //fprintf(stderr,"Building first column...\n");
      ATP_recursive(N_first);
      N_column = xmlNewChild(N_colset, NULL, BAD_CAST "column", NULL);
      N_Block = xmlNewChild(N_column, NULL, BAD_CAST "chunk", NULL);
      //fprintf(stderr,"Building second column...\n");
      ATP_recursive(N_second);
      //make sure we end the column by continuing in the master column and 
      //setting the block and line to it
      N_column = N_tempCol;
      if (xmlStrcasecmp(N_parent->name,BAD_CAST "vertical") != 0){
        N_colset = N_tempColset;
      }
    }
    else {
      if (xmlStrcasecmp(N_first->name,BAD_CAST "Textblock") == 0) {
        //fprintf(stderr,"add first as textblock\n");
        N_line = xmlNewChild(N_Block, NULL, BAD_CAST "line", NULL);
        xmlUnlinkNode(N_first);
        xmlAddChild(N_line, N_first);
        if (xmlStrcasecmp(N_second->name,BAD_CAST "Textblock") == 0) {
          //fprintf(stderr,"add second as textblock\n");
          //FIXME: this is not neat. We should ignore the cut ignoring when there are only two elements above
          //line aggregation doesn't work anyway atm.
          xmlUnlinkNode(N_second);
          xmlAddChild(N_line, N_second);
          //We have two textChunks that are going to be added to the line.
          //the following statements make the line wrap around both textblocks
          //if the firstX1 is smaller then the second X1 use the first, else use the second etc.
        }
        else {
          //fprintf(stderr,"recursing into second\n");
          ATP_recursive(N_second);
        }
      }
      else {
        N_Block = xmlNewChild(N_column, NULL, BAD_CAST "chunk", NULL);
        //fprintf(stderr,"recursing into first\n");
        ATP_recursive(N_first);
        N_Block = xmlNewChild(N_column, NULL, BAD_CAST "chunk", NULL);
        if (xmlStrcasecmp(N_second->name,BAD_CAST "Textblock") == 0) {
          //fprintf(stderr,"add second as textblock\n");
          N_line = xmlNewChild(N_Block, NULL, BAD_CAST "line", NULL);
          xmlUnlinkNode(N_second);
          xmlAddChild(N_line, N_second);
        }
        else {
          //fprintf(stderr,"recursing into second\n");
          ATP_recursive(N_second);
        }
      }
    }
    break;
  default:
    double tX1=0, tX2=0, tY1=0, tY2=0;
    //fprintf(stderr,"case default\n");
    N_line = xmlNewChild(N_Block, NULL, BAD_CAST "line", NULL);
    while (N_first){
      //xmlXPathCastStringToNumber(xmlGetProp(N_first,BAD_CAST "X1")) < tX1 ? tX1 = xmlXPathCastStringToNumber(xmlGetProp(N_first,BAD_CAST "X1")) : tX1 = tX1;
      //xmlXPathCastStringToNumber(xmlGetProp(N_first,BAD_CAST "X2")) > tX2 ? tX2 = xmlXPathCastStringToNumber(xmlGetProp(N_first,BAD_CAST "X2")) : tX2 = tX2;
      //xmlXPathCastStringToNumber(xmlGetProp(N_first,BAD_CAST "Y1")) < tY1 ? tY1 = xmlXPathCastStringToNumber(xmlGetProp(N_first,BAD_CAST "Y1")) : tY1 = tY1;
      //xmlXPathCastStringToNumber(xmlGetProp(N_first,BAD_CAST "Y2")) > tY2 ? tY2 = xmlXPathCastStringToNumber(xmlGetProp(N_first,BAD_CAST "Y2")) : tY1 = tY2;
      N_second = N_first->next;
      if (xmlStrcasecmp(N_first->name,BAD_CAST "Textblock") == 0){
        xmlUnlinkNode(N_first);
        xmlAddChild(N_line, N_first);
      }
      else { //fprintf(stderr,"This shouldn't happen! (line 700)\n");
      }
      N_first = N_second;
    }
    break;
  }
}

/*The cleanup function. It started out as a simple function to remove empty nodes
so that I could call xmladdnewchildnode as often as I liked so that I wouldn't get seg-faults
It is now a bit more advanced, makes sure the tree is as it's supposed to be and adds information too*/
void ABWOutputDev::cleanUpNode(xmlNodePtr N_parent, bool aggregateInfo){
  double tX1=-1, tX2=-1, tY1=-1, tY2=-1;
  xmlNodePtr N_cur, N_next;
  N_cur = N_parent->children;
  char buf[20];
  int prevStyle = -1;
  xmlChar *val;
  int styleLength = xmlLsCountNode(N_styleset)+1;
  float stylePos;
  int styles[styleLength];
  for (int i=1; i< styleLength; i++) { styles[i] = 0;}
  /*
  ignore two horizontal nodes with textBlocks right underneath them. They 
  signal the end of a chunk, and the horizontal seperation needs to be 
  preserved, because it means they are different lines. The second horizontal 
  therefore needs to be kept.
  */
  if ((xmlLsCountNode(N_parent) == 2)
      and
     xmlStrcasecmp(N_parent->name,BAD_CAST "horizontal") == 0
      and 
     N_cur
      and
     N_cur->next
      and
     xmlStrcasecmp(N_cur->name,BAD_CAST "horizontal") == 0 and xmlStrcasecmp(N_cur->next->name,BAD_CAST "horizontal") == 0
      and
     xmlLsCountNode(N_cur) == 1 and xmlLsCountNode(N_cur->next) == 1
      and
     xmlStrcasecmp(N_cur->children->name,BAD_CAST "Textblock") == 0 and xmlStrcasecmp(N_cur->next->children->name,BAD_CAST "Textblock") == 0
     ) {
    xmlAddPrevSibling(N_cur->next,N_cur->children); 
    xmlUnlinkNode(N_cur);
  } 
  /*
  This removes columns if one of the parts is actually a single letter.
  I found out I liked the columns better, so I have the code commented out.
  */
/*  else if ((xmlLsCountNode(N_parent) == 2)
             and
            N_cur
             and
            N_cur->next
             and 
            xmlStrcasecmp(N_cur->name,BAD_CAST "vertical") == 0
             and
            xmlStrcasecmp(N_cur->next->name,BAD_CAST "vertical") == 0
             and 
            (N_cur->children) 
             and
            (N_cur->children->children)
             and
            (N_cur->children->children->children)
             and
            xmlStrlen(N_cur->children->children->children->content) == 1) {
    N_next = N_cur->next;
    xmlAddChild(N_parent, N_next->children);
    xmlAddPrevSibling(N_next->children->children, N_cur->children);
    xmlUnlinkNode(N_cur);
    xmlUnlinkNode(N_next);
  } */else {
    while (N_cur){
      N_next = N_cur->next;
      cleanUpNode(N_cur, aggregateInfo);
      if (xmlLsCountNode(N_cur) == 0 and (xmlStrcasecmp(N_cur->name,BAD_CAST "cbr") != 0) and (xmlStrcasecmp(N_cur->name,BAD_CAST "s") != 0))
        xmlUnlinkNode(N_cur);
      //If the node is still around
      N_cur = N_next;
    }
  }
  //If a countainer element has only one child, it can be removed except for vertical
  //cuts with only one textElement;
  //the main reason for this code is to remove the crumbs after cleaning up in the loop above
  if ((xmlLsCountNode(N_parent) == 1) and ((xmlStrcasecmp(N_parent->name,BAD_CAST "horizontal") == 0) or ((xmlStrcasecmp(N_parent->name,BAD_CAST "vertical") == 0) and (xmlStrcasecmp(N_parent->children->name,BAD_CAST "Textblock") != 0)))){
    N_cur = N_parent->children;
    xmlAddPrevSibling(N_parent,N_cur);
    xmlUnlinkNode(N_parent);
  }
  //We cannot remove the page element so if it has only one childnode, we remove that childnode instead
  if ((xmlStrcasecmp(N_parent->name,BAD_CAST "page") == 0) and (xmlLsCountNode(N_parent) == 1)) {
    N_cur = N_parent->children->children;
    while (N_cur){
      N_next = N_cur->next;
      xmlUnlinkNode(N_cur);
      xmlAddChild(N_parent, N_cur);
      N_cur = N_next;
    }
    xmlUnlinkNode(N_parent->children);
  }
  //Ok, so by this time the N_parent and his children are guaranteed to be clean
  //this for loop gets information from the 'word' elements and propagates it up
  //the tree. 
  if (aggregateInfo and xmlStrcasecmp(N_parent->name,BAD_CAST "word") != 0) {
    for (N_cur = N_parent->children; N_cur; N_cur = N_cur->next){
      val = xmlGetProp(N_cur,BAD_CAST "style");
      stylePos = xmlXPathCastStringToNumber(val);
      //fprintf(stderr,"1: %f, %d\n",stylePos,int(stylePos));
      styles[int(stylePos)]=styles[int(stylePos)]+1;
      //fprintf(stderr,"2: styles[%d] = %d\n",int(stylePos),styles[int(stylePos)]);
      (xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "X1")) < tX1 or tX1 == -1)? tX1 = xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "X1")) : tX1 = tX1;
      (xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "X2")) > tX2)             ? tX2 = xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "X2")) : tX2 = tX2;
      (xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "Y1")) < tY1 or tY1 == -1)? tY1 = xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "Y1")) : tY1 = tY1;
      (xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "Y2")) > tY2)             ? tY2 = xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "Y2")) : tY2 = tY2;
    }
    sprintf(buf, "%f", tX1);     xmlSetProp(N_parent, BAD_CAST "X1", BAD_CAST buf);
    sprintf(buf, "%f", tX2);     xmlSetProp(N_parent, BAD_CAST "X2", BAD_CAST buf);
    sprintf(buf, "%f", tY1);     xmlSetProp(N_parent, BAD_CAST "Y1", BAD_CAST buf);
    sprintf(buf, "%f", tY2);     xmlSetProp(N_parent, BAD_CAST "Y2", BAD_CAST buf);
    sprintf(buf, "%f", tX2-tX1); xmlSetProp(N_parent, BAD_CAST "width", BAD_CAST buf);
    sprintf(buf, "%f", tY2-tY1); xmlSetProp(N_parent, BAD_CAST "height", BAD_CAST buf);
    prevStyle = 0;
    styles[0] = -1;
    for (int i=1; i< styleLength; i++) { if (styles[i] > styles[prevStyle]) prevStyle = i; }
    //fprintf(stderr,"%d\n", prevStyle);
    if (prevStyle > 0){
      sprintf(buf, "%d", prevStyle);     xmlSetProp(N_parent, BAD_CAST "style", BAD_CAST buf);
    }
  }
  if (N_parent->children and xmlStrcasecmp(N_parent->children->name,BAD_CAST "line") == 0 and xmlGetProp(N_parent->children,BAD_CAST "alignment") != NULL)
    xmlSetProp(N_parent, BAD_CAST "alignment", xmlGetProp(N_parent->children,BAD_CAST "alignment"));
}

void ABWOutputDev::generateParagraphs() {
  xmlNodePtr N_cur, N_parent, N_p, N_line, N_next;
  int lvl;
  //basically I first detect the text-alignment within blocks.
  //ASSUMPTION: my block seperation thing is good enough so I don't need to
  //worry about two alignments in one paragraph
  
  X1 = 0;
  X2 = pdfdoc->getPageCropWidth(G_pageNum);
  Y1 = 0;
  Y2 = pdfdoc->getPageCropHeight(G_pageNum);
  addAlignment(N_page);
  
  //then it's a switch per alignement
  N_cur = N_page->children;
  N_parent = N_page;
  lvl = 1;
  while (N_cur) {
    if (xmlStrcasecmp(N_cur->name,BAD_CAST "chunk") == 0){
      N_p = xmlNewNode(NULL, BAD_CAST "chunk");
      xmlAddPrevSibling(N_cur,N_p);
      //N_p = xmlNewChild(N_parent, NULL, BAD_CAST "chunk", NULL);
      //A new paragraph is created when:
      switch (int(xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "alignment")))){
      //left
      case 1: //the distance between the texblock X2 and the last word X2 is more than
         //the following first word width.
         N_line = N_cur->children;
         while (N_line){
           N_next = N_line->next;
           xmlUnlinkNode(N_line);
           xmlAddChild(N_p,N_line);
           xmlSetProp(N_line, BAD_CAST "alignment", BAD_CAST "1");
           if (N_next and xmlStrcasecmp(N_next->name,BAD_CAST "line") == 0){
             if (xmlXPathCastStringToNumber(xmlGetProp(N_next->children->children,BAD_CAST "width")) < (xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "width")) - xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "width")))){
               N_p = xmlNewNode(NULL, BAD_CAST "chunk");
               xmlAddPrevSibling(N_cur,N_p);
             }
           }
           N_line = N_next;
         }
         break;
      //right
      case 2: //the same but now with X1 and first word and following last word
         N_line = N_cur->children;
         while (N_line){
           N_next = N_line->next;
           xmlUnlinkNode(N_line);
           xmlAddChild(N_p,N_line);
           xmlSetProp(N_line, BAD_CAST "alignment", BAD_CAST "2");
           if (N_next and xmlStrcasecmp(N_next->name,BAD_CAST "line") == 0){
             //fprintf(stderr,"width_next=%f, X2_bl=%f, X2_w=%f\n",xmlXPathCastStringToNumber(xmlGetProp(N_next->children->children,BAD_CAST "width")),xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "width")),xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "width")));
             if (xmlXPathCastStringToNumber(xmlGetProp(N_next->children->children,BAD_CAST "width")) < (xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "width")) - xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "width")))){
               N_p = xmlNewNode(NULL, BAD_CAST "chunk");
               xmlAddPrevSibling(N_cur,N_p);
             }
           }
           N_line = N_next;
         }
         break;
      //centered
      case 3: //the combined left and right space is more than the following first word
         N_line = N_cur->children;
         while (N_line){
           N_next = N_line->next;
           xmlUnlinkNode(N_line);
           xmlAddChild(N_p,N_line);
           xmlSetProp(N_line, BAD_CAST "alignment", BAD_CAST "3");
           if (N_next and xmlStrcasecmp(N_next->name,BAD_CAST "line") == 0){
             //fprintf(stderr,"width_next=%f, X2_bl=%f, X2_w=%f\n",xmlXPathCastStringToNumber(xmlGetProp(N_next->children->children,BAD_CAST "width")),xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "width")),xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "width")));
             if (xmlXPathCastStringToNumber(xmlGetProp(N_next->children->children,BAD_CAST "width")) < (xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "width")) - xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "width")))){
               N_p = xmlNewNode(NULL, BAD_CAST "chunk");
               xmlAddPrevSibling(N_cur,N_p);
             }
           }
           N_line = N_next;
         }
         break;
      //justified
      case 4:
         //we break on all alignment=1 lines. A line with alignment=1 that is the first of a block will
         //also initiate a paragraph break before.
         N_line = N_cur->children;
         if (xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "alignment")) == 1){
           N_p = xmlNewNode(NULL, BAD_CAST "chunk");
           xmlAddPrevSibling(N_cur,N_p);
         }
         while (N_line){
           N_next = N_line->next;
           xmlUnlinkNode(N_line);
           xmlAddChild(N_p,N_line);
           if (xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "alignment")) == 1){
             N_p = xmlNewNode(NULL, BAD_CAST "chunk");
             xmlAddPrevSibling(N_cur,N_p);
           }
           xmlSetProp(N_line, BAD_CAST "alignment", BAD_CAST "4");
           N_line = N_next;
         }
         break;
      }
    }
    else if (xmlStrcasecmp(N_cur->name,BAD_CAST "colset") == 0 or xmlStrcasecmp(N_cur->name,BAD_CAST "column") == 0){
      N_parent = N_cur;
      N_cur = N_cur->children;
      lvl++;
      N_p = xmlNewNode(NULL, BAD_CAST "chunk");
      xmlAddPrevSibling(N_cur,N_p);
      continue;
    }
    if (N_cur->next)
      N_cur = N_cur->next;
    else while (lvl > 0){
      N_cur = N_parent;
      N_parent = N_cur->parent;
      lvl--;
      if (N_cur->next){
        N_cur = N_cur->next;
        break;
      }
    }
    if (lvl==0)
      N_cur = NULL;
  }
}

//function that adds an 'alignment=' property to the <chunk>s
void ABWOutputDev::addAlignment(xmlNodePtr N_parent) {
  xmlNodePtr N_chunk, N_line;
  double tX1, tX2;
  bool leftMatch, rightMatch, centerMatch;
  int leftCnt = 0, rightCnt = 0, cntrCnt = 0, justCnt = 0;
  //fprintf(stderr,"Entering addAlignment\n");
  for (N_chunk = N_parent->children; N_chunk; N_chunk = N_chunk->next) {
    if (xmlStrcasecmp(N_chunk->name,BAD_CAST "chunk") == 0){
      X1 = xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X1"));
      X2 = xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X2"));
      //fprintf(stderr,"Found chunk\n");
      //if the chunk contains only one line, we don't need to loop through it.
      if (xmlLsCountNode(N_chunk) == 1){
        //fprintf(stderr,"Processing line\n");
        //fprintf(stderr,"X1=%f, X2=%f, cX1=%f, cX2=%f\n",X1,X2,xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X1")), xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X2")));
        //fprintf(stderr,"%f\n",(xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X1")) - X1)-(X2 - xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X2"))));
        //fprintf(stderr,"cX1-X1=%f, X2-cX2=%f\n",(xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X1")) - X1),(X2 - xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X2"))));
        // a one line chunk, is either centered or left or right-aligned.
        if ((xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X1"))-X1)-(X2-xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X2"))) > 1) {
          xmlNewProp(N_chunk, BAD_CAST "alignment", BAD_CAST "2");
          xmlNewProp(N_chunk->children, BAD_CAST "alignment", BAD_CAST "2");
          //fprintf(stderr,"alignment = right\n");
        }
        else { 
        if ((xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X1"))-X1)-(X2 - xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X2")))< -1) {
          xmlNewProp(N_chunk, BAD_CAST "alignment", BAD_CAST "1");
          xmlNewProp(N_chunk->children, BAD_CAST "alignment", BAD_CAST "1");
          //fprintf(stderr,"alignment = left\n");
        }
        else {
          xmlNewProp(N_chunk, BAD_CAST "alignment", BAD_CAST "3");
          xmlNewProp(N_chunk->children, BAD_CAST "alignment", BAD_CAST "3");
          //fprintf(stderr,"alignment = center\n");
        }
        }
      }
      else {
      leftCnt = 0;
      rightCnt = 0;
      cntrCnt = 0;
      justCnt = 0;
      for (N_line = N_chunk->children; N_line; N_line = N_line->next) {
        //fprintf(stderr,"Processing line\n");
        /*
        |X1 - cX1| == 1
        |X2 - cX2| == 1
        |(cX1-X1)-(X2-cX2)| == 1
        ok, each line can be just as wide as the current set,
        it can be smaller and moved to the right
        it can be smaller and moved to the left.
        it can 
        */
        //fprintf(stderr,"X1=%f, X2=%f, cX1=%f, cX2=%f\n",X1,X2,xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "X1")), xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "X2")));
        //fprintf(stderr,"cX1-X1=%f, X2-cX2=%f\n",(xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "X1")) - X1),(X2 - xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "X2"))));
        leftMatch =  fabs(xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "X1"))-X1) < 2;
        rightMatch =  fabs(X2-xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "X2"))) < 2;
        centerMatch =  fabs((xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "X1"))-X1)-(X2-xmlXPathCastStringToNumber(xmlGetProp(N_line,BAD_CAST "X2")))) < 2;
        if (leftMatch and rightMatch) {
          xmlNewProp(N_line, BAD_CAST "alignment", BAD_CAST "4");
          justCnt++;
        }
        else if (centerMatch) {
          xmlNewProp(N_line, BAD_CAST "alignment", BAD_CAST "3");
          cntrCnt++;
        }
        else if (rightMatch) {
          xmlNewProp(N_line, BAD_CAST "alignment", BAD_CAST "2");
          rightCnt++;
        }
        else {
          xmlNewProp(N_line, BAD_CAST "alignment", BAD_CAST "1");
          leftCnt++;
        }
      }
      //there is almost always one justified line in a centered text
      //and most justified blocks have at least one left aligned line
      //fprintf(stderr,"1:%d ,2:%d ,3:%d ,4:%d\n",leftCnt,justCnt,cntrCnt,rightCnt);
      if ((leftCnt-1 >= justCnt) and (leftCnt >= rightCnt) and (leftCnt >= cntrCnt))
        xmlNewProp(N_chunk, BAD_CAST "alignment", BAD_CAST "1");
      else if ((justCnt >= leftCnt-1) and (justCnt >= rightCnt) and (justCnt >= cntrCnt))
        xmlNewProp(N_chunk, BAD_CAST "alignment", BAD_CAST "4");
      else if ((cntrCnt >= justCnt-1) and (cntrCnt >= rightCnt) and (cntrCnt >= leftCnt))
        xmlNewProp(N_chunk, BAD_CAST "alignment", BAD_CAST "3");
      else
        xmlNewProp(N_chunk, BAD_CAST "alignment", BAD_CAST "2");
      }
    } 
    else {
      if (xmlStrcasecmp(N_chunk->name,BAD_CAST "colset") == 0){
        //fprintf(stderr,"Found a colset\n");
        addAlignment(N_chunk);
      }
      else {
        if (xmlStrcasecmp(N_chunk->name,BAD_CAST "column") == 0){
          //fprintf(stderr,"Found a column\n");
          tX1 = X1;
          tX2 = X2;
          X1 = xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X1"));
          X2 = xmlXPathCastStringToNumber(xmlGetProp(N_chunk,BAD_CAST "X2"));
          addAlignment(N_chunk);
          X1 = tX1;
          X2 = tX2;
        }
        else { //fprintf(stderr,"Found something else\n");
	}
      }
    }
  }
//parse all blocks, and all lines within all blocks
//do a set of checks and tick a flag if the check fails
//check for line X1 is textBlock X1
//check for line X2 is textblock X2
//check if line is centered in textBock (LX1 != TX1 && LX2 != TX2 && LX1-TX1 == TX2=LX2)
//if the LX1 != TX1 then how much is the difference?
//a line isn't left aligned if all lines have a different X1 <= not so strong assumption.

//justified if both are straight except for a couple of (same factor sized) indents at the left
//else centered if above calculation is correct
//else left aligned if left side is more straight than right (more lines in the same X1 or common factor
//else right
}

void ABWOutputDev::setPDFDoc(PDFDoc *priv_pdfdoc) {
  pdfdoc = priv_pdfdoc;
}

void ABWOutputDev::createABW() {
  //*************************************************************
  //change styles to abiword format
  xmlNodePtr N_cur, N_next;
  xmlAttrPtr N_prop;
  char buf[500];
  for (N_cur = N_styleset->children; N_cur; N_cur = N_cur->next){
    sprintf(buf,"margin-top:0pt; color:000000; margin-left:0pt; text-position:normal; widows:2; text-indent:0in; font-variant:normal; margin-right:0pt; lang:nl-NL; line-height:1.0; font-size:%dpt; text-decoration:none; margin-bottom:0pt; bgcolor:transparent; text-align:left; font-stretch:normal;",int(xmlXPathCastStringToNumber(xmlGetProp(N_cur,BAD_CAST "size"))));
    strncat(buf,"font-family:",12);
    strncat(buf,(char *)xmlGetProp(N_cur,BAD_CAST "font"),strlen((char *)xmlGetProp(N_cur,BAD_CAST "font")));
    strncat(buf,";",1);
    strncat(buf,"font-weight:",12);
    strncat(buf,(char *)xmlGetProp(N_cur,BAD_CAST "bold"),strlen((char *)xmlGetProp(N_cur,BAD_CAST "bold")));
    strncat(buf,"font-style:",12);
    strncat(buf,(char *)xmlGetProp(N_cur,BAD_CAST "italic"),strlen((char *)xmlGetProp(N_cur,BAD_CAST "italic")));
    xmlSetProp(N_cur, BAD_CAST "props", BAD_CAST buf);
    N_prop = xmlHasProp(N_cur, BAD_CAST "id");
    if (N_prop != NULL) xmlRemoveProp(N_prop);
    N_prop = xmlHasProp(N_cur, BAD_CAST "size");
    if (N_prop != NULL) xmlRemoveProp(N_prop);
    N_prop = xmlHasProp(N_cur, BAD_CAST "bold");
    if (N_prop != NULL) xmlRemoveProp(N_prop);
    N_prop = xmlHasProp(N_cur, BAD_CAST "italic");
    if (N_prop != NULL) xmlRemoveProp(N_prop);
    N_prop = xmlHasProp(N_cur, BAD_CAST "font");
    if (N_prop != NULL) xmlRemoveProp(N_prop);
  }
  //*************************************************************
  //Change the rest of the document
  //each child of N_content is a page
  N_cur = N_content->children;
  while (N_cur){
    //we creat a section node and attach it to the root, it will com after all
    //the page nodes. Then we transform the page, and finally remove it
    N_next = N_cur->next;
    //fprintf(stderr,"***Transforming page\n");
    N_Block = xmlNewChild(N_root, NULL, BAD_CAST "section", NULL);
    transformPage(N_cur);
    xmlUnlinkNode(N_cur);
    //fprintf(stderr,"***Finished transforming page\n");
    N_cur = N_next;
  }
  cleanUpNode(N_root, false);
}

void ABWOutputDev::transformPage(xmlNodePtr N_parent){
  char buf[60];
  xmlNodePtr N_cur, N_curLine, N_curText, N_curWord, text, space;
  //translate the nodes into abiword nodes
  if (xmlStrcasecmp(N_parent->name,BAD_CAST "page") == 0){
    for (N_cur = N_parent->children; N_cur; N_cur = N_cur->next){
      //fprintf(stderr,"**pass a page child\n");
      transformPage(N_cur);
    }
  }
  if (xmlStrcasecmp(N_parent->name,BAD_CAST "chunk") == 0){
    //fprintf(stderr,"Found a chunk\n");
    //I start a <p> on each chunk and add all word containment
    N_text = xmlNewChild(N_Block, NULL, BAD_CAST "p", NULL);
    if (int(xmlXPathCastStringToNumber(xmlGetProp(N_parent,BAD_CAST "style"))) > 0){
      xmlNewProp(N_text, BAD_CAST "style", xmlGetProp(N_parent,BAD_CAST "style"));
    }
    switch (int(xmlXPathCastStringToNumber(xmlGetProp(N_parent,BAD_CAST "alignment")))){
    case 1: xmlNewProp(N_text, BAD_CAST "props", BAD_CAST "text-align:left");
           break;
    case 2: xmlNewProp(N_text, BAD_CAST "props", BAD_CAST "text-align:right");
           break;
    case 3: xmlNewProp(N_text, BAD_CAST "props", BAD_CAST "text-align:center");
           break;
    case 4: xmlNewProp(N_text, BAD_CAST "props", BAD_CAST "text-align:justify");
           break;
    }
    for (N_curLine = N_parent->children; N_curLine; N_curLine = N_curLine->next){
      //fprintf(stderr,"A line\n");
      for (N_curText = N_curLine->children; N_curText; N_curText = N_curText->next){
        //fprintf(stderr,"a textNode\n");
        for (N_curWord = N_curText->children; N_curWord; N_curWord = N_curWord->next){
          //fprintf(stderr,"a word\n");
          text = N_curWord->children;
          xmlUnlinkNode(text);
          xmlAddChild(N_text,text);
          space = xmlNewText(BAD_CAST " ");
          xmlAddChild(N_text,space);
        }
      }
    }
  }
  if (xmlStrcasecmp(N_parent->name,BAD_CAST "column") == 0){
    //fprintf(stderr,"Found a column\n");
    for (N_cur = N_parent->children; N_cur; N_cur = N_cur->next){
      transformPage(N_cur);
    }
    xmlNewChild(N_text, NULL, BAD_CAST "cbr", NULL);
  }
  if (xmlStrcasecmp(N_parent->name,BAD_CAST "colset") == 0){
    //fprintf(stderr,"Found a colset\n");
    //create new section columns: count childNodes of N_cur
    //recurse through chunks and create textNodes
    N_Block = xmlNewChild(N_root, NULL, BAD_CAST "section", NULL);
    sprintf(buf,"columns:%d",xmlLsCountNode(N_parent));
    xmlNewProp(N_Block, BAD_CAST "props", BAD_CAST buf);
    for (N_cur = N_parent->children; N_cur; N_cur = N_cur->next){
      transformPage(N_cur);
    }
    N_Block = xmlNewChild(N_root, NULL, BAD_CAST "section", NULL);
  }
  //fprintf(stderr,"at the end\n");
}
