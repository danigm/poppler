#include <QtTest/QtTest>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

class TestMetaData: public QObject
{
    Q_OBJECT
private slots:
    void checkStrings_data();
    void checkStrings();
    void checkLinearised();
    void checkPortraitOrientation();
    void checkLandscapeOrientation();
    void checkUpsideDownOrientation();
    void checkSeascapeOrientation();
    void checkVersion();
};

void TestMetaData::checkStrings_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("value");

    QTest::newRow( "Author" ) << "Author" << "Brad Hards";
    QTest::newRow( "Title" ) << "Title" << "Two pages";
    QTest::newRow( "Subject" ) << "Subject"
			       << "A two page layout for poppler testing";
    QTest::newRow( "Keywords" ) << "Keywords" << "Qt4 bindings";
    QTest::newRow( "Creator" ) << "Creator" << "iText: cgpdftops CUPS filter";
    QTest::newRow( "Producer" ) << "Producer" << "Acrobat Distiller 7.0 for Macintosh";
}

void TestMetaData::checkStrings()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    QVERIFY( doc );

    QFETCH( QString, key );
    QFETCH( QString, value );
    QCOMPARE( doc->info(key), value );
}

void TestMetaData::checkLinearised()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );

    QVERIFY( doc->isLinearized() );
}

void TestMetaData::checkPortraitOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );
  
    QCOMPARE( doc->page(0)->orientation(), Poppler::Page::Portrait );
}

void TestMetaData::checkLandscapeOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );
  
    QCOMPARE( doc->page(1)->orientation(), Poppler::Page::Landscape );
}

void TestMetaData::checkUpsideDownOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->page(2)->orientation(), Poppler::Page::UpsideDown );
}

void TestMetaData::checkSeascapeOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->page(3)->orientation(), Poppler::Page::Seascape );
}

void TestMetaData::checkVersion()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->pdfVersion(), 1.6 );
}

QTEST_MAIN(TestMetaData)
#include "check_metadata.moc"

