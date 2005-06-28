/* poppler-qt.h: qt interface to poppler
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

#ifndef __POPPLER_QT_H__
#define __POPPLER_QT_H__

#ifdef UNSTABLE_POPPLER_QT4

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtGui/QPixmap>

/**
   The Poppler Qt bindings
*/
namespace Poppler {

    class Document;

    class PageData;


    /**
       Container class for information about a font within a PDF
       document
    */
    class FontInfo {
    public:
	/**
	   Create a new font information container
	*/
	FontInfo( const QString fontName ):
	    m_fontName(fontName) {};

	/**
	   The name of the font
	*/
	QString name() const
	    { return m_fontName; }
	
    private:
	QString m_fontName;
    };


    /**
       Page within a PDF document
    */
    class Page {
	friend class Document;
    public:
	~Page();

	/**
	   Render the page to a pixmap using the Splash renderer
	*/
	void splashRenderToPixmap(QPixmap **q, int x, int y, int w, int h) const;

	/**
	   Render the page to a pixmap using the Arthur (Qt4) renderer
	*/
	void renderToPixmap(QPixmap **q, QSize size) const;

	/**
	   Returns the text that is inside a specified rectangle

	   \param rect the rectangle specifying the area of interest
	   If rect is null, all text on the page is given
	**/
	QString text(const QRectF &rect) const;

	/**
	   The dimensions of the page, in points.
	*/
	QSizeF pageSizeF() const;

	/**
	   The dimensions of the page, in points.
	*/
	QSize pageSize() const;

	enum Orientation {
	    Landscape,
	    Portrait,
	    Seascape,
	    UpsideDown
	};

	/**
	   The orientation of the page
	*/
	Orientation orientation() const;
    private:
	Page(const Document *doc, int index);
	PageData *m_page;
    };

    class DocumentData;

/**
   PDF document

   A document potentially contains multiple Pages
*/
    class Document {
	friend class Page;
  
    public:
	/**
	   The mode
	*/
	enum PageMode {
	    UseNone,     ///< No mode 
	    UseOutlines, ///< Outline mode
	    UseThumbs,   ///< Thumbnail mode
	    FullScreen,  ///< Fullscreen mode
	    UseOC        ///< OC mode
	};
  
	/**
	   Load the document from a file on disk

	   \param filePath the name (and path, if required) of the file to load
	*/
	static Document *Document::load(const QString & filePath);
  
	/**
	   Get a specified page
     
	   Note that this follows the PDF standard of being zero based - if you
	   want the first page, then you need an index of zero.

	   \param index the page number index
	*/
	Page *page(int index) const{ return new Page(this, index); }

	/**
	   \overload

	   The intent is that you can pass in a label like "ix" and
	   get the page with that label (which might be in the table of
	   contents), or pass in "1" and get the page that the user
	   expects (which might not be the first page, if there is a
	   title page and a table of contents).

	   \param label the page label
	*/
	Page *page(QString label) const;

	/**
	   The number of pages in the document
	*/
	int numPages() const;
  
	/**
	   The type of mode that is in use
	*/
	PageMode pageMode() const;

	/**
	   Provide the passwords required to unlock the document
	*/
	bool unlock(const QByteArray &ownerPassword, const QByteArray &userPassword);

	/**
	   Determine if the document is locked
	*/
	bool isLocked() const;

	/**
	   The date associated with the document

	   You would use this method with something like:
	   \code
	   QDateTime created = m_doc->date("CreationDate");
	   QDateTime modded = m_doc->date("ModDate");
	   \endcode

	   \param data the type of date that is required (such as CreationDate or ModDate)

	*/
	QDateTime date( const QString & data ) const;

	/**
	   Get specified information associated with the document

	   You would use this method with something like:
	   \code
	   QString title = m_doc->info("Title");
	   QString subject = m_doc->info("Subject");
	   \endcode

	   In addition to Title and Subject, other information that may be available
	   includes Author, Keywords, Creator and Producer.

	   \param data the information that is required
	*/
	QString info( const QString & data ) const;

	/**
	   Test if the document is encrypted
	*/
	bool isEncrypted() const;

	/**
	   Test if the document is linearised
	*/
	bool isLinearized() const;

	/**
	   Test if the permissions on the document allow it to be
	   printed
	*/
	bool okToPrint() const;

	/**
	   Test if the permissions on the document allow it to be
	   changed
	*/
	bool okToChange() const;

	/**
	   Test if the permissions on the document allow it to be
	   copied
	*/
	bool okToCopy() const;

	/**
	   Test if the permissions on the document allow notes to be
	   added
	*/
	bool okToAddNotes() const;

	/**
	   The version of the PDF specification that the document
	   conforms to
	*/
	double pdfVersion() const;
  
	/**
	   The fonts within the PDF document.

	   \note this can take a very long time to run with a large
	   document. You may wish to use the call below if you have more
	   than say 20 pages
	*/
	QList<FontInfo> fonts() const;

	/**
	   \overload

	   \param numPages the number of pages to scan
	   \param fontList pointer to the list where the font information
	   should be placed

	   \return false if the end of the document has been reached
	*/
	bool scanForFonts( int numPages, QList<FontInfo> *fontList ) const; 

	Document::~Document();
  
    private:
	DocumentData *m_doc;
	Document::Document(DocumentData *dataA);
    };

}
#endif

#endif
