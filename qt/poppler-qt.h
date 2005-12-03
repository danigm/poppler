/* poppler-qt.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, Tobias Koening
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __POPPLER_QT_H__
#define __POPPLER_QT_H__

#include <qcstring.h>
#include <qdatetime.h>
#include <qpixmap.h>

namespace Poppler {

class Document;
class Page;

/* A rectangle on a page, with coordinates in PDF points. */
class Rectangle
{
  public:
    Rectangle(double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0) : 
      m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {}
    bool isNull() const { return m_x1 == 0 && m_y1 == 0 && m_x2 == 0 && m_y2 == 0; }
  
    double m_x1;
    double m_y1;
    double m_x2;
    double m_y2;
};

class TextBox
{
 public:
    TextBox(const QString& text, const Rectangle &bBox) :
    m_text(text), m_bBox(bBox) {};

    QString getText() const { return m_text; };
    Rectangle getBoundingBox() const { return m_bBox; };

  private:
    QString m_text;
    Rectangle m_bBox;
};


/**
  Container class for information about a font within a PDF document
*/
class FontInfoData;
class FontInfo {
public:
  enum Type {
    unknown,
    Type1,
    Type1C,
    Type3,
    TrueType,
    CIDType0,
    CIDType0C,
    CIDTrueType
  };

  /**
    Create a new font information container
  */
  FontInfo( const QString &fontName, const bool isEmbedded,
            const bool isSubset, Type type );

  FontInfo();

  ~FontInfo();

  /**
    The name of the font. Can be QString::null if the font has no name
  */
  const QString &name() const;

  /**
    Whether the font is embedded in the file, or not

    \return true if the font is embedded
  */
  bool isEmbedded() const;

  /**
    Whether the font provided is only a subset of the full
    font or not. This only has meaning if the font is embedded.

    \return true if the font is only a subset
  */
  bool isSubset() const;

  /**
    The type of font encoding
  */
  Type type() const;

  const QString &typeName() const;

private:
  FontInfoData *data;
};

class PageTransitionData;
class PageTransition
{
friend class Page;
public:
  enum Type {
    Replace,
    Split,
    Blinds,
    Box,
    Wipe,
    Dissolve,
    Glitter,
    Fly,
    Push,
    Cover,
    Uncover,
    Fade
  };

  enum Alignment {
    Horizontal,
    Vertical
  };

  enum Direction {
    Inward,
    Outward
  };


  // Destructor
  ~PageTransition();

  // Get type of the transition.
  Type getType() const { return type; }

  // Get duration of the transition in seconds.
  int getDuration() const { return duration; }

  // Get dimension in which the transition effect
  // occurs.
  Alignment getAlignment() const { return alignment; }

  // Get direction of motion of the transition effect.
  Direction getDirection() const { return direction; }

  // Get direction in which the transition effect moves.
  int getAngle() const { return angle; }

  // Get starting or ending scale.
  double getScale() const { return scale; }

  // Returns true if the area to be flown is rectangular and
  // opaque.
  bool isRectangular() const { return rectangular; }

private:
  // Construct a new PageTransition object from a page dictionary.
  PageTransition( const PageTransitionData &data );

  Type type;
  int duration;
  Alignment alignment;
  Direction direction;
  int angle;
  double scale;
  bool rectangular;
};

class PageData;
class Page {
  friend class Document;
  public:
    ~Page();
    void renderToPixmap(QPixmap **q, int x, int y, int w, int h, double xres, double yres) const;

    /**
     * This is a convenience function that is equivalent to
     * renderToPixmap() with xres and yres set to 72.0. We keep it
     * only for binary compatibility
     **/
    void renderToPixmap(QPixmap **q, int x, int y, int w, int h) const;
    
    /**
     * Returns the size of the page in points
     **/
    QSize pageSize() const;

    /**
    * Returns the text that is inside the Rectangle r
    * If r is a null Rectangle all text of the page is given
    **/
    QString getText(const Rectangle &r) const;

    QValueList<TextBox*> textList() const;

    /**
    * Returns the transition of this page
    **/
    PageTransition *getTransition() const;

    enum Orientation {
      Landscape,
      Portrait,
      Seascape,
      UpsideDown
    };

    /**
    *  The orientation of the page
    **/
    Orientation orientation() const;
    
  private:
    Page(const Document *doc, int index);
    PageData *data;
};

class DocumentData;

class Document {
  friend class Page;
  
public:
  enum PageMode {
    UseNone,
    UseOutlines,
    UseThumbs,
    FullScreen,
    UseOC
  };
  
  static Document *Document::load(const QString & filePath);
  
  Page *getPage(int index) const{ return new Page(this, index); }
  
  int getNumPages() const;
  
  PageMode getPageMode() const;
  
  bool unlock(const QCString &password);
  
  bool isLocked() const;
  
  QDateTime getDate( const QString & data ) const;
  QString getInfo( const QString & data ) const;
  bool isEncrypted() const;
  bool isLinearized() const;
  bool okToPrint() const;
  bool okToChange() const;
  bool okToCopy() const;
  bool okToAddNotes() const;
  double getPDFVersion() const;

  /**
    The fonts within the PDF document.

    \note this can take a very long time to run with a large
    document. You may wish to use the call below if you have more
    than say 20 pages
  */
  QValueList<FontInfo> fonts() const;

  /**
    \overload

    \param numPages the number of pages to scan
    \param fontList pointer to the list where the font information
    should be placed

    \return false if the end of the document has been reached
  */
  bool scanForFonts( int numPages, QValueList<FontInfo> *fontList ) const;

  Document::~Document();
  
private:
  DocumentData *data;
  Document::Document(DocumentData *dataA);
};

}
#endif
