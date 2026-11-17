#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QLocale>
#include <QTranslator>

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
    MainWindow w;
    w.show();
    return QApplication::exec();
}
