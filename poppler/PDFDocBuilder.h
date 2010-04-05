//========================================================================
//
// PDFDocBuilder.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef PDFDOCBUILDER_H
#define PDFDOCBUILDER_H

#include "PDFDoc.h"
class GooString;

//------------------------------------------------------------------------
// PDFDocBuilder
//------------------------------------------------------------------------

class PDFDocBuilder {

public:

  virtual ~PDFDocBuilder() {};
  virtual PDFDoc *buildPDFDoc(const GooString &uri, GooString *ownerPassword = NULL,
      GooString *userPassword = NULL, void *guiDataA = NULL) = 0;
  virtual GBool supports(const GooString &uri) = 0;

};

#endif /* PDFDOCBUILDER_H */
