/* poppler-private.h: qt interface to poppler
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

#include <SplashOutputDev.h>
#include <PDFDoc.h>
#include <FontInfo.h>
#include <Object.h>

namespace Poppler {

class DocumentData {
  public:
    DocumentData(GooString *filePath, GooString *password) : doc(filePath,password), m_outputDev(0) {}

    ~DocumentData()
    {
        delete m_outputDev;
        delete m_fontInfoScanner;
    }

    SplashOutputDev *getOutputDev()
    {
        if (!m_outputDev)
        {
            SplashColor white;
            white[0] = 255;
            white[1] = 255;
            white[2] = 255;
            m_outputDev = new SplashOutputDev(splashModeRGB8, 4, gFalse, white);
            m_outputDev->startDoc(doc.getXRef());
        }
        return m_outputDev;
    }

  class PDFDoc doc;
  bool locked;
  FontInfoScanner *m_fontInfoScanner;
  SplashOutputDev *m_outputDev;
};

}
