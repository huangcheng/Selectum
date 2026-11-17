#pragma once

#include <QObject>
#include <QString>

#include <selectum/abi/types.h>

#define SelectumPlugin_iid "dev.selectum.ISelectumPlugin/1.0"

namespace selectum {

struct ISelectumPlugin
{
    virtual ~ISelectumPlugin() = default;

    virtual QString id() const = 0;
    virtual Result invoke(const Invocation &invocation) = 0;
};

} // namespace selectum

Q_DECLARE_INTERFACE(selectum::ISelectumPlugin, SelectumPlugin_iid)
