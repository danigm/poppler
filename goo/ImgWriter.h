//========================================================================
//
// ImgWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
//
//========================================================================

#ifndef IMGWRITER_H
#define IMGWRITER_H

#include <config.h>
#include <cstdio>
	
class ImgWriter
{
	public:
		virtual bool init(FILE *f, int width, int height) = 0;
		
		virtual bool writePointers(unsigned char **rowPointers, int rowCount) = 0;
		virtual bool writeRow(unsigned char **row) = 0;
		
		virtual bool close() = 0;
};

#endif
