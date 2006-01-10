/* poppler-page.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
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

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>
#include <QtCore/QFile>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Catalog.h>
#include <ErrorCodes.h>
#include <ArthurOutputDev.h>
#include <SplashOutputDev.h>
#include <TextOutputDev.h>
#include <splash/SplashBitmap.h>

#include "poppler-private.h"
#include "poppler-page-transition-private.h"

namespace Poppler {

class PageData {
  public:
  const Document *parentDoc;
  int index;
  PageTransition *transition;
};

Page::Page(const Document *doc, int index) {
  m_page = new PageData();
  m_page->index = index;
  m_page->parentDoc = doc;
  m_page->transition = 0;
}

Page::~Page()
{
  delete m_page->transition;
  delete m_page;
}

QImage Page::splashRenderToImage(double xres, double yres, int x, int y, int w, int h) const
{
  SplashOutputDev *output_dev = m_page->parentDoc->m_doc->getSplashOutputDev();
  
  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, xres, yres,
						 0, false, true, false, x, y, w, h);
  
  SplashBitmap *bitmap = output_dev->getBitmap ();
  int bw = bitmap->getWidth();
  int bh = bitmap->getHeight();
  
  // Produce a QImage and copy the image there pixel-by-pixel. This is
  // quite slow and rather ugly, and the following two lines would be
  // much nicer. But it does not work since the change to xpdf 3.01
  // it's worth investigating

  // -------- 
  //  SplashColorPtr color_ptr = bitmap->getDataPtr ();
  //  QImage *img = new QImage( (uchar*)color_ptr, bw, bh, QImage::Format_RGB32 );
  // --------

  QImage img( bw, bh, QImage::Format_RGB32 );
  SplashColorPtr pixel = new Guchar[4];
  for (int i = 0; i < bw; i++) {
    for (int j = 0; j < bh; j++) {
      output_dev->getBitmap()->getPixel(i, j, pixel);
      img.setPixel( i, j, qRgb( pixel[0], pixel[1], pixel[2] ) );
    }
  }
  delete[] pixel;

  return img;
}

QPixmap *Page::splashRenderToPixmap(double xres, double yres, int x, int y, int w, int h) const
{
  QImage img = splashRenderToImage(xres, yres, x, y, w, h);

  // Turn the QImage into a QPixmap
  QPixmap* out = new QPixmap(QPixmap::fromImage(img));

  return out;
}

void Page::renderToPixmap(QPixmap *pixmap, double xres, double yres) const
{
  QPainter* painter = new QPainter(pixmap);
  painter->setRenderHint(QPainter::Antialiasing);
  ArthurOutputDev output_dev(painter);
  output_dev.startDoc(m_page->parentDoc->m_doc->doc.getXRef ());
  m_page->parentDoc->m_doc->doc.displayPageSlice(&output_dev,
						 m_page->index + 1,
						 xres,
						 yres,
						 0,
						 false,
						 true,
						 false,
						 -1,
						 -1,
						 -1,
						 -1);
  painter->end();
}

QString Page::text(const QRectF &r) const
{
  TextOutputDev *output_dev;
  GooString *s;
  PDFRectangle *rect;
  QString result;
  ::Page *p;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);
  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      0, false, true, false, -1, -1, -1, -1);
  p = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1);
  if (r.isNull())
  {
    rect = p->getCropBox();
    s = output_dev->getText(rect->x1, rect->y1, rect->x2, rect->y2);
  }
  else
  {
    double height, y1, y2;
    height = p->getCropHeight();
    y1 = height - r.top();
    y2 = height - r.bottom();
    s = output_dev->getText(r.left(), y1, r.right(), y2);
  }

  result = QString::fromUtf8(s->getCString());

  delete output_dev;
  delete s;
  return result;
}

QList<TextBox*> Page::textList() const
{
  TextOutputDev *output_dev;
  
  QList<TextBox*> output_list;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);

  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      0, false, false, false, -1, -1, -1, -1);

  TextWordList *word_list = output_dev->makeWordList();
  
  if (!word_list) {
    delete output_dev;
    return output_list;
  }
  
  for (int i = 0; i < word_list->getLength(); i++) {
    TextWord *word = word_list->get(i);
    QString string = QString::fromUtf8(word->getText()->getCString());
    double xMin, yMin, xMax, yMax;
    word->getBBox(&xMin, &yMin, &xMax, &yMax);
    
    TextBox* text_box = new TextBox(string, QRectF(xMin, yMin, xMax-xMin, yMax-yMin));
    
    output_list.append(text_box);
  }
  
  delete word_list;
  delete output_dev;
  
  return output_list;
}

PageTransition *Page::transition() const
{
  if (!m_page->transition) {
    Object o;
    PageTransitionParams params;
    params.dictObj = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1)->getTrans(&o);
    m_page->transition = new PageTransition(params);
    o.free();
  }
  return m_page->transition;
}

QSizeF Page::pageSizeF() const
{
  ::Page *p;
  
  p = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1);
  if ( ( Page::Landscape == orientation() ) || (Page::Seascape == orientation() ) ) {
      return QSizeF( p->getCropHeight(), p->getCropWidth() );
  } else {
    return QSizeF( p->getCropWidth(), p->getCropHeight() );
  }
}

QSize Page::pageSize() const
{
  return QSize( (int)pageSizeF().width(), (int)pageSizeF().height() );
}

Page::Orientation Page::orientation() const
{
  int rotation = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1)->getRotate();
  switch (rotation) {
  case 90:
    return Page::Landscape;
    break;
  case 180:
    return Page::UpsideDown;
    break;
  case 270:
    return Page::Seascape;
    break;
  default:
    return Page::Portrait;
  }
}

}
