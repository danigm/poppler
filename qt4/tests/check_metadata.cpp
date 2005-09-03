#include <QtTest/QtTest>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

class TestMetaData: public QObject
{
    Q_OBJECT
private slots:
    void checkStrings_data(QtTestTable &t);
    void checkStrings();
    void checkLinearised();
    void checkPortraitOrientation();
    void checkLandscapeOrientation();
    void checkUpsideDownOrientation();
    void checkSeascapeOrientation();
    void checkVersion();
};

void TestMetaData::checkStrings_data(QtTestTable &t)
{
    t.defineElement( "QString", "key" );
    t.defineElement( "QString", "value" );

    *t.newData( "Author" ) << "Author" << "Brad Hards";
    *t.newData( "Title" ) << "Title" << "Two pages";
    *t.newData( "Subject" ) << "Subject"
			    << "A two page layout for poppler testing";
    *t.newData( "Keywords" ) << "Keywords" << "Qt4 bindings";
    *t.newData( "Creator" ) << "Creator" << "iText: cgpdftops CUPS filter";
    *t.newData( "Producer" ) << "Producer" << "Acrobat Distiller 7.0 for Macintosh";
}

void TestMetaData::checkStrings()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    VERIFY( doc );

    FETCH( QString, key );
    FETCH( QString, value );
    COMPARE( doc->info(key), value );
}

void TestMetaData::checkLinearised()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    VERIFY( doc );

    VERIFY( doc->isLinearized() );
}

void TestMetaData::checkPortraitOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    VERIFY( doc );
  
    COMPARE( doc->page(0)->orientation(), Poppler::Page::Portrait );
}

void TestMetaData::checkLandscapeOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    VERIFY( doc );
  
    COMPARE( doc->page(1)->orientation(), Poppler::Page::Landscape );
}

void TestMetaData::checkUpsideDownOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    VERIFY( doc );

    COMPARE( doc->page(2)->orientation(), Poppler::Page::UpsideDown );
}

void TestMetaData::checkSeascapeOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    VERIFY( doc );

    COMPARE( doc->page(3)->orientation(), Poppler::Page::Seascape );
}

void TestMetaData::checkVersion()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    VERIFY( doc );

    COMPARE( doc->pdfVersion(), 1.6 );
}

QTTEST_MAIN(TestMetaData)
#include "check_metadata.moc"

