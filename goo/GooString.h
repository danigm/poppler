//========================================================================
//
// GooString.h
//
// Simple variable-length string type.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef GSTRING_H
#define GSTRING_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"

class GooString {
public:

  // Create an empty string.
  GooString();

  // Create a string from a C string.
  GooString(const char *sA);

  // Create a string from <lengthA> chars at <sA>.  This string
  // can contain null characters.
  GooString(const char *sA, int lengthA);

  // Create a string from <lengthA> chars at <idx> in <str>.
  GooString(GooString *str, int idx, int lengthA);

  // Copy a string.
  GooString(GooString *str);
  GooString *copy() { return new GooString(this); }

  // Concatenate two strings.
  GooString(GooString *str1, GooString *str2);

  // Convert an integer to a string.
  static GooString *fromInt(int x);

  // Destructor.
  ~GooString();

  // Get length.
  int getLength() { return length; }

  // Get C string.
  char *getCString() { return s; }

  // Get <i>th character.
  char getChar(int i) { return s[i]; }

  // Change <i>th character.
  void setChar(int i, char c) { s[i] = c; }

  // Clear string to zero length.
  GooString *clear();

  // Append a character or string.
  GooString *append(char c);
  GooString *append(GooString *str);
  GooString *append(const char *str);
  GooString *append(const char *str, int lengthA);

  // Insert a character or string.
  GooString *insert(int i, char c);
  GooString *insert(int i, GooString *str);
  GooString *insert(int i, const char *str);
  GooString *insert(int i, const char *str, int lengthA);

  // Delete a character or range of characters.
  GooString *del(int i, int n = 1);

  // Convert string to all-upper/all-lower case.
  GooString *upperCase();
  GooString *lowerCase();

  // Compare two strings:  -1:<  0:=  +1:>
  int cmp(GooString *str);
  int cmpN(GooString *str, int n);
  int cmp(const char *sA);
  int cmpN(const char *sA, int n);

  GBool hasUnicodeMarker(void);

private:

  int length;
  char *s;

  void resize(int length1);
};

#endif
