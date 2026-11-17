#pragma once

#include <QObject>

#include <selectum/abi/plugin_interface.h>

namespace selectum {

class StubPlugin : public QObject, public ISelectumPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SelectumPlugin_iid)
    Q_INTERFACES(selectum::ISelectumPlugin)

public:
    QString id() const override;
    Result invoke(const Invocation &invocation) override;
};

} // namespace selectum
