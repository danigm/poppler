#include <QtTest/QtTest>

#include <poppler-qt4.h>

class TestSearch: public QObject
{
    Q_OBJECT
private slots:
    void bug7063();
};

void TestSearch::bug7063()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/bug7063.pdf");
    QVERIFY( doc );
   
    Poppler::Page *page = doc->page(0);
    QRectF pageRegion( QPointF(0,0), page->pageSize() );

    QCOMPARE( page->search(QString("non-ascii:"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );

    QCOMPARE( page->search(QString("Ascii"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );
    QCOMPARE( page->search(QString("Ascii"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseInsensitive), true );

    QCOMPARE( page->search(QString("latin1:"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );

    QCOMPARE( page->search(QString::fromUtf8("é"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("à"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("ç"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("search \"é\", \"à\" or \"ç\""), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("¥µ©"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), true );
    QCOMPARE( page->search(QString::fromUtf8("¥©"), pageRegion, Poppler::Page::FromTop, Poppler::Page::CaseSensitive), false );
  
    delete doc;
}

QTEST_MAIN(TestSearch)
#include "check_search.moc"

