//========================================================================
//
// GooTimer.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef GOOTIMER_H
#define GOOTIMER_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"
#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#endif

#ifdef _MSC_VER
#include <windows.h>
#endif

//------------------------------------------------------------------------
// GooTimer
//------------------------------------------------------------------------

class GooTimer {
public:

  // Create a new timer.
  GooTimer();

  void start();
  void stop();
  double getElapsed();

private:
#ifdef HAVE_GETTIMEOFDAY
  struct timeval start_time;
  struct timeval end_time;
#elif defined(_MSC_VER)
  LARGE_INTEGER start_time;
  LARGE_INTEGER end_time;
#endif
  GBool active;
};

#endif
