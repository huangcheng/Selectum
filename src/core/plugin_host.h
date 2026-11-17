#pragma once

#include <QMap>
#include <QString>
#include <QVector>

#include <memory>

#include <core/plugin_manifest.h>
#include <selectum/abi/types.h>

namespace selectum {

class SecretsStore;

class PluginHost
{
public:
    PluginHost();
    ~PluginHost();

    PluginHost(const PluginHost &) = delete;
    PluginHost &operator=(const PluginHost &) = delete;

    void setSecretsStore(SecretsStore *store);
    void setEnabled(const QString &pluginId, bool enabled);

    bool scan(const QString &pluginsRoot, QString *error);
    QVector<PluginManifest> manifests() const;
    QVector<PluginManifest> match(const SelectionContext &ctx, Trigger trigger) const;

    Result invoke(const QString &pluginId,
                  const SelectionContext &ctx,
                  Trigger trigger,
                  const QMap<QString, QString> &config);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace selectum
