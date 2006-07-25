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

#include "poppler-qt4.h"

#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <Outline.h>
#include <PDFDoc.h>
#include <PSOutputDev.h>
#include <Stream.h>
#include <UGooString.h>
#include <Catalog.h>

#include <splash/SplashBitmap.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtGui/QImage>
#include <QtCore/QByteArray>

#include "poppler-private.h"

namespace Poppler {

  int DocumentData::count = 0;

  Document *Document::load(const QString &filePath, const QByteArray &ownerPassword,
			   const QByteArray &userPassword)
    {
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
	    int numEmb = doc->doc.getCatalog()->numEmbeddedFiles();
	    if (!(0 == numEmb)) {
		// we have some embedded documents, build the list
		for (int yalv = 0; yalv < numEmb; ++yalv) {
		    EmbFile *ef = doc->doc.getCatalog()->embeddedFile(yalv);
		    pdoc->m_doc->m_embeddedFiles.append(new EmbeddedFile(ef));
		    delete ef;
		}
	    }
	    return pdoc;
	}
	else
	    delete doc;
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

    const QList<EmbeddedFile*> &Document::embeddedFiles() const
    {
	return m_doc->m_embeddedFiles;
    }

    bool Document::scanForFonts( int numPages, QList<FontInfo> *fontList ) const
    {
	GooList *items = m_doc->m_fontInfoScanner->scan( numPages );

	if ( NULL == items )
	    return false;

	for ( int i = 0; i < items->getLength(); ++i ) {
	    fontList->append( FontInfo(FontInfoData((::FontInfo*)items->get(i))) );
	}
	deleteGooList(items, ::FontInfo);
	return true;
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

    QStringList Document::infoKeys() const
    {
	QStringList keys;

	Object info;
	if ( m_doc->locked )
	    return QStringList();

	m_doc->doc.getDocInfo( &info );
	if ( !info.isDict() )
	    return QStringList();

	Dict *infoDict = info.getDict();
	// somehow iterate over keys in infoDict
	for( int i=0; i < infoDict->getLength(); ++i ) {
	    keys.append( QString::fromAscii(infoDict->getKey(i)->getCString()) );
	}

	info.free();
	return keys;
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
	Dict *infoDict = info.getDict();
	QDateTime result;

	if ( infoDict->lookup( type.toLatin1().data(), &obj )->isString() )
	{
	    char *aux = UGooString(*obj.getString()).getCString();
	    result = Poppler::convertDate(aux);
	    delete[] aux;
	}
	obj.free();
	info.free();
	return result;
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

    bool Document::hasEmbeddedFiles() const
    {
	return (!(0 == m_doc->doc.getCatalog()->numEmbeddedFiles()));
    }
    
    QDomDocument *Document::toc() const
    {
        Outline * outline = m_doc->doc.getOutline();
        if ( !outline )
            return NULL;

        GooList * items = outline->getItems();
        if ( !items || items->getLength() < 1 )
            return NULL;

        QDomDocument *toc = new QDomDocument();
        if ( items->getLength() > 0 )
           m_doc->addTocChildren( toc, toc, items );

        return toc;
    }

    LinkDestination *Document::linkDestination( const QString &name )
    {
        UGooString * namedDest = QStringToUGooString( name );
        LinkDestinationData ldd(NULL, namedDest, m_doc);
        LinkDestination *ld = new LinkDestination(ldd);
        delete namedDest;
        return ld;
    }
    
    bool Document::print(const QString &file, const QList<int> pageList, double hDPI, double vDPI, int rotate, int paperWidth, int paperHeight)
    {
        PSOutputDev *psOut = new PSOutputDev(file.toLatin1().data(), m_doc->doc.getXRef(), m_doc->doc.getCatalog(), 1, m_doc->doc.getNumPages(), psModePS, paperWidth, paperHeight, gFalse);

        if (psOut->isOk())
        {
            foreach(int page, pageList)
            {
                m_doc->doc.displayPage(psOut, page, hDPI, vDPI, rotate, gFalse, globalParams->getPSCrop(), gFalse);
            }

            delete psOut;
            return true;
        }
        else
        {
            delete psOut;
            return false;
        }
    }
    
    void Document::setPaperColor(const QColor &color)
    {
        m_doc->setPaperColor(color);
    }
    
    QColor Document::paperColor() const
    {
    	return m_doc->paperColor;
    }

    QDateTime convertDate( char *dateString )
    {
        int year;
        int mon = 1;
        int day = 1;
        int hour = 0;
        int min = 0;
        int sec = 0;
        char tz = 0x00;
        int tzHours = 0;
        int tzMins = 0;

        if ( dateString[0] == 'D' && dateString[1] == ':' )
            dateString += 2;
        if ( sscanf( dateString,
		     "%4d%2d%2d%2d%2d%2d%c%2d%*c%2d",
		     &year, &mon, &day, &hour, &min, &sec,
		     &tz, &tzHours, &tzMins ) > 0 ) {
            /* Workaround for y2k bug in Distiller 3 stolen from gpdf, hoping that it won't
             * be used after y2.2k */
            if ( year < 1930 && strlen (dateString) > 14) {
                int century, years_since_1900;
                if ( sscanf( dateString, "%2d%3d%2d%2d%2d%2d%2d",
                             &century, &years_since_1900,
                             &mon, &day, &hour, &min, &sec) == 7 )
                    year = century * 100 + years_since_1900;
                else {
                    return QDateTime();
                }
            }

            QDate d( year, mon, day );
            QTime t( hour, min, sec );
            if ( d.isValid() && t.isValid() ) {
                QDateTime dt( d, t, Qt::UTC );
                if ( tz ) {
                    // then we have some form of timezone
                    if ( 'Z' == tz  ) {
                        // We are already at UTC
                    } else if ( '+' == tz ) {
                        // local time is ahead of UTC
                        dt = dt.addSecs(-1*((tzHours*60)+tzMins)*60);
                    } else if ( '-' == tz ) {
                        // local time is behind UTC
                        dt = dt.addSecs(((tzHours*60)+tzMins)*60);
                    } else {
                        qWarning("unexpected tz val");
                    }
                }
                return dt;
            }
        }
        return QDateTime();
    }

}
