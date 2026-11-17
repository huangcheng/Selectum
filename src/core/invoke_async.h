#pragma once

#include <QMap>
#include <QString>

#include <functional>

#include <core/plugin_host.h>
#include <selectum/abi/types.h>

class QObject;

namespace selectum {

/// Runs `PluginHost::invoke` on a worker thread and posts `done` to `receiver`'s thread.
void invokeAsync(PluginHost &host,
                 const QString &pluginId,
                 const SelectionContext &ctx,
                 Trigger trigger,
                 const QMap<QString, QString> &config,
                 QObject *receiver,
                 std::function<void(Result)> done);

} // namespace selectum
