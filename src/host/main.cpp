#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QIcon>
#include <QLocale>
#include <QTranslator>

#include <core/plugin_host.h>
#include <core/stub_secrets_store.h>

namespace {

QStringList candidatePluginRoots()
{
    QStringList roots;
    const QString appDir = QCoreApplication::applicationDirPath();
    roots << QDir(appDir).filePath(QStringLiteral("plugins"));

    // macOS bundle: Selectum.app/Contents/MacOS → also try Contents/plugins
    QDir macOSDir(appDir);
    if (macOSDir.dirName() == QLatin1String("MacOS")) {
        macOSDir.cdUp();
        roots << macOSDir.filePath(QStringLiteral("plugins"));
    }

#ifdef SELECTUM_BUILD_PLUGINS_DIR
    roots << QStringLiteral(SELECTUM_BUILD_PLUGINS_DIR);
#endif

    roots.removeDuplicates();
    return roots;
}

void scanPlugins(selectum::PluginHost &host)
{
    for (const QString &root : candidatePluginRoots()) {
        if (!QDir(root).exists()) {
            continue;
        }
        QString error;
        if (host.scan(root, &error)) {
            return;
        }
    }
    // Missing/empty plugins dir: start with zero plugins, no crash.
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Selectum"));
    QApplication::setOrganizationName(QStringLiteral("Selectum"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/selectum.png")));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Selectum_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    selectum::PluginHost host;
    selectum::StubSecretsStore secrets;
    host.setSecretsStore(&secrets);
    scanPlugins(host);

    MainWindow w(&host);
    w.show();
    return QApplication::exec();
}
