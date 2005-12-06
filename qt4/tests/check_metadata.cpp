#include <QtTest/QtTest>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

class TestMetaData: public QObject
{
    Q_OBJECT
private slots:
    void checkStrings_data();
    void checkStrings();
    void checkStrings2_data();
    void checkStrings2();
    void checkLinearised();
    void checkNumPages();
    void checkDate();
    void checkPageSize();
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

void TestMetaData::checkStrings2_data()
{
    QTest::addColumn<QString>("key");
    QTest::addColumn<QString>("value");

    QTest::newRow( "Title" ) << "Title" << "Malaga hotels";
    QTest::newRow( "Author" ) << "Author" << "Brad Hards";
    QTest::newRow( "Creator" ) << "Creator" << "Safari: cgpdftops CUPS filter";
    QTest::newRow( "Producer" )  << "Producer" << "Acrobat Distiller 7.0 for Macintosh";
    QTest::newRow( "Keywords" ) << "Keywords" << "First\rSecond\rthird";
}

void TestMetaData::checkStrings2()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
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

    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );
    QEXPECT_FAIL("", "We don't yet handle linearisation correctly", Continue);
    QCOMPARE( doc->isLinearized(), false );
}

void TestMetaData::checkPortraitOrientation()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );
  
    QCOMPARE( doc->page(0)->orientation(), Poppler::Page::Portrait );
}

void TestMetaData::checkNumPages()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/doublepage.pdf");
    QVERIFY( doc );
    QCOMPARE( doc->numPages(), 2 );

    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );
    QCOMPARE( doc->numPages(), 1 );
}

void TestMetaData::checkDate()
{
    Poppler::Document *doc;

    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );
    QCOMPARE( doc->date("ModDate"), QDateTime(QDate(2005, 12, 5), QTime(20,44,46) ) );
    QCOMPARE( doc->date("CreationDate"), QDateTime(QDate(2005, 8, 13), QTime(11,12,11) ) );
}

void TestMetaData::checkPageSize()
{
    Poppler::Document *doc;

    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );
    QCOMPARE( doc->page(0)->pageSize(), QSize(595, 842) );
    QCOMPARE( doc->page(0)->pageSizeF(), QSizeF(595.22, 842) );
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

