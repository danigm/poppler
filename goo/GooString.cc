//========================================================================
//
// GooString.cc
//
// Simple variable-length string type.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "gtypes.h"
#include "GooString.h"

static inline int size(int len) {
  int delta;

  delta = len < 256 ? 7 : 255;
  return ((len + 1) + delta) & ~delta;
}

inline void GooString::resize(int length1) {
  char *s1;

  if (!s) {
    s = new char[size(length1)];
  } else if (size(length1) != size(length)) {
    s1 = new char[size(length1)];
    if (length1 < length) {
      memcpy(s1, s, length1);
      s1[length1] = '\0';
    } else {
      memcpy(s1, s, length + 1);
    }
    delete[] s;
    s = s1;
  }
}

GooString::GooString() {
  s = NULL;
  resize(length = 0);
  s[0] = '\0';
}

GooString::GooString(const char *sA) {
  int n = strlen(sA);

  s = NULL;
  resize(length = n);
  memcpy(s, sA, n + 1);
}

GooString::GooString(const char *sA, int lengthA) {
  s = NULL;
  resize(length = lengthA);
  memcpy(s, sA, length * sizeof(char));
  s[length] = '\0';
}

GooString::GooString(GooString *str, int idx, int lengthA) {
  s = NULL;
  resize(length = lengthA);
  memcpy(s, str->getCString() + idx, length);
  s[length] = '\0';
}

GooString::GooString(GooString *str) {
  s = NULL;
  resize(length = str->getLength());
  memcpy(s, str->getCString(), length + 1);
}

GooString::GooString(GooString *str1, GooString *str2) {
  int n1 = str1->getLength();
  int n2 = str2->getLength();

  s = NULL;
  resize(length = n1 + n2);
  memcpy(s, str1->getCString(), n1);
  memcpy(s + n1, str2->getCString(), n2 + 1);
}

GooString *GooString::fromInt(int x) {
  char buf[24]; // enough space for 64-bit ints plus a little extra
  GBool neg;
  Guint y;
  int i;

  i = 24;
  if (x == 0) {
    buf[--i] = '0';
  } else {
    if ((neg = x < 0)) {
      y = (Guint)-x;
    } else {
      y = (Guint)x;
    }
    while (i > 0 && y > 0) {
      buf[--i] = '0' + y % 10;
      y /= 10;
    }
    if (neg && i > 0) {
      buf[--i] = '-';
    }
  }
  return new GooString(buf + i, 24 - i);
}

GooString::~GooString() {
  delete[] s;
}

GooString *GooString::clear() {
  s[length = 0] = '\0';
  resize(0);
  return this;
}

GooString *GooString::append(char c) {
  resize(length + 1);
  s[length++] = c;
  s[length] = '\0';
  return this;
}

GooString *GooString::append(GooString *str) {
  int n = str->getLength();

  resize(length + n);
  memcpy(s + length, str->getCString(), n + 1);
  length += n;
  return this;
}

GooString *GooString::append(const char *str) {
  int n = strlen(str);

  resize(length + n);
  memcpy(s + length, str, n + 1);
  length += n;
  return this;
}

GooString *GooString::append(const char *str, int lengthA) {
  resize(length + lengthA);
  memcpy(s + length, str, lengthA);
  length += lengthA;
  s[length] = '\0';
  return this;
}

GooString *GooString::insert(int i, char c) {
  int j;

  resize(length + 1);
  for (j = length + 1; j > i; --j)
    s[j] = s[j-1];
  s[i] = c;
  ++length;
  return this;
}

GooString *GooString::insert(int i, GooString *str) {
  int n = str->getLength();
  int j;

  resize(length + n);
  for (j = length; j >= i; --j)
    s[j+n] = s[j];
  memcpy(s+i, str->getCString(), n);
  length += n;
  return this;
}

GooString *GooString::insert(int i, const char *str) {
  int n = strlen(str);
  int j;

  resize(length + n);
  for (j = length; j >= i; --j)
    s[j+n] = s[j];
  memcpy(s+i, str, n);
  length += n;
  return this;
}

GooString *GooString::insert(int i, const char *str, int lengthA) {
  int j;

  resize(length + lengthA);
  for (j = length; j >= i; --j)
    s[j+lengthA] = s[j];
  memcpy(s+i, str, lengthA);
  length += lengthA;
  return this;
}

GooString *GooString::del(int i, int n) {
  int j;

  if (n > 0) {
    if (i + n > length) {
      n = length - i;
    }
    for (j = i; j <= length - n; ++j) {
      s[j] = s[j + n];
    }
    resize(length -= n);
  }
  return this;
}

GooString *GooString::upperCase() {
  int i;

  for (i = 0; i < length; ++i) {
    if (islower(s[i]))
      s[i] = toupper(s[i]);
  }
  return this;
}

GooString *GooString::lowerCase() {
  int i;

  for (i = 0; i < length; ++i) {
    if (isupper(s[i]))
      s[i] = tolower(s[i]);
  }
  return this;
}

int GooString::cmp(GooString *str) {
  int n1, n2, i, x;
  char *p1, *p2;

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

int GooString::cmpN(GooString *str, int n) {
  int n1, n2, i, x;
  char *p1, *p2;

  n1 = length;
  n2 = str->length;
  for (i = 0, p1 = s, p2 = str->s;
       i < n1 && i < n2 && i < n;
       ++i, ++p1, ++p2) {
    x = *p1 - *p2;
    if (x != 0) {
      return x;
    }
  }
  if (i == n) {
    return 0;
  }
  return n1 - n2;
}

int GooString::cmp(const char *sA) {
  int n1, i, x;
  const char *p1, *p2;

  n1 = length;
  for (i = 0, p1 = s, p2 = sA; i < n1 && *p2; ++i, ++p1, ++p2) {
    x = *p1 - *p2;
    if (x != 0) {
      return x;
    }
  }
  if (i < n1) {
    return 1;
  }
  if (*p2) {
    return -1;
  }
  return 0;
}

int GooString::cmpN(const char *sA, int n) {
  int n1, i, x;
  const char *p1, *p2;

  n1 = length;
  for (i = 0, p1 = s, p2 = sA; i < n1 && *p2 && i < n; ++i, ++p1, ++p2) {
    x = *p1 - *p2;
    if (x != 0) {
      return x;
    }
  }
  if (i == n) {
    return 0;
  }
  if (i < n1) {
    return 1;
  }
  if (*p2) {
    return -1;
  }
  return 0;
}

GBool GooString::hasUnicodeMarker(void)
{
    return (s[0] & 0xff) == 0xfe && (s[1] & 0xff) == 0xff;
}
