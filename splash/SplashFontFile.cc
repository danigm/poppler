//========================================================================
//
// SplashFontFile.cc
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <unistd.h>
#include "goo/GooString.h"
#include "SplashFontFile.h"
#include "SplashFontFileID.h"

#ifdef VMS
#if (__VMS_VER < 70000000)
extern "C" int unlink(char *filename);
#endif
#endif

//------------------------------------------------------------------------
// SplashFontFile
//------------------------------------------------------------------------

SplashFontFile::SplashFontFile(SplashFontFileID *idA, char *fileNameA,
			       GBool deleteFileA) {
  id = idA;
  fileName = new GooString(fileNameA);
  deleteFile = deleteFileA;
  refCnt = 0;
}

SplashFontFile::~SplashFontFile() {
  if (deleteFile) {
    unlink(fileName->getCString());
  }
  delete fileName;
  delete id;
}

void SplashFontFile::incRefCnt() {
  ++refCnt;
}

void SplashFontFile::decRefCnt() {
  if (!--refCnt) {
    delete this;
  }
}
