//========================================================================
//
// CairoFontEngine.h
//
//========================================================================

#ifndef CAIROFONTENGINE_H
#define CAIROFONTENGINE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include <cairo-ft.h>

#include "GfxFont.h"

class CairoFont {
public:
  CairoFont(GfxFont *gfxFont, XRef *xref, FT_Library lib,
	    double m11, double m12, double m21, double m22);
  ~CairoFont();

  GBool matches(Ref &other, double m11, double m12, double m21, double m22);
  cairo_font_t *getFont(void);
  unsigned long getGlyph(CharCode code, Unicode *u, int uLen);
  double getSubstitutionCorrection(GfxFont *gfxFont);
private:
  int substIdx;
  Ref ref;
  cairo_font_t *cairo_font;
  FT_Face face;

  Gushort *codeToGID;
  int codeToGIDLen;
  double m11, m12, m21, m22;
};

//------------------------------------------------------------------------

#define cairoFontCacheSize 64

//------------------------------------------------------------------------
// CairoFontEngine
//------------------------------------------------------------------------

class CairoFontEngine {
public:

  // Create a font engine.
  CairoFontEngine(FT_Library libA);
  ~CairoFontEngine();

  CairoFont *getFont(GfxFont *gfxFont, XRef *xref,
		     double m11, double m12, double m21, double m22);

private:
  CairoFont *fontCache[cairoFontCacheSize];
  FT_Library lib;
};

#endif
