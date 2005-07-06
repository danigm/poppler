#include <QtCore/QtCore>

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
  
    // It is supposed to be linearised
    if ( !(doc->isLinearized() ) )
    {
	exit(2);
    }

    exit(0);
}
