//========================================================================
//
// CairoFontEngine.cc
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, Inc
//
//========================================================================

#include <config.h>

#include <math.h>
#include "config.h"
#include <string.h>
#include "CairoFontEngine.h"
#include "CharCodeToUnicode.h"
#include "GlobalParams.h"
#include <fofi/FoFiTrueType.h>
#include <fofi/FoFiType1C.h>
#include "goo/gfile.h"
#include "Error.h"
#include "Gfx.h"
#include "Page.h"
#include "Parser.h"
#include "Lexer.h"
#include "CairoOutputDev.h"

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

static void fileWrite(void *stream, char *data, int len) {
  fwrite(data, 1, len, (FILE *)stream);
}

static void
init_cairo_matrix(cairo_matrix_t *matrix, double *d)
{
  matrix->xx = d[0];
  matrix->yx = -d[2];
  matrix->xy = d[1];
  matrix->yy = -d[3];
  matrix->x0 = d[2] + d[4];
  matrix->y0 = d[3] + d[5];
}		  

static cairo_user_data_key_t cairo_font_face_key;

//------------------------------------------------------------------------
// CairoType3Font
//------------------------------------------------------------------------

class CairoType3Font : public CairoFont {
public:
  CairoType3Font(GfxFont *gfxFont, XRef *xref);

private:

  Gfx8BitFont *gfxFont;

  /* Static functions for implementing the cairo user font interface. */
  static void *scaled_font_create (cairo_scaled_font_t        *scaled_font,
				   const cairo_matrix_t       *font_matrix,
				   const cairo_matrix_t       *ctm,
				   const cairo_font_options_t *options,
				   cairo_font_extents_t       *metrics);
  static void scaled_font_destroy (void *closure);

  static unsigned long ucs4_to_index(void              *closure,
				     unsigned long      ucs4);

  static cairo_status_t get_glyph_metrics(void                 *closure,
					  unsigned long         index,
					  cairo_font_options_t *font_options,
					  cairo_text_extents_t *metrics);
  
  static cairo_status_t render_glyph(void                 *closure,
				     unsigned long         index,
				     cairo_font_options_t *font_options,
				     cairo_t              *cr);

  static cairo_user_font_backend_t font_backend;
};

cairo_user_font_backend_t CairoType3Font::font_backend = {
  CairoType3Font::scaled_font_create,
  CairoType3Font::scaled_font_destroy,  
  CairoType3Font::ucs4_to_index,
  CairoType3Font::get_glyph_metrics,  
  CairoType3Font::render_glyph
};

CairoType3Font::CairoType3Font(GfxFont *gfxFontA, XRef *xref)
  : CairoFont(gfxFontA, xref)
{
  this->gfxFont = (Gfx8BitFont *) gfxFontA;
  cairo_font_face = cairo_user_font_face_create (&font_backend);

  printf ("created type3 font\n");
}

void *
CairoType3Font::scaled_font_create (cairo_scaled_font_t        *scaled_font,
				    const cairo_matrix_t       *font_matrix,
				    const cairo_matrix_t       *ctm,
				    const cairo_font_options_t *options,
				    cairo_font_extents_t       *metrics)
{
  return cairo_scaled_font_get_font_face (scaled_font);
}

void
CairoType3Font::scaled_font_destroy (void *closure)
{
}

unsigned long
CairoType3Font::ucs4_to_index(void          *closure,
			      unsigned long  ucs4)
{
  return 0;
}

cairo_status_t
CairoType3Font::get_glyph_metrics(void                 *closure,
				  unsigned long         index,
				  cairo_font_options_t *font_options,
				  cairo_text_extents_t *metrics)
{
  CairoType3Font *font;
  double *bbox;
  Object charProc, obj;
  Object argObjs[maxArgs];
  Parser *parser;
  char *name;
  double args[maxArgs];
  int i, numArgs;
  cairo_matrix_t font_matrix;
  cairo_font_face_t *face = (cairo_font_face_t *) closure;

  font = (CairoType3Font *)
    cairo_font_face_get_user_data (face, &cairo_font_face_key);

  font->gfxFont->getCharProc(index, &charProc);
  if (!charProc.isStream()) {
      charProc.free();
      return CAIRO_STATUS_SUCCESS;
  }
  parser = new Parser(font->xref, new Lexer(font->xref, &charProc));

  numArgs = 0;
  parser->getObj(&obj);
  while (!obj.isEOF()) {

    // got a command - execute it
    if (obj.isCmd()) {

      name = obj.getCmd();
      if (strcmp(name, "d0") == 0) {
	/* FIXME: Handle d0 glyphs. */
      } else if (strcmp(name, "d1") == 0) {
	if (numArgs < 6)
	  goto cont;
	for (i = 0; i < 6; i++) {
	  if (!argObjs[numArgs - 6 + i].isNum())
	    goto cont;
	  args[i] = argObjs[numArgs - 6 + i].getNum();
	}

	/* Transform glyph coordinates to text coordinates, which is
	 * what cairo expects. */
	init_cairo_matrix (&font_matrix, font->gfxFont->getFontMatrix());
	cairo_matrix_transform_distance (&font_matrix, &args[0], &args[1]);
	cairo_matrix_transform_distance (&font_matrix, &args[2], &args[3]);
	cairo_matrix_transform_distance (&font_matrix, &args[4], &args[5]);

	metrics->x_bearing = args[2];
	metrics->y_bearing = args[5];
	metrics->width = args[4] - args[2];
	metrics->height = args[3] - args[5];
	metrics->x_advance = args[0];
	metrics->y_advance = args[1];

	break;
      }
    cont:
      obj.free();
      for (i = 0; i < numArgs; ++i)
	argObjs[i].free();
      numArgs = 0;
    } else if (numArgs < maxArgs) {
      argObjs[numArgs++] = obj;
    }

    parser->getObj(&obj);
  }
  obj.free();
  for (i = 0; i < numArgs; ++i)
    argObjs[i].free();

  delete parser;

  return CAIRO_STATUS_SUCCESS;
}

cairo_status_t
CairoType3Font::render_glyph(void                 *closure,
			     unsigned long         index,
			     cairo_font_options_t *font_options,
			     cairo_t              *cr)
{
  Dict *resources;
  Gfx *gfx;
  Object charProc;
  CairoOutputDev *out;
  CairoType3Font *font;
  cairo_matrix_t font_matrix;
  cairo_font_face_t *face = (cairo_font_face_t *) closure;

  cairo_surface_t *target;
  int width, height;

  target = cairo_get_target (cr);
  width = cairo_image_surface_get_width (target);
  height = cairo_image_surface_get_height (target);
  PDFRectangle box(0, 0, width, height);

  font = (CairoType3Font *)
    cairo_font_face_get_user_data (face, &cairo_font_face_key);

  out = new CairoOutputDev();
  out->setCairo(cr);
  resources = font->gfxFont->getResources();

  gfx = new Gfx(font->xref, out, resources, &box, NULL);

  font->gfxFont->getCharProc(index, &charProc);

  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba (cr, 0, 0, 0, 0);
  cairo_paint (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  cairo_move_to (cr, -10, 0);
  cairo_line_to (cr, 10, 0);
  cairo_move_to (cr, 0, -10);
  cairo_line_to (cr, 0, 10);
  cairo_move_to (cr, -10, -10);
  cairo_line_to (cr, 10, 10);
  cairo_move_to (cr, 10, -10);
  cairo_line_to (cr, -10, 10);
  cairo_set_line_width (cr, 5);
  cairo_stroke (cr);

  init_cairo_matrix(&font_matrix, font->gfxFont->getFontMatrix());
  cairo_transform (cr, &font_matrix);

  if (charProc.isStream()) {
    gfx->display(&charProc, gFalse);
  } else {
    error(-1, "Missing or bad Type3 CharProc entry");
  }

  delete gfx;
  delete out;

  return CAIRO_STATUS_SUCCESS;
}

//------------------------------------------------------------------------
// CairoFTFont
//------------------------------------------------------------------------

class CairoFTFont : public CairoFont {
public:
  CairoFTFont(GfxFont *gfxFont, XRef *xref, FT_Library lib, GBool useCIDs);
  virtual ~CairoFTFont();

  GBool matches(Ref &other);
  cairo_font_face_t *getFontFace(void);
  unsigned long getGlyph(CharCode code, Unicode *u, int uLen);

private:
  FT_Face face;
};

CairoFTFont::CairoFTFont(GfxFont *gfxFont, XRef *xref,
			 FT_Library lib, GBool useCIDs)
  : CairoFont(gfxFont, xref)
{
  Ref embRef;
  Object refObj, strObj;
  GooString *tmpFileName, *fileName, *substName,*tmpFileName2;
  DisplayFontParam *dfp;
  FILE *tmpFile;
  int c, i, n, code, cmap;
  GfxFontType fontType;
  char **enc;
  char *name;
  FoFiTrueType *ff;
  FoFiType1C *ff1c;
  CharCodeToUnicode *ctu;
  Unicode uBuf[8];
  
  dfp = NULL;
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
    if (gfxFont->getName()) {
      dfp = globalParams->getDisplayFont(gfxFont);
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
      error(-1, "could not create type1 face");
      goto err2;
    }
    
    enc = ((Gfx8BitFont *)gfxFont)->getEncoding();
    
    codeToGID = (Gushort *)gmallocn(256, sizeof(int));
    codeToGIDLen = 256;
    for (i = 0; i < 256; ++i) {
      codeToGID[i] = 0;
      if ((name = enc[i])) {
	codeToGID[i] = (Gushort)FT_Get_Name_Index(face, name);
      }
    }
    break;
    
  case fontCIDType2:
    codeToGID = NULL;
    n = 0;
    if (dfp) {
      // create a CID-to-GID mapping, via Unicode
      if ((ctu = ((GfxCIDFont *)gfxFont)->getToUnicode())) {
        if ((ff = FoFiTrueType::load(fileName->getCString()))) {
          // look for a Unicode cmap
          for (cmap = 0; cmap < ff->getNumCmaps(); ++cmap) {
            if ((ff->getCmapPlatform(cmap) == 3 &&
                 ff->getCmapEncoding(cmap) == 1) ||
                 ff->getCmapPlatform(cmap) == 0) {
              break;
            }
          }
          if (cmap < ff->getNumCmaps()) {
            // map CID -> Unicode -> GID
            n = ctu->getLength();
            codeToGID = (Gushort *)gmallocn(n, sizeof(Gushort));
            for (code = 0; code < n; ++code) {
              if (ctu->mapToUnicode(code, uBuf, 8) > 0) {
                  codeToGID[code] = ff->mapCodeToGID(cmap, uBuf[0]);
              } else {
                codeToGID[code] = 0;
              }
            }
          }
          delete ff;
        }
        ctu->decRefCnt();
      } else {
        error(-1, "Couldn't find a mapping to Unicode for font '%s'",
              gfxFont->getName() ? gfxFont->getName()->getCString()
                        : "(unnamed)");
      }
    } else {
      if (((GfxCIDFont *)gfxFont)->getCIDToGID()) {
	n = ((GfxCIDFont *)gfxFont)->getCIDToGIDLen();
	codeToGID = (Gushort *)gmallocn(n, sizeof(Gushort));
	memcpy(codeToGID, ((GfxCIDFont *)gfxFont)->getCIDToGID(),
	       n * sizeof(Gushort));
      }
    }
    codeToGIDLen = n;
    /* Fall through */
  case fontTrueType:
    if (!(ff = FoFiTrueType::load(fileName->getCString()))) {
      error(-1, "failed to load truetype font\n");
      goto err2;
    }
    /* This might be set already for the CIDType2 case */
    if (fontType == fontTrueType) {
      codeToGID = ((Gfx8BitFont *)gfxFont)->getCodeToGIDMap(ff);
      codeToGIDLen = 256;
    }
    if (!openTempFile(&tmpFileName2, &tmpFile, "wb", NULL)) {
      delete ff;
      error(-1, "failed to open truetype tempfile\n");
      goto err2;
    }
    ff->writeTTF(&fileWrite, tmpFile);
    fclose(tmpFile);
    delete ff;

    if (FT_New_Face(lib, tmpFileName2->getCString(), 0, &face)) {
      error(-1, "could not create truetype face\n");
      goto err2;
    }
    unlink (tmpFileName2->getCString());
    delete tmpFileName2;
    break;
    
  case fontCIDType0:
  case fontCIDType0C:

    codeToGID = NULL;
    codeToGIDLen = 0;

    if (!useCIDs)
    {
      if ((ff1c = FoFiType1C::load(fileName->getCString()))) {
        codeToGID = ff1c->getCIDToGIDMap(&codeToGIDLen);
        delete ff1c;
      }
    }

    if (FT_New_Face(lib, fileName->getCString(), 0, &face)) {
      gfree(codeToGID);
      codeToGID = NULL;
      error(-1, "could not create cid face\n");
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
    delete tmpFileName;
  }

  cairo_font_face = cairo_ft_font_face_create_for_ft_face (face,
							   FT_LOAD_NO_HINTING |
							   FT_LOAD_NO_BITMAP);
  return;
 err2:
  /* hmm? */
  printf ("some font thing failed\n");
}

CairoFTFont::~CairoFTFont() {
  FT_Done_Face (face);
}

//------------------------------------------------------------------------
// CairoFont
//------------------------------------------------------------------------

void cairo_font_destroy (void *data)
{
  CairoFont *font = (CairoFont *) data;

  font->unreference();
}

CairoFont *
CairoFont::create(GfxFont *gfxFont, XRef *xref, FT_Library lib, GBool useCIDs)
{
  CairoFont *font;

  switch (gfxFont->getType()) {
  case fontType3:
    font = new CairoType3Font(gfxFont, xref);
    break;
  default:
    font = new CairoFTFont(gfxFont, xref, lib, useCIDs);
    break;
  }

  if (font->cairo_font_face == NULL)
    error(-1, "could not create cairo font\n");
  else
    cairo_font_face_set_user_data (font->cairo_font_face, 
				   &cairo_font_face_key,
				   font->reference(),
				   cairo_font_destroy);

  return font;
}

CairoFont::CairoFont(GfxFont *gfxFont, XRef *xrefA)
{
  codeToGID = NULL;
  codeToGIDLen = 0;
  ref = *gfxFont->getID();
  xref = xrefA;
  refcount = 1;
}

CairoFont::~CairoFont()
{
  gfree(codeToGID);
}

GBool
CairoFont::matches(Ref &other) {
  return (other.num == ref.num && other.gen == ref.gen);
}

cairo_font_face_t *
CairoFont::getFontFace(void) {
  return cairo_font_face;
}

unsigned long
CairoFont::getGlyph(CharCode code,
		    Unicode *u, int uLen) {
  FT_UInt gid;

  if (codeToGID && code < (unsigned) codeToGIDLen) {
    gid = (FT_UInt)codeToGID[code];
  } else {
    gid = (FT_UInt)code;
  }
  return gid;
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
  
  FT_Int major, minor, patch;
  // as of FT 2.1.8, CID fonts are indexed by CID instead of GID
  FT_Library_Version(lib, &major, &minor, &patch);
  useCIDs = major > 2 ||
            (major == 2 && (minor > 1 || (minor == 1 && patch > 7)));
}

CairoFontEngine::~CairoFontEngine() {
  int i;
  
  for (i = 0; i < cairoFontCacheSize; ++i) {
    if (fontCache[i])
      fontCache[i]->unreference();
  }
}

CairoFont *
CairoFontEngine::getFont(GfxFont *gfxFont, XRef *xref) {
  int i, j;
  Ref ref;
  CairoFont *font;

  ref = *gfxFont->getID();

  for (i = 0; i < cairoFontCacheSize; ++i) {
    font = fontCache[i];
    if (font && font->matches(ref)) {
      for (j = i; j > 0; --j) {
	fontCache[j] = fontCache[j-1];
      }
      fontCache[0] = font;
      return font;
    }
  }
  
  font = CairoFont::create (gfxFont, xref, lib, useCIDs);
  if (fontCache[cairoFontCacheSize - 1]) {
    fontCache[cairoFontCacheSize - 1]->unreference();
  }
  for (j = cairoFontCacheSize - 1; j > 0; --j) {
    fontCache[j] = fontCache[j-1];
  }
  fontCache[0] = font;
  return font;
}
