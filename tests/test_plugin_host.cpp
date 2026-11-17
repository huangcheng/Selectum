#include <QtTest>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <core/plugin_host.h>
#include <core/stub_secrets_store.h>
#include <selectum/abi/types.h>

using namespace selectum;

class TestPluginHost : public QObject
{
    Q_OBJECT

private slots:
    void scanEmptyDir_okNoManifests();
    void scanLoadsStubManifest();
    void matchAndInvokeStub_returnsOkMessage();
    void invokeMissingPlugin_returnsError();
    void disabledStub_notMatched();
};

static QString stubPluginsRoot()
{
    return QStringLiteral(SELECTUM_TEST_PLUGINS_DIR);
}

static SelectionContext sampleContext(const QString &text = QStringLiteral("hello"))
{
    SelectionContext ctx;
    ctx.text = text;
    ctx.appId = QStringLiteral("com.example.app");
    ctx.editable = false;
    ctx.trigger = Trigger::Selection;
    return ctx;
}

void TestPluginHost::scanEmptyDir_okNoManifests()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());

    PluginHost host;
    QString error;
    QVERIFY2(host.scan(tmp.path(), &error), qPrintable(error));
    QCOMPARE(host.manifests().size(), 0);
}

void TestPluginHost::scanLoadsStubManifest()
{
    PluginHost host;
    QString error;
    QVERIFY2(host.scan(stubPluginsRoot(), &error), qPrintable(error));

    const auto manifests = host.manifests();
    QCOMPARE(manifests.size(), 1);
    QCOMPARE(manifests[0].id, QStringLiteral("com.selectum.stub"));
    QCOMPARE(manifests[0].entryNative, QStringLiteral("stub"));
}

void TestPluginHost::matchAndInvokeStub_returnsOkMessage()
{
    PluginHost host;
    StubSecretsStore secrets;
    host.setSecretsStore(&secrets);

    QString error;
    QVERIFY2(host.scan(stubPluginsRoot(), &error), qPrintable(error));

    const auto matched = host.match(sampleContext(), Trigger::Selection);
    QCOMPARE(matched.size(), 1);
    QCOMPARE(matched[0].id, QStringLiteral("com.selectum.stub"));

    const Result result = host.invoke(QStringLiteral("com.selectum.stub"),
                                      sampleContext(QStringLiteral("world")),
                                      Trigger::Selection,
                                      {});
    QCOMPARE(result.status, ResultStatus::Ok);
    QCOMPARE(result.message, QStringLiteral("stub:world"));
}

void TestPluginHost::invokeMissingPlugin_returnsError()
{
    PluginHost host;
    QString error;
    QVERIFY2(host.scan(stubPluginsRoot(), &error), qPrintable(error));

    const Result result = host.invoke(QStringLiteral("com.selectum.missing"),
                                      sampleContext(),
                                      Trigger::Selection,
                                      {});
    QCOMPARE(result.status, ResultStatus::Error);
    QVERIFY(!result.message.isEmpty());
}

void TestPluginHost::disabledStub_notMatched()
{
    PluginHost host;
    QString error;
    QVERIFY2(host.scan(stubPluginsRoot(), &error), qPrintable(error));

    host.setEnabled(QStringLiteral("com.selectum.stub"), false);

    const auto matched = host.match(sampleContext(), Trigger::Selection);
    QCOMPARE(matched.size(), 0);
}

QTEST_MAIN(TestPluginHost)
#include "test_plugin_host.moc"
