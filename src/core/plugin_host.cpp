#include <core/plugin_host.h>

#include <QDir>
#include <QPluginLoader>
#include <QUuid>

#include <vector>

#include <core/match_engine.h>
#include <core/secrets_store.h>
#include <selectum/abi/plugin_interface.h>

namespace selectum {
namespace {

struct LoadedPlugin {
    QPluginLoader loader;
    ISelectumPlugin *instance = nullptr;
};

struct PluginEntry {
    PluginManifest manifest;
    QString pluginDir;
    bool enabled = true;
    std::unique_ptr<LoadedPlugin> loaded;
};

QString nativeLibraryFileName(const QString &entryNative)
{
#if defined(Q_OS_WIN)
    return entryNative + QStringLiteral(".dll");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("lib") + entryNative + QStringLiteral(".dylib");
#else
    return QStringLiteral("lib") + entryNative + QStringLiteral(".so");
#endif
}

Result ensureLoaded(PluginEntry &entry, QString *error)
{
    if (entry.loaded && entry.loaded->instance) {
        return Result{ResultStatus::Ok, {}, {}, {}};
    }

    const QString libPath = QDir(entry.pluginDir).filePath(nativeLibraryFileName(entry.manifest.entryNative));
    auto loaded = std::make_unique<LoadedPlugin>();
    loaded->loader.setFileName(libPath);
    QObject *object = loaded->loader.instance();
    if (!object) {
        const QString message = QStringLiteral("failed to load plugin %1 from %2: %3")
                                    .arg(entry.manifest.id, libPath, loaded->loader.errorString());
        if (error) {
            *error = message;
        }
        return Result{ResultStatus::Error, message, {}, {}};
    }

    auto *plugin = qobject_cast<ISelectumPlugin *>(object);
    if (!plugin) {
        const QString message =
            QStringLiteral("plugin %1 does not implement ISelectumPlugin").arg(entry.manifest.id);
        if (error) {
            *error = message;
        }
        loaded->loader.unload();
        return Result{ResultStatus::Error, message, {}, {}};
    }

    loaded->instance = plugin;
    entry.loaded = std::move(loaded);
    if (error) {
        error->clear();
    }
    return Result{ResultStatus::Ok, {}, {}, {}};
}

} // namespace

struct PluginHost::Impl {
    SecretsStore *secrets = nullptr;
    std::vector<PluginEntry> plugins;
};

PluginHost::PluginHost()
    : m_impl(std::make_unique<Impl>())
{
}

PluginHost::~PluginHost() = default;

void PluginHost::setSecretsStore(SecretsStore *store)
{
    m_impl->secrets = store;
}

void PluginHost::setEnabled(const QString &pluginId, bool enabled)
{
    for (PluginEntry &entry : m_impl->plugins) {
        if (entry.manifest.id == pluginId) {
            entry.enabled = enabled;
            return;
        }
    }
}

bool PluginHost::scan(const QString &pluginsRoot, QString *error)
{
    m_impl->plugins.clear();

    QDir root(pluginsRoot);
    if (!root.exists()) {
        if (error) {
            *error = QStringLiteral("plugins root does not exist: %1").arg(pluginsRoot);
        }
        return false;
    }

    const QFileInfoList dirs = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &dirInfo : dirs) {
        QString loadError;
        const auto manifest = loadPluginManifest(dirInfo.absoluteFilePath(), &loadError);
        if (!manifest.has_value()) {
            continue;
        }

        PluginEntry entry;
        entry.manifest = *manifest;
        entry.pluginDir = dirInfo.absoluteFilePath();
        entry.enabled = true;
        m_impl->plugins.push_back(std::move(entry));
    }

    if (error) {
        error->clear();
    }
    return true;
}

QVector<PluginManifest> PluginHost::manifests() const
{
    QVector<PluginManifest> out;
    out.reserve(m_impl->plugins.size());
    for (const PluginEntry &entry : m_impl->plugins) {
        out.append(entry.manifest);
    }
    return out;
}

QVector<PluginManifest> PluginHost::match(const SelectionContext &ctx, Trigger trigger) const
{
    QVector<PluginRecord> records;
    records.reserve(m_impl->plugins.size());
    for (const PluginEntry &entry : m_impl->plugins) {
        PluginRecord record;
        record.id = entry.manifest.id;
        record.match = entry.manifest.match;
        record.enabled = entry.enabled;
        records.append(record);
    }

    const QVector<PluginRecord> matched = matchPlugins(records, ctx, trigger);

    QVector<PluginManifest> out;
    out.reserve(matched.size());
    for (const PluginRecord &record : matched) {
        for (const PluginEntry &entry : m_impl->plugins) {
            if (entry.manifest.id == record.id) {
                out.append(entry.manifest);
                break;
            }
        }
    }
    return out;
}

Result PluginHost::invoke(const QString &pluginId,
                          const SelectionContext &ctx,
                          Trigger trigger,
                          const QMap<QString, QString> &config)
{
    PluginEntry *entry = nullptr;
    for (PluginEntry &candidate : m_impl->plugins) {
        if (candidate.manifest.id == pluginId) {
            entry = &candidate;
            break;
        }
    }
    if (!entry) {
        return Result{ResultStatus::Error,
                      QStringLiteral("plugin not found: %1").arg(pluginId),
                      {},
                      {}};
    }

    QString loadError;
    const Result loadResult = ensureLoaded(*entry, &loadError);
    if (loadResult.status != ResultStatus::Ok) {
        return loadResult;
    }

    Invocation invocation;
    invocation.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    invocation.pluginId = pluginId;
    invocation.context = ctx;
    invocation.context.trigger = trigger;
    invocation.config = config;

    if (m_impl->secrets) {
        for (const ConfigEntry &configEntry : entry->manifest.config) {
            if (configEntry.type == QStringLiteral("secret") && !configEntry.key.isEmpty()) {
                invocation.secrets.insert(configEntry.key,
                                          m_impl->secrets->get(pluginId, configEntry.key));
            }
        }
    }

    return entry->loaded->instance->invoke(invocation);
}

} // namespace selectum
