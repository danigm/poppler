#include <qapplication.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <ctype.h>
#include <poppler-qt.h>

class PDFDisplay : public QWidget           // picture display widget
{
public:
    PDFDisplay( const char *fileName );
   ~PDFDisplay();
protected:
    void        paintEvent( QPaintEvent * );
private:
    QPixmap	*pixmap;
    Poppler::Document *doc;
};

PDFDisplay::PDFDisplay( const char *fileName )
{
  doc = Poppler::Document::load(fileName);
  if (doc) {
    Poppler::Page *page = doc->getPage(0);
    if (page) {
      page->renderToPixmap(&pixmap, -1, -1, -1, -1);
      delete page;
    }
  } else {
    printf("doc not loaded\n");
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
  if (pixmap)
    paint.drawPixmap(0, 0, *pixmap);
}

int main( int argc, char **argv )
{
  QApplication a( argc, argv );               // QApplication required!

  if ( argc != 2 )  {                          // use argument as file name
    printf("usage: test-poppler-qt filename\n");
    exit(1);
  }
  PDFDisplay test( argv[1] );        // create picture display
  a.setMainWidget( &test);                // set main widget
  test.setCaption("Poppler-Qt Test");
  test.show();                            // show it

  return a.exec();                        // start event loop
}
