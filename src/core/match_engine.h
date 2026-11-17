#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

#include <selectum/abi/types.h>

namespace selectum {

struct MatchRules {
    int minLength = 0;
    bool editableOnly = false;
    QStringList appAllow;
    QStringList appDeny;
    QString regex;
    QStringList triggers;
};

struct PluginRecord {
    QString id;
    MatchRules match;
    bool enabled = true;
};

QVector<PluginRecord> matchPlugins(const QVector<PluginRecord> &plugins,
                                   const SelectionContext &ctx,
                                   Trigger trigger);

} // namespace selectum
