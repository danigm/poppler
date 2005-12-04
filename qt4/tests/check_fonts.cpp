#include <QtTest/QtTest>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

class TestFontsData: public QObject
{
    Q_OBJECT
private slots:
    void checkNoFonts();
    void checkType1();
};

void TestFontsData::checkNoFonts()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/tests/image.pdf");
    QVERIFY( doc );

    QList<Poppler::FontInfo> listOfFonts = doc->fonts();
    QCOMPARE( listOfFonts.size(), 0 );
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
}

QTEST_MAIN(TestFontsData)
#include "check_fonts.moc"

