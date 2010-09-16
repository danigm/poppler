#include <QtTest/QtTest>

#include <poppler-qt4.h>
#include <poppler-private.h>

Q_DECLARE_METATYPE(GooString*)
Q_DECLARE_METATYPE(Unicode*)

class TestStrings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void check_unicodeToQString_data();
    void check_unicodeToQString();
    void check_QStringToGooString_data();
    void check_QStringToGooString();
};

void TestStrings::initTestCase()
{
    qRegisterMetaType<GooString*>("GooString*");
    qRegisterMetaType<Unicode*>("Unicode*");
}

void TestStrings::check_unicodeToQString_data()
{
    QTest::addColumn<Unicode*>("data");
    QTest::addColumn<int>("length");
    QTest::addColumn<QString>("result");

    {
    const int l = 1;
    Unicode *u = new Unicode[l];
    u[0] = int('a');
    QTest::newRow("a") << u << l << QString::fromUtf8("a");
    }
    {
    const int l = 1;
    Unicode *u = new Unicode[l];
    u[0] = 0x0161;
    QTest::newRow("\u0161") << u << l << QString::fromUtf8("\u0161");
    }
    {
    const int l = 2;
    Unicode *u = new Unicode[l];
    u[0] = int('a');
    u[1] = int('b');
    QTest::newRow("ab") << u << l << QString::fromUtf8("ab");
    }
    {
    const int l = 2;
    Unicode *u = new Unicode[l];
    u[0] = int('a');
    u[1] = 0x0161;
    QTest::newRow("a\u0161") << u << l << QString::fromUtf8("a\u0161");
    }
}

void TestStrings::check_unicodeToQString()
{
    QFETCH(Unicode*, data);
    QFETCH(int, length);
    QFETCH(QString, result);

    QCOMPARE(Poppler::unicodeToQString(data, length), result);

    delete [] data;
}

void TestStrings::check_QStringToGooString_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<GooString*>("result");

    QTest::newRow("<null>") << QString()
                            << new GooString("");
    QTest::newRow("<empty>") << QString::fromUtf8("")
                             << new GooString("");
    QTest::newRow("a") << QString::fromUtf8("a")
                       << new GooString("a");
    QTest::newRow("ab") << QString::fromUtf8("ab")
                        << new GooString("ab");
}

void TestStrings::check_QStringToGooString()
{
    QFETCH(QString, string);
    QFETCH(GooString*, result);

    GooString *goo = Poppler::QStringToGooString(string);
    QCOMPARE(goo->getCString(), result->getCString());

    delete goo;
    delete result;
}

QTEST_MAIN(TestStrings)

#include "check_strings.moc"
