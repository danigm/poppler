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

#include <Object.h>
#include <SplashOutputDev.h>
#include <Link.h>
#include <PDFDoc.h>
#include <FontInfo.h>
#include <UGooString.h>

namespace Poppler {
    
/* borrowed from kpdf */
static QString unicodeToQString(Unicode* u, int len) {
  QString ret;
  ret.setLength(len);
  QChar* qch = (QChar*) ret.unicode();
  for (;len;--len)
    *qch++ = (QChar) *u++;
  return ret;
}

static UGooString *QStringToUGooString(const QString &s) {
  int len = s.length();
  Unicode *u = (Unicode *)gmallocn(s.length(), sizeof(Unicode));
  for (int i = 0; i < len; ++i)
    u[i] = s.at(i).unicode();
  return new UGooString(u, len);
}
    
class LinkDestinationData {
  public:
     LinkDestinationData( LinkDest *l, UGooString *nd, Poppler::DocumentData *pdfdoc ) : ld(l), namedDest(nd), doc(pdfdoc)
     {
     }
	
     LinkDest *ld;
     UGooString *namedDest;
     Poppler::DocumentData *doc;
};

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
            m_outputDev = new SplashOutputDev(splashModeRGB8Qt, 4, gFalse, white);
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
