//========================================================================
//
// JpegWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2010 Adrian Johnson <ajohnson@redneon.com>
//
//========================================================================

#ifndef JPEGWRITER_H
#define JPEGWRITER_H

#include <config.h>

#ifdef ENABLE_LIBJPEG

#include <cstdio>
#include <jpeglib.h>
#include "ImgWriter.h"

class JpegWriter : public ImgWriter
{
	public:
		JpegWriter();
		~JpegWriter();
		
		bool init(FILE *f, int width, int height, int hDPI, int vDPI);
		
		bool writePointers(unsigned char **rowPointers, int rowCount);
		bool writeRow(unsigned char **row);
		
		bool close();
	
	private:
		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
};

#endif

#endif
