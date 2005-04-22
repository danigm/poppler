//========================================================================
//
// CairoFontEngine.cc
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, Inc
//
//========================================================================

#include <config.h>

#include "config.h"
#include <string.h>
#include "CairoFontEngine.h"
#include "GlobalParams.h"
#include <fofi/FoFiTrueType.h>
#include <fofi/FoFiType1C.h>
#include "goo/gfile.h"
#include "Error.h"

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

static void fileWrite(void *stream, char *data, int len) {
  fwrite(data, 1, len, (FILE *)stream);
}

//------------------------------------------------------------------------
// Font substitutions
//------------------------------------------------------------------------

struct CairoOutFontSubst {
  char *name;
  double mWidth;
};

// index: {symbolic:12, fixed:8, serif:4, sans-serif:0} + bold*2 + italic
static CairoOutFontSubst cairoOutSubstFonts[16] = {
  {"Helvetica",             0.833},
  {"Helvetica-Oblique",     0.833},
  {"Helvetica-Bold",        0.889},
  {"Helvetica-BoldOblique", 0.889},
  {"Times-Roman",           0.788},
  {"Times-Italic",          0.722},
  {"Times-Bold",            0.833},
  {"Times-BoldItalic",      0.778},
  {"Courier",               0.600},
  {"Courier-Oblique",       0.600},
  {"Courier-Bold",          0.600},
  {"Courier-BoldOblique",   0.600},
  {"Symbol",                0.576},
  {"Symbol",                0.576},
  {"Symbol",                0.576},
  {"Symbol",                0.576}
};


//------------------------------------------------------------------------
// CairoFont
//------------------------------------------------------------------------

/* For the 0.3 release of poppler we're creating a CairoFont, and
 * thus, an FT_Face for each size of each font we encounter.  We
 * should of course be able to share FT_Face instances across all
 * sizes of a font.  However, to do that we need to be able to create
 * a cairo_unscaled_font_t, which is not possible with the cairo-0.4.0
 * API which is what we're targeting with this release. */

CairoFont::CairoFont(GfxFont *gfxFont, XRef *xref, FT_Library lib, 
		     double m11, double m12, double m21, double m22) {
  Ref embRef;
  Object refObj, strObj;
  GooString *tmpFileName, *fileName, *substName,*tmpFileName2;
  DisplayFontParam *dfp;
  FILE *tmpFile;
  int c, i, n;
  GfxFontType fontType;
  char **enc;
  char *name;
  FoFiTrueType *ff;
  FoFiType1C *ff1c;
  cairo_matrix_t *matrix;
  
  codeToGID = NULL;
  codeToGIDLen = 0;
  substIdx = -1;
  cairo_font = NULL;
  
  ref = *gfxFont->getID();
  fontType = gfxFont->getType();

  tmpFileName = NULL;

  if (gfxFont->getEmbeddedFontID(&embRef)) {
    if (!openTempFile(&tmpFileName, &tmpFile, "wb", NULL)) {
      error(-1, "Couldn't create temporary font file");
      goto err2;
    }
    
    refObj.initRef(embRef.num, embRef.gen);
    refObj.fetch(xref, &strObj);
    refObj.free();
    strObj.streamReset();
    while ((c = strObj.streamGetChar()) != EOF) {
      fputc(c, tmpFile);
    }
    strObj.streamClose();
    strObj.free();
    fclose(tmpFile);
    fileName = tmpFileName;
    
  } else if (!(fileName = gfxFont->getExtFontFile())) {
    // look for a display font mapping or a substitute font
    dfp = NULL;
    if (gfxFont->isCIDFont()) {
      if (((GfxCIDFont *)gfxFont)->getCollection()) {
	dfp = globalParams->
	  getDisplayCIDFont(gfxFont->getName(),
			    ((GfxCIDFont *)gfxFont)->getCollection());
      }
    } else {
      if (gfxFont->getName()) {
	dfp = globalParams->getDisplayFont(gfxFont->getName());
      }
	if (!dfp) {
	  // 8-bit font substitution
	  if (gfxFont->isFixedWidth()) {
	    substIdx = 8;
	  } else if (gfxFont->isSerif()) {
	    substIdx = 4;
	  } else {
	    substIdx = 0;
	  }
	  if (gfxFont->isBold()) {
	    substIdx += 2;
	  }
	  if (gfxFont->isItalic()) {
	    substIdx += 1;
	  }
	  substName = new GooString(cairoOutSubstFonts[substIdx].name);
	  dfp = globalParams->getDisplayFont(substName);
	  delete substName;
	}
    }
    if (!dfp) {
      error(-1, "Couldn't find a font for '%s'",
	    gfxFont->getName() ? gfxFont->getName()->getCString()
	    : "(unnamed)");
      goto err2;
    }
    switch (dfp->kind) {
    case displayFontT1:
      fileName = dfp->t1.fileName;
      fontType = gfxFont->isCIDFont() ? fontCIDType0 : fontType1;
      break;
    case displayFontTT:
      fileName = dfp->tt.fileName;
      fontType = gfxFont->isCIDFont() ? fontCIDType2 : fontTrueType;
      break;
    }
  }

  switch (fontType) {
  case fontType1:
  case fontType1C:
    if (FT_New_Face(lib, fileName->getCString(), 0, &face)) {
      goto err2;
    }
    
    enc = ((Gfx8BitFont *)gfxFont)->getEncoding();
    
    codeToGID = (Gushort *)gmalloc(256 * sizeof(int));
    codeToGIDLen = 256;
    for (i = 0; i < 256; ++i) {
      codeToGID[i] = 0;
      if ((name = enc[i])) {
	codeToGID[i] = (Gushort)FT_Get_Name_Index(face, name);
      }
    }
    break;
    
  case fontCIDType2:
    n = ((GfxCIDFont *)gfxFont)->getCIDToGIDLen();
    codeToGIDLen = n;
    codeToGID = (Gushort *)gmalloc(n * sizeof(Gushort));
    memcpy(codeToGID, ((GfxCIDFont *)gfxFont)->getCIDToGID(),
	   n * sizeof(Gushort));
    /* Fall through */
  case fontTrueType:
    if (!(ff = FoFiTrueType::load(fileName->getCString()))) {
      goto err2;
    }
    /* This might be set already for the CIDType2 case */
    if (codeToGID == NULL) {
      codeToGID = ((Gfx8BitFont *)gfxFont)->getCodeToGIDMap(ff);
      codeToGIDLen = 256;
    }
    if (!openTempFile(&tmpFileName2, &tmpFile, "wb", NULL)) {
      delete ff;
      goto err2;
    }
    ff->writeTTF(&fileWrite, tmpFile);
    fclose(tmpFile);
    delete ff;

    if (FT_New_Face(lib, tmpFileName2->getCString(), 0, &face)) {
      goto err2;
    }
    unlink (tmpFileName2->getCString());
    delete tmpFileName2;
    break;
    
  case fontCIDType0:
  case fontCIDType0C:
    // check for a CFF font
    if ((ff1c = FoFiType1C::load(fileName->getCString()))) {
      codeToGID = ff1c->getCIDToGIDMap(&codeToGIDLen);
      delete ff1c;
    } else {
      codeToGID = NULL;
      codeToGIDLen = 0;
    }
    if (FT_New_Face(lib, tmpFileName2->getCString(), 0, &face)) {
      gfree(codeToGID);
      goto err2;
    }
    break;
    
  default:
    printf ("font type not handled\n");
    goto err2;
    break;
  }

  // delete the (temporary) font file -- with Unix hard link
  // semantics, this will remove the last link; otherwise it will
  // return an error, leaving the file to be deleted later
  if (fileName == tmpFileName) {
    unlink (fileName->getCString());
  }

  this->m11 = m11;
  this->m12 = m12;
  this->m21 = m21;
  this->m22 = m22;
  matrix = cairo_matrix_create ();
  cairo_matrix_set_affine (matrix, m11, m12, m21, m22, 0, 0);
  cairo_font = cairo_ft_font_create_for_ft_face (face, FT_LOAD_NO_HINTING,
						 matrix);
  cairo_matrix_destroy (matrix);
  if (cairo_font == NULL)
    goto err2; /* this doesn't do anything, but it looks like we're
		* handling the error */

  return;
 err2:
  /* hmm? */
  printf ("some font thing failed\n");
}

CairoFont::~CairoFont() {
  cairo_font_destroy (cairo_font);

  /* cairo_font_t's created from an FT_Face are never cached so we can
   * free the font here.  There might be glyphs in the cairo glyph
   * cache referencing this font, but since we're throwing this font
   * away, they won't be used and will slowly fall out of the
   * cache. */
  FT_Done_Face (face);
}

GBool
CairoFont::matches(Ref &other,
		   double m11, double m12, double m21, double m22) {
  return (other.num == ref.num && other.gen == ref.gen &&
	  this->m11 == m11 && this->m12 == m12 &&
	  this->m21 == m21 && this->m22 == m22);
}

cairo_font_t *
CairoFont::getFont(void) {
  return cairo_font;
}

unsigned long
CairoFont::getGlyph(CharCode code,
		    Unicode *u, int uLen) {
  FT_UInt gid;

  if (codeToGID && code < codeToGIDLen) {
    gid = (FT_UInt)codeToGID[code];
  } else {
    gid = (FT_UInt)code;
  }
  return gid;
}

double
CairoFont::getSubstitutionCorrection(GfxFont *gfxFont)
{
  double w1, w2;
  CharCode code;
  char *name;
  
  // for substituted fonts: adjust the font matrix -- compare the
  // width of 'm' in the original font and the substituted font
  if (substIdx >= 0) {
    for (code = 0; code < 256; ++code) {
      if ((name = ((Gfx8BitFont *)gfxFont)->getCharName(code)) &&
	  name[0] == 'm' && name[1] == '\0') {
	break;
      }
    }
    if (code < 256) {
      w1 = ((Gfx8BitFont *)gfxFont)->getWidth(code);
      w2 = cairoOutSubstFonts[substIdx].mWidth;
      if (!gfxFont->isSymbolic()) {
	// if real font is substantially narrower than substituted
	// font, reduce the font size accordingly
	if (w1 > 0.01 && w1 < 0.9 * w2) {
	  w1 /= w2;
	  return w1;
	}
      }
    }
  }
  return 1.0;
}


//------------------------------------------------------------------------
// CairoFontEngine
//------------------------------------------------------------------------

CairoFontEngine::CairoFontEngine(FT_Library libA) {
  int i;

  lib = libA;
  for (i = 0; i < cairoFontCacheSize; ++i) {
    fontCache[i] = NULL;
  }
}

CairoFontEngine::~CairoFontEngine() {
  int i;
  
  for (i = 0; i < cairoFontCacheSize; ++i) {
    if (fontCache[i])
      delete fontCache[i];
  }
}

CairoFont *
CairoFontEngine::getFont(GfxFont *gfxFont, XRef *xref,
			 double m11, double m12, double m21, double m22) {
  int i, j;
  Ref ref;
  CairoFont *font;
  GfxFontType fontType;
  
  fontType = gfxFont->getType();
  if (fontType == fontType3) {
    /* Need to figure this out later */
    //    return NULL;
  }

  ref = *gfxFont->getID();

  for (i = 0; i < cairoFontCacheSize; ++i) {
    font = fontCache[i];
    if (font && font->matches(ref, m11, m12, m21, m22)) {
      for (j = i; j > 0; --j) {
	fontCache[j] = fontCache[j-1];
      }
      fontCache[0] = font;
      return font;
    }
  }
  
  font = new CairoFont (gfxFont, xref, lib, m11, m12, m21, m22);
  if (fontCache[cairoFontCacheSize - 1]) {
    delete fontCache[cairoFontCacheSize - 1];
  }
  for (j = cairoFontCacheSize - 1; j > 0; --j) {
    fontCache[j] = fontCache[j-1];
  }
  fontCache[0] = font;
  return font;
}

