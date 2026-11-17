#pragma once

#include <QString>

namespace selectum {

class SecretsStore
{
public:
    virtual ~SecretsStore() = default;

    virtual QString get(const QString &pluginId, const QString &key) const = 0;
    virtual void set(const QString &pluginId, const QString &key, const QString &value) = 0;
};

} // namespace selectum
