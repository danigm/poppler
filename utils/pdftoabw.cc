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
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
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
#include "goo/gfile.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

static int firstPage = 1;
static int lastPage = 0;
static GBool printHelp = gFalse;
GBool stout = gFalse;
static char ownerPassword[33] = "";
static char userPassword[33] = "";

// static char textEncName[128] = "";

static ArgDesc argDesc[] = {
  {"-f",      argInt,      &firstPage,     0,
   "first page to convert"},
  {"-l",      argInt,      &lastPage,      0,
   "last page to convert"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"--help",   argFlag,     &printHelp,     0,
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
//  GooString *abwFileName = NULL;
  ABWOutputDev *abwOut = NULL;
//  GBool ok;
  GooString *ownerPW, *userPW;
  Object info;

  int result = 1;
  
  char * outpName;
  xmlDocPtr XMLdoc = NULL;

  // parse args
  parseArgs(argDesc, &argc, argv);
  globalParams = new GlobalParams();

  fileName = new GooString(argv[1]);
  if (stout || (argc < 2)){
    outpName = "-";
  }
  else {
    outpName = argv[2];
  }

  if (ownerPassword[0]) {
    ownerPW = new GooString(ownerPassword);
  } else {
    ownerPW = NULL;
  }
  if (userPassword[0]) {
    userPW = new GooString(userPassword);
  } else {
    userPW = NULL;
  }

  doc = new PDFDoc(fileName, ownerPW, userPW);

  if (userPW) {
    delete userPW;
  }
  if (ownerPW) {
    delete ownerPW;
  }

  if (!doc || !doc->isOk())
    {
      fprintf (stderr, "Error opening PDF %s\n", fileName->getCString());
      goto error;
    }

  // check for copy permission
  if (!doc->okToCopy()) {
    fprintf(stderr, "Copying of text from this document is not allowed.\n");
    goto error;
  }

  XMLdoc = xmlNewDoc(BAD_CAST "1.0");
  abwOut = new ABWOutputDev(XMLdoc);
  abwOut->setPDFDoc(doc);

  if (lastPage == 0 || lastPage > doc->getNumPages ()) lastPage = doc->getNumPages();
  if (firstPage < 1) firstPage = 1;

  if (abwOut->isOk())
  {
    doc->displayPages(abwOut, firstPage, lastPage, 72, 72, 0, gTrue, gFalse, gFalse);
    abwOut->createABW();
  }

  if (xmlSaveFormatFileEnc(outpName, XMLdoc, "UTF-8", 1) == -1)
    {
      fprintf (stderr, "Error saving to %s\n", outpName);
      goto error;
    }

  result = 0;

 error:
  // clean up
  if(globalParams) delete globalParams;
  if(doc) delete doc;
  if(XMLdoc) xmlFreeDoc(XMLdoc);
  if(abwOut) delete abwOut;
  
  // check for memory leaks
  Object::memCheck(stderr);
  gMemReport(stderr);

  return result;
}
