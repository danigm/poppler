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
#include <QtCore/QVector>
#include <QtGui/QPixmap>
#include <QtXml/QDomDocument>

#include <poppler-annotation.h>
#include <poppler-link.h>
#include <poppler-page-transition.h>

class EmbFile;

/**
   The Poppler Qt bindings
*/
namespace Poppler {

    class Document;

    class PageData;

    class TextBoxData;

    /**
        describes the physical location of text on a document page
       
        This very simple class describes the physical location of text
	on the page. It consists of
	- a QString that contains the text
	- a QRectF that gives a box that describes where on the page
	the text is found.
    */
    class TextBox {
    friend class Page;
    public:
      /**
	 The default constructor sets the text and the rectangle the
	 contains the text. Coordinated for the bBox are in points =
	 1/72 of an inch.
      */
      TextBox(const QString& text, const QRectF &bBox);

      /**
	  returns the text
      */      
      const QString &text() const;

      /**
	  returns the position of the text, in point, i.e., 1/72 of
	 an inch
      */
      const QRectF &boundingBox() const;

      TextBox *nextWord() const;

      double edge(int i) const;

      bool hasSpaceAfter() const;

    private:
	TextBoxData *m_data;
    };


    /**
       Container class for information about a font within a PDF
       document
    */
    class FontInfoData;
    class FontInfo {
    public:
	enum Type {
	    unknown = 0,
	    Type1 = 1,
	    Type1C = 2,
	    Type3 = 3,
	    TrueType = 4,
	    CIDType0 = 5,
	    CIDType0C = 6,
	    CIDTrueType = 7
	};
	
	/**
	   Create a new font information container
	*/
	FontInfo( const FontInfoData &fid );
	
	FontInfo( const FontInfo &fi );
	
	~FontInfo();

	/**
	   The name of the font. Can be QString::null if the font has no name
	*/
	const QString &name() const;

	/**
	   The path of the font file used to represent this font on this system.
	*/
	const QString &file() const;

	/**
	   Whether the font is embedded in the file, or not

	   \return true if the font is embedded
	*/
	bool isEmbedded() const;

	/**
	   Whether the font provided is only a subset of the full
	   font or not. This only has meaning if the font is embedded.

	   \return true if the font is only a subset
	*/
	bool isSubset() const;

	/**
	   The type of font encoding

	   \return a enumerated value corresponding to the font encoding used

	   \sa typeName for a string equivalent
	*/
	Type type() const;

	/**
	   The name of the font encoding used

	   \note if you are looking for the name of the font (as opposed to the
	   encoding format used), you probably want name().

	   \sa type for a enumeration version
	*/
	const QString typeName() const;

    private:
	FontInfoData *m_data;
    };


    /**
       Container class for an embedded file with a PDF document
    */
    class EmbeddedFileData;
    class EmbeddedFile {
    public:
	/**
	   Create a new embedded file container
	*/
	EmbeddedFile(EmbFile *embfile);
	
	EmbeddedFile(const EmbeddedFile &ef);
	
	~EmbeddedFile();

	/**
	   The name associated with the file
	*/
	QString name() const;

	/**
	   The description associated with the file, if any.

	   This will return an empty QString if there is no description element
	*/
	QString description() const;

	/**
	   The modification date for the embedded file, if known.
	*/
	QDateTime modDate() const;

	/**
	   The creation date for the embedded file, if known.
	*/
	QDateTime createDate() const;

	/**
	   The data as an array
	*/
	QByteArray data();

	/**
	   A QDataStream for the actual data?
	*/
	//QDataStream dataStream() const;

    private:
	EmbeddedFileData *m_embeddedFile;
    };


    /**
       Page within a PDF document
    */
    class Page {
	friend class Document;
    public:
	~Page();

	enum Rotation { Rotate0 = 0, Rotate90 = 1, Rotate180 = 2, Rotate270 = 3 };

	/** 
	   Render the page to a QImage using the Splash renderer
	   
	   This method can be used to render the page to a QPixmap. It
	   uses the "Splash" rendering engine. This method is reasonably
	   well-tested and has produced good output so far.
	   
	   If x=y=w=h=-1, the method will automatically compute the
           size of the pixmap from the horizontal and vertical
           resolutions specified in xres and yres. Otherwise, the
           method renders only a part of the page, specified by the
           parameters (x, y, w, h) in pixel coordinates. The QPixmap
           returned then has size (w, h), independent of the page
           size.

	   \param x specifies the left x-coordinate of the box, in
	   pixels.

	   \param y specifies the top y-coordinate of the box, in
	   pixels.

	   \param w specifies the width of the box, in pixels.

	   \param h specifies the height of the box, in pixels.

	   \param xres horizontal resolution of the graphics device,
	   in dots per inch

	   \param yres vertical resolution of the graphics device, in
	   dots per inch
	
	   \param doLinks calculate links
	   
	   \param rotate how to rotate the page

	   \warning The parameter (x,y,w,h) are not
	   well-tested. Unusual or meaningless parameters may lead to
	   rather unexpected results.

	   \returns a QImage of the page, or a null image on failure.
        */
    QImage splashRenderToImage(double xres=72.0, double yres=72.0, int x=-1, int y=-1, int w=-1, int h=-1, bool doLinks = false, Rotation rotate = Rotate0) const;
	/**
	   Render the page to a QPixmap using the Splash renderer
	   
	   This member function is provided for convenience. It behaves essentially like the above function.
	   It is used as follows.

@code
Poppler::Page* pdfPage;

// Generate a QPixmap of the rendered page
QPixmap* pixmap = pdfPage->splashRenderToPixmap(0, 0, 0, 0, xres, yres );
if (pixmap == 0) {
  ... error message ...
  return;
}

... use pixmap ...

delete pixmap;
@endcode
	 */
	
	QPixmap *splashRenderToPixmap(double xres=72.0, double yres=72.0, int x=-1, int y=-1, int w=-1, int h=-1, bool doLinks = false, Rotation rotate = Rotate0) const;

	/**
	    Render the page to a pixmap using the Arthur (Qt4) renderer
	   
	   \param q pointer to a QPixmap that is already set to the
	   intended size.

	   \param xres horizontal resolution of the graphics device,
	   in dots per inch

	   \param yres vertical resolution of the graphics device, in
	   dots per inch
	   
	   You are meant to create the pixmap before passing it to
	   this routine, using something like:
	   \code
	   QPixmap* myPixmap = new QPixmap(page->pageSize());
	   page->renderToPixmap(myPixmap);
	   \endcode

	   \warning This is a work in progress. Results are unlikely to be of
	   high quality. 
	*/
        void renderToPixmap(QPixmap *q, double xres=72.0, double yres=72.0) const;

	/**
	   Returns the text that is inside a specified rectangle

	   \param rect the rectangle specifying the area of interest,
	   with coordinates given in points, i.e., 1/72th of an inch.
	   If rect is null, all text on the page is given
	**/
	QString text(const QRectF &rect) const;
	
	
	enum SearchDirection { FromTop, NextResult, PreviousResult };
	enum SearchMode { CaseSensitive, CaseInsensitive };
	
	/**
	   Returns true if the specified text was found.

	   \param text the text the search
	   \param rect in all directions is used to return where the text was found, for NextResult and PreviousResult
	               indicates where to continue searching for
	   \param direction in which direction do the search
	   \param caseSensitive be case sensitive?
	**/
	bool search(const QString &text, QRectF &rect, SearchDirection direction, SearchMode caseSensitive, Rotation rotate = Rotate0) const;
	

	/**
	   Returns a list of text of the page

	   This method returns a QList of TextBoxes that contain all
	   the text of the page, with roughly one text word of text
	   per TextBox item.
	   
	   For text written in western languages (left-to-right and
	   up-to-down), the QList contains the text in the proper
	   order.

	   \warning This method is not tested with Asian scripts
	*/
	QList<TextBox*> textList(Rotation rotate = Rotate0) const;

	/**
	   \return The dimensions of the page, in points (i.e. 1/72th on an inch)
	*/
	QSizeF pageSizeF() const;

	/**
	   The dimensions of the page, in pixels.
	*/
	QSize pageSize() const;

	/**
	  \brief Returns the transition of this page

	  @returns a pointer to a PageTransition structure that
	  defines how transition to this page shall be performed. The
	  PageTransition structure is owned by this page, and will
	  automatically be destroyed when this page class is
	  destroyed.
	**/
	PageTransition *transition() const;
	
	/**
	   Types of orientations that are possible
	*/
	enum Orientation {
	    Landscape, ///< Landscape orientation (portrait, with 90 degrees clockwise rotation )
	    Portrait, ///< Normal portrait orientation
	    Seascape, ///< Seascape orientation (portrait, with 270 degrees clockwise rotation)
	    UpsideDown ///< Upside down orientation (portrait, with 180 degrees rotation)
	};

	/**
	   The orientation of the page
	*/
	Orientation orientation() const;
	
	/**
	  The default CTM
	*/
	void defaultCTM(double *CTM, double dpiX, double dpiY, int rotate, bool upsideDown);
	
	/**
	  Gets the links of the page once it has been rendered if doLinks was true
	*/
	QList<Link*> links() const;
	
	/**
	 Returns the annotations of the page
	*/
	QList<Annotation*> annotations () const;
	
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
	   The page mode
	*/
	enum PageMode {
	    UseNone,     ///< No mode - neither document outline nor thumbnail images are visible
	    UseOutlines, ///< Document outline visible
	    UseThumbs,   ///< Thumbnail images visible
	    FullScreen,  ///< Fullscreen mode (no menubar, windows controls etc)
	    UseOC,       ///< Optional content group panel visible
	    UseAttach    ///< Attachments panel visible
	};
  
	/**
	   The page layout
	*/
	enum PageLayout {
	    NoLayout,   ///< Layout not specified
	    SinglePage, ///< Display a single page
	    OneColumn,  ///< Display a single column of pages
	    TwoColumnLeft, ///< Display the pages in two columns, with odd-numbered pages on the left
	    TwoColumnRight, ///< Display the pages in two columns, with odd-numbered pages on the right
	    TwoPageLeft, ///< Display the pages two at a time, with odd-numbered pages on the left
	    TwoPageRight, ///< Display the pages two at a time, with odd-numbered pages on the right
	};

	/**
	   Load the document from a file on disk

	   \param filePath the name (and path, if required) of the file to load

	   \warning The application owns the pointer to Document, and this should
	   be deleted when no longer required.
	*/
	static Document *load(const QString & filePath,
			      const QByteArray &ownerPassword=QByteArray(),
			      const QByteArray &userPassword=QByteArray());
  
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
	   The type of mode that should be used by the application
	   when the document is opened. Note that while this is 
	   called page mode, it is really viewer application mode.
	*/
	PageMode pageMode() const;

	/**
	   The layout that pages should be shown in when the document
	   is first opened.  This basically describes how pages are
	   shown relative to each other.
	*/
	PageLayout pageLayout() const;

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

	   \sa infoKeys() to get a list of the available keys
	*/
	QString info( const QString & data ) const;

	/**
	   Obtain a list of the available string information keys.
	*/
	QStringList infoKeys() const;

	/**
	   Test if the document is encrypted
	*/
	bool isEncrypted() const;

	/**
	   Test if the document is linearised

	   In some cases, this is called "fast web view", since it
	   is mostly an optimisation for viewing over the Web.
	*/
	bool isLinearized() const;

	/**
	   Test if the permissions on the document allow it to be
	   printed
	*/
	bool okToPrint() const;

	/**
	   Test if the permissions on the document allow it to be
	   printed at high resolution
	*/
	bool okToPrintHighRes() const;

	/**
	   Test if the permissions on the document allow it to be
	   changed.

	   \note depending on the type of change, it may be more
	   appropriate to check other properties as well.
	*/
	bool okToChange() const;

	/**
	   Test if the permissions on the document allow the
	   contents to be copied / extracted
	*/
	bool okToCopy() const;

	/**
	   Test if the permissions on the document allow annotations
	   to be added or modified, and interactive form fields (including
	   signature fields) to be completed.
	*/
	bool okToAddNotes() const;

	/**
	   Test if the permissions on the document allow interactive
	   form fields (including signature fields) to be completed.

	   \note this can be true even if okToAddNotes() is false - this
	   means that only form completion is permitted.
	*/
	bool okToFillForm() const;

	/**
	   Test if the permissions on the document allow interactive
	   form fields (including signature fields) to be set, created and
	   modified
	*/
	bool okToCreateFormFields() const;

	/**
	   Test if the permissions on the document allow content extraction
	   (text and perhaps other content) for accessibility usage (eg for
	   a screen reader)
	*/
	bool okToExtractForAccessibility() const;

	/**
	   Test if the permissions on the document allow it to be
	   "assembled" - insertion, rotation and deletion of pages;
	   or creation of bookmarks and thumbnail images. This can
	   be true even if okToChange is false.
	*/
	bool okToAssemble() const;

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


	/**
	   The documents embedded within the PDF document.

	   \note there are two types of embedded document - this call
	   only accesses documents that are embedded at the document level.
	*/
	const QList<EmbeddedFile*> &embeddedFiles() const;

	/**
	   Whether there are any documents embedded in this PDF document.
	*/
	bool hasEmbeddedFiles() const;
	
	/**
	  Gets the TOC of the Document, it is application responsabiliy to delete
	  it when no longer needed
	
	  * In the tree the tag name is the 'screen' name of the entry. A tag can have
	  * attributes. Here follows the list of tag attributes with meaning:
	  * - Destination: A string description of the referred destination
	  * - DestinationName: A 'named reference' to the viewport that must be converted
	  *      using linkDestination( *destination_name* )
	  * - ExternalFileName: A link to a external filename
	
	  Returns NULL if the Document does not have TOC
	*/
	QDomDocument *toc() const;
	
	LinkDestination *linkDestination( const QString &name );
	
	/**
	  If you are using QPrinter you can get paper size by doing
	  QPrinter dummy(QPrinter::PrinterResolution);
	  dummy.setFullPage(true);
	  dummy.setPageSize(myPageSize);
	  width = dummy.width();
	  height = dummy.height();
	*/
	bool print(const QString &fileName, const QList<int> pageList, double hDPI, double vDPI, int rotate, int paperWidth, int paperHeight);
	
	/**
	  Sets the paper color, if not set it is white by default.
	 */
	void setPaperColor(const QColor &color);
	QColor paperColor() const;

	~Document();
  
    private:
	DocumentData *m_doc;
	Document(DocumentData *dataA);
    };

    /**
       Conversion from PDF date string format to QDateTime
    */
    QDateTime convertDate( char *dateString );

}
#endif

#endif
