//========================================================================
//
// CairoOutputDevImage.h
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, INC
//
//========================================================================

#ifndef CAIROOUTPUTDEVIMAGE_H
#define CAIROOUTPUTDEVIMAGE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "splash/SplashTypes.h"
#include "CairoOutputDev.h"
#include "GfxState.h"

//------------------------------------------------------------------------
// CairoOutputDevImage
//------------------------------------------------------------------------

class CairoOutputDevImage: public CairoOutputDev {
public:

  // Constructor.
  CairoOutputDevImage(void);

  // Destructor.
  virtual ~CairoOutputDevImage();

  virtual void createCairo(GfxState *state);

  SplashBitmap *getBitmap();
  
private:
  unsigned char *pixels;
  int pixels_w, pixels_h;
};

#endif
