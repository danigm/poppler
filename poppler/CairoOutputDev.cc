//========================================================================
//
// CairoOutputDev.cc
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
#include "GfxState.h"
#include "GfxFont.h"
#include "Link.h"
#include "CharCodeToUnicode.h"
#include "FontEncodingTables.h"
#include <fofi/FoFiTrueType.h>
#include <splash/SplashBitmap.h>
#include "CairoOutputDev.h"
#include "CairoFontEngine.h"

//------------------------------------------------------------------------

#define soutRound(x) ((int)(x + 0.5))

//#define LOG_CAIRO

#ifdef LOG_CAIRO
#define LOG(x) (x)
#else
#define LOG(x)
#endif


//------------------------------------------------------------------------
// CairoOutputDev
//------------------------------------------------------------------------

CairoOutputDev::CairoOutputDev(void) {
  xref = NULL;

  FT_Init_FreeType(&ft_lib);
  fontEngine = NULL;
}

CairoOutputDev::~CairoOutputDev() {
  if (fontEngine) {
    delete fontEngine;
  }
  cairo_destroy (cairo);
  FT_Done_FreeType(ft_lib);

}

void CairoOutputDev::startDoc(XRef *xrefA) {
  xref = xrefA;
  if (fontEngine) {
    delete fontEngine;
  }
  fontEngine = new CairoFontEngine(ft_lib);
}

void CairoOutputDev::startPage(int pageNum, GfxState *state) {
  cairo_destroy (cairo);
  createCairo (state);
  
  cairo_init_clip (cairo);
  cairo_set_rgb_color (cairo, 0, 0, 0);
  cairo_set_operator (cairo, CAIRO_OPERATOR_OVER);
  cairo_set_line_cap (cairo, CAIRO_LINE_CAP_BUTT);
  cairo_set_line_join (cairo, CAIRO_LINE_JOIN_MITER);
  cairo_set_dash (cairo, NULL, 0, 0.0);
  cairo_set_miter_limit (cairo, 10);
  cairo_set_tolerance (cairo, 1);
}

void CairoOutputDev::endPage() {
}

void CairoOutputDev::drawLink(Link *link, Catalog *catalog) {
}

void CairoOutputDev::saveState(GfxState *state) {
  LOG(printf ("save\n"));
  cairo_save (cairo);
}

void CairoOutputDev::restoreState(GfxState *state) {
  LOG(printf ("restore\n"));
  cairo_restore (cairo);
  /* TODO: Is this really needed for cairo? Maybe not */
  needFontUpdate = gTrue;
}

void CairoOutputDev::updateAll(GfxState *state) {
  updateLineDash(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateLineWidth(state);
  updateFlatness(state);
  updateMiterLimit(state);
  updateFillColor(state);
  updateStrokeColor(state);
  needFontUpdate = gTrue;
}

void CairoOutputDev::updateCTM(GfxState *state, double m11, double m12,
				double m21, double m22,
				double m31, double m32) {
  updateLineDash(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateLineWidth(state);
}

void CairoOutputDev::updateLineDash(GfxState *state) {
  double *dashPattern;
  int dashLength;
  double dashStart;
  double *transformedDash;
  double transformedStart;
  int i;

  state->getLineDash(&dashPattern, &dashLength, &dashStart);

  transformedDash = new double[dashLength];
  
  for (i = 0; i < dashLength; ++i) {
    transformedDash[i] =  state->transformWidth(dashPattern[i]);
  }
  transformedStart = state->transformWidth(dashStart);
  cairo_set_dash (cairo, transformedDash, dashLength, transformedStart);
  delete [] transformedDash;
}

void CairoOutputDev::updateFlatness(GfxState *state) {
  cairo_set_tolerance (cairo, state->getFlatness());
}

void CairoOutputDev::updateLineJoin(GfxState *state) {
  switch (state->getLineJoin()) {
  case 0:
    cairo_set_line_join (cairo, CAIRO_LINE_JOIN_MITER);
    break;
  case 1:
    cairo_set_line_join (cairo, CAIRO_LINE_JOIN_ROUND);
    break;
  case 2:
    cairo_set_line_join (cairo, CAIRO_LINE_JOIN_BEVEL);
    break;
  }
}

void CairoOutputDev::updateLineCap(GfxState *state) {
  switch (state->getLineCap()) {
  case 0:
    cairo_set_line_cap (cairo, CAIRO_LINE_CAP_BUTT);
    break;
  case 1:
    cairo_set_line_cap (cairo, CAIRO_LINE_CAP_ROUND);
    break;
  case 2:
    cairo_set_line_cap (cairo, CAIRO_LINE_CAP_SQUARE);
    break;
  }
}

void CairoOutputDev::updateMiterLimit(GfxState *state) {
  cairo_set_miter_limit (cairo, state->getMiterLimit());
}

void CairoOutputDev::updateLineWidth(GfxState *state) {
  LOG(printf ("line width: %f\n", state->getTransformedLineWidth()));
  cairo_set_line_width (cairo, state->getTransformedLineWidth());
}

void CairoOutputDev::updateFillColor(GfxState *state) {
  state->getFillRGB(&fill_color);
  LOG(printf ("fill color: %f %f %f\n", fill_color.r, fill_color.g, fill_color.b));
}

void CairoOutputDev::updateStrokeColor(GfxState *state) {
  state->getStrokeRGB(&stroke_color);
  LOG(printf ("stroke color: %f %f %f\n", stroke_color.r, stroke_color.g, stroke_color.b));
}

void CairoOutputDev::updateFont(GfxState *state) {
  cairo_font_t *font;
  double w;

  LOG(printf ("updateFont() font=%s\n", state->getFont()->getName()->getCString()));
  
  /* Needs to be rethough, since fonts are now handled by cairo */
  needFontUpdate = gFalse;

  currentFont = fontEngine->getFont (state->getFont(), xref);

  double m11, m12, m21, m22;
  
  state->getFontTransMat(&m11, &m12, &m21, &m22);
  m11 *= state->getHorizScaling();
  m12 *= state->getHorizScaling();

  LOG(printf ("font matrix: %f %f %f %f\n", m11, m12, m21, m22));

  w = currentFont->getSubstitutionCorrection(state->getFont());
  
  cairo_matrix_t *matrix = cairo_matrix_create ();
  cairo_matrix_set_affine (matrix,
			   m11, m21,
			   -m12*w, -m22*w,
			   0, 0);
  
  font = currentFont->getFont(matrix);
  if (font)
    cairo_set_font (cairo, font);
  cairo_matrix_destroy (matrix);
}

void CairoOutputDev::doPath(GfxState *state, GfxPath *path,
			    GBool snapToGrid) {
  GfxSubpath *subpath;
  double x1, y1, x2, y2, x3, y3;
  int i, j;

  for (i = 0; i < path->getNumSubpaths(); ++i) {
    subpath = path->getSubpath(i);
    if (subpath->getNumPoints() > 0) {
      state->transform(subpath->getX(0), subpath->getY(0), &x1, &y1);
      if (snapToGrid) {
	x1 = round (x1); y1 = round (y1);
      }
      cairo_move_to (cairo, x1, y1);
      LOG (printf ("move_to %f, %f\n", x1, y1));
      j = 1;
      while (j < subpath->getNumPoints()) {
	if (subpath->getCurve(j)) {
	  if (snapToGrid) {
	    x1 = round (x1); y1 = round (y1);
	    x2 = round (x2); y2 = round (y2);
	    x3 = round (x3); y3 = round (y3);
	  }
	  state->transform(subpath->getX(j), subpath->getY(j), &x1, &y1);
	  state->transform(subpath->getX(j+1), subpath->getY(j+1), &x2, &y2);
	  state->transform(subpath->getX(j+2), subpath->getY(j+2), &x3, &y3);
	  cairo_curve_to (cairo, 
			  x1, y1,
			  x2, y2,
			  x3, y3);
	  LOG (printf ("curve_to %f, %f  %f, %f  %f, %f\n", x1, y1, x2, y2, x3, y3));
	  j += 3;
	} else {
	  state->transform(subpath->getX(j), subpath->getY(j), &x1, &y1);
	  if (snapToGrid) {
	    x1 = round (x1); y1 = round (y1);
	  }
	  cairo_line_to (cairo, x1, y1);
	  LOG(printf ("line_to %f, %f\n", x1, y1));
	  ++j;
	}
      }
      if (subpath->isClosed()) {
	LOG (printf ("close\n"));
	cairo_close_path (cairo);
      }
    }
  }
}

void CairoOutputDev::stroke(GfxState *state) {
  doPath (state, state->getPath(), gFalse);
  cairo_set_rgb_color (cairo,
		       stroke_color.r, stroke_color.g, stroke_color.b);
  LOG(printf ("stroke\n"));
  cairo_stroke (cairo);
}

void CairoOutputDev::fill(GfxState *state) {
  doPath (state, state->getPath(), gFalse);
  cairo_set_fill_rule (cairo, CAIRO_FILL_RULE_WINDING);
  cairo_set_rgb_color (cairo,
		       fill_color.r, fill_color.g, fill_color.b);
  LOG(printf ("fill\n"));
  cairo_fill (cairo);
}

void CairoOutputDev::eoFill(GfxState *state) {
  doPath (state, state->getPath(), gFalse);
  cairo_set_fill_rule (cairo, CAIRO_FILL_RULE_EVEN_ODD);
  cairo_set_rgb_color (cairo,
		       fill_color.r, fill_color.g, fill_color.b);
  LOG(printf ("fill-eo\n"));
  cairo_fill (cairo);
}

void CairoOutputDev::clip(GfxState *state, GBool snapToGrid) {
  doPath (state, state->getPath(), snapToGrid);
  cairo_set_fill_rule (cairo, CAIRO_FILL_RULE_WINDING);
  cairo_clip (cairo);
  cairo_new_path (cairo); /* Consume path */
  LOG (printf ("clip\n"));
}

void CairoOutputDev::eoClip(GfxState *state) {
  doPath (state, state->getPath(), gFalse);
  cairo_set_fill_rule (cairo, CAIRO_FILL_RULE_EVEN_ODD);
  cairo_clip (cairo);
  cairo_new_path (cairo); /* Consume path */
  LOG (printf ("clip-eo\n"));
}

void CairoOutputDev::drawChar(GfxState *state, double x, double y,
			       double dx, double dy,
			       double originX, double originY,
			       CharCode code, Unicode *u, int uLen) {
  cairo_glyph_t glyph;
  double x1, y1;
  int render;

  LOG (printf ("drawChar %d '%c'\n", code, code));
  
  if (needFontUpdate) {
    updateFont(state);
  }
  if (!currentFont) {
    return;
  }
  
  // check for invisible text -- this is used by Acrobat Capture
  render = state->getRender();
  if (render == 3) {
    return;
  }

  x -= originX;
  y -= originY;
  state->transform(x, y, &x1, &y1);

  glyph.index = currentFont->getGlyph (code, u, uLen);
  glyph.x = x1;
  glyph.y = y1;

  // fill
  if (!(render & 1)) {
    LOG (printf ("fill glyph\n"));
    cairo_set_rgb_color (cairo,
			 fill_color.r, fill_color.g, fill_color.b);
    cairo_show_glyphs (cairo, &glyph, 1);
  }

  // stroke
  if ((render & 3) == 1 || (render & 3) == 2) {
    LOG (printf ("stroke glyph\n"));
    cairo_set_rgb_color (cairo,
			 stroke_color.r, stroke_color.g, stroke_color.b);
    cairo_glyph_path (cairo, &glyph, 1);
    cairo_stroke (cairo);
  }
  

  // clip
  if (render & 4) {
    printf ("TODO: clip to glyph\n");
    /* TODO: This reqires us to concatenat all clip paths
       until endTextObject */
  }

}

GBool CairoOutputDev::beginType3Char(GfxState *state, double x, double y,
				      double dx, double dy,
				      CharCode code, Unicode *u, int uLen) {
  return gFalse;
}

void CairoOutputDev::endType3Char(GfxState *state) {
}

void CairoOutputDev::type3D0(GfxState *state, double wx, double wy) {
}

void CairoOutputDev::type3D1(GfxState *state, double wx, double wy,
			      double llx, double lly, double urx, double ury) {
}

void CairoOutputDev::endTextObject(GfxState *state) {
}


void CairoOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				    int width, int height, GBool invert,
				    GBool inlineImg) {
  char *buffer;
  char *dest;
  cairo_surface_t *image;
  int x, y;
  ImageStream *imgStr;
  Guchar pix;
  double *ctm;
  cairo_matrix_t *mat;

  /* TODO: I'm not sure how to implement this, and its currently crashing.
     Disabling for now */

  return;
  
  buffer = (char *)malloc (width * height);

  if (buffer == NULL) {
    error(-1, "Unable to allocate memory for image.");
    return;
  }

  /* TODO: Do we want to cache these? */
  imgStr = new ImageStream(str, width,
			   1, 1);
  imgStr->reset();

  for (y = 0; y < height; y++) {
    dest = buffer + y * width;
    for (x = 0; x < width; x++) {
      imgStr->getPixel(&pix);
      if (invert)
	pix ^= 1;
      
      if (pix)
	*dest++ = 255;
      else
	*dest++ = 0;
    }
  }

  cairo_save (cairo);

  ctm = state->getCTM();
  mat = cairo_matrix_create ();
  LOG (printf ("drawImageMask %dx%d, matrix: %f, %f, %f, %f, %f, %f\n",
	       width, height,
	       ctm[0], ctm[1],
	       ctm[2], ctm[3],
	       ctm[4], ctm[5]));
  cairo_matrix_set_affine (mat,
			   ctm[0]/width, ctm[1]/width,
			   -ctm[2]/height, -ctm[3]/height,
			   ctm[2] + ctm[4], ctm[3] + ctm[5]);
  cairo_concat_matrix (cairo, mat);
  cairo_matrix_destroy (mat);

  /* TODO: Should we use A1 here? I assume that is bit-packed */
  image = cairo_surface_create_for_image (
              buffer, CAIRO_FORMAT_A8, width, height, width);
  cairo_surface_set_filter (image, CAIRO_FILTER_BEST);

  /* TODO: Is this the right way to do image masks? */
  cairo_set_rgb_color (cairo,
		       fill_color.r, fill_color.g, fill_color.b);
  cairo_show_surface (cairo, image, width, height);

  cairo_restore (cairo);

  cairo_surface_destroy (image);
  free (buffer);
  delete imgStr;

  
}

void CairoOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
				int width, int height,
				GfxImageColorMap *colorMap,
				int *maskColors, GBool inlineImg) {
  char *buffer;
  char *dest;
  cairo_surface_t *image;
  int x, y;
  ImageStream *imgStr;
  Guchar *pix;
  GfxRGB rgb;
  int alpha, i;
  double *ctm;
  cairo_matrix_t *mat;
  int is_identity_transform;
  
  buffer = (char *)malloc (width * height * 4);

  if (buffer == NULL) {
    error(-1, "Unable to allocate memory for image.");
    return;
  }

  /* TODO: Do we want to cache these? */
  imgStr = new ImageStream(str, width,
			   colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();
  
  /* ICCBased color space doesn't do any color correction
   * so check its underlying color space as well */
  is_identity_transform = colorMap->getColorSpace()->getMode() == csDeviceRGB ||
		  colorMap->getColorSpace()->getMode() == csICCBased && 
		  ((GfxICCBasedColorSpace*)colorMap->getColorSpace())->getAlt()->getMode() == csDeviceRGB;
  
  for (y = 0; y < height; y++) {
    dest = buffer + y * 4 * width;
    pix = imgStr->getLine();
    for (x = 0; x < width; x++, pix += colorMap->getNumPixelComps()) {
      if (maskColors) {
	alpha = 0;
	for (i = 0; i < colorMap->getNumPixelComps(); ++i) {
	  if (pix[i] < maskColors[2*i] ||
	      pix[i] > maskColors[2*i+1]) {
	    alpha = 255;
	    break;
	  }
	}
      } else {
	alpha = 255;
      }
      if (is_identity_transform) {
	*dest++ = pix[2];
	*dest++ = pix[1];
	*dest++ = pix[0];
      } else {      
	colorMap->getRGB(pix, &rgb);
	*dest++ = soutRound(255 * rgb.b);
	*dest++ = soutRound(255 * rgb.g);
	*dest++ = soutRound(255 * rgb.r);
      }
      *dest++ = alpha;
    }
  }

  cairo_save (cairo);

  ctm = state->getCTM();
  mat = cairo_matrix_create ();
  LOG (printf ("draw image %dx%d, matrix: %f, %f, %f, %f, %f, %f\n",
	       width, height,
	       ctm[0], ctm[1],
	       ctm[2], ctm[3],
	       ctm[4], ctm[5]));
  cairo_matrix_set_affine (mat,
			   ctm[0]/width, ctm[1]/width,
			   -ctm[2]/height, -ctm[3]/height,
			   ctm[2] + ctm[4], ctm[3] + ctm[5]);
  cairo_concat_matrix (cairo, mat);
  cairo_matrix_destroy (mat);
  
  image = cairo_surface_create_for_image (
              buffer, CAIRO_FORMAT_ARGB32, width, height, width * 4);
  cairo_surface_set_filter (image, CAIRO_FILTER_BEST);
  cairo_show_surface (cairo, image, width, height);

  cairo_restore (cairo);
  
  cairo_surface_destroy (image);
  free (buffer);
  delete imgStr;
}
