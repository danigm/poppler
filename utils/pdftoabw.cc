//========================================================================
//
// pdftohtml.cc
//
//
// Copyright 1999-2000 G. Ovtcharov
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include "parseargs.h"
#include "goo/GooString.h"
#include "goo/gmem.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "ABWOutputDev.h"
#include "PSOutputDev.h"
#include "GlobalParams.h"
#include "Error.h"
#include "UGooString.h"
#include "goo/gfile.h"
#include <libxml/parser.h>
#include <libxml/tree.h>


static int firstPage = 1;
static int lastPage = 0;
GBool printCommands = gTrue;
GBool prettyPrint = gFalse;
static GBool printHelp = gFalse;
GBool stout=gFalse;

static char ownerPassword[33] = "";
static char userPassword[33] = "";

static GooString* getInfoString(Dict *infoDict, char *key);
static GooString* getInfoDate(Dict *infoDict, char *key);

xmlDocPtr XMLdoc;

static char textEncName[128] = "";

static ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to convert"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"--format",   argFlag,     &prettyPrint,     0,
   "print usage information"},
  {"--stdout"  ,argFlag,    &stout,         0,
   "use standard output"},
  {"--opw",    argString,   ownerPassword,  sizeof(ownerPassword),
   "owner password (for encrypted files)"},
  {"--upw",    argString,   userPassword,   sizeof(userPassword),
   "user password (for encrypted files)"},
  {NULL}
};

int main(int argc, char *argv[]) {
  PDFDoc *doc = NULL;
  GooString *fileName = NULL;
  GooString *docTitle = NULL;
  GooString *author = NULL, *keywords = NULL, *subject = NULL, *date = NULL;
  GooString *htmlFileName = NULL;
  GooString *psFileName = NULL;
  ABWOutputDev *htmlOut = NULL;
  PSOutputDev *psOut = NULL;
  GBool ok;
  char *p;
  char extension[16] = "png";
  GooString *ownerPW, *userPW;
  Object info;
  
  char * outpName;

  // parse args
  parseArgs(argDesc, &argc, argv);
  globalParams = new GlobalParams();

  fileName = new GooString(argv[1]);
  /*
  if (stout){*/
    outpName = "-";
/*  }
  else {
    //FIXME: add outputfilename stuff
  }
  */
  doc = new PDFDoc(fileName);
  XMLdoc = xmlNewDoc(BAD_CAST "1.0");
  htmlOut = new ABWOutputDev(XMLdoc);
  htmlOut->setPDFDoc(doc);
  /* check for copy permission
  if (!doc->okToCopy()) {
    error(-1, "Copying of text from this document is not allowed.");
    goto error;
  }*/

  // write text file

  if (lastPage == 0) lastPage = doc->getNumPages();

  if (htmlOut->isOk())
  {
    doc->displayPages(htmlOut, 1, lastPage, 72, 72, 0, gTrue, gFalse, gFalse);
		htmlOut->createABW();
  }
  xmlSaveFormatFileEnc(outpName, XMLdoc, "UTF-8", 1);
  // clean up
 error:
  if(globalParams) delete globalParams;
  //if(fileName) delete fileName;
  if(doc) delete doc;
  if(XMLdoc) xmlFreeDoc(XMLdoc);
  if(htmlOut) delete htmlOut;
  
  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return 0;
}
