//========================================================================
//
// CairoOutputDevX.h
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, INC
//
//========================================================================

#ifndef CAIROOUTPUTDEVX_H
#define CAIROOUTPUTDEVX_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <X11/Xlib.h>
#include "CairoOutputDev.h"
#include "GfxState.h"
#include "splash/SplashTypes.h"

class SplashPattern;

//------------------------------------------------------------------------
// CairoOutputDevX
//------------------------------------------------------------------------

class CairoOutputDevX: public CairoOutputDev {
public:

  // Constructor.
  CairoOutputDevX(Display *displayA, int screenNumA,
		  Visual *visualA, Colormap colormapA,
		  GBool reverseVideoA, SplashColor paperColorA,
		  GBool installCmapA, int rgbCubeSizeA,
		  GBool incrementalUpdateA,
		  void (*redrawCbkA)(void *data),
		  void *redrawCbkDataA);

  // Destructor.
  virtual ~CairoOutputDevX();

  // End a page.
  virtual void endPage();

  // Dump page contents to display.
  virtual void dump();

  //----- special access

  int getBitmapWidth();
  int getBitmapHeight();
  
  // Clear out the document (used when displaying an empty window).
  void clear();
  
  // Copy the rectangle (srcX, srcY, width, height) to (destX, destY)
  // in destDC.
  void redraw(int srcX, int srcY,
	      Drawable destDrawable, GC destGC,
	      int destX, int destY,
	      int width, int height);

  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, Unicode *u, int uLen);

  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);

  virtual void startPage(int pageNum, GfxState *state);

  virtual void updateFont(GfxState *state);

  // Find a string.  If <startAtTop> is true, starts looking at the
  // top of the page; else if <startAtLast> is true, starts looking
  // immediately after the last find result; else starts looking at
  // <xMin>,<yMin>.  If <stopAtBottom> is true, stops looking at the
  // bottom of the page; else if <stopAtLast> is true, stops looking
  // just before the last find result; else stops looking at
  // <xMax>,<yMax>.
  GBool findText(Unicode *s, int len,
		 GBool startAtTop, GBool stopAtBottom,
		 GBool startAtLast, GBool stopAtLast,
		 int *xMin, int *yMin,
		 int *xMax, int *yMax);

  // Get the text which is inside the specified rectangle.
  GooString *getText(int xMin, int yMin, int xMax, int yMax);

  // XOR a rectangular region in the bitmap with <pattern>.  <pattern>
  // is passed to Splash::setFillPattern, so it should not be used
  // after calling this function.
  void xorRectangle(int x0, int y0, int x1, int y1, SplashPattern *pattern);
  
private:
  virtual void createCairo(GfxState *state);

  GBool incrementalUpdate;      // incrementally update the display?
  void (*redrawCbk)(void *data);
  void *redrawCbkData;
  TextPage *text;               // text from the current page

  Display *display;		// X display pointer
  int screenNum;                // X screen
  Visual *visual;		// X visual
  Guint depth;			// visual depth
  GBool trueColor;		// set if using a TrueColor visual
  
  Pixmap pixmap;
  int pixmap_w;
  int pixmap_h;
};

#endif
