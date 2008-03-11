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
class Form;
class FormWidget;
class PDFRectangle;
class Movie;

enum AnnotLineEndingStyle {
  annotLineEndingSquare,        // Square
  annotLineEndingCircle,        // Circle
  annotLineEndingDiamond,       // Diamond
  annotLineEndingOpenArrow,     // OpenArrow
  annotLineEndingClosedArrow,   // ClosedArrow
  annotLineEndingNone,          // None
  annotLineEndingButt,          // Butt
  annotLineEndingROpenArrow,    // ROpenArrow
  annotLineEndingRClosedArrow,  // RClosedArrow
  annotLineEndingSlash          // Slash
};

enum AnnotExternalDataType {
  annotExternalDataMarkupUnknown,
  annotExternalDataMarkup3D       // Markup3D
};

//------------------------------------------------------------------------
// AnnotCalloutLine
//------------------------------------------------------------------------

class AnnotCalloutLine {
public:

  AnnotCalloutLine(double x1, double y1, double x2, double y2);
  virtual ~AnnotCalloutLine() { }

  double getX1() const { return x1; }
  double getY1() const { return y1; }
  double getX2() const { return x2; }
  double getY2() const { return y2; }
  
protected:

  double x1, y1, x2, y2;
};

//------------------------------------------------------------------------
// AnnotCalloutMultiLine
//------------------------------------------------------------------------

class AnnotCalloutMultiLine: public AnnotCalloutLine {
public:

  AnnotCalloutMultiLine(double x1, double y1, double x2, double y2,
    double x3, double y3);

  double getX3() const { return x3; }
  double getY3() const { return y3; }

protected:

  double x3, y3;
};

//------------------------------------------------------------------------
// AnnotBorderEffect
//------------------------------------------------------------------------

class AnnotBorderEffect {
public:

  enum AnnotBorderEffectType {
    borderEffectNoEffect, // S
    borderEffectCloudy    // C
  };

  AnnotBorderEffect(Dict *dict);

  AnnotBorderEffectType getEffectType() const { return effectType; }
  double getIntensity() const { return intensity; }

private:

  AnnotBorderEffectType effectType; // S  (Default S)
  double intensity;                 // I  (Default 0)
};

//------------------------------------------------------------------------
// AnnotQuadrilateral
//------------------------------------------------------------------------

class AnnotQuadrilaterals {
  class AnnotQuadrilateral {
  public:
    AnnotQuadrilateral(double x1, double y1, double x2, double y2, double x3,
      double y3, double x4, double y4);

    double x1, y1, x2, y2, x3, y3, x4, y4;
  };

public:

  AnnotQuadrilaterals(Array *array, PDFRectangle *rect);
  virtual ~AnnotQuadrilaterals();

  double getX1(int quadrilateral);
  double getY1(int quadrilateral);
  double getX2(int quadrilateral);
  double getY2(int quadrilateral);
  double getX3(int quadrilateral);
  double getY3(int quadrilateral);
  double getX4(int quadrilateral);
  double getY4(int quadrilateral);
  int getQuadrilateralsLength() const { return quadrilateralsLength; }
protected:

  AnnotQuadrilateral** quadrilaterals;
  int quadrilateralsLength;
};

//------------------------------------------------------------------------
// AnnotQuadPoints
//------------------------------------------------------------------------

class AnnotQuadPoints {
public:
  
  AnnotQuadPoints(double x1, double y1, double x2, double y2, double x3,
      double y3, double x4, double y4);

  double getX1() const { return x1; }
  double getY1() const { return y1; }
  double getX2() const { return x2; }
  double getY2() const { return y2; }
  double getX3() const { return x3; }
  double getY3() const { return y3; }
  double getX4() const { return x4; }
  double getY4() const { return y4; }

protected:
  double x1, y1, x2, y2, x3, y3, x4, y4;
};

//------------------------------------------------------------------------
// AnnotBorder
//------------------------------------------------------------------------

class AnnotBorder {
public:
  enum AnnotBorderType {
    typeUnknown,
    typeArray,
    typeBS,
  };

  enum AnnotBorderStyle {
    borderSolid,      // Solid
    borderDashed,     // Dashed
    borderBeveled,    // Beveled
    borderInset,      // Inset
    borderUnderlined, // Underlined
  };

  AnnotBorder();
  virtual ~AnnotBorder();

  virtual AnnotBorderType getType() const { return type; }
  virtual double getWidth() const { return width; }
  virtual int getDashLength() const { return dashLength; }
  virtual double *getDash() const { return dash; }
  virtual AnnotBorderStyle getStyle() const { return style; }

protected:
  AnnotBorderType type;
  double width;
  int dashLength;
  double *dash;
  AnnotBorderStyle style;
};

//------------------------------------------------------------------------
// AnnotBorderArray
//------------------------------------------------------------------------

class AnnotBorderArray: public AnnotBorder {
public:
  AnnotBorderArray();
  AnnotBorderArray(Array *array);

  virtual double getHorizontalCorner() const { return horizontalCorner; }
  virtual double getVerticalCorner() const { return verticalCorner; }

protected:
  static const int DASH_LIMIT = 10; // implementation note 82 in Appendix H.
  double horizontalCorner;          // (Default 0)
  double verticalCorner;            // (Default 0)
  // double width;                  // (Default 1)  (inherited from AnnotBorder)
};

//------------------------------------------------------------------------
// AnnotBorderBS
//------------------------------------------------------------------------

class AnnotBorderBS: public AnnotBorder {
public:

  AnnotBorderBS();
  AnnotBorderBS(Dict *dict);

private:
  // double width;           // W  (Default 1)   (inherited from AnnotBorder)
  // AnnotBorderStyle style; // S  (Default S)   (inherited from AnnotBorder)
  // double *dash;           // D  (Default [3]) (inherited from AnnotBorder)
};

//------------------------------------------------------------------------
// AnnotColor
//------------------------------------------------------------------------

class AnnotColor {
public:

  enum AnnotColorSpace {
    colorTransparent = 0,
    colorGray        = 1,
    colorRGB         = 3,
    colorCMYK        = 4
  };

  AnnotColor();
  AnnotColor(Array *array);
  ~AnnotColor();

  AnnotColorSpace getSpace() const { return (AnnotColorSpace) length; }
  double *getValues() const { return values; }

private:

  double *values;
  int length;
};

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
// AnnotIconFit
//------------------------------------------------------------------------

class AnnotIconFit {
public:

  enum AnnotIconFitScaleWhen {
    scaleAlways,  // A
    scaleBigger,  // B
    scaleSmaller, // S
    scaleNever    // N
  };

  enum AnnotIconFitScale {
    scaleAnamorphic,  // A
    scaleProportional // P
  };

  AnnotIconFit(Dict *dict);

  AnnotIconFitScaleWhen getScaleWhen() { return scaleWhen; }
  AnnotIconFitScale getScale() { return scale; }
  double getLeft() { return left; }
  double getBottom() { return bottom; }
  bool getFullyBounds() { return fullyBounds; }

protected:

  AnnotIconFitScaleWhen scaleWhen;  // SW (Default A)
  AnnotIconFitScale scale;          // S  (Default P)
  double left;                      // A  (Default [0.5 0.5]
  double bottom;                    // Only if scale is P
  bool fullyBounds;                 // FB (Default false)
};

//------------------------------------------------------------------------
// AnnotAppearance
//------------------------------------------------------------------------

class AnnotAppearance {
public:

  enum AnnotAppearanceType {
    appearNormal,
    appearRollover,
    appearDown
  };

  AnnotAppearance(Dict *dict);
};

//------------------------------------------------------------------------
// AnnotAppearanceCharacs
//------------------------------------------------------------------------

class AnnotAppearanceCharacs {
public:

  enum AnnotAppearanceCharacsTextPos {
    captionNoIcon,    // 0
    captionNoCaption, // 1
    captionBelow,     // 2
    captionAbove,     // 3
    captionRight,     // 4
    captionLeft,      // 5
    captionOverlaid   // 6
  };

  AnnotAppearanceCharacs(Dict *dict);
  ~AnnotAppearanceCharacs();

  int getRotation() { return rotation; }
  AnnotColor *getBorderColor() { return borderColor; }
  AnnotColor *getBackColor() { return backColor; }
  GooString *getNormalCaption() { return normalCaption; }
  GooString *getRolloverCaption() { return rolloverCaption; }
  GooString *getAlternateCaption() { return alternateCaption; }
  AnnotIconFit *getIconFit() { return iconFit; }
  AnnotAppearanceCharacsTextPos getPosition() { return position; }

protected:

  int rotation;                           // R  (Default 0)
  AnnotColor *borderColor;                // BC
  AnnotColor *backColor;                  // BG
  GooString *normalCaption;               // CA
  GooString *rolloverCaption;             // RC
  GooString *alternateCaption;            // AC
  // I
  // RI
  // IX
  AnnotIconFit *iconFit;                  // IF
  AnnotAppearanceCharacsTextPos position; // TP (Default 0)
};

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

class Annot {
public:
  enum AnnotFlag {
    flagUnknown        = 0x0000,
    flagInvisible      = 0x0001,
    flagHidden         = 0x0002,
    flagPrint          = 0x0004,
    flagNoZoom         = 0x0008,
    flagNoRotate       = 0x0010,
    flagNoView         = 0x0020,
    flagReadOnly       = 0x0040,
    flagLocked         = 0x0080,
    flagToggleNoView   = 0x0100,
    flagLockedContents = 0x0200
  };

  enum AnnotSubtype {
    typeUnknown,        //                 0
    typeText,           // Text            1
    typeLink,           // Link            2
    typeFreeText,       // FreeText        3
    typeLine,           // Line            4
    typeSquare,         // Square          5
    typeCircle,         // Circle          6
    typePolygon,        // Polygon         7
    typePolyLine,       // PolyLine        8
    typeHighlight,      // Highlight       9
    typeUnderline,      // Underline      10
    typeSquiggly,       // Squiggly       11
    typeStrikeOut,      // StrikeOut      12
    typeStamp,          // Stamp          13
    typeCaret,          // Caret          14
    typeInk,            // Ink            15
    typePopup,          // Popup          16
    typeFileAttachment, // FileAttachment 17
    typeSound,          // Sound          18
    typeMovie,          // Movie          19
    typeWidget,         // Widget         20
    typeScreen,         // Screen         21
    typePrinterMark,    // PrinterMark    22
    typeTrapNet,        // TrapNet        23
    typeWatermark,      // Watermark      24
    type3D              // 3D             25
  };

  Annot(XRef *xrefA, Dict *dict, Catalog* catalog);
  Annot(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  virtual ~Annot();
  GBool isOk() { return ok; }

  virtual void draw(Gfx *gfx, GBool printing);
  // Get appearance object.
  Object *getAppearance(Object *obj) { return appearance.fetch(xref, obj); }

  GBool match(Ref *refA)
    { return ref.num == refA->num && ref.gen == refA->gen; }

  double getXMin();
  double getYMin();

  double getFontSize() { return fontSize; }

  // getters
  AnnotSubtype getType() const { return type; }
  PDFRectangle *getRect() const { return rect; }
  GooString *getContents() const { return contents; }
  Dict *getPageDict() const { return pageDict; }
  GooString *getName() const { return name; }
  GooString *getModified() const { return modified; }
  Guint getFlags() const { return flags; }
  /*Dict *getAppearDict() const { return appearDict; }*/
  GooString *getAppearState() const { return appearState; }
  AnnotBorder *getBorder() const { return border; }
  AnnotColor *getColor() const { return color; }
  int getTreeKey() const { return treeKey; }
  Dict *getOptionalContent() const { return optionalContent; }

  int getId() { return ref.num; }

private:
  void readArrayNum(Object *pdfArray, int key, double *value);
  // write vStr[i:j[ in appearBuf

  void initialize (XRef *xrefA, Dict *dict, Catalog *catalog);


protected:
  void setColor(Array *a, GBool fill, int adjust);
  void drawCircle(double cx, double cy, double r, GBool fill);
  void drawCircleTopLeft(double cx, double cy, double r);
  void drawCircleBottomRight(double cx, double cy, double r);
  
  // required data
  AnnotSubtype type;                // Annotation type
  PDFRectangle *rect;               // Rect

  // optional data
  GooString *contents;              // Contents
  Dict *pageDict;                   // P
  GooString *name;                  // NM
  GooString *modified;              // M
  Guint flags;                      // F (must be a 32 bit unsigned int)
  //Dict *appearDict;                 // AP (should be correctly parsed)
  Ref appRef;                       //the reference to the indirect appearance object in XRef 
  Object appearance;     // a reference to the Form XObject stream
                         //   for the normal appearance
  GooString *appearState;           // AS
  int treeKey;                      // Struct Parent;
  Dict *optionalContent;            // OC

  XRef *xref;			// the xref table for this PDF file
  Ref ref;                      // object ref identifying this annotation
  GooString *appearBuf;
  AnnotBorder *border;          // Border, BS
  AnnotColor *color;            // C
  double fontSize; 
  GBool ok;

  bool hasRef;
};

//------------------------------------------------------------------------
// AnnotPopup
//------------------------------------------------------------------------

class AnnotPopup: public Annot {
public:
  AnnotPopup(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  virtual ~AnnotPopup();

  Dict *getParent() const { return parent; }
  GBool getOpen() const { return open; }

protected:
  void initialize(XRef *xrefA, Dict *dict, Catalog *catalog);

  Dict *parent; // Parent
  GBool open;   // Open
};

//------------------------------------------------------------------------
// AnnotMarkup
//------------------------------------------------------------------------

class AnnotMarkup: public Annot {
public:
  enum  AnnotMarkupReplyType {
    replyTypeR,     // R
    replyTypeGroup  // Group
  };

  AnnotMarkup(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  virtual ~AnnotMarkup();

  // getters
  GooString *getLabel() const { return label; }
  AnnotPopup *getPopup() const { return popup; }
  double getOpacity() const { return opacity; }
  // getRC
  GooString *getDate() const { return date; }
  Dict *getInReplyTo() const { return inReplyTo; }
  GooString *getSubject() const { return subject; }
  AnnotMarkupReplyType getReplyTo() const { return replyTo; }
  AnnotExternalDataType getExData() const { return exData; }

protected:
  GooString *label;             // T            (Default autor)
  AnnotPopup *popup;            // Popup
  double opacity;               // CA           (Default 1.0)
  // RC
  GooString *date;              // CreationDate
  Dict *inReplyTo;              // IRT
  GooString *subject;           // Subj
  AnnotMarkupReplyType replyTo; // RT           (Default R)
  // this object is overrided by the custom intent fields defined in some
  // annotation types.
  //GooString *intent;          // IT
  AnnotExternalDataType exData; // ExData

private:
  void initialize(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
};

//------------------------------------------------------------------------
// AnnotText
//------------------------------------------------------------------------

class AnnotText: public AnnotMarkup {
public:
  enum AnnotTextIcon {
    iconComment,      // Comment
    iconKey,          // Key
    iconNote,         // Note
    iconHelp,         // Help
    iconNewParagraph, // NewParagraph
    iconParagraph,    // Paragraph
    iconInsert        // Insert
  };

  enum AnnotTextState {
    stateUnknown,
    // Marked state model
    stateMarked,    // Marked
    stateUnmarked,  // Unmarked
    // Review state model
    stateAccepted,  // Accepted
    stateRejected,  // Rejected
    stateCancelled, // Cancelled
    stateCompleted, // Completed
    stateNone       // None
  };

  AnnotText(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);

  // getters
  GBool getOpen() const { return open; }
  AnnotTextIcon getIcon() const { return icon; }
  AnnotTextState getState() const { return state; }

  // setters
  void setModified(GooString *date);

private:

  void initialize(XRef *xrefA, Catalog *catalog, Dict *dict);

  GBool open;                       // Open       (Default false)
  AnnotTextIcon icon;               // Name       (Default Note)
  AnnotTextState state;             // State      (Default Umarked if
                                    //             StateModel Marked
                                    //             None if StareModel Review)
};

//------------------------------------------------------------------------
// AnnotMovie
//------------------------------------------------------------------------



class AnnotMovie: public Annot {
 public:
  enum PosterType {
    posterTypeNone,
    posterTypeStream,
    posterTypeFromMovie
  };

  enum RepeatMode {
    repeatModeOnce,
    repeatModeOpen,
    repeatModeRepeat,
    repeatModePalindrome
  };

  struct Time {
    Time() { units_per_second = 0; }
    Gulong units;
    int units_per_second; // 0 : defined by movie
  };

  AnnotMovie(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  ~AnnotMovie();

  GooString* getTitle() { return title; }
  GooString* getFileName() { return fileName; }
  int getRotationAngle() { return rotationAngle; }

  PosterType getPosterType() { return posterType; }
  Stream* getPosterStream() { return posterStream; }

  Time getStart() { return start; }
  Time getDuration() { return duration; }
  double getRate() { return rate; }
  double getVolume() { return volume; }

  GBool getShowControls() { return showControls; }
  RepeatMode getRepeatMode() { return repeatMode; }
  GBool getSynchronousPlay() { return synchronousPlay; }

  GBool needFloatingWindow() { return hasFloatingWindow; }
  GBool needFullscreen() { return isFullscreen; }
  
  
  void getMovieSize(int& width, int& height);
  void getZoomFactor(int& num, int& denum);
  void getWindowPosition(double& x, double& y) { x = FWPosX; y = FWPosY; }

  Movie* getMovie() { return movie; }

 private:
  void initialize(XRef *xrefA, Catalog *catalog, Dict *dict);

  GooString* title;      // T
  GooString* fileName;   // Movie/F

  int width;             // Movie/Aspect
  int height;            // Movie/Aspect
  int rotationAngle;     // Movie/Rotate

  PosterType posterType; // Movie/Poster
  Stream* posterStream;

  Time start;            // A/Start
  Time duration;         // A/Duration
  double rate;           // A/Rate
  double volume;         // A/Volume
  
  GBool showControls;    // A/ShowControls
  
  RepeatMode repeatMode; // A/Mode
  
  GBool synchronousPlay; // A/Synchronous

  // floating window
  GBool hasFloatingWindow; 
  unsigned short FWScaleNum; // A/FWScale
  unsigned short FWScaleDenum;
  GBool isFullscreen;

  double FWPosX;         // A/FWPosition
  double FWPosY; 

  Movie* movie;
};


//------------------------------------------------------------------------
// AnnotScreen
//------------------------------------------------------------------------

class AnnotScreen: public Annot {
 public:

  AnnotScreen(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  ~AnnotScreen();

  GooString* getTitle() { return title; }

  AnnotAppearanceCharacs *getAppearCharacs() { return appearCharacs; }
  Object* getAction() { return &action; }
  Object* getAdditionActions() { return &additionAction; }

 private:
  void initialize(XRef *xrefA, Catalog *catalog, Dict *dict);


  GooString* title;                      // T

  AnnotAppearanceCharacs* appearCharacs; // MK

  Object action;                         // A
  Object additionAction;                 // AA
};

//------------------------------------------------------------------------
// AnnotLink
//------------------------------------------------------------------------

class AnnotLink: public Annot {
public:

  enum AnnotLinkEffect {
    effectNone,     // N
    effectInvert,   // I
    effectOutline,  // O
    effectPush      // P
  };

  AnnotLink(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  virtual ~AnnotLink();

  virtual void draw(Gfx *gfx, GBool printing);

  // getters
  Dict *getActionDict() const { return actionDict; }
  // getDest
  AnnotLinkEffect getLinkEffect() const { return linkEffect; }
  Dict *getUriAction() const { return uriAction; }
  AnnotQuadrilaterals *getQuadrilaterals() const { return quadrilaterals; }

protected:

  void initialize(XRef *xrefA, Catalog *catalog, Dict *dict);

  Dict *actionDict;                    // A
  //Dest
  AnnotLinkEffect linkEffect;          // H          (Default I)
  Dict *uriAction;                     // PA

  AnnotQuadrilaterals *quadrilaterals; // QuadPoints
};

//------------------------------------------------------------------------
// AnnotFreeText
//------------------------------------------------------------------------

class AnnotFreeText: public AnnotMarkup {
public:

  enum AnnotFreeTextQuadding {
    quaddingLeftJustified,  // 0
    quaddingCentered,       // 1
    quaddingRightJustified  // 2
  };

  enum AnnotFreeTextIntent {
    intentFreeText,           // FreeText
    intentFreeTextCallout,    // FreeTextCallout
    intentFreeTextTypeWriter  // FreeTextTypeWriter
  };

  AnnotFreeText(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  virtual ~AnnotFreeText();

  // getters
  GooString *getAppearanceString() const { return appearanceString; }
  AnnotFreeTextQuadding getQuadding() const { return quadding; }
  // return rc
  GooString *getStyleString() const { return styleString; }
  AnnotCalloutLine *getCalloutLine() const {  return calloutLine; }
  AnnotFreeTextIntent getIntent() const { return intent; }
  AnnotBorderEffect *getBorderEffect() const { return borderEffect; }
  PDFRectangle *getRectangle() const { return rectangle; }
  AnnotLineEndingStyle getEndStyle() const { return endStyle; }

protected:

  void initialize(XRef *xrefA, Catalog *catalog, Dict *dict);

  // required
  GooString *appearanceString;      // DA

  // optional
  AnnotFreeTextQuadding quadding;   // Q  (Default 0)
  // RC
  GooString *styleString;           // DS
  AnnotCalloutLine *calloutLine;    // CL
  AnnotFreeTextIntent intent;       // IT
  AnnotBorderEffect *borderEffect;  // BE
  PDFRectangle *rectangle;          // RD
  // inherited  from Annot
  // AnnotBorderBS border;          // BS
  AnnotLineEndingStyle endStyle;    // LE (Default None)
};

//------------------------------------------------------------------------
// AnnotLine
//------------------------------------------------------------------------

class AnnotLine: public AnnotMarkup {
public:

  enum AnnotLineIntent {
    intentLineArrow,    // LineArrow
    intentLineDimension // LineDimension
  };

  enum AnnotLineCaptionPos {
    captionPosInline, // Inline
    captionPosTop     // Top
  };

  AnnotLine(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  virtual ~AnnotLine();

  // getters
  AnnotLineEndingStyle getStartStyle() const { return startStyle; }
  AnnotLineEndingStyle getEndStyle() const { return endStyle; }
  AnnotColor *getInteriorColor() const { return interiorColor; }
  double getLeaderLineLength() const { return leaderLineLength; }
  double getLeaderLineExtension() const { return leaderLineExtension; }
  bool getCaption() const { return caption; }
  AnnotLineIntent getIntent() const { return intent; }
  double  getLeaderLineOffset() const { return leaderLineOffset; }
  AnnotLineCaptionPos getCaptionPos() const { return captionPos; }
  Dict *getMeasure() const { return measure; }
  double getCaptionTextHorizontal() const { return captionTextHorizontal; }
  double getCaptionTextVertical() const { return captionTextVertical; }

protected:

  void initialize(XRef *xrefA, Catalog *catalog, Dict *dict);

  // required
  double x1, y1, x2, y2;            // L
  
  // optional
  // inherited  from Annot
  // AnnotBorderBS border;          // BS
  AnnotLineEndingStyle startStyle;  // LE       (Default [/None /None])
  AnnotLineEndingStyle endStyle;    //
  AnnotColor *interiorColor;        // IC
  double leaderLineLength;          // LL       (Default 0)
  double leaderLineExtension;       // LLE      (Default 0)
  bool caption;                     // Cap      (Default false)
  AnnotLineIntent intent;           // IT
  double leaderLineOffset;          // LLO
  AnnotLineCaptionPos captionPos;   // CP       (Default Inline)
  Dict *measure;                    // Measure
  double captionTextHorizontal;     // CO       (Default [0, 0])
  double captionTextVertical;       //
};

//------------------------------------------------------------------------
// AnnotTextMarkup
//------------------------------------------------------------------------

class AnnotTextMarkup: public AnnotMarkup {
public:

  AnnotTextMarkup(XRef *xrefA, Catalog *catalog, Dict *dict);
  virtual ~AnnotTextMarkup();

  AnnotQuadrilaterals *getQuadrilaterals() const { return quadrilaterals; }

protected:

  void initialize(XRef *xrefA, Catalog *catalog, Dict *dict);
  
  AnnotQuadrilaterals *quadrilaterals; // QuadPoints
};

//------------------------------------------------------------------------
// AnnotWidget
//------------------------------------------------------------------------

class AnnotWidget: public Annot {
public:

  enum AnnotWidgetHighlightMode {
    highlightModeNone,    // N
    highlightModeInvert,  // I
    highlightModeOutline, // O
    highlightModePush     // P,T
  };

  AnnotWidget(XRef *xrefA, Dict *dict, Catalog *catalog, Object *obj);
  virtual ~AnnotWidget();

  virtual void draw(Gfx *gfx, GBool printing);

  void generateFieldAppearance ();

  AnnotWidgetHighlightMode getMode() { return mode; }
  AnnotAppearanceCharacs *getAppearCharacs() { return appearCharacs; }
  Dict *getAction() { return action; }
  Dict *getAdditionActions() { return additionActions; }
  Dict *getParent() { return parent; }

private:

  void initialize(XRef *xrefA, Catalog *catalog, Dict *dict);

  void drawText(GooString *text, GooString *da, GfxFontDict *fontDict,
		GBool multiline, int comb, int quadding,
		GBool txField, GBool forceZapfDingbats,
		GBool password=false);
  void drawListBox(GooString **text, GBool *selection,
		   int nOptions, int topIdx,
		   GooString *da, GfxFontDict *fontDict, GBool quadding);
  void layoutText(GooString *text, GooString *outBuf, int *i, GfxFont *font,
		  double *width, double widthLimit, int *charCount,
		  GBool noReencode);
  void writeString(GooString *str, GooString *appearBuf);

  Form *form;
  FormWidget *widget;                     // FormWidget object for this annotation
  AnnotWidgetHighlightMode mode;          // H  (Default I)
  AnnotAppearanceCharacs *appearCharacs;  // MK
  Dict *action;                           // A
  Dict *additionActions;                  // AA
  // inherited  from Annot
  // AnnotBorderBS border;                // BS
  Dict *parent;                           // Parent
  GBool regen;
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

private:
  Annot* createAnnot(XRef *xref, Dict* dict, Catalog *catalog, Object *obj);
  Annot *findAnnot(Ref *ref);

  Annot **annots;
  int nAnnots;
};

#endif
