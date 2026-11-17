#pragma once

#include <QHash>
#include <QString>

#include <core/secrets_store.h>

namespace selectum {

class StubSecretsStore : public SecretsStore
{
public:
    QString get(const QString &pluginId, const QString &key) const override;
    void set(const QString &pluginId, const QString &key, const QString &value) override;

private:
    static QString compoundKey(const QString &pluginId, const QString &key);

    QHash<QString, QString> m_secrets;
};

} // namespace selectum
