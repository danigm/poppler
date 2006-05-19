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
  static CairoFont *create(GfxFont *gfxFont, XRef *xref,
			   FT_Library lib, GBool useCIDs);

  CairoFont *reference() { refcount++; return this; }
  void unreference() { if (--refcount == 0) delete this; }

  GBool matches(Ref &other);
  cairo_font_face_t *getFontFace(void);
  unsigned long getGlyph(CharCode code, Unicode *u, int uLen);

protected:
  CairoFont(GfxFont *gfxFont, XRef *xref);
  virtual ~CairoFont();

  Ref ref;
  XRef *xref;
  cairo_font_face_t *cairo_font_face;
  Gushort *codeToGID;
  int codeToGIDLen;
  int refcount;
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

  CairoFont *getFont(GfxFont *gfxFont, XRef *xref);

private:
  CairoFont *fontCache[cairoFontCacheSize];
  FT_Library lib;
  GBool useCIDs;
};

#endif
