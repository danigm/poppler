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
class GfxFontDict;

//------------------------------------------------------------------------
// AnnotBorderStyle
//------------------------------------------------------------------------

enum AnnotBorderType {
  annotBorderSolid,
  annotBorderDashed,
  annotBorderBeveled,
  annotBorderInset,
  annotBorderUnderlined
};

class AnnotBorderStyle {
public:

  AnnotBorderStyle(AnnotBorderType typeA, double widthA,
		   double *dashA, int dashLengthA,
		   double rA, double gA, double bA);
  ~AnnotBorderStyle();

  AnnotBorderType getType() { return type; }
  double getWidth() { return width; }
  void getDash(double **dashA, int *dashLengthA)
    { *dashA = dash; *dashLengthA = dashLength; }
  void getColor(double *rA, double *gA, double *bA)
    { *rA = r; *gA = g; *bA = b; }

private:

  AnnotBorderType type;
  double width;
  double *dash;
  int dashLength;
  double r, g, b;
};

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

class Annot {
public:

  Annot(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog);
  Annot(XRef *xrefA, Dict *acroForm, Dict *dict, const Ref& aref, Catalog *catalog);
  ~Annot();
  GBool isOk() { return ok; }

  void draw(Gfx *gfx, GBool printing);

  // Get appearance object.
  Object *getAppearance(Object *obj) { return appearance.fetch(xref, obj); }
  GBool textField() { return isTextField; }

  AnnotBorderStyle *getBorderStyle () { return borderStyle; }

  GBool match(Ref *refA)
    { return ref.num == refA->num && ref.gen == refA->gen; }

  void generateFieldAppearance(Dict *field, Dict *annot, Dict *acroForm);

  double getXMin() { return xMin; }
  double getYMin() { return yMin; }

  double getFontSize() { return fontSize; }

private:
//  void writeTextString (GooString* vStr, CharCodeToUnicode* ccToUnicode, GooString* appearBuf, GfxFont* font);
//  void generateAppearance(Dict *acroForm, Dict *dict);
  void setColor(Array *a, GBool fill, int adjust);
  void drawText(GooString *text, GooString *da, GfxFontDict *fontDict,
		GBool multiline, int comb, int quadding,
		GBool txField, GBool forceZapfDingbats);
  void drawListBox(GooString **text, GBool *selection,
		   int nOptions, int topIdx,
		   GooString *da, GfxFontDict *fontDict, GBool quadding);
  void getNextLine(GooString *text, int start,
		   GfxFont *font, double fontSize, double wMax,
		   int *end, double *width, int *next);
  void drawCircle(double cx, double cy, double r, GBool fill);
  void drawCircleTopLeft(double cx, double cy, double r);
  void drawCircleBottomRight(double cx, double cy, double r);
  Object *fieldLookup(Dict *field, char *key, Object *obj);
  void readArrayNum(Object *pdfArray, int key, double *value);
  // write vStr[i:j[ in appearBuf
  void writeTextString (GooString *text, GooString *appearBuf, int *i, int j, CharCodeToUnicode *ccToUnicode); 

  void initialize (XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog);

  XRef *xref;			// the xref table for this PDF file
  Ref ref; // object ref identifying this annotation
  GooString *type; // annotation type
  Object appearance;		// a reference to the Form XObject stream
				//   for the normal appearance
  GooString *appearBuf;
  Guint flags;
  double xMin, yMin,		// annotation rectangle
         xMax, yMax;
  AnnotBorderStyle *borderStyle;
  double fontSize; 
  GBool ok;
  GBool regen, isTextField;
  GBool isMultiline, isListbox;
  
  bool hasRef;
  bool hidden;
};

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

class Annots {
public:

  // Build a list of Annot objects.
  Annots(XRef *xref, Catalog *catalog, Object *annotsObj);

  ~Annots();

  // Iterate through list of annotations.
  int getNumAnnots() { return nAnnots; }
  Annot *getAnnot(int i) { return annots[i]; }

  // (Re)generate the appearance streams for all annotations belonging
  // to a form field.
  void generateAppearances(Dict *acroForm);


private:
  void scanFieldAppearances(Dict *node, Ref *ref, Dict *parent,
			    Dict *acroForm);
  Annot *findAnnot(Ref *ref);

  Annot **annots;
  int nAnnots;
};

#endif
