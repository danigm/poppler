#include <QtCore/QtCore>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );               // QApplication required!

    Poppler::Document *doc = Poppler::Document::load("../../../test/unittestcases/UseNone.pdf");
    if (!doc)
    {
	exit(1);
    }
  
    if ( !(doc->pageMode() == Poppler::Document::UseNone ) )
    {
	exit(1);
    }

    exit(0);
}
