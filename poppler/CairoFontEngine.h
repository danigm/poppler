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
  CairoFont(GfxFont *gfxFont, XRef *xref, FT_Library lib);
  ~CairoFont();

  GBool matches(Ref &other);
  cairo_font_t *getFont(cairo_matrix_t *font_scale);
  unsigned long getGlyph(CharCode code,
			 Unicode *u, int uLen);
  double getSubstitutionCorrection(GfxFont *gfxFont);
private:
  int substIdx;
  Ref ref;
  cairo_font_t *cairo_font;
  FT_Face face;

  Gushort *codeToGID;
  int codeToGIDLen;
};

//------------------------------------------------------------------------

#define cairoFontCacheSize 16

//------------------------------------------------------------------------
// CairoFontEngine
//------------------------------------------------------------------------

class CairoFontEngine {
public:

  // Create a font engine.
  CairoFontEngine(FT_Library libA);
  ~CairoFontEngine();

  CairoFont *getFont(GfxFont *gfxFont, XRef *xref);

private:
  CairoFont *fontCache[cairoFontCacheSize];
  FT_Library lib;
};

#endif
