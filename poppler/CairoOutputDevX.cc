//========================================================================
//
// CairoOutputDevX.cc
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
#include <cairo/cairo-xlib.h>

#include "goo/gfile.h"
#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include <fofi/FoFiTrueType.h>
#include <splash/SplashBitmap.h>
#include "TextOutputDev.h"
#include "CairoOutputDevX.h"

//------------------------------------------------------------------------
// CairoOutputDevX
//------------------------------------------------------------------------

CairoOutputDevX::CairoOutputDevX(Display *displayA, int screenNumA,
				 Visual *visualA, Colormap colormapA,
				 GBool reverseVideoA, SplashColor paperColorA,
				 GBool installCmapA, int rgbCubeSizeA,
				 GBool incrementalUpdateA,
				 void (*redrawCbkA)(void *data),
				 void *redrawCbkDataA) {
  XVisualInfo visualTempl;
  XVisualInfo *visualList;
  int nVisuals;

  pixmap = 0;
  pixmap_w = 1;
  pixmap_h = 1;
  
  incrementalUpdate = incrementalUpdateA;
  redrawCbk = redrawCbkA;
  redrawCbkData = redrawCbkDataA;

  //----- set up the X color stuff

  display = displayA;
  screenNum = screenNumA;
  visual = visualA;

  // check for TrueColor visual
  //~ this should scan the list, not just look at the first one
  visualTempl.visualid = XVisualIDFromVisual(visual);
  visualList = XGetVisualInfo(display, VisualIDMask,
			      &visualTempl, &nVisuals);
  if (nVisuals < 1) {
    // this shouldn't happen
    XFree((XPointer)visualList);
    visualList = XGetVisualInfo(display, VisualNoMask, &visualTempl,
				&nVisuals);
  }
  depth = visualList->depth;
  XFree((XPointer)visualList);

  text = new TextPage(gFalse);

  createCairo (NULL);
}

CairoOutputDevX::~CairoOutputDevX() {
  delete text;
}

void
CairoOutputDevX::createCairo(GfxState *state) {
  int w, h;
  XGCValues gcv;
  GC gc;

  w = state ? (int)(state->getPageWidth() + 0.5) : 1;
  h = state ? (int)(state->getPageHeight() + 0.5) : 1;

  if (!pixmap || w != pixmap_w || h != pixmap_h) {
    if (pixmap) {
      XFreePixmap(display, pixmap);
    }
    pixmap_w = w;
    pixmap_h = h;
    Window window =
      XCreateSimpleWindow(display, RootWindow (display, screenNum), 0, 0, 1, 1, 0,
			  WhitePixel(display, screenNum), WhitePixel(display, screenNum));
    
    pixmap = XCreatePixmap(display, window, w, h,
			   depth);
    XDestroyWindow (display, window);
  }

  gcv.foreground = WhitePixel(display, screenNum);
  gc = XCreateGC(display, pixmap, GCForeground, &gcv);
  XFillRectangle(display, pixmap, gc, 0, 0, w, h);

  cairo = cairo_create ();
  cairo_set_target_drawable (cairo, display, pixmap);

  XFreeGC(display, gc);
}

void CairoOutputDevX::drawChar(GfxState *state, double x, double y,
			       double dx, double dy,
			       double originX, double originY,
			       CharCode code, Unicode *u, int uLen) {
  text->addChar(state, x, y, dx, dy, code, u, uLen);
  CairoOutputDev::drawChar(state, x, y, dx, dy, originX, originY,
			   code, u, uLen);
}

GBool CairoOutputDevX::beginType3Char(GfxState *state, double x, double y,
				      double dx, double dy,
				      CharCode code, Unicode *u, int uLen) {
  text->addChar(state, x, y, dx, dy, code, u, uLen);
  return CairoOutputDev::beginType3Char(state, x, y, dx, dy, code, u, uLen);
}

int CairoOutputDevX::getBitmapWidth() {
  return pixmap_w;
};
int CairoOutputDevX::getBitmapHeight() {
  return pixmap_h;
};

void CairoOutputDevX::clear() {
  startDoc(NULL);
  startPage(0, NULL);
}

void CairoOutputDevX::startPage(int pageNum, GfxState *state) {
  CairoOutputDev::startPage(pageNum, state);
  text->startPage(state);
}

void CairoOutputDevX::updateFont(GfxState *state) {
  CairoOutputDev::updateFont(state);
  text->updateFont(state);
}

void CairoOutputDevX::endPage() {
  CairoOutputDev::endPage();
  if (!incrementalUpdate) {
    (*redrawCbk)(redrawCbkData);
  }
  text->coalesce(gTrue);
}

void CairoOutputDevX::dump() {
  if (incrementalUpdate) {
    (*redrawCbk)(redrawCbkData);
  }
}

void CairoOutputDevX::redraw(int srcX, int srcY,
			     Drawable destDrawable, GC destGC,
			     int destX, int destY,
			     int width, int height) {
  XCopyArea(display, pixmap, destDrawable, destGC,
	    srcX, srcY, width, height,
	    destX, destY);
}

#define xoutRound(x) ((int)((x) + 0.5))

GBool CairoOutputDevX::findText(Unicode *s, int len,
				GBool startAtTop, GBool stopAtBottom,
				GBool startAtLast, GBool stopAtLast,
				int *xMin, int *yMin,
				int *xMax, int *yMax) {
  double xMin1, yMin1, xMax1, yMax1;
  
  xMin1 = (double)*xMin;
  yMin1 = (double)*yMin;
  xMax1 = (double)*xMax;
  yMax1 = (double)*yMax;
  if (text->findText(s, len, startAtTop, stopAtBottom,
		     startAtLast, stopAtLast,
		     &xMin1, &yMin1, &xMax1, &yMax1)) {
    *xMin = xoutRound(xMin1);
    *xMax = xoutRound(xMax1);
    *yMin = xoutRound(yMin1);
    *yMax = xoutRound(yMax1);
    return gTrue;
  }
  return gFalse;
}

GooString *CairoOutputDevX::getText(int xMin, int yMin, int xMax, int yMax) {
  return text->getText((double)xMin, (double)yMin,
		       (double)xMax, (double)yMax);
}

void CairoOutputDevX::xorRectangle(int x0, int y0, int x1, int y1,
				   SplashPattern *pattern) {
  return;
}

