//========================================================================
//
// Dict.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include <string.h>
#include "goo/gmem.h"
#include "Object.h"
#include "UGooString.h"
#include "XRef.h"
#include "Dict.h"

//------------------------------------------------------------------------
// Dict
//------------------------------------------------------------------------

Dict::Dict(XRef *xrefA) {
  xref = xrefA;
  entries = NULL;
  size = length = 0;
  ref = 1;
}

Dict::~Dict() {
  int i;

  for (i = 0; i < length; ++i) {
    delete entries[i].key;
    entries[i].val.free();
  }
  gfree(entries);
}

void Dict::addOwnKeyVal(UGooString *key, Object *val) {
  if (length == size) {
    if (length == 0) {
      size = 8;
    } else {
      size *= 2;
    }
    entries = (DictEntry *)greallocn(entries, size, sizeof(DictEntry));
  }
  entries[length].key = key;
  entries[length].val = *val;
  ++length;
}

inline DictEntry *Dict::find(const UGooString &key) {
  int i;

  for (i = 0; i < length; ++i) {
    if (!key.cmp(entries[i].key))
      return &entries[i];
  }
  return NULL;
}

GBool Dict::is(char *type) {
  DictEntry *e;

  return (e = find("Type")) && e->val.isName(type);
}

Object *Dict::lookup(const UGooString &key, Object *obj) {
  DictEntry *e;

  return (e = find(key)) ? e->val.fetch(xref, obj) : obj->initNull();
}

Object *Dict::lookupNF(const UGooString &key, Object *obj) {
  DictEntry *e;

  return (e = find(key)) ? e->val.copy(obj) : obj->initNull();
}

GBool Dict::lookupInt(const char *key, const char *alt_key, int *value)
{
  Object obj1;
  GBool success = gFalse;
  
  lookup ((char *) key, &obj1);
  if (obj1.isNull () && alt_key != NULL) {
    obj1.free ();
    lookup ((char *) alt_key, &obj1);
  }
  if (obj1.isInt ()) {
    *value = obj1.getInt ();
    success = gTrue;
  }

  obj1.free ();

  return success;
}

UGooString *Dict::getKey(int i) {
  return entries[i].key;
}

Object *Dict::getVal(int i, Object *obj) {
  return entries[i].val.fetch(xref, obj);
}

Object *Dict::getValNF(int i, Object *obj) {
  return entries[i].val.copy(obj);
}
