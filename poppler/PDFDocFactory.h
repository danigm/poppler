//========================================================================
//
// PDFDocFactory.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
//
//========================================================================

#ifndef PDFDOCFACTORY_H
#define PDFDOCFACTORY_H

#include "PDFDoc.h"

class GooList;
class GooString;
class PDFDocBuilder;

//------------------------------------------------------------------------
// PDFDocFactory
//------------------------------------------------------------------------

class PDFDocFactory {

public:

  PDFDocFactory(GooList *pdfDocBuilders = NULL);
  ~PDFDocFactory();

  PDFDoc *createPDFDoc(GooString* uri, GooString *ownerPassword = NULL,
      GooString *userPassword = NULL, void *guiDataA = NULL);

  void registerPDFDocBuilder(PDFDocBuilder *pdfDocBuilder);

private:

  GooList *builders;

};

#endif /* PDFDOCFACTORY_H */
