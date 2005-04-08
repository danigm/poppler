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

#include <poppler-qt.h>
#include <qfile.h>
#include <qimage.h>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Catalog.h>
#include <ErrorCodes.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#include "poppler-private.h"

namespace Poppler {

class PageData {
  public:
  const Document *doc;
  int index;
};

Page::Page(const Document *doc, int index) {
  data = new PageData();
  data->index = index;
  data->doc = doc;
}

Page::~Page()
{
  delete data;
}

void Page::renderToPixmap(QPixmap **q, int x, int y, int w, int h) const
{
  SplashOutputDev *output_dev;
  SplashColor white;
  SplashBitmap *bitmap;
  white.rgb8 = splashMakeRGB8(255,255,255);
  SplashColorPtr color_ptr;
  output_dev = new SplashOutputDev(splashModeRGB8, gFalse, white);
  output_dev->startDoc(data->doc->data->doc.getXRef ());
  
  data->doc->data->doc.displayPageSlice(output_dev, data->index + 1, 72, 72,
      0, -1, -1, -1, -1,
      true,
      false);
  bitmap = output_dev->getBitmap ();
  color_ptr = bitmap->getDataPtr ();
  QImage * img = new QImage( (uchar*)color_ptr.rgb8, bitmap->getWidth(), bitmap->getHeight(), 32, 0, 0, QImage::IgnoreEndian );
  *q = new QPixmap( *img );
  
  delete img;
  delete output_dev;
}

}
