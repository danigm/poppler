//========================================================================
//
// UGooString.cc
//
// Unicode string
//
// Copyright 2005 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include <string.h>

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "PDFDocEncoding.h"
#include "UGooString.h"

int inline UGooString::roundedSize(int len) {
  int delta;
  if (len <= STR_STATIC_SIZE-1)
      return STR_STATIC_SIZE;
  delta = len < 256 ? 7 : 255;
  return ((len + 1) + delta) & ~delta;
}

// Make sure that the buffer is big enough to contain <newLength> characters
// plus terminating 0.
// We assume that if this is being called from the constructor, <s> was set
// to NULL and <length> was set to 0 to indicate unused string before calling us.
void inline UGooString::resize(int newLength) {
  Unicode *s1 = s;

  if (!s || (roundedSize(length) != roundedSize(newLength))) {
    // requires re-allocating data for string
    if (newLength < STR_STATIC_SIZE)
        s1 = sStatic;
    else
        s1 = new Unicode[roundedSize(newLength)];

    // we had to re-allocate the memory, so copy the content of previous
    // buffer into a new buffer
    if (s) {
      if (newLength < length) {
        memcpy(s1, s, newLength);
      } else {
        memcpy(s1, s, length);
      }
    }
    if (s != sStatic)
      delete[] s;
  }

  s = s1;
  length = newLength;
  s[length] = '\0';
}

UGooString::UGooString()
{
  s = NULL;
  length = 0;
  resize(0);
}

UGooString::UGooString(GooString &str)
{
  s = NULL;
  length = 0;
  if (str.hasUnicodeMarker())
  {
    resize((str.getLength() - 2) / 2);
    for (int j = 0; j < length; ++j) {
      s[j] = ((str.getChar(2 + 2*j) & 0xff) << 8) | (str.getChar(3 + 2*j) & 0xff);
    }
  } else
    Set(str.getCString(), str.getLength());
}

UGooString::UGooString(Unicode *u, int strLen)
{
  resize(strLen);
  s = u;
}

UGooString::UGooString(const UGooString &str)
{
  s = NULL;
  length = 0;
  Set(str);
}

UGooString::UGooString(const char *str, int strLen)
{
  s = NULL;
  length = 0;
  if (CALC_STRING_LEN == strLen)
    strLen = strlen(str);
  Set(str, strLen);
}

UGooString *UGooString::Set(const UGooString &str)
{
  resize(str.length);
  memcpy(s, str.s, length * sizeof(Unicode));
  return this;
}

UGooString* UGooString::Set(const char *str, int strLen)
{
  int  j;
  bool foundUnencoded = false;

  if (CALC_STRING_LEN == strLen)
    strLen = strlen(str);

  resize(strLen);
  for (j = 0; !foundUnencoded && j < length; ++j) {
    s[j] = pdfDocEncoding[str[j] & 0xff];
    if (!s[j]) {
        foundUnencoded = true;
        break;
    }
  }
  if ( foundUnencoded )
  {
    for (j = 0; j < length; ++j) {
      s[j] = str[j];
    }
  }
  return this;
}

UGooString *UGooString::clear()
{
    resize(0);
    return this;
}

UGooString::~UGooString()
{
  if (s != sStatic)
    delete[] s;
}

int UGooString::cmp(const UGooString &str) const
{
    return cmp(&str);
}

int UGooString::cmp(const UGooString *str) const
{
  int n1, n2, i, x;
  Unicode *p1, *p2;

  n1 = length;
  n2 = str->length;
  for (i = 0, p1 = s, p2 = str->s; i < n1 && i < n2; ++i, ++p1, ++p2) {
    x = *p1 - *p2;
    if (x != 0) {
      return x;
    }
  }
  return n1 - n2;
}

// FIXME: 
// a) this is confusing because GooString::getCSTring() returns a pointer
//    but UGooString returns a newly allocated copy. Should give this
//    a different name, like copyAsAscii() or copyAsGooString()
// b) this interface requires copying. It should be changed to take a
//    GooString& as a param and put the data inside it so that it uses
//    caching optimization of GooString. Callers should be changed to use
//    this new interface
char *UGooString::getCString() const
{
  char *res = new char[length + 1];
  for (int i = 0; i < length; i++) res[i] = s[i];
  res[length] = '\0';
  return res;
}

