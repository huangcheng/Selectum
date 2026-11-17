#include <selectum/abi/json.h>

#include <QJsonObject>
#include <QString>

namespace selectum {
namespace {

QString triggerToString(Trigger trigger)
{
    switch (trigger) {
    case Trigger::Selection:
        return QStringLiteral("selection");
    case Trigger::Palette:
        return QStringLiteral("palette");
    case Trigger::Hotkey:
        return QStringLiteral("hotkey");
    case Trigger::Clipboard:
        return QStringLiteral("clipboard");
    }
    return QStringLiteral("selection");
}

bool triggerFromString(const QString &value, Trigger *out)
{
    if (value == QLatin1String("selection")) {
        *out = Trigger::Selection;
        return true;
    }
    if (value == QLatin1String("palette")) {
        *out = Trigger::Palette;
        return true;
    }
    if (value == QLatin1String("hotkey")) {
        *out = Trigger::Hotkey;
        return true;
    }
    if (value == QLatin1String("clipboard")) {
        *out = Trigger::Clipboard;
        return true;
    }
    return false;
}

QString resultStatusToString(ResultStatus status)
{
    switch (status) {
    case ResultStatus::Ok:
        return QStringLiteral("ok");
    case ResultStatus::Error:
        return QStringLiteral("error");
    case ResultStatus::Cancelled:
        return QStringLiteral("cancelled");
    }
    return QStringLiteral("ok");
}

bool resultStatusFromString(const QString &value, ResultStatus *out)
{
    if (value == QLatin1String("ok")) {
        *out = ResultStatus::Ok;
        return true;
    }
    if (value == QLatin1String("error")) {
        *out = ResultStatus::Error;
        return true;
    }
    if (value == QLatin1String("cancelled")) {
        *out = ResultStatus::Cancelled;
        return true;
    }
    return false;
}

QJsonObject stringMapToJson(const QMap<QString, QString> &map)
{
    QJsonObject object;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        object.insert(it.key(), it.value());
    }
    return object;
}

bool stringMapFromJson(const QJsonObject &json, QMap<QString, QString> *out)
{
    QMap<QString, QString> map;
    for (auto it = json.begin(); it != json.end(); ++it) {
        if (!it.value().isString()) {
            return false;
        }
        map.insert(it.key(), it.value().toString());
    }
    *out = map;
    return true;
}

} // namespace

QJsonObject toJson(const ScreenRect &value)
{
    return QJsonObject{
        {QStringLiteral("x"), value.x},
        {QStringLiteral("y"), value.y},
        {QStringLiteral("w"), value.w},
        {QStringLiteral("h"), value.h},
    };
}

bool fromJson(const QJsonObject &json, ScreenRect *out)
{
    if (!out) {
        return false;
    }
    if (!json.contains(QLatin1String("x")) || !json.value(QLatin1String("x")).isDouble()
        || !json.contains(QLatin1String("y")) || !json.value(QLatin1String("y")).isDouble()
        || !json.contains(QLatin1String("w")) || !json.value(QLatin1String("w")).isDouble()
        || !json.contains(QLatin1String("h")) || !json.value(QLatin1String("h")).isDouble()) {
        return false;
    }
    out->x = json.value(QLatin1String("x")).toDouble();
    out->y = json.value(QLatin1String("y")).toDouble();
    out->w = json.value(QLatin1String("w")).toDouble();
    out->h = json.value(QLatin1String("h")).toDouble();
    return true;
}

QJsonObject toJson(const SelectionContext &value)
{
    return QJsonObject{
        {QStringLiteral("text"), value.text},
        {QStringLiteral("appId"), value.appId},
        {QStringLiteral("pageUrl"), value.pageUrl},
        {QStringLiteral("title"), value.title},
        {QStringLiteral("editable"), value.editable},
        {QStringLiteral("screenRect"), toJson(value.screenRect)},
        {QStringLiteral("trigger"), triggerToString(value.trigger)},
    };
}

bool fromJson(const QJsonObject &json, SelectionContext *out)
{
    if (!out) {
        return false;
    }
    if (!json.contains(QLatin1String("text")) || !json.value(QLatin1String("text")).isString()
        || !json.contains(QLatin1String("appId")) || !json.value(QLatin1String("appId")).isString()
        || !json.contains(QLatin1String("pageUrl")) || !json.value(QLatin1String("pageUrl")).isString()
        || !json.contains(QLatin1String("title")) || !json.value(QLatin1String("title")).isString()
        || !json.contains(QLatin1String("editable")) || !json.value(QLatin1String("editable")).isBool()
        || !json.contains(QLatin1String("screenRect")) || !json.value(QLatin1String("screenRect")).isObject()
        || !json.contains(QLatin1String("trigger")) || !json.value(QLatin1String("trigger")).isString()) {
        return false;
    }

    SelectionContext parsed;
    parsed.text = json.value(QLatin1String("text")).toString();
    parsed.appId = json.value(QLatin1String("appId")).toString();
    parsed.pageUrl = json.value(QLatin1String("pageUrl")).toString();
    parsed.title = json.value(QLatin1String("title")).toString();
    parsed.editable = json.value(QLatin1String("editable")).toBool();
    if (!fromJson(json.value(QLatin1String("screenRect")).toObject(), &parsed.screenRect)) {
        return false;
    }
    if (!triggerFromString(json.value(QLatin1String("trigger")).toString(), &parsed.trigger)) {
        return false;
    }
    *out = parsed;
    return true;
}

QJsonObject toJson(const Invocation &value)
{
    return QJsonObject{
        {QStringLiteral("id"), value.id},
        {QStringLiteral("pluginId"), value.pluginId},
        {QStringLiteral("context"), toJson(value.context)},
        {QStringLiteral("config"), stringMapToJson(value.config)},
        {QStringLiteral("secrets"), stringMapToJson(value.secrets)},
        {QStringLiteral("locale"), value.locale},
    };
}

bool fromJson(const QJsonObject &json, Invocation *out)
{
    if (!out) {
        return false;
    }
    if (!json.contains(QLatin1String("id")) || !json.value(QLatin1String("id")).isString()
        || !json.contains(QLatin1String("pluginId")) || !json.value(QLatin1String("pluginId")).isString()
        || !json.contains(QLatin1String("context")) || !json.value(QLatin1String("context")).isObject()
        || !json.contains(QLatin1String("config")) || !json.value(QLatin1String("config")).isObject()
        || !json.contains(QLatin1String("secrets")) || !json.value(QLatin1String("secrets")).isObject()
        || !json.contains(QLatin1String("locale")) || !json.value(QLatin1String("locale")).isString()) {
        return false;
    }

    Invocation parsed;
    parsed.id = json.value(QLatin1String("id")).toString();
    parsed.pluginId = json.value(QLatin1String("pluginId")).toString();
    if (!fromJson(json.value(QLatin1String("context")).toObject(), &parsed.context)) {
        return false;
    }
    if (!stringMapFromJson(json.value(QLatin1String("config")).toObject(), &parsed.config)) {
        return false;
    }
    if (!stringMapFromJson(json.value(QLatin1String("secrets")).toObject(), &parsed.secrets)) {
        return false;
    }
    parsed.locale = json.value(QLatin1String("locale")).toString();
    *out = parsed;
    return true;
}

QJsonObject toJson(const Result &value)
{
    return QJsonObject{
        {QStringLiteral("status"), resultStatusToString(value.status)},
        {QStringLiteral("message"), value.message},
        {QStringLiteral("openUrl"), value.openUrl},
        {QStringLiteral("replaceText"), value.replaceText},
    };
}

bool fromJson(const QJsonObject &json, Result *out)
{
    if (!out) {
        return false;
    }
    if (!json.contains(QLatin1String("status")) || !json.value(QLatin1String("status")).isString()
        || !json.contains(QLatin1String("message")) || !json.value(QLatin1String("message")).isString()
        || !json.contains(QLatin1String("openUrl")) || !json.value(QLatin1String("openUrl")).isString()
        || !json.contains(QLatin1String("replaceText"))
            || !json.value(QLatin1String("replaceText")).isString()) {
        return false;
    }

    Result parsed;
    if (!resultStatusFromString(json.value(QLatin1String("status")).toString(), &parsed.status)) {
        return false;
    }
    parsed.message = json.value(QLatin1String("message")).toString();
    parsed.openUrl = json.value(QLatin1String("openUrl")).toString();
    parsed.replaceText = json.value(QLatin1String("replaceText")).toString();
    *out = parsed;
    return true;
}

} // namespace selectum
