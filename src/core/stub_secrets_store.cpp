#include <core/stub_secrets_store.h>

namespace selectum {

QString StubSecretsStore::compoundKey(const QString &pluginId, const QString &key)
{
    return pluginId + QLatin1Char('\0') + key;
}

QString StubSecretsStore::get(const QString &pluginId, const QString &key) const
{
    return m_secrets.value(compoundKey(pluginId, key));
}

void StubSecretsStore::set(const QString &pluginId, const QString &key, const QString &value)
{
    m_secrets.insert(compoundKey(pluginId, key), value);
}

} // namespace selectum
