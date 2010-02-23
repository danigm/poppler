//========================================================================
//
// StdinCachedFile.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
//
//========================================================================

#ifndef STDINCACHELOADER_H
#define STDINCACHELOADER_H

#include "CachedFile.h"

class StdinCacheLoader : public CachedFileLoader {

public:

  size_t init(GooString *dummy, CachedFile* cachedFile);
  int load(GooVector<ByteRange> *ranges, CachedFileWriter *writer);

};

#endif

