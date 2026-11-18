#include <QtTest/QtTest>

#include <platform/selection_settle.h>

class TestSelectionSettle : public QObject
{
    Q_OBJECT

private slots:
    void emptyRejected()
    {
        QVERIFY(!selectum::shouldEmitSettledSelection(QString(), QString(), true));
        QVERIFY(!selectum::shouldEmitSettledSelection(QStringLiteral("a"), QString(), true));
    }

    void whitespaceRejected()
    {
        QVERIFY(!selectum::shouldEmitSettledSelection(QString(), QStringLiteral("   \n\t"), true));
    }

    void sameTextRejected()
    {
        QVERIFY(!selectum::shouldEmitSettledSelection(QStringLiteral("hello"),
                                                      QStringLiteral("hello"),
                                                      true));
    }

    void newTextOnSettleAccepted()
    {
        QVERIFY(selectum::shouldEmitSettledSelection(QString(), QStringLiteral("hello"), true));
        QVERIFY(selectum::shouldEmitSettledSelection(QStringLiteral("old"),
                                                     QStringLiteral("hello"),
                                                     true));
    }

    void newTextWithoutSettleRejected()
    {
        QVERIFY(!selectum::shouldEmitSettledSelection(QString(), QStringLiteral("hello"), false));
    }
};

QTEST_MAIN(TestSelectionSettle)
#include "test_selection_settle.moc"
