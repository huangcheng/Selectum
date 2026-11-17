#include <QtTest>

#include <QDir>
#include <QFile>

#include <core/plugin_manifest.h>

using namespace selectum;

class TestPluginManifest : public QObject
{
    Q_OBJECT

private slots:
    void parsesValidFixture();
    void rejectsMissingId();
    void rejectsWrongSchemaVersion();
    void parsesSecretConfigEntry();
};

static QString fixturesRoot()
{
    return QStringLiteral(SELECTUM_TEST_FIXTURES_DIR);
}

void TestPluginManifest::parsesValidFixture()
{
    QString error;
    const auto dir = QDir(fixturesRoot()).filePath(QStringLiteral("valid_stub"));
    const auto manifest = loadPluginManifest(dir, &error);

    QVERIFY2(manifest.has_value(), qPrintable(error));
    QCOMPARE(manifest->id, QStringLiteral("com.selectum.stub"));
    QCOMPARE(manifest->entryNative, QStringLiteral("stub"));
    QCOMPARE(manifest->match.minLength, 1);
    QCOMPARE(manifest->schemaVersion, 1);
    QCOMPARE(manifest->apiVersion, 1);
    QCOMPARE(manifest->match.triggers, QStringList{QStringLiteral("selection")});
}

void TestPluginManifest::rejectsMissingId()
{
    const QByteArray json = R"({
      "schemaVersion": 1,
      "name": "No Id",
      "version": "0.1.0",
      "apiVersion": 1,
      "entry": { "native": "x" }
    })";

    QString error;
    const auto manifest = parsePluginManifest(json, &error);
    QVERIFY(!manifest.has_value());
    QVERIFY(!error.isEmpty());
}

void TestPluginManifest::rejectsWrongSchemaVersion()
{
    QString error;
    const auto dir = QDir(fixturesRoot()).filePath(QStringLiteral("bad_schema"));
    const auto manifest = loadPluginManifest(dir, &error);

    QVERIFY(!manifest.has_value());
    QVERIFY(!error.isEmpty());
}

void TestPluginManifest::parsesSecretConfigEntry()
{
    const QByteArray json = R"({
      "schemaVersion": 1,
      "id": "com.selectum.secret",
      "name": "Secret Plugin",
      "version": "0.1.0",
      "apiVersion": 1,
      "entry": { "native": "secret" },
      "config": [
        {
          "key": "apiToken",
          "type": "secret",
          "label": "API token",
          "required": true
        }
      ]
    })";

    QString error;
    const auto manifest = parsePluginManifest(json, &error);

    QVERIFY2(manifest.has_value(), qPrintable(error));
    QCOMPARE(manifest->config.size(), 1);
    QCOMPARE(manifest->config[0].key, QStringLiteral("apiToken"));
    QCOMPARE(manifest->config[0].type, QStringLiteral("secret"));
    QCOMPARE(manifest->config[0].label, QStringLiteral("API token"));
    QCOMPARE(manifest->config[0].required, true);
}

QTEST_MAIN(TestPluginManifest)
#include "test_plugin_manifest.moc"
