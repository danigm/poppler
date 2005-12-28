#include <QtTest/QtTest>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

class TestFontsData: public QObject
{
    Q_OBJECT
private slots:
    void checkNoFonts();
    void checkType1();
    void checkType3();
    void checkTrueType();
};

void TestFontsData::checkNoFonts()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/image.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 0 );

    delete doc;
}

void TestFontsData::checkType1()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/text.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 1 );
    QCOMPARE( listOfFonts.at(0).name(), QString("Helvetica") );
    QCOMPARE( listOfFonts.at(0).type(), Poppler::FontInfo::Type1 );
    QCOMPARE( listOfFonts.at(0).typeName(), QString("Type 1") );

    QCOMPARE( listOfFonts.at(0).isEmbedded(), false );
    QCOMPARE( listOfFonts.at(0).isSubset(), false );

    delete doc;
}

void TestFontsData::checkType3()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/type3.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 2 );
    QCOMPARE( listOfFonts.at(0).name(), QString("Helvetica") );
    QCOMPARE( listOfFonts.at(0).type(), Poppler::FontInfo::Type1 );
    QCOMPARE( listOfFonts.at(0).typeName(), QString("Type 1") );

    QCOMPARE( listOfFonts.at(0).isEmbedded(), false );
    QCOMPARE( listOfFonts.at(0).isSubset(), false );

    QCOMPARE( listOfFonts.at(1).name(), QString("") );
    QCOMPARE( listOfFonts.at(1).type(), Poppler::FontInfo::Type3 );
    QCOMPARE( listOfFonts.at(1).typeName(), QString("Type 3") );

    QCOMPARE( listOfFonts.at(1).isEmbedded(), true );
    QCOMPARE( listOfFonts.at(1).isSubset(), false );

    delete doc;
}

void TestFontsData::checkTrueType()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/truetype.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 2 );
    QCOMPARE( listOfFonts.at(0).name(), QString("Arial-BoldMT") );
    QCOMPARE( listOfFonts.at(0).type(), Poppler::FontInfo::TrueType );
    QCOMPARE( listOfFonts.at(0).typeName(), QString("TrueType") );

    QCOMPARE( listOfFonts.at(0).isEmbedded(), false );
    QCOMPARE( listOfFonts.at(0).isSubset(), false );

    QCOMPARE( listOfFonts.at(1).name(), QString("ArialMT") );
    QCOMPARE( listOfFonts.at(1).type(), Poppler::FontInfo::TrueType );
    QCOMPARE( listOfFonts.at(1).typeName(), QString("TrueType") );

    QCOMPARE( listOfFonts.at(1).isEmbedded(), false );
    QCOMPARE( listOfFonts.at(1).isSubset(), false );

    delete doc;
}

QTEST_MAIN(TestFontsData)
#include "check_fonts.moc"

