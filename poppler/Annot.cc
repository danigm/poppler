//========================================================================
//
// Annot.cc
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <math.h>
#include "goo/gmem.h"
#include "GooList.h"
#include "Error.h"
#include "Object.h"
#include "Catalog.h"
#include "Gfx.h"
#include "Lexer.h"
#include "Annot.h"
#include "GfxFont.h"
#include "CharCodeToUnicode.h"
#include "Form.h"
#include "Error.h"
#include "Page.h"
#include "XRef.h"

#define annotFlagHidden    0x0002
#define annotFlagPrint     0x0004
#define annotFlagNoView    0x0020

#define fieldFlagReadOnly           0x00000001
#define fieldFlagRequired           0x00000002
#define fieldFlagNoExport           0x00000004
#define fieldFlagMultiline          0x00001000
#define fieldFlagPassword           0x00002000
#define fieldFlagNoToggleToOff      0x00004000
#define fieldFlagRadio              0x00008000
#define fieldFlagPushbutton         0x00010000
#define fieldFlagCombo              0x00020000
#define fieldFlagEdit               0x00040000
#define fieldFlagSort               0x00080000
#define fieldFlagFileSelect         0x00100000
#define fieldFlagMultiSelect        0x00200000
#define fieldFlagDoNotSpellCheck    0x00400000
#define fieldFlagDoNotScroll        0x00800000
#define fieldFlagComb               0x01000000
#define fieldFlagRichText           0x02000000
#define fieldFlagRadiosInUnison     0x02000000
#define fieldFlagCommitOnSelChange  0x04000000

#define fieldQuadLeft   0
#define fieldQuadCenter 1
#define fieldQuadRight  2

// distance of Bezier control point from center for circle approximation
// = (4 * (sqrt(2) - 1) / 3) * r
#define bezierCircle 0.55228475

AnnotLineEndingStyle parseAnnotLineEndingStyle(GooString *string) {
  if (string != NULL) {
    if (string->cmp("Square")) {
      return annotLineEndingSquare;
    } else if (string->cmp("Circle")) {
      return annotLineEndingCircle;
    } else if (string->cmp("Diamond")) {
      return annotLineEndingDiamond;
    } else if (string->cmp("OpenArrow")) {
      return annotLineEndingOpenArrow;
    } else if (string->cmp("ClosedArrow")) {
      return annotLineEndingClosedArrow;
    } else if (string->cmp("Butt")) {
      return annotLineEndingButt;
    } else if (string->cmp("ROpenArrow")) {
      return annotLineEndingROpenArrow;
    } else if (string->cmp("RClosedArrow")) {
      return annotLineEndingRClosedArrow;
    } else if (string->cmp("Slash")) {
      return annotLineEndingSlash;
    } else {
      return annotLineEndingNone;
    }
  } else {
    return annotLineEndingNone;
  }  
}

AnnotExternalDataType parseAnnotExternalData(Dict* dict) {
  Object obj1;
  AnnotExternalDataType type;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString *typeName = new GooString(obj1.getName());

    if (!typeName->cmp("Markup3D")) {
      type = annotExternalDataMarkup3D;
    } else {
      type = annotExternalDataMarkupUnknown;
    }
    delete typeName;
  } else {
    type = annotExternalDataMarkupUnknown;
  }
  obj1.free();

  return type;
}

//------------------------------------------------------------------------
// AnnotBorderEffect
//------------------------------------------------------------------------

AnnotBorderEffect::AnnotBorderEffect(Dict *dict) {
  Object obj1;

  if (dict->lookup("S", &obj1)->isName()) {
    GooString *effectName = new GooString(obj1.getName());

    if (!effectName->cmp("C"))
      effectType = borderEffectCloudy;
    else
      effectType = borderEffectNoEffect;
    delete effectName;
  } else {
    effectType = borderEffectNoEffect;
  }
  obj1.free();

  if ((dict->lookup("I", &obj1)->isNum()) && effectType == borderEffectCloudy) {
    intensity = obj1.getNum();
  } else {
    intensity = 0;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotCalloutLine
//------------------------------------------------------------------------

AnnotCalloutLine::AnnotCalloutLine(double x1, double y1, double x2, double y2) {
  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
}

//------------------------------------------------------------------------
// AnnotCalloutMultiLine
//------------------------------------------------------------------------

AnnotCalloutMultiLine::AnnotCalloutMultiLine(double x1, double y1, double x2,
    double y2, double x3, double y3) : AnnotCalloutLine(x1, y1, x2, y2) {
  this->x3 = x3;
  this->y3 = y3;
}

//------------------------------------------------------------------------
// AnnotQuadrilateral
//------------------------------------------------------------------------

AnnotQuadrilateral::AnnotQuadrilateral(double x1, double y1,
        double x2, double y2, double x3, double y3, double x4, double y4) {
  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
  this->x3 = x3;
  this->y3 = y3;
  this->x4 = x4;
  this->y4 = y4;
}

//------------------------------------------------------------------------
// AnnotQuadPoints
//------------------------------------------------------------------------

AnnotQuadPoints::AnnotQuadPoints(double x1, double y1, double x2, double y2,
    double x3, double y3, double x4, double y4) {
  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
  this->x3 = x3;
  this->y3 = y3;
  this->x4 = x4;
  this->y4 = y4;
}

//------------------------------------------------------------------------
// AnnotBorder
//------------------------------------------------------------------------
AnnotBorder::AnnotBorder() {
  width = 1;
  dashLength = 0;
  dash = NULL;
  style = borderSolid;
}

AnnotBorder::~AnnotBorder() {
  if (dash)
    gfree (dash); 
}
  
//------------------------------------------------------------------------
// AnnotBorderArray
//------------------------------------------------------------------------

AnnotBorderArray::AnnotBorderArray() {
  horizontalCorner = 0;
  verticalCorner = 0;
}

AnnotBorderArray::AnnotBorderArray(Array *array) {
  Object obj1;
  int arrayLength = array->getLength();

  if (arrayLength >= 3) {
    // implementation note 81 in Appendix H.

    if (array->get(0, &obj1)->isNum())
      horizontalCorner = obj1.getNum();
    obj1.free();

    if (array->get(1, &obj1)->isNum())
      verticalCorner = obj1.getNum();
    obj1.free();

    if (array->get(2, &obj1)->isNum())
      width = obj1.getNum();
    obj1.free();

    // TODO: check not all zero ? (Line Dash Pattern Page 217 PDF 8.1)
    if (arrayLength > 3) {
      GBool correct = gTrue;
      int tempLength = array->getLength() - 3;
      double *tempDash = (double *) gmallocn (tempLength, sizeof (double));

      for(int i = 0; i < tempLength && i < DASH_LIMIT && correct; i++) {

        if (array->get((i + 3), &obj1)->isNum()) {
          tempDash[i] = obj1.getNum();

          if (tempDash[i] < 0)
            correct = gFalse;

        } else {
          correct = gFalse;
        }
        obj1.free();
      }

      if (correct) {
        dashLength = tempLength;
        dash = tempDash;
        style = borderDashed;
      } else {
        gfree (tempDash);
      }
    }
  }
}

//------------------------------------------------------------------------
// AnnotBorderBS
//------------------------------------------------------------------------

AnnotBorderBS::AnnotBorderBS() {
}

AnnotBorderBS::AnnotBorderBS(Dict *dict) {
  Object obj1, obj2;

  // acroread 8 seems to need both W and S entries for
  // any border to be drawn, even though the spec
  // doesn't claim anything of that sort. We follow
  // that behaviour by veryifying both entries exist
  // otherwise we set the borderWidth to 0
  // --jrmuizel
  dict->lookup("W", &obj1);
  dict->lookup("S", &obj2);
  if (obj1.isNum() && obj2.isName()) {
    GooString *styleName = new GooString(obj2.getName());

    width = obj1.getNum();

    if (!styleName->cmp("S")) {
      style = borderSolid;
    } else if (!styleName->cmp("D")) {
      style = borderDashed;
    } else if (!styleName->cmp("B")) {
      style = borderBeveled;
    } else if (!styleName->cmp("I")) {
      style = borderInset;
    } else if (!styleName->cmp("U")) {
      style = borderUnderlined;
    } else {
      style = borderSolid;
    }
    delete styleName;
  } else {
    width = 0;
  }
  obj2.free();
  obj1.free();

  // TODO: check not all zero (Line Dash Pattern Page 217 PDF 8.1)
  if (dict->lookup("D", &obj1)->isArray()) {
    GBool correct = gTrue;
    int tempLength = obj1.arrayGetLength();
    double *tempDash = (double *) gmallocn (tempLength, sizeof (double));

    for(int i = 0; i < tempLength && correct; i++) {
      Object obj2;

      if (obj1.arrayGet(i, &obj2)->isNum()) {
        tempDash[i] = obj2.getNum();

        if (tempDash[i] < 0)
          correct = gFalse;
      } else {
        correct = gFalse;
      }
      obj2.free();
    }

    if (correct) {
      dashLength = tempLength;
      dash = tempDash;
      style = borderDashed;
    } else {
      gfree (tempDash);
    }

  }

  if (!dash) {
    dashLength = 1;
    dash = (double *) gmallocn (dashLength, sizeof (double));
    dash[0] = 3;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotColor
//------------------------------------------------------------------------

AnnotColor::AnnotColor() {
  length = 0;
  values = NULL;
}

AnnotColor::AnnotColor(Array *array) {
  // TODO: check what Acrobat does in the case of having more than 5 numbers.
  if (array->getLength() < 5) {
    length = array->getLength();
    values = (double *) gmallocn (length, sizeof(double));

    for(int i = 0; i < length; i++) {  
      Object obj1;

      if (array->get(i, &obj1)->isNum()) {
        values[i] = obj1.getNum();

        if (values[i] < 0 || values[i] > 1)
          values[i] = 0;
      } else {
        values[i] = 0;
      }
      obj1.free();
    }
  }
}

AnnotColor::AnnotColorSpace AnnotColor::getSpace() const {
  return (AnnotColor::AnnotColorSpace) length;
}

double AnnotColor::getValue(int i) const {
  if (i >= 0 && i < length) 
    return values[i];
  return 0;
}

AnnotColor::~AnnotColor() {
  if (values)
    gfree (values);
}

//------------------------------------------------------------------------
// AnnotBorderStyle
//------------------------------------------------------------------------

AnnotBorderStyle::AnnotBorderStyle(AnnotBorderType typeA, double widthA,
				   double *dashA, int dashLengthA,
				   double rA, double gA, double bA) {
  type = typeA;
  width = widthA;
  dash = dashA;
  dashLength = dashLengthA;
  r = rA;
  g = gA;
  b = bA;
}

AnnotBorderStyle::~AnnotBorderStyle() {
  if (dash) {
    gfree(dash);
  }
}

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

Annot::Annot(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog* catalog) {
  hasRef = false;
  flags = flagUnknown;
  type = typeUnknown;
  initialize (xrefA, acroForm, dict, catalog);
}

Annot::Annot(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog* catalog, Object *obj) {
  if (obj->isRef()) {
    hasRef = gTrue;
    ref = obj->getRef();
  } else {
    hasRef = gFalse;
  }
  flags = flagUnknown;
  type = typeUnknown;
  initialize (xrefA, acroForm, dict, catalog);
}

void Annot::initialize(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog) {
  Object apObj, asObj, obj1, obj2, obj3;

  appRef.num = 0;
  appRef.gen = 65535;
  ok = gTrue;
  xref = xrefA;
  appearBuf = NULL;
  fontSize = 0;
  widget = NULL;

  //----- get the FormWidget
  if (hasRef) {
    Form *form = catalog->getForm ();
    
    if (form)
      widget = form->findWidgetByRef (ref);
  }

  //----- parse the rectangle
  rect = new PDFRectangle();
  if (dict->lookup("Rect", &obj1)->isArray() && obj1.arrayGetLength() == 4) {
    Object obj2;
    (obj1.arrayGet(0, &obj2)->isNum() ? rect->x1 = obj2.getNum() : rect->x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? rect->y1 = obj2.getNum() : rect->y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? rect->x2 = obj2.getNum() : rect->x2 = 1);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? rect->y2 = obj2.getNum() : rect->y2 = 1);
    obj2.free();

    if (rect->x1 > rect->x2) {
      double t = rect->x1;
      rect->x1 = rect->x2;
      rect->x2 = t;
    }

    if (rect->y1 > rect->y2) {
      double t = rect->y1;
      rect->y1 = rect->y2;
      rect->y2 = t;
    }
  } else {
    rect->x1 = rect->y1 = 0;
    rect->x2 = rect->y2 = 1;
    error(-1, "Bad bounding box for annotation");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Contents", &obj1)->isString()) {
    contents = obj1.getString()->copy();
  } else {
    contents = NULL;
  }
  obj1.free();

  /* TODO: Page Object indirect reference (should be parsed ?) */
  pageDict = NULL;
  /*if (dict->lookup("P", &obj1)->isDict()) {
    pageDict = NULL;
  } else {
    pageDict = NULL;
  }
  obj1.free();
  */

  if (dict->lookup("NM", &obj1)->isString()) {
    name = obj1.getString()->copy();
  } else {
    name = NULL;
  }
  obj1.free();

  if (dict->lookup("M", &obj1)->isString()) {
    modified = obj1.getString()->copy();
  } else {
    modified = NULL;
  }
  obj1.free();

  //----- get the flags
  if (dict->lookup("F", &obj1)->isInt()) {
    flags |= obj1.getInt();
  } else {
    flags = flagUnknown;
  }
  obj1.free();

  // check if field apperances need to be regenerated
  // Only text or choice fields needs to have appearance regenerated
  // see section 8.6.2 "Variable Text" of PDFReference
  regen = gFalse;
  Form::fieldLookup(dict, "FT", &obj3);
  if (obj3.isName("Tx") || obj3.isName("Ch")) {
    if (acroForm) {
      acroForm->lookup("NeedAppearances", &obj1);
      if (obj1.isBool() && obj1.getBool()) {
        regen = gTrue;
      }
      obj1.free();
    }
  }
  obj3.free();

  if (dict->lookup("AP", &obj1)->isDict()) {
    Object obj2;

    if (dict->lookup("AS", &obj2)->isName()) {
      Object obj3;

      appearState = new GooString(obj2.getName());
      if (obj1.dictLookup("N", &obj3)->isDict()) {
        Object obj4;

        if (obj3.dictLookupNF(appearState->getCString(), &obj4)->isRef()) {
          obj4.copy(&appearance);
        } else {
          obj4.free();
          if (obj3.dictLookupNF("Off", &obj4)->isRef()) {
            obj4.copy(&appearance);
          } else
            regen = gTrue;
        } 
        obj4.free();
      }
      obj3.free();
    } else {
      obj2.free();

      appearState = NULL;
      if (obj1.dictLookupNF("N", &obj2)->isRef()) {
        obj2.copy(&appearance);
      } else
        regen = gTrue;
    }
    obj2.free();
  } else {
    appearState = NULL;
    // If field doesn't have an AP we'll have to generate it
    regen = gTrue;
  }
  obj1.free();

  //----- parse the border style
  if (dict->lookup("BS", &obj1)->isDict()) {
    border = new AnnotBorderBS(obj1.getDict());
  } else {
    obj1.free();

    if (dict->lookup("Border", &obj1)->isArray())
      border = new AnnotBorderArray(obj1.getArray());
    else
      // Adobe draws no border at all if the last element is of
      // the wrong type.
      border = NULL;
  }
  obj1.free();

  if (dict->lookup("C", &obj1)->isArray()) {
    color = new AnnotColor(obj1.getArray());
  } else {
    color = NULL;
  }
  obj1.free();

  if (dict->lookup("StructParent", &obj1)->isInt()) {
    treeKey = obj1.getInt();
  } else {
    treeKey = 0;
  }
  obj1.free();

  /* TODO: optional content should be parsed */
  optionalContent = NULL;
  
  /*if (dict->lookup("OC", &obj1)->isDict()) {
    optionalContent = NULL;
  } else {
    optionalContent = NULL;
  }
  obj1.free();
  */
}

double Annot::getXMin() {
  return rect->x1;
}

double Annot::getYMin() {
  return rect->y1;
}

void Annot::readArrayNum(Object *pdfArray, int key, double *value) {
  Object valueObject;

  pdfArray->arrayGet(key, &valueObject);
  if (valueObject.isNum()) {
    *value = valueObject.getNum();
  } else {
    *value = 0;
    ok = gFalse;
  }
  valueObject.free();
}

Annot::~Annot() {
  delete rect;

  if (contents)
    delete contents;

  if (pageDict)
    delete pageDict;

  if (name)
    delete name;

  if (modified)
    delete modified;

  appearance.free();

  if (appearState)
    delete appearState;

  if (border)
    delete border;

  if (color)
    delete color;

  if (optionalContent)
    delete optionalContent;
}

void Annot::generateFieldAppearance(Dict *field, Dict *annot, Dict *acroForm) {
  Object mkObj, ftObj, appearDict, drObj, obj1, obj2, obj3;
  Dict *mkDict;
  MemStream *appearStream;
  GfxFontDict *fontDict;
  GBool hasCaption;
  double w, dx, dy, r;
  double *dash;
  GooString *caption, *da;
  GooString **text;
  GBool *selection;
  int dashLength, ff, quadding, comb, nOptions, topIdx, i, j;
  GBool modified;

  // must be a Widget annotation
  if (type == typeWidget) {
    return;
  }

  // do not regenerate appearence if widget has not changed
  if (widget && widget->isModified ()) {
    modified = gTrue;
  } else {
    modified = gFalse;
  }

  // only regenerate when it doesn't have an AP or
  // it already has an AP but widget has been modified
  if (!regen && !modified) {
    return;
  }

  appearBuf = new GooString ();
  // get the appearance characteristics (MK) dictionary
  if (annot->lookup("MK", &mkObj)->isDict()) {
    mkDict = mkObj.getDict();
  } else {
    mkDict = NULL;
  }
  // draw the background
  if (mkDict) {
    if (mkDict->lookup("BG", &obj1)->isArray() &&
        obj1.arrayGetLength() > 0) {
      setColor(obj1.getArray(), gTrue, 0);
      appearBuf->appendf("0 0 {0:.2f} {1:.2f} re f\n",
          rect->x2 - rect->x1, rect->y2 - rect->y1);
    }
    obj1.free();
  }

  // get the field type
  Form::fieldLookup(field, "FT", &ftObj);

  // get the field flags (Ff) value
  if (Form::fieldLookup(field, "Ff", &obj1)->isInt()) {
    ff = obj1.getInt();
  } else {
    ff = 0;
  }
  obj1.free();

  // draw the border
  if (mkDict && border) {
    w = border->getWidth();
    if (w > 0) {
      mkDict->lookup("BC", &obj1);
      if (!(obj1.isArray() && obj1.arrayGetLength() > 0)) {
        mkDict->lookup("BG", &obj1);
      }
      if (obj1.isArray() && obj1.arrayGetLength() > 0) {
        dx = rect->x2 - rect->x1;
        dy = rect->y2 - rect->y1;

        // radio buttons with no caption have a round border
        hasCaption = mkDict->lookup("CA", &obj2)->isString();
        obj2.free();
        if (ftObj.isName("Btn") && (ff & fieldFlagRadio) && !hasCaption) {
          r = 0.5 * (dx < dy ? dx : dy);
          switch (border->getStyle()) {
            case AnnotBorder::borderDashed:
              appearBuf->append("[");
              dashLength = border->getDashLength();
              dash = border->getDash();
              for (i = 0; i < dashLength; ++i) {
                appearBuf->appendf(" {0:.2f}", dash[i]);
              }
              appearBuf->append("] 0 d\n");
              // fall through to the solid case
            case AnnotBorder::borderSolid:
            case AnnotBorder::borderUnderlined:
              appearBuf->appendf("{0:.2f} w\n", w);
              setColor(obj1.getArray(), gFalse, 0);
              drawCircle(0.5 * dx, 0.5 * dy, r - 0.5 * w, gFalse);
              break;
            case AnnotBorder::borderBeveled:
            case AnnotBorder::borderInset:
              appearBuf->appendf("{0:.2f} w\n", 0.5 * w);
              setColor(obj1.getArray(), gFalse, 0);
              drawCircle(0.5 * dx, 0.5 * dy, r - 0.25 * w, gFalse);
              setColor(obj1.getArray(), gFalse,
                  border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
              drawCircleTopLeft(0.5 * dx, 0.5 * dy, r - 0.75 * w);
              setColor(obj1.getArray(), gFalse,
                  border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
              drawCircleBottomRight(0.5 * dx, 0.5 * dy, r - 0.75 * w);
              break;
          }

        } else {
          switch (border->getStyle()) {
            case AnnotBorder::borderDashed:
              appearBuf->append("[");
              dashLength = border->getDashLength();
              dash = border->getDash();
              for (i = 0; i < dashLength; ++i) {
                appearBuf->appendf(" {0:.2f}", dash[i]);
              }
              appearBuf->append("] 0 d\n");
              // fall through to the solid case
            case AnnotBorder::borderSolid:
              appearBuf->appendf("{0:.2f} w\n", w);
              setColor(obj1.getArray(), gFalse, 0);
              appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re s\n",
                  0.5 * w, dx - w, dy - w);
              break;
            case AnnotBorder::borderBeveled:
            case AnnotBorder::borderInset:
              setColor(obj1.getArray(), gTrue,
                  border->getStyle() == AnnotBorder::borderBeveled ? 1 : -1);
              appearBuf->append("0 0 m\n");
              appearBuf->appendf("0 {0:.2f} l\n", dy);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", w, dy - w);
              appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
              appearBuf->append("f\n");
              setColor(obj1.getArray(), gTrue,
                  border->getStyle() == AnnotBorder::borderBeveled ? -1 : 1);
              appearBuf->append("0 0 m\n");
              appearBuf->appendf("{0:.2f} 0 l\n", dx);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
              appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, w);
              appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
              appearBuf->append("f\n");
              break;
            case AnnotBorder::borderUnderlined:
              appearBuf->appendf("{0:.2f} w\n", w);
              setColor(obj1.getArray(), gFalse, 0);
              appearBuf->appendf("0 0 m {0:.2f} 0 l s\n", dx);
              break;
          }

          // clip to the inside of the border
          appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re W n\n",
              w, dx - 2 * w, dy - 2 * w);
        }
      }
      obj1.free();
    }
  }

  // get the resource dictionary
  acroForm->lookup("DR", &drObj);

  // build the font dictionary
  if (drObj.isDict() && drObj.dictLookup("Font", &obj1)->isDict()) {
    fontDict = new GfxFontDict(xref, NULL, obj1.getDict());
  } else {
    fontDict = NULL;
  }
  obj1.free();

  // get the default appearance string
  if (Form::fieldLookup(field, "DA", &obj1)->isNull()) {
    obj1.free();
    acroForm->lookup("DA", &obj1);
  }
  if (obj1.isString()) {
    da = obj1.getString()->copy();
    //TODO: look for a font size / name HERE
    // => create a function
  } else {
    da = NULL;
  }
  obj1.free();

  // draw the field contents
  if (ftObj.isName("Btn")) {
    caption = NULL;
    if (mkDict) {
      if (mkDict->lookup("CA", &obj1)->isString()) {
        caption = obj1.getString()->copy();
      }
      obj1.free();
    }
    // radio button
    if (ff & fieldFlagRadio) {
      //~ Acrobat doesn't draw a caption if there is no AP dict (?)
      if (Form::fieldLookup(field, "V", &obj1)->isName()) {
        if (annot->lookup("AS", &obj2)->isName(obj1.getName()) &&
	    strcmp (obj1.getName(), "Off") != 0) {
          if (caption) {
            drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
                gFalse, gTrue);
          } else {
            if (mkDict) {
              if (mkDict->lookup("BC", &obj3)->isArray() &&
                  obj3.arrayGetLength() > 0) {
                dx = rect->x2 - rect->x1;
                dy = rect->y2 - rect->y1;
                setColor(obj3.getArray(), gTrue, 0);
                drawCircle(0.5 * dx, 0.5 * dy, 0.2 * (dx < dy ? dx : dy),
                    gTrue);
              }
              obj3.free();
            }
          }
        }
        obj2.free();
      }
      obj1.free();
      // pushbutton
    } else if (ff & fieldFlagPushbutton) {
      if (caption) {
        drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
            gFalse, gFalse);
      }
      // checkbox
    } else {
      // According to the PDF spec the off state must be named "Off",
      // and the on state can be named anything, but Acrobat apparently
      // looks for "Yes" and treats anything else as off.
      if (Form::fieldLookup(field, "V", &obj1)->isName("Yes")) {
        if (!caption) {
          caption = new GooString("3"); // ZapfDingbats checkmark
        }
        drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
            gFalse, gTrue);
      }
      obj1.free();
    }
    if (caption) {
      delete caption;
    }
  } else if (ftObj.isName("Tx")) {
    //~ value strings can be Unicode
    if (Form::fieldLookup(field, "V", &obj1)->isString()) {
      if (Form::fieldLookup(field, "Q", &obj2)->isInt()) {
        quadding = obj2.getInt();
      } else {
        quadding = fieldQuadLeft;
      }
      obj2.free();
      comb = 0;
      if (ff & fieldFlagComb) {
        if (Form::fieldLookup(field, "MaxLen", &obj2)->isInt()) {
          comb = obj2.getInt();
        }
        obj2.free();
      }
      drawText(obj1.getString(), da, fontDict,
          ff & fieldFlagMultiline, comb, quadding, gTrue, gFalse, ff & fieldFlagPassword);
    }
    obj1.free();
  } else if (ftObj.isName("Ch")) {
    //~ value/option strings can be Unicode
    if (Form::fieldLookup(field, "Q", &obj1)->isInt()) {
      quadding = obj1.getInt();
    } else {
      quadding = fieldQuadLeft;
    }
    obj1.free();
    // combo box
    if (ff & fieldFlagCombo) {
      if (Form::fieldLookup(field, "V", &obj1)->isString()) {
        drawText(obj1.getString(), da, fontDict,
            gFalse, 0, quadding, gTrue, gFalse);
        //~ Acrobat draws a popup icon on the right side
      }
      obj1.free();
      // list box
    } else {
      if (field->lookup("Opt", &obj1)->isArray()) {
        nOptions = obj1.arrayGetLength();
        // get the option text
        text = (GooString **)gmallocn(nOptions, sizeof(GooString *));
        for (i = 0; i < nOptions; ++i) {
          text[i] = NULL;
          obj1.arrayGet(i, &obj2);
          if (obj2.isString()) {
            text[i] = obj2.getString()->copy();
          } else if (obj2.isArray() && obj2.arrayGetLength() == 2) {
            if (obj2.arrayGet(1, &obj3)->isString()) {
              text[i] = obj3.getString()->copy();
            }
            obj3.free();
          }
          obj2.free();
          if (!text[i]) {
            text[i] = new GooString();
          }
        }
        // get the selected option(s)
        selection = (GBool *)gmallocn(nOptions, sizeof(GBool));
        //~ need to use the I field in addition to the V field
	Form::fieldLookup(field, "V", &obj2);
        for (i = 0; i < nOptions; ++i) {
          selection[i] = gFalse;
          if (obj2.isString()) {
            if (!obj2.getString()->cmp(text[i])) {
              selection[i] = gTrue;
            }
          } else if (obj2.isArray()) {
            for (j = 0; j < obj2.arrayGetLength(); ++j) {
              if (obj2.arrayGet(j, &obj3)->isString() &&
                  !obj3.getString()->cmp(text[i])) {
                selection[i] = gTrue;
              }
              obj3.free();
            }
          }
        }
        obj2.free();
        // get the top index
        if (field->lookup("TI", &obj2)->isInt()) {
          topIdx = obj2.getInt();
        } else {
          topIdx = 0;
        }
        obj2.free();
        // draw the text
        drawListBox(text, selection, nOptions, topIdx, da, fontDict, quadding);
        for (i = 0; i < nOptions; ++i) {
          delete text[i];
        }
        gfree(text);
        gfree(selection);
      }
      obj1.free();
    }
  } else if (ftObj.isName("Sig")) {
    //~unimp
  } else {
    error(-1, "Unknown field type");
  }

  if (da) {
    delete da;
  }

  // build the appearance stream dictionary
  appearDict.initDict(xref);
  appearDict.dictAdd(copyString("Length"),
      obj1.initInt(appearBuf->getLength()));
  appearDict.dictAdd(copyString("Subtype"), obj1.initName("Form"));
  obj1.initArray(xref);
  obj1.arrayAdd(obj2.initReal(0));
  obj1.arrayAdd(obj2.initReal(0));
  obj1.arrayAdd(obj2.initReal(rect->x2 - rect->x1));
  obj1.arrayAdd(obj2.initReal(rect->y2 - rect->y1));
  appearDict.dictAdd(copyString("BBox"), &obj1);

  // set the resource dictionary
  if (drObj.isDict()) {
    appearDict.dictAdd(copyString("Resources"), drObj.copy(&obj1));
  }
  drObj.free();

  // build the appearance stream
  appearStream = new MemStream(strdup(appearBuf->getCString()), 0,
      appearBuf->getLength(), &appearDict);
  appearance.free();
  appearance.initStream(appearStream);
  delete appearBuf;

  appearStream->setNeedFree(gTrue);

  if (widget->isModified()) {
    //create a new object that will contains the new appearance
    
    //if we already have a N entry in our AP dict, reuse it
    if (annot->lookup("AP", &obj1)->isDict() &&
        obj1.dictLookupNF("N", &obj2)->isRef()) {
      appRef = obj2.getRef();
    }

    // this annot doesn't have an AP yet, create one
    if (appRef.num == 0)
      appRef = xref->addIndirectObject(&appearance);
    else // since we reuse the already existing AP, we have to notify the xref about this update
      xref->setModifiedObject(&appearance, appRef);

    // update object's AP and AS
    Object apObj;
    apObj.initDict(xref);

    Object oaRef;
    oaRef.initRef(appRef.num, appRef.gen);

    apObj.dictSet("N", &oaRef);
    annot->set("AP", &apObj);
    Dict* d = new Dict(annot);
    Object dictObj;
    dictObj.initDict(d);

    xref->setModifiedObject(&dictObj, ref);
  }

  if (fontDict) {
    delete fontDict;
  }
  ftObj.free();
  mkObj.free();
}


// Set the current fill or stroke color, based on <a> (which should
// have 1, 3, or 4 elements).  If <adjust> is +1, color is brightened;
// if <adjust> is -1, color is darkened; otherwise color is not
// modified.
void Annot::setColor(Array *a, GBool fill, int adjust) {
  Object obj1;
  double color[4];
  int nComps, i;

  nComps = a->getLength();
  if (nComps > 4) {
    nComps = 4;
  }
  for (i = 0; i < nComps && i < 4; ++i) {
    if (a->get(i, &obj1)->isNum()) {
      color[i] = obj1.getNum();
    } else {
      color[i] = 0;
    }
    obj1.free();
  }
  if (nComps == 4) {
    adjust = -adjust;
  }
  if (adjust > 0) {
    for (i = 0; i < nComps; ++i) {
      color[i] = 0.5 * color[i] + 0.5;
    }
  } else if (adjust < 0) {
    for (i = 0; i < nComps; ++i) {
      color[i] = 0.5 * color[i];
    }
  }
  if (nComps == 4) {
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:c}\n",
        color[0], color[1], color[2], color[3],
        fill ? 'k' : 'K');
  } else if (nComps == 3) {
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:s}\n",
        color[0], color[1], color[2],
        fill ? "rg" : "RG");
  } else {
    appearBuf->appendf("{0:.2f} {1:c}\n",
        color[0],
        fill ? 'g' : 'G');
  }
}

void Annot::writeTextString (GooString *text, GooString *appearBuf, int *i, int j,
                             CharCodeToUnicode *ccToUnicode, GBool password)
{
  CharCode c;
  int charSize;

  if (*i == 0 && text->hasUnicodeMarker()) {
    //we need to have an even number of chars
    if (text->getLength () % 2 != 0) {
      error(-1, "Annot::writeTextString, bad unicode string");
      return;
    }
    //skip unicode marker an go one char forward because we read character by pairs
    (*i) += 3;
    charSize = 2;
  } else
    charSize = 1;

  for (; (*i) < j; (*i)+=charSize) {
    // Render '*' instead of characters for password
    if (password)
      appearBuf->append('*');
    else {
      c = text->getChar(*i);
      if (ccToUnicode && text->hasUnicodeMarker()) {
        char ctmp[2];
        ctmp[0] = text->getChar((*i)-1);
        ctmp[1] = text->getChar((*i));
        ccToUnicode->mapToCharCode((Unicode*)ctmp, &c, 2);
        if (c == '(' || c == ')' || c == '\\')
          appearBuf->append('\\');
        appearBuf->append(c);
      } else {
        c &= 0xff;
        if (c == '(' || c == ')' || c == '\\') {
          appearBuf->append('\\');
          appearBuf->append(c);
        } else if (c < 0x20 || c >= 0x80) {
          appearBuf->appendf("\\{0:03o}", c);
        } else {
          appearBuf->append(c);
        }
      }
    }
  }
}

// Draw the variable text or caption for a field.
void Annot::drawText(GooString *text, GooString *da, GfxFontDict *fontDict,
    GBool multiline, int comb, int quadding,
    GBool txField, GBool forceZapfDingbats,
    GBool password) {
  GooList *daToks;
  GooString *tok;
  GfxFont *font;
  double fontSize, fontSize2, borderWidth, x, xPrev, y, w, w2, wMax;
  int tfPos, tmPos, i, j, k;

  //~ if there is no MK entry, this should use the existing content stream,
  //~ and only replace the marked content portion of it
  //~ (this is only relevant for Tx fields)
  
  // parse the default appearance string
  tfPos = tmPos = -1;
  if (da) {
    daToks = new GooList();
    i = 0;
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
        ++i;
      }
      if (i < da->getLength()) {
        for (j = i + 1;
            j < da->getLength() && !Lexer::isSpace(da->getChar(j));
            ++j) ;
        daToks->append(new GooString(da, i, j - i));
        i = j;
      }
    }
    for (i = 2; i < daToks->getLength(); ++i) {
      if (i >= 2 && !((GooString *)daToks->get(i))->cmp("Tf")) {
        tfPos = i - 2;
      } else if (i >= 6 && !((GooString *)daToks->get(i))->cmp("Tm")) {
        tmPos = i - 6;
      }
    }
  } else {
    daToks = NULL;
  }

  // force ZapfDingbats
  //~ this should create the font if needed (?)
  if (forceZapfDingbats) {
    if (tfPos >= 0) {
      tok = (GooString *)daToks->get(tfPos);
      if (tok->cmp("/ZaDb")) {
        tok->clear();
        tok->append("/ZaDb");
      }
    }
  }
  // get the font and font size
  font = NULL;
  fontSize = 0;
  if (tfPos >= 0) {
    tok = (GooString *)daToks->get(tfPos);
    if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
      if (!fontDict || !(font = fontDict->lookup(tok->getCString() + 1))) {
        error(-1, "Unknown font in field's DA string");
      }
    } else {
      error(-1, "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GooString *)daToks->get(tfPos + 1);
    fontSize = atof(tok->getCString());
  } else {
    error(-1, "Missing 'Tf' operator in field's DA string");
  }
  if (!font) {
    return;
  }

  // get the border width
  borderWidth = border ? border->getWidth() : 0;

  // setup
  if (txField) {
    appearBuf->append("/Tx BMC\n");
  }
  appearBuf->append("q\n");
  appearBuf->append("BT\n");
  // multi-line text
  if (multiline) {
    // note: the comb flag is ignored in multiline mode

    wMax = rect->x2 - rect->x1 - 2 * borderWidth - 4;

    // compute font autosize
    if (fontSize == 0) {
      for (fontSize = 20; fontSize > 1; --fontSize) {
        y = rect->y2 - rect->y1;
        w2 = 0;
        i = 0;
        while (i < text->getLength()) {
          getNextLine(text, i, font, fontSize, wMax, &j, &w, &k);
          if (w > w2) {
            w2 = w;
          }
          i = k;
          y -= fontSize;
        }
        // approximate the descender for the last line
        if (y >= 0.33 * fontSize) {
          break;
        }
      }
      if (tfPos >= 0) {
        tok = (GooString *)daToks->get(tfPos + 1);
        tok->clear();
        tok->appendf("{0:.2f}", fontSize);
      }
    }

    // starting y coordinate
    // (note: each line of text starts with a Td operator that moves
    // down a line)
    y = rect->y2 - rect->y1;

    // set the font matrix
    if (tmPos >= 0) {
      tok = (GooString *)daToks->get(tmPos + 4);
      tok->clear();
      tok->append('0');
      tok = (GooString *)daToks->get(tmPos + 5);
      tok->clear();
      tok->appendf("{0:.2f}", y);
    }

    // write the DA string
    if (daToks) {
      for (i = 0; i < daToks->getLength(); ++i) {
        appearBuf->append((GooString *)daToks->get(i))->append(' ');
      }
    }

    // write the font matrix (if not part of the DA string)
    if (tmPos < 0) {
      appearBuf->appendf("1 0 0 1 0 {0:.2f} Tm\n", y);
    }

    // write a series of lines of text
    i = 0;
    xPrev = 0;
    while (i < text->getLength()) {

      getNextLine(text, i, font, fontSize, wMax, &j, &w, &k);

      // compute text start position
      switch (quadding) {
        case fieldQuadLeft:
        default:
          x = borderWidth + 2;
          break;
        case fieldQuadCenter:
          x = (rect->x2 - rect->x1 - w) / 2;
          break;
        case fieldQuadRight:
          x = rect->x2 - rect->x1 - borderWidth - 2 - w;
          break;
      }

      // draw the line
      appearBuf->appendf("{0:.2f} {1:.2f} Td\n", x - xPrev, -fontSize);
      appearBuf->append('(');
      writeTextString (text, appearBuf, &i, j, font->getToUnicode(), password);
      appearBuf->append(") Tj\n");

      // next line
      i = k;
      xPrev = x;
    }

    // single-line text
  } else {
    //~ replace newlines with spaces? - what does Acrobat do?

    // comb formatting
    if (comb > 0) {
      // compute comb spacing
      w = (rect->x2 - rect->x1 - 2 * borderWidth) / comb;

      // compute font autosize
      if (fontSize == 0) {
        fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
        if (w < fontSize) {
          fontSize = w;
        }
        fontSize = floor(fontSize);
        if (tfPos >= 0) {
          tok = (GooString *)daToks->get(tfPos + 1);
          tok->clear();
          tok->appendf("{0:.2f}", fontSize);
        }
      }

      // compute text start position
      switch (quadding) {
        case fieldQuadLeft:
        default:
          x = borderWidth + 2;
          break;
        case fieldQuadCenter:
          x = borderWidth + 2 + 0.5 * (comb - text->getLength()) * w;
          break;
        case fieldQuadRight:
          x = borderWidth + 2 + (comb - text->getLength()) * w;
          break;
      }
      y = 0.5 * (rect->y2 - rect->y1) - 0.4 * fontSize;

      // set the font matrix
      if (tmPos >= 0) {
        tok = (GooString *)daToks->get(tmPos + 4);
        tok->clear();
        tok->appendf("{0:.2f}", x);
        tok = (GooString *)daToks->get(tmPos + 5);
        tok->clear();
        tok->appendf("{0:.2f}", y);
      }

      // write the DA string
      if (daToks) {
        for (i = 0; i < daToks->getLength(); ++i) {
          appearBuf->append((GooString *)daToks->get(i))->append(' ');
        }
      }

      // write the font matrix (if not part of the DA string)
      if (tmPos < 0) {
        appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
      }
      // write the text string
      //~ this should center (instead of left-justify) each character within
      //~     its comb cell
      for (i = 0; i < text->getLength(); ++i) {
        if (i > 0) {
          appearBuf->appendf("{0:.2f} 0 Td\n", w);
        }
        appearBuf->append('(');
        //~ it would be better to call it only once for the whole string instead of once for
        //each character => but we need to handle centering in writeTextString
        writeTextString (text, appearBuf, &i, i+1, font->getToUnicode(), password);
        appearBuf->append(") Tj\n");
      }

      // regular (non-comb) formatting
    } else {
      // compute string width
      if (font && !font->isCIDFont()) {
        w = 0;
        for (i = 0; i < text->getLength(); ++i) {
          w += ((Gfx8BitFont *)font)->getWidth(text->getChar(i));
        }
      } else {
        // otherwise, make a crude estimate
        w = text->getLength() * 0.5;
      }

      // compute font autosize
      if (fontSize == 0) {
        fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
        fontSize2 = (rect->x2 - rect->x1 - 4 - 2 * borderWidth) / w;
        if (fontSize2 < fontSize) {
          fontSize = fontSize2;
        }
        fontSize = floor(fontSize);
        if (tfPos >= 0) {
          tok = (GooString *)daToks->get(tfPos + 1);
          tok->clear();
          tok->appendf("{0:.2f}", fontSize);
        }
      }

      // compute text start position
      w *= fontSize;
      switch (quadding) {
        case fieldQuadLeft:
        default:
          x = borderWidth + 2;
          break;
        case fieldQuadCenter:
          x = (rect->x2 - rect->x1 - w) / 2;
          break;
        case fieldQuadRight:
          x = rect->x2 - rect->x1 - borderWidth - 2 - w;
          break;
      }
      y = 0.5 * (rect->y2 - rect->y1) - 0.4 * fontSize;

      // set the font matrix
      if (tmPos >= 0) {
        tok = (GooString *)daToks->get(tmPos + 4);
        tok->clear();
        tok->appendf("{0:.2f}", x);
        tok = (GooString *)daToks->get(tmPos + 5);
        tok->clear();
        tok->appendf("{0:.2f}", y);
      }

      // write the DA string
      if (daToks) {
        for (i = 0; i < daToks->getLength(); ++i) {
          appearBuf->append((GooString *)daToks->get(i))->append(' ');
        }
      }

      // write the font matrix (if not part of the DA string)
      if (tmPos < 0) {
        appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
      }
      // write the text string
      appearBuf->append('(');
      i=0;
      writeTextString (text, appearBuf, &i, text->getLength(), font->getToUnicode(), password);
      appearBuf->append(") Tj\n");
    }
  }
  // cleanup
  appearBuf->append("ET\n");
  appearBuf->append("Q\n");
  if (txField) {
    appearBuf->append("EMC\n");
  }
  if (daToks) {
    deleteGooList(daToks, GooString);
  }
}

// Draw the variable text or caption for a field.
void Annot::drawListBox(GooString **text, GBool *selection,
			int nOptions, int topIdx,
			GooString *da, GfxFontDict *fontDict, GBool quadding) {
  GooList *daToks;
  GooString *tok;
  GfxFont *font;
  double fontSize, fontSize2, borderWidth, x, y, w, wMax;
  int tfPos, tmPos, i, j;

  //~ if there is no MK entry, this should use the existing content stream,
  //~ and only replace the marked content portion of it
  //~ (this is only relevant for Tx fields)

  // parse the default appearance string
  tfPos = tmPos = -1;
  if (da) {
    daToks = new GooList();
    i = 0;
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
	++i;
       }
      if (i < da->getLength()) {
	for (j = i + 1;
	     j < da->getLength() && !Lexer::isSpace(da->getChar(j));
	     ++j) ;
	daToks->append(new GooString(da, i, j - i));
	i = j;
      }
    }
    for (i = 2; i < daToks->getLength(); ++i) {
      if (i >= 2 && !((GooString *)daToks->get(i))->cmp("Tf")) {
	tfPos = i - 2;
      } else if (i >= 6 && !((GooString *)daToks->get(i))->cmp("Tm")) {
	tmPos = i - 6;
      }
    }
  } else {
    daToks = NULL;
  }

  // get the font and font size
  font = NULL;
  fontSize = 0;
  if (tfPos >= 0) {
    tok = (GooString *)daToks->get(tfPos);
    if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
      if (!fontDict || !(font = fontDict->lookup(tok->getCString() + 1))) {
        error(-1, "Unknown font in field's DA string");
      }
    } else {
      error(-1, "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GooString *)daToks->get(tfPos + 1);
    fontSize = atof(tok->getCString());
  } else {
    error(-1, "Missing 'Tf' operator in field's DA string");
  }
  if (!font) {
    return;
  }

  // get the border width
  borderWidth = border ? border->getWidth() : 0;

  // compute font autosize
  if (fontSize == 0) {
    wMax = 0;
    for (i = 0; i < nOptions; ++i) {
      if (font && !font->isCIDFont()) {
        w = 0;
        for (j = 0; j < text[i]->getLength(); ++j) {
          w += ((Gfx8BitFont *)font)->getWidth(text[i]->getChar(j));
        }
      } else {
        // otherwise, make a crude estimate
        w = text[i]->getLength() * 0.5;
      }
      if (w > wMax) {
        wMax = w;
      }
    }
    fontSize = rect->y2 - rect->y1 - 2 * borderWidth;
    fontSize2 = (rect->x2 - rect->x1 - 4 - 2 * borderWidth) / wMax;
    if (fontSize2 < fontSize) {
      fontSize = fontSize2;
    }
    fontSize = floor(fontSize);
    if (tfPos >= 0) {
      tok = (GooString *)daToks->get(tfPos + 1);
      tok->clear();
      tok->appendf("{0:.2f}", fontSize);
    }
  }
  // draw the text
  y = rect->y2 - rect->y1 - 1.1 * fontSize;
  for (i = topIdx; i < nOptions; ++i) {
    // setup
    appearBuf->append("q\n");

    // draw the background if selected
    if (selection[i]) {
      appearBuf->append("0 g f\n");
      appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} re f\n",
          borderWidth,
          y - 0.2 * fontSize,
          rect->x2 - rect->x1 - 2 * borderWidth,
          1.1 * fontSize);
    }

    // setup
    appearBuf->append("BT\n");

    // compute string width
    if (font && !font->isCIDFont()) {
      w = 0;
      for (j = 0; j < text[i]->getLength(); ++j) {
        w += ((Gfx8BitFont *)font)->getWidth(text[i]->getChar(j));
      }
    } else {
      // otherwise, make a crude estimate
      w = text[i]->getLength() * 0.5;
    }

    // compute text start position
    w *= fontSize;
    switch (quadding) {
      case fieldQuadLeft:
      default:
        x = borderWidth + 2;
        break;
      case fieldQuadCenter:
        x = (rect->x2 - rect->x1 - w) / 2;
        break;
      case fieldQuadRight:
        x = rect->x2 - rect->x1 - borderWidth - 2 - w;
        break;
    }

    // set the font matrix
    if (tmPos >= 0) {
      tok = (GooString *)daToks->get(tmPos + 4);
      tok->clear();
      tok->appendf("{0:.2f}", x);
      tok = (GooString *)daToks->get(tmPos + 5);
      tok->clear();
      tok->appendf("{0:.2f}", y);
    }

    // write the DA string
    if (daToks) {
      for (j = 0; j < daToks->getLength(); ++j) {
        appearBuf->append((GooString *)daToks->get(j))->append(' ');
      }
    }

    // write the font matrix (if not part of the DA string)
    if (tmPos < 0) {
      appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
    }

    // change the text color if selected
    if (selection[i]) {
      appearBuf->append("1 g\n");
    }

    // write the text string
    appearBuf->append('(');
    j = 0;
    writeTextString (text[i], appearBuf, &j, text[i]->getLength(), font->getToUnicode(), false);
    appearBuf->append(") Tj\n");

    // cleanup
    appearBuf->append("ET\n");
    appearBuf->append("Q\n");

    // next line
    y -= 1.1 * fontSize;
  }

  if (daToks) {
    deleteGooList(daToks, GooString);
  }
}

// Figure out how much text will fit on the next line.  Returns:
// *end = one past the last character to be included
// *width = width of the characters start .. end-1
// *next = index of first character on the following line
void Annot::getNextLine(GooString *text, int start,
    GfxFont *font, double fontSize, double wMax,
    int *end, double *width, int *next) {
  double w, dw;
  int j, k, c;

  // figure out how much text will fit on the line
  //~ what does Adobe do with tabs?
  w = 0;
  for (j = start; j < text->getLength() && w <= wMax; ++j) {
    c = text->getChar(j) & 0xff;
    if (c == 0x0a || c == 0x0d) {
      break;
    }
    if (font && !font->isCIDFont()) {
      dw = ((Gfx8BitFont *)font)->getWidth(c) * fontSize;
    } else {
      // otherwise, make a crude estimate
      dw = 0.5 * fontSize;
    }
    w += dw;
  }
  if (w > wMax) {
    for (k = j; k > start && text->getChar(k-1) != ' '; --k) ;
    for (; k > start && text->getChar(k-1) == ' '; --k) ;
    if (k > start) {
      j = k;
    }
    if (j == start) {
      // handle the pathological case where the first character is
      // too wide to fit on the line all by itself
      j = start + 1;
    }
  }
  *end = j;

  // compute the width
  w = 0;
  for (k = start; k < j; ++k) {
    if (font && !font->isCIDFont()) {
      dw = ((Gfx8BitFont *)font)->getWidth(text->getChar(k)) * fontSize;
    } else {
      // otherwise, make a crude estimate
      dw = 0.5 * fontSize;
    }
    w += dw;
  }
  *width = w;

  // next line
  while (j < text->getLength() && text->getChar(j) == ' ') {
    ++j;
  }
  if (j < text->getLength() && text->getChar(j) == 0x0d) {
    ++j;
  }
  if (j < text->getLength() && text->getChar(j) == 0x0a) {
    ++j;
  }
  *next = j;
}

// Draw an (approximate) circle of radius <r> centered at (<cx>, <cy>).
// If <fill> is true, the circle is filled; otherwise it is stroked.
void Annot::drawCircle(double cx, double cy, double r, GBool fill) {
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx + r, cy);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + r, cy + bezierCircle * r,
      cx + bezierCircle * r, cy + r,
      cx, cy + r);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - bezierCircle * r, cy + r,
      cx - r, cy + bezierCircle * r,
      cx - r, cy);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - r, cy - bezierCircle * r,
      cx - bezierCircle * r, cy - r,
      cx, cy - r);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + bezierCircle * r, cy - r,
      cx + r, cy - bezierCircle * r,
      cx + r, cy);
  appearBuf->append(fill ? "f\n" : "s\n");
}

// Draw the top-left half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void Annot::drawCircleTopLeft(double cx, double cy, double r) {
  double r2;

  r2 = r / sqrt(2.0);
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx + r2, cy + r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + (1 - bezierCircle) * r2,
      cy + (1 + bezierCircle) * r2,
      cx - (1 - bezierCircle) * r2,
      cy + (1 + bezierCircle) * r2,
      cx - r2,
      cy + r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - (1 + bezierCircle) * r2,
      cy + (1 - bezierCircle) * r2,
      cx - (1 + bezierCircle) * r2,
      cy - (1 - bezierCircle) * r2,
      cx - r2,
      cy - r2);
  appearBuf->append("S\n");
}

// Draw the bottom-right half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void Annot::drawCircleBottomRight(double cx, double cy, double r) {
  double r2;

  r2 = r / sqrt(2.0);
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
      cx - r2, cy - r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx - (1 - bezierCircle) * r2,
      cy - (1 + bezierCircle) * r2,
      cx + (1 - bezierCircle) * r2,
      cy - (1 + bezierCircle) * r2,
      cx + r2,
      cy - r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
      cx + (1 + bezierCircle) * r2,
      cy - (1 - bezierCircle) * r2,
      cx + (1 + bezierCircle) * r2,
      cy + (1 - bezierCircle) * r2,
      cx + r2,
      cy + r2);
  appearBuf->append("S\n");
}

void Annot::draw(Gfx *gfx, GBool printing) {
  Object obj;

  // check the flags
  if ((flags & annotFlagHidden) ||
      (printing && !(flags & annotFlagPrint)) ||
      (!printing && (flags & annotFlagNoView))) {
    return;
  }

  // draw the appearance stream
  appearance.fetch(xref, &obj);
  gfx->drawAnnot(&obj, (type == typeLink) ? border : (AnnotBorder *)NULL, color,
      rect->x1, rect->y1, rect->x2, rect->y2);
  obj.free();
}

//------------------------------------------------------------------------
// AnnotPopup
//------------------------------------------------------------------------

AnnotPopup::AnnotPopup(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, acroForm, dict, catalog, obj) {
  type = typePopup;
  initialize(xrefA, acroForm, dict, catalog);
}

AnnotPopup::~AnnotPopup() {
  /*
  if (parent)
    delete parent;
  */
}

void AnnotPopup::initialize(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog) {
  Object obj1;
  /*
  if (dict->lookup("Parent", &obj1)->isDict()) {
    parent = NULL;
  } else {
    parent = NULL;
  }
  obj1.free();
  */
  if (dict->lookup("Open", &obj1)->isBool()) {
    open = obj1.getBool();
  } else {
    open = gFalse;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotMarkup
//------------------------------------------------------------------------
 
AnnotMarkup::AnnotMarkup(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog, Object *obj) {
  initialize(xrefA, acroForm, dict, catalog, obj);
}

AnnotMarkup::~AnnotMarkup() {
  if (label)
    delete label;

  if (popup)
    delete popup;

  if (date)
    delete date;

  if (inReplyTo)
    delete inReplyTo;

  if (subject)
    delete subject;
}

void AnnotMarkup::initialize(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog, Object *obj) {
  Object obj1;

  if (dict->lookup("T", &obj1)->isString()) {
    label = obj1.getString()->copy();
  } else {
    label = NULL;
  }
  obj1.free();

  if (dict->lookup("Popup", &obj1)->isDict()) {
    popup = new AnnotPopup(xrefA, acroForm, obj1.getDict(), catalog, obj);
  } else {
    popup = NULL;
  }
  obj1.free();

  if (dict->lookup("CA", &obj1)->isNum()) {
    opacity = obj1.getNum();
  } else {
    opacity = 1.0;
  }
  obj1.free();

  if (dict->lookup("CreationDate", &obj1)->isString()) {
    date = obj1.getString()->copy();
  } else {
    date = NULL;
  }
  obj1.free();

  if (dict->lookup("IRT", &obj1)->isDict()) {
    inReplyTo = obj1.getDict();
  } else {
    inReplyTo = NULL;
  }
  obj1.free();

  if (dict->lookup("Subj", &obj1)->isString()) {
    subject = obj1.getString()->copy();
  } else {
    subject = NULL;
  }
  obj1.free();

  if (dict->lookup("RT", &obj1)->isName()) {
    GooString *replyName = new GooString(obj1.getName());

    if (!replyName->cmp("R")) {
      replyTo = replyTypeR;
    } else if (!replyName->cmp("Group")) {
      replyTo = replyTypeGroup;
    } else {
      replyTo = replyTypeR;
    }
    delete replyName;
  } else {
    replyTo = replyTypeR;
  }
  obj1.free();

  if (dict->lookup("ExData", &obj1)->isDict()) {
    exData = parseAnnotExternalData(obj1.getDict());
  } else {
    exData = annotExternalDataMarkupUnknown;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotText
//------------------------------------------------------------------------

AnnotText::AnnotText(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, acroForm, dict, catalog, obj), AnnotMarkup(xref, acroForm, dict, catalog, obj) {

  type = typeText;
  flags |= flagNoZoom & flagNoRotate;
  initialize (xrefA, catalog, dict);
}

void AnnotText::setModified(GooString *date) {
  if (date) {
    delete modified;
    modified = new GooString(date);
  }
}

void AnnotText::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if (dict->lookup("Open", &obj1)->isBool())
    open = obj1.getBool();
  else
    open = gFalse;
  obj1.free();

  if (dict->lookup("Name", &obj1)->isName()) {
    GooString *iconName = new GooString(obj1.getName());

    if (!iconName->cmp("Comment")) {
      icon = iconComment;
    } else if (!iconName->cmp("Key")) {
      icon = iconKey;
    } else if (!iconName->cmp("Help")) {
      icon = iconHelp;
    } else if (!iconName->cmp("NewParagraph")) {
      icon = iconNewParagraph;
    } else if (!iconName->cmp("Paragraph")) {
      icon = iconParagraph;
    } else if (!iconName->cmp("Insert")) {
      icon = iconInsert;
    } else {
      icon = iconNote;
    }
    delete iconName;
  } else {
    icon = iconNote;
  }
  obj1.free();

  if (dict->lookup("StateModel", &obj1)->isString()) {
    Object obj2;
    GooString *modelName = obj1.getString();

    if (dict->lookup("State", &obj2)->isString()) {
      GooString *stateName = obj2.getString();

      if (!stateName->cmp("Marked")) {
        state = stateMarked;
      } else if (!stateName->cmp("Unmarked")) {
        state = stateUnmarked;
      } else if (!stateName->cmp("Accepted")) {
        state = stateAccepted;
      } else if (!stateName->cmp("Rejected")) {
        state = stateRejected;
      } else if (!stateName->cmp("Cancelled")) {
        state = stateCancelled;
      } else if (!stateName->cmp("Completed")) {
        state = stateCompleted;
      } else if (!stateName->cmp("None")) {
        state = stateNone;
      } else {
        state = stateUnknown;
      }

      delete stateName;
    } else {
      state = stateUnknown;
    }
    obj2.free();

    if (!modelName->cmp("Marked")) {
      switch (state) {
        case stateUnknown:
          state = stateMarked;
          break;
        case stateAccepted:
        case stateRejected:
        case stateCancelled:
        case stateCompleted:
        case stateNone:
          state = stateUnknown;
          break;
        default:
          break;
      }
    } else if (!modelName->cmp("Review")) {
      switch (state) {
        case stateUnknown:
          state = stateNone;
          break;
        case stateMarked:
        case stateUnmarked:
          state = stateUnknown;
          break;
        default:
          break;
      }
    } else {
      state = stateUnknown;
    }

    delete modelName;
  } else {
    state = stateUnknown;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// AnnotLink
//------------------------------------------------------------------------

AnnotLink::AnnotLink(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, acroForm, dict, catalog, obj) {

  type = typeLink;
  initialize (xrefA, catalog, dict);
}

AnnotLink::~AnnotLink() {
  /*
  if (actionDict)
    delete actionDict;

  if (uriAction)
    delete uriAction;
  */
  if (quadrilaterals) {
    for(int i = 0; i < quadrilateralsLength; i++)
      delete quadrilaterals[i];

    gfree (quadrilaterals);
  }
}

void AnnotLink::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;
  /*
  if (dict->lookup("A", &obj1)->isDict()) {
    actionDict = NULL;
  } else {
    actionDict = NULL;
  }
  obj1.free();
  */
  if (dict->lookup("H", &obj1)->isName()) {
    GooString *effect = new GooString(obj1.getName());

    if (!effect->cmp("N")) {
      linkEffect = effectNone;
    } else if (!effect->cmp("I")) {
      linkEffect = effectInvert;
    } else if (!effect->cmp("O")) {
      linkEffect = effectOutline;
    } else if (!effect->cmp("P")) {
      linkEffect = effectPush;
    } else {
      linkEffect = effectInvert;
    }
    delete effect;
  } else {
    linkEffect = effectInvert;
  }
  obj1.free();
  /*
  if (dict->lookup("PA", &obj1)->isDict()) {
    uriAction = NULL;
  } else {
    uriAction = NULL;
  }
  obj1.free();
  */
  /*
   * TODO:
   * QuadPoints should be ignored if any coordinate in the array lies outside
   * the region specified by Rect.
   */
  if (dict->lookup("QuadPoints", &obj1)->isArray()) {
    parseQuadPointsArray(obj1.getArray());
  } else {
    quadrilaterals = NULL;
  }
  obj1.free();
}

GBool AnnotLink::parseQuadPointsArray(Array *array) {
  int arrayLength = array->getLength();
  GBool correct = gTrue;
  int quadsLength = 0, i = 0;
  AnnotQuadrilateral **quads;
  double *quadArray;

  // default values
  quadrilaterals = NULL;
  quadrilateralsLength = 0;

  if ((arrayLength % 8) != 0)
    return gFalse;

  quadsLength = arrayLength / 8;
  quads = (AnnotQuadrilateral **) gmallocn
      ((quadsLength), sizeof(AnnotQuadrilateral *));
  quadArray = (double *) gmallocn (8, sizeof(double));

  while (i < (quadsLength) && correct) {
    for (int j = 0; j < 8 && correct; j++) {
      Object obj;
      if (array->get(i * 8 + j, &obj)->isNum()) {
        quadArray[j] = obj.getNum();
        if (quadArray[j] < rect->x1 || quadArray[j] > rect->x2 ||
            quadArray[j] < rect->y1 || quadArray[j] < rect->y2)
          correct = gFalse;
      } else {
          correct = gFalse;
      }
    }

    if (correct)
      quads[i] = new AnnotQuadrilateral(quadArray[0], quadArray[1],
                                        quadArray[2], quadArray[3],
                                        quadArray[4], quadArray[5],
                                        quadArray[6], quadArray[7]);
    i++;
  }

  gfree (quadArray);

  if (!correct) {
    for (int j = 0; j < i; j++)
      delete quads[j];
    gfree (quads);
    return gFalse;
  }

  quadrilateralsLength = quadsLength;
  quadrilaterals = quads;

  return gTrue;
}

//------------------------------------------------------------------------
// AnnotFreeText
//------------------------------------------------------------------------

AnnotFreeText::AnnotFreeText(XRef *xrefA, Dict *acroForm, Dict *dict, Catalog *catalog, Object *obj) :
    Annot(xrefA, acroForm, dict, catalog, obj), AnnotMarkup(xref, acroForm, dict, catalog, obj) {
  type = typeFreeText;
  initialize(xrefA, catalog, dict);
}

AnnotFreeText::~AnnotFreeText() {
  delete appearanceString;

  if (styleString)
    delete styleString;

  if (calloutLine)
    delete calloutLine;

  if (borderEffect)
    delete borderEffect;

  if (rectangle)
    delete rectangle;
}

void AnnotFreeText::initialize(XRef *xrefA, Catalog *catalog, Dict *dict) {
  Object obj1;

  if (dict->lookup("DA", &obj1)->isString()) {
    appearanceString = obj1.getString()->copy();
  } else {
    appearanceString = new GooString();
    error(-1, "Bad appearance for annotation");
    ok = gFalse;
  }
  obj1.free();

  if (dict->lookup("Q", &obj1)->isInt()) {
    quadding = (AnnotFreeTextQuadding) obj1.getInt();
  } else {
    quadding = quaddingLeftJustified;
  }
  obj1.free();

  if (dict->lookup("DS", &obj1)->isString()) {
    styleString = obj1.getString()->copy();
  } else {
    styleString = NULL;
  }
  obj1.free();

  if (dict->lookup("CL", &obj1)->isArray() && obj1.arrayGetLength() >= 4) {
    double x1, y1, x2, y2;
    Object obj2;

    (obj1.arrayGet(0, &obj2)->isNum() ? x1 = obj2.getNum() : x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? y1 = obj2.getNum() : y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? x2 = obj2.getNum() : x2 = 0);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? y2 = obj2.getNum() : y2 = 0);
    obj2.free();

    if (obj1.arrayGetLength() == 6) {
      double x3, y3;
      (obj1.arrayGet(4, &obj2)->isNum() ? x3 = obj2.getNum() : x3 = 0);
      obj2.free();
      (obj1.arrayGet(5, &obj2)->isNum() ? y3 = obj2.getNum() : y3 = 0);
      obj2.free();
      calloutLine = new AnnotCalloutMultiLine(x1, y1, x2, y2, x3, y3);
    } else {
      calloutLine = new AnnotCalloutLine(x1, y1, x2, y2);
    }
  } else {
    calloutLine = NULL;
  }
  obj1.free();

  if (dict->lookup("IT", &obj1)->isName()) {
    GooString *intentName = new GooString(obj1.getName());

    if (!intentName->cmp("FreeText")) {
      intent = intentFreeText;
    } else if (!intentName->cmp("FreeTextCallout")) {
      intent = intentFreeTextCallout;
    } else if (!intentName->cmp("FreeTextTypeWriter")) {
      intent = intentFreeTextTypeWriter;
    } else {
      intent = intentFreeText;
    }
    delete intentName;
  } else {
    intent = intentFreeText;
  }
  obj1.free();

  if (dict->lookup("BE", &obj1)->isDict()) {
    borderEffect = new AnnotBorderEffect(obj1.getDict());
  } else {
    borderEffect = NULL;
  }
  obj1.free();

  if (dict->lookup("RD", &obj1)->isArray() && obj1.arrayGetLength() == 4) {
    Object obj2;
    rectangle = new PDFRectangle();

    (obj1.arrayGet(0, &obj2)->isNum() ? rectangle->x1 = obj2.getNum() :
      rectangle->x1 = 0);
    obj2.free();
    (obj1.arrayGet(1, &obj2)->isNum() ? rectangle->y1 = obj2.getNum() :
      rectangle->y1 = 0);
    obj2.free();
    (obj1.arrayGet(2, &obj2)->isNum() ? rectangle->x2 = obj2.getNum() :
      rectangle->x2 = 1);
    obj2.free();
    (obj1.arrayGet(3, &obj2)->isNum() ? rectangle->y2 = obj2.getNum() :
      rectangle->y2 = 1);
    obj2.free();

    if (rectangle->x1 > rectangle->x2) {
      double t = rectangle->x1;
      rectangle->x1 = rectangle->x2;
      rectangle->x2 = t;
    }
    if (rectangle->y1 > rectangle->y2) {
      double t = rectangle->y1;
      rectangle->y1 = rectangle->y2;
      rectangle->y2 = t;
    }

    if ((rectangle->x1 + rectangle->x2) > (rect->x2 - rect->x1))
      rectangle->x1 = rectangle->x2 = 0;

    if ((rectangle->y1 + rectangle->y2) > (rect->y2 - rect->y1))
      rectangle->y1 = rectangle->y2 = 0;
  } else {
    rectangle = NULL;
  }
  obj1.free();

  if (dict->lookup("LE", &obj1)->isName()) {
    GooString *styleName = new GooString(obj1.getName());
    endStyle = parseAnnotLineEndingStyle(styleName);
    delete styleName;
  } else {
    endStyle = annotLineEndingNone;
  }
  obj1.free();
}

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

Annots::Annots(XRef *xref, Catalog *catalog, Object *annotsObj) {
  Dict *acroForm;
  Annot *annot;
  Object obj1;
  int size;
  int i;

  annots = NULL;
  size = 0;
  nAnnots = 0;

  acroForm = catalog->getAcroForm()->isDict() ?
    catalog->getAcroForm()->getDict() : NULL;
  if (annotsObj->isArray()) {
    for (i = 0; i < annotsObj->arrayGetLength(); ++i) {
      //get the Ref to this annot and pass it to Annot constructor 
      //this way, it'll be possible for the annot to retrieve the corresponding
      //form widget
      Object obj2;
      if (annotsObj->arrayGet(i, &obj1)->isDict()) {
        annotsObj->arrayGetNF(i, &obj2);
        annot = createAnnot (xref, acroForm, obj1.getDict(), catalog, &obj2);
        if (annot->isOk()) {
          if (nAnnots >= size) {
            size += 16;
            annots = (Annot **)greallocn(annots, size, sizeof(Annot *));
          }
          annots[nAnnots++] = annot;
        } else {
          delete annot;
        }
      }
      obj2.free();
      obj1.free();
    }
  }
}

Annot *Annots::createAnnot(XRef *xref, Dict *acroForm, Dict* dict, Catalog *catalog, Object *obj) {
  Annot *annot;
  Object obj1;

  if (dict->lookup("Subtype", &obj1)->isName()) {
    GooString *typeName = new GooString(obj1.getName());

    if (!typeName->cmp("Text")) {
      annot = new AnnotText(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Link")) {
      annot = new AnnotLink(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("FreeText")) {
      annot = new AnnotFreeText(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Line")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Square")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Circle")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Polygon")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("PolyLine")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Highlight")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Underline")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Squiggly")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("StrikeOut")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Stamp")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Caret")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Ink")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("FileAttachment")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Sound")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Movie")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Widget")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Screen")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("PrinterMark")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("TrapNet")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("Watermark")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else if (!typeName->cmp("3D")) {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    } else {
      annot = new Annot(xref, acroForm, dict, catalog, obj);
    }

    delete typeName;
  } else {
    annot = NULL;
  }
  obj1.free();

  return annot;
}

void Annots::generateAppearances(Dict *acroForm) {
  Object obj1, obj2;
  Ref ref;
  int i;
  
  if (acroForm->lookup("Fields", &obj1)->isArray()) {
    for (i = 0; i < obj1.arrayGetLength(); ++i) {
      if (obj1.arrayGetNF(i, &obj2)->isRef()) {
        ref = obj2.getRef();
        obj2.free();
        obj1.arrayGet(i, &obj2);
      } else {
        ref.num = ref.gen = -1;
      }
      if (obj2.isDict()) {
        scanFieldAppearances(obj2.getDict(), &ref, NULL, acroForm);
      }
      obj2.free();
    }
  }
  obj1.free();
}

void Annots::scanFieldAppearances(Dict *node, Ref *ref, Dict *parent,
    Dict *acroForm) {
  Annot *annot;
  Object obj1, obj2;
  Ref ref2;
  int i;

  // non-terminal node: scan the children
  if (node->lookup("Kids", &obj1)->isArray()) {
    for (i = 0; i < obj1.arrayGetLength(); ++i) {
      if (obj1.arrayGetNF(i, &obj2)->isRef()) {
        ref2 = obj2.getRef();
        obj2.free();
        obj1.arrayGet(i, &obj2);
      } else {
        ref2.num = ref2.gen = -1;
      }
      if (obj2.isDict()) {
        scanFieldAppearances(obj2.getDict(), &ref2, node, acroForm);
      }
      obj2.free();
    }
    obj1.free();
    return;
  }
  obj1.free();

  // terminal node: this is either a combined annot/field dict, or an
  // annot dict whose parent is a field
  if ((annot = findAnnot(ref))) {
    node->lookupNF("Parent", &obj1);
    if (!parent || !obj1.isNull()) {
      annot->generateFieldAppearance(node, node, acroForm);
    } else {
      annot->generateFieldAppearance(parent, node, acroForm);
    }
    obj1.free();
  }
}

Annot *Annots::findAnnot(Ref *ref) {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    if (annots[i]->match(ref)) {
      return annots[i];
    }
  }
  return NULL;
}


Annots::~Annots() {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    delete annots[i];
  }
  gfree(annots);
}
