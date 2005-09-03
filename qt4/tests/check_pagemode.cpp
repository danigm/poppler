#include <QtTest/QtTest>

#define UNSTABLE_POPPLER_QT4
#include <poppler-qt4.h>

class TestPageMode: public QObject
{
    Q_OBJECT
private slots:
    void checkNone();
    void checkFullScreen();
    void checkAttachments();
    void checkThumbs();
    void checkOC();
};

void TestPageMode::checkNone()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/UseNone.pdf");
    VERIFY( doc );
  
    COMPARE( doc->pageMode(), Poppler::Document::UseNone );
}

void TestPageMode::checkFullScreen()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/FullScreen.pdf");
    VERIFY( doc );

    COMPARE( doc->pageMode(), Poppler::Document::FullScreen );
}

void TestPageMode::checkAttachments()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/UseAttachments.pdf");
    VERIFY( doc );
  
    COMPARE( doc->pageMode(), Poppler::Document::UseAttach );
}

void TestPageMode::checkThumbs()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/UseThumbs.pdf");
    VERIFY( doc );

    COMPARE( doc->pageMode(), Poppler::Document::UseThumbs );
}

void TestPageMode::checkOC()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/UseOC.pdf");
    VERIFY( doc );

    COMPARE( doc->pageMode(), Poppler::Document::UseOC );
}

QTTEST_MAIN(TestPageMode)
#include "check_pagemode.moc"

