//========================================================================
//
// Dict.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef DICT_H
#define DICT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"

class UGooString;
//------------------------------------------------------------------------
// Dict
//------------------------------------------------------------------------

struct DictEntry {
  UGooString *key;
  Object val;
};

class Dict {
public:

  // Constructor.
  Dict(XRef *xrefA);

  // Destructor.
  ~Dict();

  // Reference counting.
  int incRef() { return ++ref; }
  int decRef() { return --ref; }

  // Get number of entries.
  int getLength() { return length; }

  // Add an entry
  void add(const UGooString &key, Object *val);

  // Check if dictionary is of specified type.
  GBool is(char *type);

  // Look up an entry and return the value.  Returns a null object
  // if <key> is not in the dictionary.
  Object *lookup(const UGooString &key, Object *obj);
  Object *lookupNF(const UGooString &key, Object *obj);
  GBool lookupInt(const char *key, const char *alt_key, int *value);

  // Iterative accessors.
  UGooString *getKey(int i);
  Object *getVal(int i, Object *obj);
  Object *getValNF(int i, Object *obj);

  // Set the xref pointer.  This is only used in one special case: the
  // trailer dictionary, which is read before the xref table is
  // parsed.
  void setXRef(XRef *xrefA) { xref = xrefA; }

private:

  XRef *xref;			// the xref table for this PDF file
  DictEntry *entries;		// array of entries
  int size;			// size of <entries> array
  int length;			// number of entries in dictionary
  int ref;			// reference count

  DictEntry *find(const UGooString &key);
};

#endif
