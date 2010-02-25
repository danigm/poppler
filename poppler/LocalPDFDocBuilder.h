//========================================================================
//
// LocalPDFDocBuilder.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
//
//========================================================================

#ifndef LOCALPDFDOCBUILDER_H
#define LOCALPDFDOCBUILDER_H

#include "PDFDocBuilder.h"

//------------------------------------------------------------------------
// LocalPDFDocBuilder
//------------------------------------------------------------------------

class LocalPDFDocBuilder : public PDFDocBuilder {

public:

  PDFDoc *buildPDFDoc(GooString* uri, GooString *ownerPassword = NULL,
    GooString *userPassword = NULL, void *guiDataA = NULL);
  GBool supports(GooString* uri);

};

#endif /* LOCALPDFDOCBUILDER_H */
