#include <stdlib.h>
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <ctype.h>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );               // QApplication required!

    Poppler::Document *doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    if (!doc)
    {
	exit(1);
    }
  
    if ( !(doc->info("Author") == QString("Brad Hards") ) )
    {
	exit(2);
    }

    exit(0);
}
