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
#include <Catalog.h>
#include <ErrorCodes.h>
#include <SplashOutputDev.h>
#include <TextOutputDev.h>
#include <splash/SplashBitmap.h>
#include "poppler-private.h"

namespace Poppler {

class PageTransitionData {
  public:
  Object *trans;
};

//------------------------------------------------------------------------
// PageTransition
//------------------------------------------------------------------------

PageTransition::PageTransition(const PageTransitionData &data)
  : type(Replace),
    duration(1),
    alignment(Horizontal),
    direction(Inward),
    angle(0),
    scale(1.0),
    rectangular(false)
{
  Object obj;
  Object *dictObj = data.trans;

  if (dictObj->isDict()) {
    Dict *transDict = dictObj->getDict();

    if (transDict->lookup("S", &obj)->isName()) {
      const char *s = obj.getName();
      if (strcmp("R", s) == 0)
        type = Replace;
      else if (strcmp("Split", s) == 0)
        type = Split;
      else if (strcmp("Blinds", s) == 0)
        type = Blinds;
      else if (strcmp("Box", s) == 0)
        type = Box;
      else if (strcmp("Wipe", s) == 0)
        type = Wipe;
      else if (strcmp("Dissolve", s) == 0)
        type = Dissolve;
      else if (strcmp("Glitter", s) == 0)
        type = Glitter;
      else if (strcmp("Fly", s) == 0)
        type = Fly;
      else if (strcmp("Push", s) == 0)
        type = Push;
      else if (strcmp("Cover", s) == 0)
        type = Cover;
      else if (strcmp("Uncover", s) == 0)
        type = Push;
      else if (strcmp("Fade", s) == 0)
        type = Cover;
    }
    obj.free();

    if (transDict->lookup("D", &obj)->isInt()) {
      duration = obj.getInt();
    }
    obj.free();

    if (transDict->lookup("Dm", &obj)->isName()) {
      const char *dm = obj.getName();
      if ( strcmp( "H", dm ) == 0 )
        alignment = Horizontal;
      else if ( strcmp( "V", dm ) == 0 )
        alignment = Vertical;
    }
    obj.free();

    if (transDict->lookup("M", &obj)->isName()) {
      const char *m = obj.getName();
      if ( strcmp( "I", m ) == 0 )
        direction = Inward;
      else if ( strcmp( "O", m ) == 0 )
        direction = Outward;
    }
    obj.free();

    if (transDict->lookup("Di", &obj)->isInt()) {
      angle = obj.getInt();
    }
    obj.free();

    if (transDict->lookup("Di", &obj)->isName()) {
      if ( strcmp( "None", obj.getName() ) == 0 )
        angle = 0;
    }
    obj.free();

    if (transDict->lookup("SS", &obj)->isReal()) {
      scale = obj.getReal();
    }
    obj.free();

    if (transDict->lookup("B", &obj)->isBool()) {
      rectangular = obj.getBool();
    }
    obj.free();
  }
}

PageTransition::~PageTransition()
{
}


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
  SplashOutputDev *output_dev;
  SplashBitmap *bitmap;
  SplashColor white;
  white[0] = 255;
  white[1] = 255;
  white[2] = 255;
  SplashColorPtr color_ptr;
  output_dev = new SplashOutputDev(splashModeRGB8, 4, gFalse, white);
  output_dev->startDoc(data->doc->data->doc.getXRef ());
  
  data->doc->data->doc.displayPageSlice(output_dev, data->index + 1, 72, 72,
      0, false, false, false, -1, -1, -1, -1);
  bitmap = output_dev->getBitmap ();
  color_ptr = bitmap->getDataPtr ();
  int bw = output_dev->getBitmap()->getWidth();
  int bh = output_dev->getBitmap()->getHeight();
  QImage * img = new QImage( bw, bh, 32 );
  SplashColorPtr pixel = new Guchar[4];
  for (int i = 0; i < bw; i++)
  {
    for (int j = 0; j < bh; j++)
    {
      output_dev->getBitmap()->getPixel(i, j, pixel);
      img->setPixel( i, j, qRgb( pixel[0], pixel[1], pixel[2] ) );
    }
  }
  delete[] pixel;
  *q = new QPixmap( *img );
  
  delete img;
  delete output_dev;
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

PageTransition *Page::getTransition() const
{
  if (!data->transition) 
  {
    Object o;
    PageTransitionData ptd;
    ptd.trans = data->doc->data->doc.getCatalog()->getPage(data->index + 1)->getTrans(&o);
    data->transition = new PageTransition(ptd);
    o.free();
  }
  return data->transition;
}

}
