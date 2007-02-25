//========================================================================
//
// Annot.cc
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include "goo/gmem.h"
#include "Object.h"
#include "Catalog.h"
#include "Gfx.h"
#include "Lexer.h"
#include "UGooString.h"
#include "Annot.h"
#include "GfxFont.h"
#include "CharCodeToUnicode.h"
#include "Form.h"
#include "Error.h"

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

Annot::Annot(XRef *xrefA, Dict *acroForm, Dict *dict, const Ref& aref, Catalog* catalog)
{
  hasRef = true;
  ref = aref; 
  initialize (xrefA, acroForm, dict, catalog);
}

Annot::Annot(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog* catalog) {
  hasRef = false;
  initialize (xrefA, acroForm, dict, catalog);
}

void Annot::initialize(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog) {
  Object apObj, asObj, obj1, obj2;
  double t;

  ok = gTrue;
  xref = xrefA;
  appearBuf = NULL;
  fontSize = 0;
  widget = NULL;

  if (dict->lookup("Rect", &obj1)->isArray() &&
      obj1.arrayGetLength() == 4) {
    readArrayNum(&obj1, 0, &xMin);
    readArrayNum(&obj1, 1, &yMin);
    readArrayNum(&obj1, 2, &xMax);
    readArrayNum(&obj1, 3, &yMax);
    if (ok) {
      if (xMin > xMax) {
        t = xMin; xMin = xMax; xMax = t;
      }
      if (yMin > yMax) {
        t = yMin; yMin = yMax; yMax = t;
      }
    } else {
      //~ this should return an error
      xMin = yMin = 0;
      xMax = yMax = 1;
    }
  } else {
    //~ this should return an error
    xMin = yMin = 0;
    xMax = yMax = 1;
  }
  obj1.free();

  //check for hidden annot
  hidden = false;
  Object obj3;
  if (dict->lookup("F", &obj3)->isInt()) {
    int flags = obj3.getInt();
    if (flags & 0x2) 
      hidden = true;
  }
  obj3.free();

  // check if field apperances need to be regenerated
  regen = gFalse;
  if (acroForm) {
    acroForm->lookup("NeedAppearances", &obj1);
    if (obj1.isBool() && obj1.getBool()) {
      regen = gTrue;
    }
    obj1.free();
  }
  regen = gTrue;

  /*TODO: appearance generation isn't complete :
   non-comprehensive list of missings things by Leonard Rosenthol :
   * Doesn't support walking up the parents of the field for /DA information
   * Doesn't support field attributes
         border style (width, color, etc.)
         background color
         flags (password, comb, etc.)
         etc.
   * Only works for text fields, basic support for combos
  */

  //try to find the corresponding FormWidget
  if (hasRef && catalog->getForm()) {
    widget = catalog->getForm()->findWidgetByRef(ref); 
  }

  isMultiline = isListbox = false;
  if (widget) { 
    if (widget->getType() == formText) {
      isTextField = true;
      isMultiline = static_cast<FormWidgetText*>(widget)->isMultiline();
    } else if (widget->getType() == formChoice) {
      isTextField = true;
      isListbox = static_cast<FormWidgetChoice*>(widget)->isListBox();
    }
  }

  if (regen && isTextField) {
    generateAppearance(acroForm, dict);
  } else {
    if (dict->lookup("AP", &apObj)->isDict()) {
      if (dict->lookup("AS", &asObj)->isName()) {
	if (apObj.dictLookup("N", &obj1)->isDict()) {
	  if (obj1.dictLookupNF(asObj.getName(), &obj2)->isRef()) {
	    obj2.copy(&appearance);
	    ok = gTrue;
	  } else {
	    obj2.free();
	    if (obj1.dictLookupNF("Off", &obj2)->isRef()) {
	      obj2.copy(&appearance);
	      ok = gTrue;
	    }
	  }
	  obj2.free();
	}
	obj1.free();
      } else {
	if (apObj.dictLookupNF("N", &obj1)->isRef()) {
	  obj1.copy(&appearance);
	  ok = gTrue;
	}
	obj1.free();
      }
      asObj.free();
    }
    apObj.free();
  }
}

void Annot::readArrayNum(Object *pdfArray, int key, double *value) {
  Object valueObject;

  pdfArray->arrayGet(key, &valueObject);
  if (valueObject.isNum()) {
    *value = valueObject.getNum();
  } else {
    *value = 0;
    ok = gFalse;
  }
  valueObject.free();
}

Annot::~Annot() {
  appearance.free();
  if (appearBuf) {
    delete appearBuf;
  }
}

void Annot::writeTextString (GooString* vStr, CharCodeToUnicode* ccToUnicode, GooString* appearBuf, GfxFont* font)
{
  int i0,i1; //i0 = line begin index, i1 = line end index
  int charSize;
  CharCode c;
  char buf[256];
  double currentLineWidth= 0.0; //width filled by displayed chars (used for multilines)

  if (vStr->hasUnicodeMarker()) {
    //skip unicode marker and go one char forward because we read character by pairs
    i0 = 3;
    charSize = 2;
  } else {
    i0 = 0;
    charSize = 1;
  }
  while (i0 < vStr->getLength()) {
    //find next end of line
    for (i1 = i0; i1 < vStr->getLength(); i1++) {
      if (vStr->getChar(i1) == '\n' || vStr->getChar(i1) == '\r')
        break;
    }
    appearBuf->append('(');

    for(; i0 < i1; i0 += charSize) {
      c = vStr->getChar(i0);
      if (ccToUnicode && vStr->hasUnicodeMarker()) {
        char ctmp[2];
        ctmp[0] = vStr->getChar(i0-1);
        ctmp[1] = vStr->getChar(i0);
        ccToUnicode->mapToCharCode((Unicode*)ctmp, &c, 2);
        appearBuf->append(c);
      } else {
        c &= 0xff;
        if (c == '(' || c == ')' || c == '\\') {
          appearBuf->append('\\');
          appearBuf->append(c);
        } else if (c < 0x20 || c >= 0x80) {
          sprintf(buf, "\\%03o", c);
          appearBuf->append(buf);
        } else {
          appearBuf->append(c);
        }
      }
      if (font) {
        if (font->isCIDFont()) {
          currentLineWidth += fontSize*static_cast<GfxCIDFont*>(font)->getWidth((char*)&c, 1);
        } else { //Gfx8Bit
          currentLineWidth += fontSize*static_cast<Gfx8BitFont*>(font)->getWidth((char)c);
        }

        if (isMultiline && (currentLineWidth >= xMax - xMin)) {
          currentLineWidth = 0;
          break;
        }
      } else {
      
      }
    }
    appearBuf->append(") Tj\n");
    appearBuf->append("T*\n");
    i0 = i0 + charSize;
  }
}

void Annot::generateAppearance(Dict *acroForm, Dict *dict) {
  MemStream *appearStream;
  Object daObj, vObj, drObj, appearDict, obj1, obj2, resObj;
  GooString *daStr, *daStr1, *vStr, *s;
  GooString *fontName=NULL;
  char buf[256];
  int c;
  int i0, i1;
  GfxFont *font = NULL;


  //~ DA can be inherited
  if (dict->lookup("DA", &daObj)->isString()) {
    daStr = daObj.getString();

    // look for a font size
    //~ may want to parse the DS entry in place of this (if it exists)
    daStr1 = NULL;
    fontSize = 10;
    for (i1 = daStr->getLength() - 2; i1 >= 0; --i1) {
      if (daStr->getChar(i1) == 'T' && daStr->getChar(i1+1) == 'f') {
	for (--i1; i1 >= 0 && Lexer::isSpace(daStr->getChar(i1)); --i1) ;
	for (i0 = i1; i0 >= 0 && !Lexer::isSpace(daStr->getChar(i0)); --i0) ;
	if (i0 >= 0) {
	  ++i0;
	  ++i1;
	  s = new GooString(daStr, i0, i1 - i0);
	  fontSize = atof(s->getCString());
	  delete s;

	  // autosize the font
	  if (fontSize == 0) {
	    fontSize = 0.67 * (yMax - yMin);
	    daStr1 = new GooString(daStr, 0, i0);
	    sprintf(buf, "%.2f", fontSize);
	    daStr1->append(buf);
	    daStr1->append(daStr->getCString() + i1,
			   daStr->getLength() - i1);
	  }
	}

        //find the font name
        i1=i0;
        for (--i1; i1 >= 0 && Lexer::isSpace(daStr->getChar(i1)); --i1) ;
        for (i0 = i1; i0 >= 0 && !Lexer::isSpace(daStr->getChar(i0)); --i0) ;
        if (i0<0) i0=0;
        if (i0 != i1) {
          ++i0;
          ++i1;
          fontName = new GooString(daStr, i0, i1 - i0);
	}

        break;
      }
    }

    //init appearance dictionnary
    appearDict.initDict(xref);

    // find the resource dictionary
    dict->lookup("DR", &drObj);
    if (!drObj.isDict()) {
      dict->lookup("Parent", &obj1);
      while (obj1.isDict()) {
	drObj.free();
	obj1.dictLookup("DR", &drObj);
	if (drObj.isDict()) {
	  break;
	}
	obj1.dictLookup("Parent", &obj2);
	obj1.free();
	obj1 = obj2;
      }
      obj1.free();
      if (!drObj.isDict()) {
	if (acroForm) {
	  drObj.free();
	  acroForm->lookup("DR", &drObj);
	}
      }
    }
    CharCodeToUnicode* ccToUnicode = NULL;
    if (drObj.isDict()) {
      //Lookup for the font name in the font entry of the ressource dictionnary
      //also lookup in BaseFont/Name of each DR entry
      //without that second lookup, forms-scribus.pdf doesn't find font 
      if (fontName && drObj.dictLookup("Font", &obj1)->isDict()) {
        Object obj3, obj4;
        //--
        bool found = false;
        if (obj1.dictLookup(*fontName, &obj3)->isDict() && obj3.dictLookup("Type", &obj4)->isName("Font")) {
          found = true;
        }
        if (!found) { //font not found in DR, try to lookup in each entry BaseFont/Name
          error(-1, "Can't find font name in DR, trying to lookup in BaseFont/Name");
          for(int i=0; i<obj1.dictGetLength(); i++) {
            if (!found && obj1.dictGetVal(i, &obj3)->isDict()) {
              if (obj3.dictLookup("Type", &obj4)->isName("Font")) {
                obj4.free();
                if (obj3.dictLookup("Name", &obj4)->isName()) {
                  if (fontName->cmp(obj4.getName()) == 0) {
                    found = true;
                    break;
                  }
                  obj4.free();
                }
                if (obj3.dictLookup("BaseFont", &obj4)->isName()) {
                  if (fontName->cmp(obj4.getName()) == 0) {
                    found = true;
                    break;
                  }
                  obj4.free();
                }
              }
              obj3.free();
            }
          }
        }
        if (found) {
          obj4.free();
          obj1.dictLookupNF(*fontName, &obj4);
          Ref r;
          if (obj4.isRef()) r = obj4.getRef();
          else r.gen = r.num = 0;
          font = GfxFont::makeFont(xref, "temp", r, obj3.getDict());
          ccToUnicode = font->getToUnicode();
        }
        obj3.free();
      }
      obj1.free();
      appearDict.dictAdd("Resources", drObj.copy(&resObj));
    }
    drObj.free();

    // build the appearance stream contents
    appearBuf = new GooString();
   
    //TODO: support for dash and style options from BS dict
    bool drawBorder = false;
    if (dict->lookup("BS", &obj1)->isDict()) {
      Object obj3,obj4;
      Dict *bsDict = obj1.getDict();
      if (bsDict->lookup("Type", &obj3)->isName("Border")) {
        //width
        if (bsDict->lookup("W", &obj4)->isInt()) {
          sprintf(buf, "%i w\n", obj4.getInt());
          appearBuf->append(buf);
          drawBorder = true;
        }
        obj4.free();
      }
      obj3.free();
    }
    obj1.free();
    //border rendering
    if (drawBorder && !hidden) {
      sprintf(buf, "0 0 %f %f re\n", xMax-xMin, yMax-yMin);
      appearBuf->append(buf);
      appearBuf->append("S\n");
    }

    appearBuf->append("/Tx BMC\n");
    appearBuf->append("q BT\n");
    appearBuf->append(daStr1 ? daStr1 : daStr)->append("\n");

    sprintf(buf, "1 0 0 1 %.2f %.2f Tm\n", 2.0, yMax - yMin - fontSize);
    appearBuf->append(buf);
    sprintf(buf, "%g TL\n", fontSize);
    appearBuf->append(buf);
    
    if (hidden) {
      //don't display hidden annot
    } else if (isListbox) {
      FormWidgetChoice* choice = static_cast<FormWidgetChoice*>(widget);
      for(int i=0; i<choice->getNumChoices(); i++) {
        if (choice->isSelected(i)) { //selected choice
          //TODO: The color of the highlighting rect should depend on the color of the font. 
          //highlight with a black background rect
          appearBuf->append("q\n");
          sprintf(buf, "0 %f %f %f re\n", yMax-yMin-(i+1)*fontSize, xMax-xMin, fontSize);
          appearBuf->append(buf);
          appearBuf->append("f\n");
          appearBuf->append("Q\n");
          //draw the text in white
          appearBuf->append("1.0 1.0 1.0 rg\n");
          writeTextString(choice->getChoice(i), ccToUnicode, appearBuf, font);
          appearBuf->append("0.0 0.0 0.0 rg\n");
        } else
          writeTextString(choice->getChoice(i), ccToUnicode, appearBuf, font);
      }
      vObj.free();
    } else if (dict->lookup("V", &vObj)->isString()) {
      //~ handle quadding -- this requires finding the font and using
      //~   the encoding and char widths
      vStr = vObj.getString();
      writeTextString(vStr, ccToUnicode, appearBuf, font);
      vObj.free();
    }

    appearBuf->append("ET Q\n");
    appearBuf->append("EMC\n");

    // build the appearance stream dictionary
    appearDict.dictAdd("Length", obj1.initInt(appearBuf->getLength()));
    appearDict.dictAdd("Subtype", obj1.initName("Form"));
    obj1.initArray(xref);
    obj1.arrayAdd(obj2.initReal(0));
    obj1.arrayAdd(obj2.initReal(0));
    obj1.arrayAdd(obj2.initReal(xMax - xMin));
    obj1.arrayAdd(obj2.initReal(yMax - yMin));
    appearDict.dictAdd("BBox", &obj1);

    // build the appearance stream
    appearStream = new MemStream(appearBuf->getCString(), 0,
				 appearBuf->getLength(), &appearDict);
    appearance.free();
    appearance.initStream(appearStream);
    ok = gTrue;

    if (daStr1) {
      delete daStr1;
    }
  }
  daObj.free();

  if (fontName) delete fontName;
  if (font) delete font;
}

void Annot::draw(Gfx *gfx) {
  Object obj;
  if (appearance.fetch(xref, &obj)->isStream()) {
    gfx->doAnnot(&obj, xMin, yMin, xMax, yMax);
  }
  obj.free();
}

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

Annots::Annots(XRef *xref, Catalog *catalog, Object *annotsObj) {
  Dict *acroForm;
  Annot *annot;
  Object obj1;
  int size;
  int i;

  annots = NULL;
  size = 0;
  nAnnots = 0;

  acroForm = catalog->getAcroForm()->isDict() ?
               catalog->getAcroForm()->getDict() : NULL;
  if (annotsObj->isArray()) {
    for (i = 0; i < annotsObj->arrayGetLength(); ++i) {
      //get the Ref to this annot and pass it to Annot constructor 
      //this way, it'll be possible for the annot to retrieve the corresponding
      //form widget
      Object obj2;
      Ref* pref;
      if (annotsObj->arrayGet(i, &obj1)->isDict()) {
        if (annotsObj->arrayGetNF(i, &obj2)->isRef())
          annot = new Annot(xref, acroForm, obj1.getDict(), obj2.getRef(), catalog);
        else 
          annot = new Annot(xref, acroForm, obj1.getDict(), catalog);
	if (annot->isOk()) {
	  if (nAnnots >= size) {
	    size += 16;
	    annots = (Annot **)greallocn(annots, size, sizeof(Annot *));
	  }
	  annots[nAnnots++] = annot;
	} else {
	  delete annot;
	}
      }
      obj2.free();
      obj1.free();
    }
  }
}

Annots::~Annots() {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    delete annots[i];
  }
  gfree(annots);
}
