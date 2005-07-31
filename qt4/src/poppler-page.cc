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
#include <TextOutputDev.h>
#include "poppler-private.h"

#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>

namespace Poppler {

class PageData {
  public:
  const Document *parentDoc;
  int index;
};

Page::Page(const Document *doc, int index) {
  m_page = new PageData();
  m_page->index = index;
  m_page->parentDoc = doc;
}

Page::~Page()
{
  delete m_page;
}


void Page::splashRenderToPixmap(QPixmap **q, int x, int y, int w, int h) const
{
  SplashOutputDev *output_dev;
  SplashColor white;
  SplashBitmap *bitmap;
  white.rgb8 = splashMakeRGB8(255,255,255);
  SplashColorPtr color_ptr;
  output_dev = new SplashOutputDev(splashModeRGB8, gFalse, white);
  output_dev->startDoc(m_page->parentDoc->m_doc->doc.getXRef ());
  
  m_page->parentDoc->m_doc->doc.displayPageSlice(output_dev, m_page->index + 1, 72, 72,
      0, -1, -1, -1, -1,
      true,
      false);
  bitmap = output_dev->getBitmap ();
  color_ptr = bitmap->getDataPtr ();

  QImage * img = new QImage( (uchar*)color_ptr.rgb8, bitmap->getWidth(), bitmap->getHeight(), QImage::Format_RGB32 );
  *q = new QPixmap(QPixmap::fromImage(*img));
  
  delete img;
  delete output_dev;
}

void Page::renderToPixmap(QPixmap *pixmap) const
{
  QPainter* painter = new QPainter(pixmap);
  painter->setRenderHint(QPainter::Antialiasing);
  ArthurOutputDev output_dev(painter);
  output_dev.startDoc(m_page->parentDoc->m_doc->doc.getXRef ());
  m_page->parentDoc->m_doc->doc.displayPageSlice(&output_dev,
						 m_page->index + 1,
						 72,
						 72,
						 0,
						 -1,
						 -1,
						 -1,
						 -1,
						 true,
						 false);
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
      0, -1, -1, -1, -1,
      true,
      false);
  p = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1);
  if (r.isNull())
  {
    rect = p->getBox();
    s = output_dev->getText(rect->x1, rect->y1, rect->x2, rect->y2);
  }
  else
  {
    double height, y1, y2;
    height = p->getHeight();
    y1 = height - r.top();
    y2 = height - r.bottom();
    s = output_dev->getText(r.left(), y1, r.right(), y2);
  }

  result = QString::fromUtf8(s->getCString());

  delete output_dev;
  delete s;
  return result;
}

QSizeF Page::pageSizeF() const
{
  ::Page *p;
  
  p = m_page->parentDoc->m_doc->doc.getCatalog()->getPage(m_page->index + 1);
  if ( ( Page::Landscape == orientation() ) || (Page::Seascape == orientation() ) ) {
      return QSizeF( p->getHeight(), p->getWidth() );
  } else {
    return QSizeF( p->getWidth(), p->getHeight() );
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
