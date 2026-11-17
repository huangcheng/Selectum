#include <core/plugin_manifest.h>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace selectum {
namespace {

void setError(QString *error, const QString &message)
{
    if (error) {
        *error = message;
    }
}

QStringList parseStringList(const QJsonValue &value)
{
    QStringList out;
    if (!value.isArray()) {
        return out;
    }
    const QJsonArray arr = value.toArray();
    out.reserve(arr.size());
    for (const QJsonValue &item : arr) {
        if (item.isString()) {
            out.append(item.toString());
        }
    }
    return out;
}

MatchRules parseMatch(const QJsonObject &obj)
{
    MatchRules match;
    if (obj.contains(QStringLiteral("minLength"))) {
        match.minLength = obj.value(QStringLiteral("minLength")).toInt(0);
    }
    if (obj.contains(QStringLiteral("editableOnly"))) {
        match.editableOnly = obj.value(QStringLiteral("editableOnly")).toBool(false);
    }
    match.appAllow = parseStringList(obj.value(QStringLiteral("appAllow")));
    match.appDeny = parseStringList(obj.value(QStringLiteral("appDeny")));
    const QJsonValue regexValue = obj.value(QStringLiteral("regex"));
    if (regexValue.isString()) {
        match.regex = regexValue.toString();
    }
    return match;
}

QVector<ConfigEntry> parseConfig(const QJsonValue &value)
{
    QVector<ConfigEntry> entries;
    if (!value.isArray()) {
        return entries;
    }
    const QJsonArray arr = value.toArray();
    entries.reserve(arr.size());
    for (const QJsonValue &item : arr) {
        if (!item.isObject()) {
            continue;
        }
        const QJsonObject obj = item.toObject();
        ConfigEntry entry;
        entry.key = obj.value(QStringLiteral("key")).toString();
        entry.type = obj.value(QStringLiteral("type")).toString();
        entry.label = obj.value(QStringLiteral("label")).toString();
        entry.required = obj.value(QStringLiteral("required")).toBool(false);
        entries.append(entry);
    }
    return entries;
}

bool requireString(const QJsonObject &obj, const char *key, QString *out, QString *error)
{
    const QJsonValue value = obj.value(QLatin1String(key));
    if (!value.isString() || value.toString().isEmpty()) {
        setError(error, QStringLiteral("missing or empty required field: %1").arg(QLatin1String(key)));
        return false;
    }
    *out = value.toString();
    return true;
}

} // namespace

std::optional<PluginManifest> parsePluginManifest(const QByteArray &json, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        setError(error, QStringLiteral("invalid JSON: %1").arg(parseError.errorString()));
        return std::nullopt;
    }

    const QJsonObject root = doc.object();
    PluginManifest manifest;

    const QJsonValue schemaValue = root.value(QStringLiteral("schemaVersion"));
    if (!schemaValue.isDouble()) {
        setError(error, QStringLiteral("missing or invalid schemaVersion"));
        return std::nullopt;
    }
    manifest.schemaVersion = schemaValue.toInt();
    if (manifest.schemaVersion != 1) {
        setError(error, QStringLiteral("unsupported schemaVersion: %1").arg(manifest.schemaVersion));
        return std::nullopt;
    }

    const QJsonValue apiValue = root.value(QStringLiteral("apiVersion"));
    if (!apiValue.isDouble()) {
        setError(error, QStringLiteral("missing or invalid apiVersion"));
        return std::nullopt;
    }
    manifest.apiVersion = apiValue.toInt();
    if (manifest.apiVersion != 1) {
        setError(error, QStringLiteral("unsupported apiVersion: %1").arg(manifest.apiVersion));
        return std::nullopt;
    }

    if (!requireString(root, "id", &manifest.id, error)) {
        return std::nullopt;
    }
    if (!requireString(root, "name", &manifest.name, error)) {
        return std::nullopt;
    }
    if (!requireString(root, "version", &manifest.version, error)) {
        return std::nullopt;
    }

    const QJsonValue entryValue = root.value(QStringLiteral("entry"));
    if (!entryValue.isObject()) {
        setError(error, QStringLiteral("missing or invalid entry"));
        return std::nullopt;
    }
    const QJsonObject entryObj = entryValue.toObject();
    const QJsonValue nativeValue = entryObj.value(QStringLiteral("native"));
    if (!nativeValue.isString() || nativeValue.toString().isEmpty()) {
        setError(error, QStringLiteral("missing or empty required field: entry.native"));
        return std::nullopt;
    }
    manifest.entryNative = nativeValue.toString();

    if (root.contains(QStringLiteral("icon"))) {
        const QJsonValue iconValue = root.value(QStringLiteral("icon"));
        if (iconValue.isString()) {
            manifest.icon = iconValue.toString();
        }
    }

    if (root.contains(QStringLiteral("match")) && root.value(QStringLiteral("match")).isObject()) {
        manifest.match = parseMatch(root.value(QStringLiteral("match")).toObject());
    }

    // Top-level triggers belong on MatchRules (consumed by match engine).
    manifest.match.triggers = parseStringList(root.value(QStringLiteral("triggers")));
    manifest.capabilities = parseStringList(root.value(QStringLiteral("capabilities")));
    manifest.config = parseConfig(root.value(QStringLiteral("config")));

    if (error) {
        error->clear();
    }
    return manifest;
}

std::optional<PluginManifest> loadPluginManifest(const QString &pluginDir, QString *error)
{
    const QString path = QDir(pluginDir).filePath(QStringLiteral("plugin.json"));
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(error, QStringLiteral("cannot open %1: %2").arg(path, file.errorString()));
        return std::nullopt;
    }
    return parsePluginManifest(file.readAll(), error);
}

} // namespace selectum
