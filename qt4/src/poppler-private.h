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

#include <PDFDoc.h>
#include <GfxState.h>
#include <FontInfo.h>
#include <SplashOutputDev.h>

namespace Poppler {

    class DocumentData {
    public:
	DocumentData(GooString *filePath, GooString *ownerPassword, GooString *userPassword) :
	    doc(filePath, ownerPassword, userPassword), m_splashOutputDev(0)
	    {
		// It might be more appropriate to delete these in PDFDoc
		delete ownerPassword;
		delete userPassword;
	    }
	
	~DocumentData()
	{
		qDeleteAll(m_embeddedFiles);
		delete m_splashOutputDev;
		delete m_fontInfoScanner;
	}
	
	SplashOutputDev *getSplashOutputDev()
	{
		if (!m_splashOutputDev)
		{
			SplashColor white;
			white[0] = 255;
			white[1] = 255;
			white[2] = 255;
			m_splashOutputDev = new SplashOutputDev(splashModeRGB8, 4, gFalse, white);
			m_splashOutputDev->startDoc(doc.getXRef());
		}
		return m_splashOutputDev;
	}
	
	class PDFDoc doc;
	bool locked;
	FontInfoScanner *m_fontInfoScanner;
	SplashOutputDev *m_splashOutputDev;
	QList<EmbeddedFile*> m_embeddedFiles;
    };

    class FontInfoData
    {
	public:
		FontInfoData( const FontInfoData &fid )
		{
			fontName = fid.fontName;
			fontFile = fid.fontFile;
			isEmbedded = fid.isEmbedded;
			isSubset = fid.isSubset;
			type = fid.type;
		}
		
		FontInfoData( ::FontInfo* fi )
		{
			if (fi->getName()) fontName = fi->getName()->getCString();
			if (fi->getFile()) fontFile = fi->getFile()->getCString();
			isEmbedded = fi->getEmbedded();
			isSubset = fi->getSubset();
			type = (Poppler::FontInfo::Type)fi->getType();
		}

		QString fontName;
		QString fontFile;
		bool isEmbedded;
		bool isSubset;
		FontInfo::Type type;
    };

}


