//========================================================================
//
// Annot.h
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef ANNOT_H
#define ANNOT_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

class XRef;
class Gfx;
class Catalog;
class CharCodeToUnicode;
class GfxFont;
class FormWidget;

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

class Annot {
public:

  Annot(XRef *xrefA, Dict *acroForm, Dict *dict, Ref* aref, Catalog* catalog);
  ~Annot();
  GBool isOk() { return ok; }

  void draw(Gfx *gfx);

  // Get appearance object.
  Object *getAppearance(Object *obj) { return appearance.fetch(xref, obj); }
  GBool textField() { return isTextField; }

  double getXMin() { return xMin; }
  double getYMin() { return yMin; }

  double getFontSize() { return fontSize; }

private:
  void writeTextString (GooString* vStr, CharCodeToUnicode* ccToUnicode, GooString* appearBuf, GfxFont* font);
  void generateAppearance(Dict *acroForm, Dict *dict);
  void readArrayNum(Object *pdfArray, int key, double *value);

  XRef *xref;			// the xref table for this PDF file
  Object appearance;		// a reference to the Form XObject stream
				//   for the normal appearance
  GooString *appearBuf;
  double xMin, yMin,		// annotation rectangle
         xMax, yMax;
  double fontSize; 
  GBool ok;
  GBool regen, isTextField;
  GBool isMultiline, isListbox;
  
  bool hasRef;
  bool hidden;
  Ref ref;
  FormWidget* widget;
};

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

class Annots {
public:

  // Extract non-link annotations from array of annotations.
  Annots(XRef *xref, Catalog *catalog, Object *annotsObj);

  ~Annots();

  // Iterate through list of annotations.
  int getNumAnnots() { return nAnnots; }
  Annot *getAnnot(int i) { return annots[i]; }

private:

  Annot **annots;
  int nAnnots;
};

#endif
