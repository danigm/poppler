#include <QtTest/QtTest>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

class TestPermissions: public QObject
{
    Q_OBJECT
private slots:
    void permissions1();
};

void TestPermissions::permissions1()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    VERIFY( doc );
  
    // we are allowed to print
    VERIFY( doc->okToPrint() );

    // we are not allowed to change
    VERIFY( !(doc->okToChange()) );

    // we are not allowed to copy or extract content
    VERIFY( !(doc->okToCopy()) );

    // we are not allowed to print at high resolution
    VERIFY( !(doc->okToPrintHighRes()) );

    // we are not allowed to fill forms
    VERIFY( !(doc->okToFillForm()) );

    // we are allowed to extract content for accessibility
    VERIFY( doc->okToExtractForAccessibility() );

    // we are allowed to assemble this document
    VERIFY( doc->okToAssemble() );
}

QTTEST_MAIN(TestPermissions)
#include "check_permissions.moc"

