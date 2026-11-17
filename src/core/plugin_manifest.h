#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <optional>

#include <core/match_engine.h>

namespace selectum {

struct ConfigEntry {
    QString key;
    QString type;
    QString label;
    bool required = false;
};

struct PluginManifest {
    QString id;
    QString name;
    QString version;
    QString icon;
    QString entryNative;
    int schemaVersion = 0;
    int apiVersion = 0;
    MatchRules match;
    QVector<ConfigEntry> config;
    QStringList capabilities;
};

std::optional<PluginManifest> parsePluginManifest(const QByteArray &json, QString *error);
std::optional<PluginManifest> loadPluginManifest(const QString &pluginDir, QString *error);

} // namespace selectum
