//========================================================================
//
// CachedFile.h
//
// Caching files support.
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2009 Stefan Thomas <thomas@eload24.com>
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef CACHEDFILE_H
#define CACHEDFILE_H

#include "poppler-config.h"

#include "goo/gtypes.h"
#include "Object.h"
#include "Stream.h"
#include "goo/GooVector.h"

//------------------------------------------------------------------------

#define CachedFileChunkSize 8192

class GooString;
class CachedFileLoader;

//------------------------------------------------------------------------

class CachedFile {

friend class CachedFileWriter;

public:

  CachedFile(CachedFileLoader *cacheLoader, GooString *uri);

  Guint getLength() { return length; }
  long int tell();
  int seek(long int offset, int origin);
  size_t read(void * ptr, size_t unitsize, size_t count);
  size_t write(const char *ptr, size_t size, size_t fromByte);
  int cache(const GooVector<ByteRange> &ranges);

  // Reference counting.
  void incRefCnt();
  void decRefCnt();

private:

  ~CachedFile();

  enum ChunkState {
    chunkStateNew = 0,
    chunkStateLoaded
  };

  typedef struct {
    ChunkState state;
    char data[CachedFileChunkSize];
  } Chunk;

  int cache(size_t offset, size_t length);

  CachedFileLoader *loader;
  GooString *uri;

  size_t length;
  size_t streamPos;

  GooVector<Chunk> *chunks;

  int refCnt;  // reference count

};

//------------------------------------------------------------------------

class CachedFileWriter {

public:

  CachedFileWriter(CachedFile *cachedFile, GooVector<int> *chunksA);
  ~CachedFileWriter();

  size_t write(const char *ptr, size_t size);

private:

  CachedFile *cachedFile;
  GooVector<int> *chunks;
  GooVector<int>::iterator it;
  size_t offset;

};

//------------------------------------------------------------------------

class CachedFileLoader {

public:

  virtual ~CachedFileLoader() {};
  virtual size_t init(GooString *uri, CachedFile *cachedFile) = 0;
  virtual int load(const GooVector<ByteRange> &ranges, CachedFileWriter *writer) = 0;

};

//------------------------------------------------------------------------

#endif
