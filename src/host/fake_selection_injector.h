#pragma once

#include <QString>

#include <selectum/abi/types.h>

namespace selectum {

class FakeSelectionInjector
{
public:
    static SelectionContext makeContext(const QString &text,
                                        const QString &appId = QStringLiteral("com.selectum.fake"),
                                        bool editable = false,
                                        Trigger trigger = Trigger::Selection);
};

} // namespace selectum
