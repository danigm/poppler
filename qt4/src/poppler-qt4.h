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

#include "poppler-annotation.h"
#include "poppler-link.h"
#include "poppler-page-transition.h"

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QSet>
#include <QtCore/QVector>
#include <QtGui/QPixmap>
#include <QtXml/QDomDocument>

class EmbFile;
class Sound;

/**
   The %Poppler Qt4 binding.
*/
namespace Poppler {

    class Document;

    class PageData;

    class FormField;

    class TextBoxData;

    class PSConverter;

    /**
        Describes the physical location of text on a document page
       
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
	 The default constructor sets the \p text and the rectangle that
	 contains the text. Coordinated for the \p bBox are in points =
	 1/72 of an inch.
      */
      TextBox(const QString& text, const QRectF &bBox);
      /**
	Destructor.
      */
      ~TextBox();

      /**
	  Returns the text of this text box
      */
      QString text() const;

      /**
	  Returns the position of the text, in point, i.e., 1/72 of
	 an inch
      */
      QRectF boundingBox() const;

      TextBox *nextWord() const;

      double edge(int i) const;

      bool hasSpaceAfter() const;

    private:
        Q_DISABLE_COPY(TextBox)

	TextBoxData *m_data;
    };


    class FontInfoData;
    /**
       Container class for information about a font within a PDF
       document
    */
    class FontInfo {
    public:
	enum Type {
		unknown,
		Type1,
		Type1C,
		Type1COT,
		Type3,
		TrueType,
		TrueTypeOT,
		CIDType0,
		CIDType0C,
		CIDType0COT,
		CIDTrueType,
		CIDTrueTypeOT
	};
	
	/**
	   Create a new font information container.
	*/
	FontInfo( const FontInfoData &fid );
	
	/**
	   Copy constructor.
	*/
	FontInfo( const FontInfo &fi );
	
	/**
	   Destructor.
	*/
	~FontInfo();

	/**
	   The name of the font. Can be QString::null if the font has no name
	*/
	QString name() const;

	/**
	   The path of the font file used to represent this font on this system,
	   or a null string is the font is embedded
	*/
	QString file() const;

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
	QString typeName() const;

	FontInfo& operator=( const FontInfo &fi );

    private:
	FontInfoData *m_data;
    };


    class EmbeddedFileData;
    /**
       Container class for an embedded file with a PDF document
    */
    class EmbeddedFile {
    public:
	/// \cond PRIVATE
	EmbeddedFile(EmbFile *embfile);
	/// \endcond
	
	/**
	   Destructor.
	*/
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
	   The size of the file.
	
	   This will return < 0 if there is no size element
	*/
	int size() const;

	/**
	   The modification date for the embedded file, if known.
	*/
	QDateTime modDate() const;

	/**
	   The creation date for the embedded file, if known.
	*/
	QDateTime createDate() const;
	
	/**
		The checksum of the file.
		
		This will return an empty QString if there is no checksum element
	*/
	QByteArray checksum() const;

	/**
	   The data as a byte array
	*/
	QByteArray data();

	/**
	   A QDataStream for the actual data?
	*/
	//QDataStream dataStream() const;

    private:
	Q_DISABLE_COPY(EmbeddedFile)

	EmbeddedFileData *m_embeddedFile;
    };


    /**
       Page within a PDF document
    */
    class Page {
	friend class Document;
    public:
	/**
	   Destructor.
	*/
	~Page();

	enum Rotation { Rotate0 = 0, Rotate90 = 1, Rotate180 = 2, Rotate270 = 3 };

	/**
	   The kinds of page actions
	*/
	enum PageAction {
	    Opening,   ///< The action when a page is "opened"
	    Closing    ///< The action when a page is "closed"
	};

	/** 
	   Render the page to a QImage using the current Document renderer
	   (see Document::renderBackend())
	   
	   If \p x = \p y = \p w = \p h = -1, the method will automatically
           compute the size of the image from the horizontal and vertical
           resolutions specified in \p xres and \p yres. Otherwise, the
           method renders only a part of the page, specified by the
           parameters (\p x, \p y, \p w, \p h) in pixel coordinates. The returned
           QImage then has size (\p w, \p h), independent of the page
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
	
	   \param rotate how to rotate the page

	   \note if the current Document renderer does not appear among the
	   Document::availableRenderBackends(), the result is \em always a null QImage.

	   \warning The parameter (\p x, \p y, \p w, \p h) are not
	   well-tested. Unusual or meaningless parameters may lead to
	   rather unexpected results.

	   \returns a QImage of the page, or a null image on failure.
        */
	QImage renderToImage(double xres=72.0, double yres=72.0, int x=-1, int y=-1, int w=-1, int h=-1, Rotation rotate = Rotate0) const;

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
	   The size of the page, in pixels
	*/
	QSize pageSize() const;

	/**
	  Returns the transition of this page

	  \returns a pointer to a PageTransition structure that
	  defines how transition to this page shall be performed. The
	  PageTransition structure is owned by this page, and will
	  automatically be destroyed when this page class is
	  destroyed.
	**/
	PageTransition *transition() const;
	
	/**
	  Gets the page action specified, or NULL if there is no action
	**/
	Link *action( PageAction act ) const;
	
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
	  Gets the links of the page
	*/
	QList<Link*> links() const;
	
	/**
	 Returns the annotations of the page
	*/
	QList<Annotation*> annotations() const;

	/**
	 Returns the form fields on the page
	*/
	QList<FormField*> formFields() const;

	/**
	 Returns the page duration. That is the time, in seconds, that the page
	 should be displayed before the presentation automatically advances to the next page.
	 Returns < 0 if duration is not set.
	*/
	double duration() const;
	
	/**
	   Returns the label of the page, or a null string is the page has no label.
	**/
	QString label() const;
	
    private:
	Q_DISABLE_COPY(Page)

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
	    TwoPageRight ///< Display the pages two at a time, with odd-numbered pages on the right
	};

	/**
	   The render backends available
	*/
	enum RenderBackend {
	    SplashBackend,   ///< Splash backend
	    ArthurBackend   ///< Arthur (Qt4) backend
	};

	/**
	   The render hints available
	*/
	enum RenderHint {
	    Antialiasing = 0x00000001,      ///< Antialiasing for graphics
	    TextAntialiasing = 0x00000002   ///< Antialiasing for text
	};
	Q_DECLARE_FLAGS( RenderHints, RenderHint )

	/**
	   Load the document from a file on disk

	   \param filePath the name (and path, if required) of the file to load

	   \return NULL on error

	   \warning The application owns the pointer to Document, and this should
	   be deleted when no longer required.
	
	   \warning The returning document may be locked.
	*/
	static Document *load(const QString & filePath,
			      const QByteArray &ownerPassword=QByteArray(),
			      const QByteArray &userPassword=QByteArray());
	
	/**
	   Load the document from memory

	   \param fileContents the file contents. They are copied so there is no need 
	                       to keep the byte array around for the full life time of 
	                       the document.

	   \warning The application owns the pointer to Document, and this should
	   be deleted when no longer required.
	*/
	static Document *loadFromData(const QByteArray &fileContents,
			      const QByteArray &ownerPassword=QByteArray(),
			      const QByteArray &userPassword=QByteArray());
  
	/**
	   Get a specified Page
     
	   Note that this follows the PDF standard of being zero based - if you
	   want the first page, then you need an index of zero.

	   \param index the page number index
	*/
	Page *page(int index) const;

	/**
	   \overload


	   The intent is that you can pass in a label like "ix" and
	   get the page with that label (which might be in the table of
	   contents), or pass in "1" and get the page that the user
	   expects (which might not be the first page, if there is a
	   title page and a table of contents).

	   \param label the page label
	*/
	Page *page(const QString &label) const;

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
QDateTime modified = m_doc->date("ModDate");
	   \endcode

	   The available dates are:
	   - CreationDate: the date of creation of the document
	   - ModDate: the date of the last change in the document

	   \param data the type of date that is required

	*/
	QDateTime date( const QString & data ) const;

	/**
	   Get specified information associated with the document

	   You would use this method with something like:
	   \code
QString title = m_doc->info("Title");
QString subject = m_doc->info("Subject");
	   \endcode

	   In addition to \c Title and \c Subject, other information that may
	   be available include \c Author, \c Keywords, \c Creator and \c Producer.

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
	   or creation of bookmarks and thumbnail images.

	   \note this can be true even if okToChange() is false
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
	QList<EmbeddedFile*> embeddedFiles() const;

	/**
	   Whether there are any documents embedded in this PDF document.
	*/
	bool hasEmbeddedFiles() const;
	
	/**
	  Gets the TOC of the Document, it is application responsabiliy to delete
	  it when no longer needed
	
	  In the tree the tag name is the 'screen' name of the entry. A tag can have
	  attributes. Here follows the list of tag attributes with meaning:
	  - Destination: A string description of the referred destination
	  - DestinationName: A 'named reference' to the viewport that must be converted
	       using \p linkDestination( \em destination_name )
	  - ExternalFileName: A link to a external filename
	  - Open: A bool value that tells whether the subbranch of the item is open or not
	
	  Returns NULL if the Document does not have TOC
	*/
	QDomDocument *toc() const;
	
	LinkDestination *linkDestination( const QString &name );
	
	/**
	  Sets the paper color

	  \param color the new paper color
	 */
	void setPaperColor(const QColor &color);
	/**
	  The paper color

	  The default color is white.
	 */
	QColor paperColor() const;

	/**
	 Sets the backend used to render the pages.

	 \note setting a rendering backend that does not appear in the
	 availableRenderBackends() will always result in null QImage's.

	 \param backend the new rendering backend
	 */
	void setRenderBackend( RenderBackend backend );
	/**
	  The currently set render backend

	  The default backend is \ref SplashBackend
	 */
	RenderBackend renderBackend() const;

	/**
	  The available rendering backends.
	 */
	static QSet<RenderBackend> availableRenderBackends();

	/**
	 Sets the render \p hint .

	 \note some hints may not be supported by some rendering backends.

	 \param on whether the flag should be added or removed.
	 */
	void setRenderHint( RenderHint hint, bool on = true );
	/**
	  The currently set render hints.
	 */
	RenderHints renderHints() const;
	
	/**
	  Gets a new PS converter for this document.

	  The caller gets the ownership of the returned converter.
	 */
	PSConverter *psConverter() const;
	
	/**
	  Gets the metadata stream contents
	*/
	QString metadata() const;

	/**
	   Destructor.
	*/
	~Document();
  
    private:
	Q_DISABLE_COPY(Document)

	DocumentData *m_doc;
	
	Document(DocumentData *dataA);
	static Document *checkDocument(DocumentData *doc);
    };
    
    class PSConverterData;
    /**
       Converts a PDF to PS

       Sizes have to be in Points (1/72 inch)

       If you are using QPrinter you can get paper size by doing:
       \code
QPrinter dummy(QPrinter::PrinterResolution);
dummy.setFullPage(true);
dummy.setPageSize(myPageSize);
width = dummy.width();
height = dummy.height();
       \endcode
    */
    class PSConverter
    {
        friend class Document;
        public:
            /**
              Destructor.
            */
            ~PSConverter();

            /** Sets the output file name. Mandatory. */
            void setOutputFileName(const QString &outputFileName);

            /** Sets the list of pages to print. Mandatory. */
            void setPageList(const QList<int> &pageList);

            /**
              Sets the title of the PS Document. Optional
            */
            void setTitle(const QString &title);

            /**
              Sets the horizontal DPI. Defaults to 72.0
            */
            void setHDPI(double hDPI);

            /**
              Sets the vertical DPI. Defaults to 72.0
            */
            void setVDPI(double vDPI);

            /**
              Sets the rotate. Defaults to not rotated
            */
            void setRotate(int rotate);

            /**
              Sets the output paper width. Has to be set.
            */
            void setPaperWidth(int paperWidth);

            /**
              Sets the output paper height. Has to be set.
            */
            void setPaperHeight(int paperHeight);

            /**
              Sets the output right margin. Defaults to 0
            */
            void setRightMargin(int marginRight);

            /**
              Sets the output bottom margin. Defaults to 0
            */
            void setBottomMargin(int marginBottom);

            /**
              Sets the output left margin. Defaults to 0
            */
            void setLeftMargin(int marginLeft);

            /**
              Sets the output top margin. Defaults to 0
            */
            void setTopMargin(int marginTop);

            /**
              Defines if margins have to be strictly followed (even if that
              means changing aspect ratio), or if the margins can be adapted
              to keep aspect ratio.

              Defaults to false.
            */
            void setStrictMargins(bool strictMargins);

            /** Defines if the page will be rasterized to an image before printing. Defaults to false */
            void setForceRasterize(bool forceRasterize);

            /**
              Does the conversion.

              \return whether the conversion succeeded
            */
            bool convert();

        private:
            Q_DISABLE_COPY(PSConverter)

            PSConverter(DocumentData *document);

            PSConverterData *m_data;
    };

    /**
       Conversion from PDF date string format to QDateTime
    */
    QDateTime convertDate( char *dateString );

    class SoundData;
    /**
       Container class for a sound file in a PDF document.

	A sound can be either External (in that case should be loaded the file
       whose url is represented by url() ), or Embedded, and the player has to
       play the data contained in data().
    */
    class SoundObject {
    public:
	/**
	   The type of sound
	*/
	enum SoundType {
	    External,     ///< The real sound file is external
	    Embedded      ///< The sound is contained in the data
	};

	/**
	   The encoding format used for the sound
	*/
	enum SoundEncoding {
	    Raw,          ///< Raw encoding, with unspecified or unsigned values in the range [ 0, 2^B - 1 ]
	    Signed,       ///< Twos-complement values
	    muLaw,        ///< mu-law-encoded samples
	    ALaw          ///< A-law-encoded samples
	};

	/// \cond PRIVATE
	SoundObject(Sound *popplersound);
	/// \endcond
	
	~SoundObject();

	/**
	   Is the sound embedded (\ref Embedded ) or external (\ref External )?
	*/
	SoundType soundType() const;

	/**
	   The URL of the sound file to be played, in case of \ref External
	*/
	QString url() const;

	/**
	   The data of the sound, in case of \ref Embedded
	*/
	QByteArray data() const;

	/**
	   The sampling rate of the sound
	*/
	double samplingRate() const;

	/**
	   The number of sound channels to use to play the sound
	*/
	int channels() const;

	/**
	   The number of bits per sample value per channel
	*/
	int bitsPerSample() const;

	/**
	   The encoding used for the sound
	*/
	SoundEncoding soundEncoding() const;

    private:
	Q_DISABLE_COPY(SoundObject)

	SoundData *m_soundData;
    };

}

#endif
