#pragma once

#include <QMap>
#include <QString>

namespace selectum {

enum class Trigger {
    Selection,
    Palette,
    Hotkey,
    Clipboard,
};

enum class ResultStatus {
    Ok,
    Error,
    Cancelled,
};

struct ScreenRect {
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
};

struct SelectionContext {
    QString text;
    QString appId;
    QString pageUrl;
    QString title;
    bool editable = false;
    ScreenRect screenRect;
    Trigger trigger = Trigger::Selection;
};

struct Invocation {
    QString id;
    QString pluginId;
    SelectionContext context;
    QMap<QString, QString> config;
    QMap<QString, QString> secrets;
    QString locale;
};

struct Result {
    ResultStatus status = ResultStatus::Ok;
    QString message;
    QString openUrl;
    QString replaceText;
};

} // namespace selectum
