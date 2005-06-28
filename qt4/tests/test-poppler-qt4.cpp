#include <stdlib.h>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <ctype.h>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

class PDFDisplay : public QWidget           // picture display widget
{
public:
    PDFDisplay( Poppler::Document *d );
    ~PDFDisplay();
protected:
    void        paintEvent( QPaintEvent * );
private:
    QPixmap	*pixmap;
    Poppler::Document *doc;
};

PDFDisplay::PDFDisplay( Poppler::Document *d )
{
    doc = d;
    if (doc) {
	Poppler::Page *page = doc->page("1");
	if (page) {
	    page->renderToPixmap(&pixmap, page->pageSize());
	    delete page;
	}
    } else {
	qWarning() << "doc not loaded";
    }
}

PDFDisplay::~PDFDisplay()
{
    delete doc;
    delete pixmap;
}

void PDFDisplay::paintEvent( QPaintEvent *e )
{
    QPainter paint( this );                     // paint widget
    if (pixmap) {
	paint.drawPixmap(0, 0, *pixmap);
    } else {
	qWarning() << "no pixmap";
    }
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );               // QApplication required!

    if ( argc < 2  || (argc == 3 && strcmp(argv[2], "-extract") != 0) || argc > 3)
    {
	// use argument as file name
	qWarning() << "usage: test-poppler-qt filename [-extract]";
	exit(1);
    }
  
    Poppler::Document *doc = Poppler::Document::load(argv[1]);
    if (!doc)
    {
	qWarning() << "doc not loaded";
	exit(1);
    }
  
    // output some meta-data
    qDebug() << "    PDF Version: " << doc->pdfVersion();
    qDebug() << "          Title: " << doc->info("Title");
    qDebug() << "        Subject: " << doc->info("Subject");
    qDebug() << "         Author: " << doc->info("Author");
    qDebug() << "      Key words: " << doc->info("Keywords");
    qDebug() << "        Creator: " << doc->info("Creator");
    qDebug() << "       Producer: " << doc->info("Producer");
    qDebug() << "   Date created: " << doc->date("CreationDate").toString();
    qDebug() << "  Date modified: " << doc->date("ModDate").toString();
    qDebug() << "Number of pages: " << doc->numPages();
    qDebug() << "     Linearised: " << doc->isLinearized();
    qDebug() << "      Encrypted: " << doc->isEncrypted();
    qDebug() << "    OK to print: " << doc->okToPrint();
    qDebug() << "     OK to copy: " << doc->okToCopy();
    qDebug() << "   OK to change: " << doc->okToChange();
    qDebug() << "OK to add notes: " << doc->okToAddNotes();
    qDebug() << "      Page mode: " << doc->pageMode();
    QStringList fontNameList;
    foreach( Poppler::FontInfo font, doc->fonts() )
	fontNameList += font.name();
    qDebug() << "          Fonts: " << fontNameList.join( ", " );

    Poppler::Page *page = doc->page(0);
    qDebug() << "    Page 1 size: " << page->pageSize().width()/72 << "inches x " << page->pageSize().height()/72 << "inches";
    if ( page->orientation() == Poppler::Page::Landscape ) {
	qDebug() << "Landscape layout";
    } else if ( page->orientation() == Poppler::Page::Portrait ) {
	qDebug() << "Portrait layout";
    } else {
	qDebug() << "Unknown layout";
    }


    if (argc == 2)
    {  
	PDFDisplay test( doc );        // create picture display
	test.setWindowTitle("Poppler-Qt4 Test");
	test.show();                            // show it

	return a.exec();                        // start event loop
    }
    else
    {
	Poppler::Page *page = doc->page(0);

	QLabel *l = new QLabel(page->text(QRectF()), 0);
	l->show();
	delete page;
	delete doc;
	return a.exec();
    }
}
