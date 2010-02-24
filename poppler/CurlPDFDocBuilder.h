//========================================================================
//
// CurlPDFDocBuilder.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
//
//========================================================================

#ifndef CURLPDFDOCBUILDER_H
#define CURLPDFDOCBUILDER_H

#include "PDFDocBuilder.h"

//------------------------------------------------------------------------
// CurlPDFDocBuilder
//------------------------------------------------------------------------

class CurlPDFDocBuilder : public PDFDocBuilder {

public:

  PDFDoc *buildPDFDoc(GooString* uri, GooString *ownerPassword = NULL,
    GooString *userPassword = NULL, void *guiDataA = NULL);
  GBool supports(GooString* uri);

};

#endif /* CURLPDFDOCBUILDER_H */
