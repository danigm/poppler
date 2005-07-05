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
  
    // we are allowed to print
    if ( !(doc->okToPrint() ) )
    {
	exit(2);
    }

    // we are not allowed to change
    if ( (doc->okToChange()) )
    {
	exit(3);
    }

    // we are not allowed to copy or extract content
    if ( (doc->okToCopy() ) )
    {
	exit(4);
    }

    // we are not allowed to print at high resolution
    if ( (doc->okToPrintHighRes() ) )
    {
	exit(5);
    }

    // we are not allowed to fill forms
    if ( (doc->okToFillForm() ) )
    {
	exit(6);
    }

    // we are allowed to extract content for accessibility
    if ( (!doc->okToExtractForAccessibility() ) )
    {
	exit(7);
    }

    // we are allowed to assemble this document
    if ( (!doc->okToAssemble() ) )
    {
	exit(8);
    }

    exit(0);
}
