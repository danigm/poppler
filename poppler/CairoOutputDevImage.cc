//========================================================================
//
// CairoOutputDevImage.cc
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, Inc
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <string.h>
#include <math.h>
#include <cairo.h>

#include "goo/gfile.h"
#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include <fofi/FoFiTrueType.h>
#include "CairoOutputDevImage.h"

//------------------------------------------------------------------------
// CairoOutputDevImage
//------------------------------------------------------------------------

CairoOutputDevImage::CairoOutputDevImage(void) {
  pixels = NULL;
  createCairo (NULL);
}

CairoOutputDevImage::~CairoOutputDevImage() {
  gfree (pixels);
}

void
CairoOutputDevImage::createCairo(GfxState *state) {
  int w, h;
  cairo_surface_t* surface;

  w = state ? (int)(state->getPageWidth() + 0.5) : 1;
  h = state ? (int)(state->getPageHeight() + 0.5) : 1;

  if (!pixels || w != pixels_w || h != pixels_h) {
    if (pixels) {
      gfree(pixels);
    }
    pixels_w = w;
    pixels_h = h;
    pixels = (unsigned char *)gmalloc (pixels_w * pixels_h * 4);
  }

  memset (pixels, 0xff, pixels_w * pixels_h * 4);

  surface = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_ARGB32,
	  					pixels_w, pixels_h, 
						pixels_w*4);
  cairo = cairo_create (surface);
  cairo_surface_destroy (surface);
}


void CairoOutputDevImage::getBitmap(unsigned char **data,
				    int *width, int *height,
				    int *rowstride) {
  int w, h;
  unsigned char *src;
  unsigned int *dest;

  *data = pixels;
  *width = pixels_w;
  *height = pixels_h;
  *rowstride = 4 * pixels_w;
}


