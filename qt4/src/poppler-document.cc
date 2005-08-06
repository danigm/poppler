/* poppler-document.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
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
#include <QtCore/QByteArray>
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <Catalog.h>
#include <ErrorCodes.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#include "poppler-private.h"

namespace Poppler {

  Document *Document::load(const QString &filePath, const QByteArray &ownerPassword,
			   const QByteArray &userPassword)
    {
	if (!globalParams) {
	    globalParams = new GlobalParams("/etc/xpdfrc");
	}

	DocumentData *doc = new DocumentData(new GooString(QFile::encodeName(filePath)), 
					     new GooString(ownerPassword.data()),
					     new GooString(userPassword.data()));
	Document *pdoc;
	if (doc->doc.isOk() || doc->doc.getErrorCode() == errEncrypted) {
	    pdoc = new Document(doc);
	    if (doc->doc.getErrorCode() == errEncrypted)
		pdoc->m_doc->locked = true;
	    else
		pdoc->m_doc->locked = false;
	    pdoc->m_doc->m_fontInfoScanner = new FontInfoScanner(&(doc->doc));
	    return pdoc;
	}
	else
	    return NULL;

    }

    Document::Document(DocumentData *dataA)
    {
	m_doc = dataA;
    }

    Document::~Document()
    {
	delete m_doc;
    }

    bool Document::isLocked() const
    {
	return m_doc->locked;
    }

    bool Document::unlock(const QByteArray &ownerPassword,
			  const QByteArray &userPassword)
    {
	if (m_doc->locked) {
	    /* racier then it needs to be */
	    GooString *ownerPwd = new GooString(ownerPassword.data());
	    GooString *userPwd = new GooString(userPassword.data());
	    DocumentData *doc2 = new DocumentData(m_doc->doc.getFileName(),
						  ownerPwd,
						  userPwd);
	    delete ownerPwd;
	    delete userPwd;
	    if (!doc2->doc.isOk()) {
		delete doc2;
	    } else {
		delete m_doc;
		m_doc = doc2;
		m_doc->locked = false;
	    }
	}
	return m_doc->locked;
    }

    Document::PageMode Document::pageMode(void) const
    {
	switch (m_doc->doc.getCatalog()->getPageMode()) {
	case Catalog::pageModeNone:
	    return UseNone;
	case Catalog::pageModeOutlines:
	    return UseOutlines;
	case Catalog::pageModeThumbs:
	    return UseThumbs;
	case Catalog::pageModeFullScreen:
	    return FullScreen;
	case Catalog::pageModeOC:
	    return UseOC;
	case Catalog::pageModeAttach:
	    return UseAttach;
	default:
	    return UseNone;
	}
    }

    Document::PageLayout Document::pageLayout(void) const
    {
	switch (m_doc->doc.getCatalog()->getPageLayout()) {
	case Catalog::pageLayoutNone:
	    return NoLayout;
	case Catalog::pageLayoutSinglePage:
	    return SinglePage;
	case Catalog::pageLayoutOneColumn:
	    return OneColumn;
	case Catalog::pageLayoutTwoColumnLeft:
	    return TwoColumnLeft;
	case Catalog::pageLayoutTwoColumnRight:
	    return TwoColumnRight;
	case Catalog::pageLayoutTwoPageLeft:
	    return TwoPageLeft;
	case Catalog::pageLayoutTwoPageRight:
	    return TwoPageRight;
	default:
	    return NoLayout;
	}
    }

    int Document::numPages() const
    {
	return m_doc->doc.getNumPages();
    }

    QList<FontInfo> Document::fonts() const
    {
	QList<FontInfo> ourList;
	scanForFonts(numPages(), &ourList);
	return ourList;
    }

    bool Document::scanForFonts( int numPages, QList<FontInfo> *fontList ) const
    {
	GooList *items = m_doc->m_fontInfoScanner->scan( numPages );

	if ( NULL == items )
	    return false;

	for ( int i = 0; i < items->getLength(); ++i ) {
	    if (((::FontInfo*)items->get(i))->getName())
		fontList->append(FontInfo(((::FontInfo*)items->get(i))->getName()->getCString(),
			      ((::FontInfo*)items->get(i))->getEmbedded(),
			      ((::FontInfo*)items->get(i))->getSubset(),
			      (Poppler::FontInfo::Type)((::FontInfo*)items->get(i))->getType()
		));
	    else
		fontList->append(FontInfo(QString::null,
			      ((::FontInfo*)items->get(i))->getEmbedded(),
			      ((::FontInfo*)items->get(i))->getSubset(),
			      (Poppler::FontInfo::Type)((::FontInfo*)items->get(i))->getType()
		));
	}
	return true;
    }


    /* borrowed from kpdf */
    static QString unicodeToQString(Unicode* u, int len) {
	QString ret;
	ret.resize(len);
	QChar* qch = (QChar*) ret.unicode();
	for (;len;--len)
	    *qch++ = (QChar) *u++;
	return ret;
    }

    /* borrowed from kpdf */
    QString Document::info( const QString & type ) const
    {
	// [Albert] Code adapted from pdfinfo.cc on xpdf
	Object info;
	if ( m_doc->locked )
	    return NULL;

	m_doc->doc.getDocInfo( &info );
	if ( !info.isDict() )
	    return NULL;

	QString result;
	Object obj;
	GooString *s1;
	GBool isUnicode;
	Unicode u;
	int i;
	Dict *infoDict = info.getDict();

	if ( infoDict->lookup( type.toLatin1().data(), &obj )->isString() )
	{
	    s1 = obj.getString();
	    if ( ( s1->getChar(0) & 0xff ) == 0xfe && ( s1->getChar(1) & 0xff ) == 0xff )
	    {
		isUnicode = gTrue;
		i = 2;
	    }
	    else
	    {
		isUnicode = gFalse;
		i = 0;
	    }
	    while ( i < obj.getString()->getLength() )
	    {
		if ( isUnicode )
		{
		    u = ( ( s1->getChar(i) & 0xff ) << 8 ) | ( s1->getChar(i+1) & 0xff );
		    i += 2;
		}
		else
		{
		    u = s1->getChar(i) & 0xff;
		    ++i;
		}
		result += unicodeToQString( &u, 1 );
	    }
	    obj.free();
	    info.free();
	    return result;
	}
	obj.free();
	info.free();
	return NULL;
    }

    /* borrowed from kpdf */
    QDateTime Document::date( const QString & type ) const
    {
	// [Albert] Code adapted from pdfinfo.cc on xpdf
	if ( m_doc->locked )
	    return QDateTime();

	Object info;
	m_doc->doc.getDocInfo( &info );
	if ( !info.isDict() ) {
	    info.free();
	    return QDateTime();
	}

	Object obj;
	char *s;
	int year, mon, day, hour, min, sec;
	Dict *infoDict = info.getDict();
	QString result;

	if ( infoDict->lookup( type.toLatin1().data(), &obj )->isString() )
	{
	    s = obj.getString()->getCString();
	    if ( s[0] == 'D' && s[1] == ':' )
		s += 2;
	    /* FIXME process time zone on systems that support it */  
	    if ( sscanf( s, "%4d%2d%2d%2d%2d%2d", &year, &mon, &day, &hour, &min, &sec ) == 6 )
	    {
		/* Workaround for y2k bug in Distiller 3 stolen from gpdf, hoping that it won't
		 *   * be used after y2.2k */
		if ( year < 1930 && strlen (s) > 14) {
		    int century, years_since_1900;
		    if ( sscanf( s, "%2d%3d%2d%2d%2d%2d%2d",
				 &century, &years_since_1900,
				 &mon, &day, &hour, &min, &sec) == 7 )
			year = century * 100 + years_since_1900;
		    else {
			obj.free();
			info.free();
			return QDateTime();
		    }
		}

		QDate d( year, mon, day );  //CHECK: it was mon-1, Jan->0 (??)
		QTime t( hour, min, sec );
		if ( d.isValid() && t.isValid() ) {
		    obj.free();
		    info.free();
		    return QDateTime( d, t );
		}
	    }
	}
	obj.free();
	info.free();
	return QDateTime();
    }

    bool Document::isEncrypted() const
    {
	return m_doc->doc.isEncrypted();
    }

    bool Document::isLinearized() const
    {
	return m_doc->doc.isLinearized();
    }

    bool Document::okToPrint() const
    {
	return m_doc->doc.okToPrint();
    }

    bool Document::okToPrintHighRes() const
    {
	return m_doc->doc.okToPrintHighRes();
    }

    bool Document::okToChange() const
    {
	return m_doc->doc.okToChange();
    }

    bool Document::okToCopy() const
    {
	return m_doc->doc.okToCopy();
    }

    bool Document::okToAddNotes() const
    {
	return m_doc->doc.okToAddNotes();
    }

    bool Document::okToFillForm() const
    {
	return m_doc->doc.okToFillForm();
    }

    bool Document::okToCreateFormFields() const
    {
	return ( okToFillForm() && okToChange() );
    }

    bool Document::okToExtractForAccessibility() const
    {
	return m_doc->doc.okToAccessibility();
    }

    bool Document::okToAssemble() const
    {
	return m_doc->doc.okToAssemble();
    }

    double Document::pdfVersion() const
    {
	return m_doc->doc.getPDFVersion();
    }

    Page *Document::page(QString label) const
    {
	GooString label_g(label.toAscii().data());
	int index;

	if (!m_doc->doc.getCatalog()->labelToIndex (&label_g, &index))
	    return NULL;

	return page(index);
    }

}
