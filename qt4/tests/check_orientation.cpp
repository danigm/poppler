#include <stdlib.h>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <ctype.h>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );               // QApplication required!

    Poppler::Document *doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    if (!doc)
    {
	exit(1);
    }
  
    Poppler::Page *page = doc->page(0);
    if ( !( page->orientation() == Poppler::Page::Portrait ) ) {
	exit(2);
    }

    page = doc->page(1);
    if ( !( page->orientation() == Poppler::Page::Landscape ) ) {
	exit(3);
    }

    page = doc->page(2);
    if ( !( page->orientation() == Poppler::Page::UpsideDown) ) {
	exit(4);
    }

    page = doc->page(3);
    if ( !( page->orientation() == Poppler::Page::Seascape ) ) {
	exit(5);
    }

    exit(0);
}
