/* poppler-page.cc: qt interface to poppler
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

#include <poppler-qt.h>
#include <qfile.h>
#include <qimage.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Private.h>
#include <Catalog.h>
#include <ErrorCodes.h>
#include <SplashOutputDev.h>
#include <TextOutputDev.h>
#include <splash/SplashBitmap.h>
#include "poppler-private.h"

namespace Poppler {

class PageData {
  public:
  const Document *doc;
  int index;
  PageTransition *transition;
};

Page::Page(const Document *doc, int index) {
  data = new PageData();
  data->index = index;
  data->doc = doc;
  data->transition = 0;
}

Page::~Page()
{
  delete data->transition;
  delete data;
}

void Page::renderToPixmap(QPixmap **q, int x, int y, int w, int h) const
{
  renderToPixmap(q, x, y, w, h, 72.0, 72.0);
}

void Page::renderToPixmap(QPixmap **q, int x, int y, int w, int h, double xres, double yres) const
{
  QImage img = renderToImage(xres, yres);
  *q = new QPixmap( img );
}

QImage Page::renderToImage(double xres, double yres) const
{
  SplashOutputDev *output_dev;
  SplashBitmap *bitmap;
  SplashColorPtr color_ptr;
  output_dev = data->doc->data->getOutputDev();

  data->doc->data->doc.displayPageSlice(output_dev, data->index + 1, xres, yres,
      0, false, false, false, -1, -1, -1, -1);
  bitmap = output_dev->getBitmap ();
  color_ptr = bitmap->getDataPtr ();
  int bw = output_dev->getBitmap()->getWidth();
  int bh = output_dev->getBitmap()->getHeight();

  QImage img( bw, bh, 32 );
  SplashColorPtr pixel = new Guchar[4];
  for (int i = 0; i < bw; i++)
  {
    for (int j = 0; j < bh; j++)
    {
      output_dev->getBitmap()->getPixel(i, j, pixel);
      img.setPixel( i, j, qRgb( pixel[0], pixel[1], pixel[2] ) );
    }
  }
  delete[] pixel;

  return img;
}

QString Page::getText(const Rectangle &r) const
{
  TextOutputDev *output_dev;
  GooString *s;
  PDFRectangle *rect;
  QString result;
  ::Page *p;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);
  data->doc->data->doc.displayPageSlice(output_dev, data->index + 1, 72, 72,
      0, false, false, false, -1, -1, -1, -1);
  p = data->doc->data->doc.getCatalog()->getPage(data->index + 1);
  if (r.isNull())
  {
    rect = p->getCropBox();
    s = output_dev->getText(rect->x1, rect->y1, rect->x2, rect->y2);
  }
  else
  {
    double height, y1, y2;
    height = p->getCropHeight();
    y1 = height - r.m_y2;
    y2 = height - r.m_y1;
    s = output_dev->getText(r.m_x1, y1, r.m_x2, y2);
  }

  result = QString::fromUtf8(s->getCString());

  delete output_dev;
  delete s;
  return result;
}

QValueList<TextBox*> Page::textList() const
{
  TextOutputDev *output_dev;
  
  QValueList<TextBox*> output_list;
  
  output_dev = new TextOutputDev(0, gFalse, gFalse, gFalse);

  data->doc->data->doc.displayPageSlice(output_dev, data->index + 1, 72, 72,
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
    
    TextBox* text_box = new TextBox(string, Rectangle(xMin, yMin, xMax, yMax));
    
    output_list.append(text_box);
  }
  
  delete word_list;
  delete output_dev;
  
  return output_list;
}

PageTransition *Page::getTransition() const
{
  if (!data->transition) 
  {
    Object o;
    PageTransitionParams params;
    params.dictObj = data->doc->data->doc.getCatalog()->getPage(data->index + 1)->getTrans(&o);
    data->transition = new PageTransition(params);
    o.free();
  }
  return data->transition;
}

QSize Page::pageSize() const
{
  ::Page *p = data->doc->data->doc.getCatalog()->getPage(data->index + 1);
  return QSize( (int)p->getMediaWidth(), (int)p->getMediaHeight() );
}

Page::Orientation Page::orientation() const
{
  ::Page *p = data->doc->data->doc.getCatalog()->getPage(data->index + 1);

  int rotation = p->getRotate();
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
