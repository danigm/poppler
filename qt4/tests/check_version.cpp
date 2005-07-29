#include <QtCore/QtCore>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );               // QApplication required!

    Poppler::Document *doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    if (!doc)
    {
	exit(1);
    }
  
    if ( !(doc->pdfVersion() == 1.6 ) )
    {
	exit(1);
    }

    exit(0);
}
