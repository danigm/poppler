//========================================================================
//
// PDFDocEncoding.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef PDFDOCENCODING_H
#define PDFDOCENCODING_H

#include "CharTypes.h"

class GooString;

extern Unicode pdfDocEncoding[256];

char* pdfDocEncodingToUTF16 (GooString* orig, int* length);

#endif
