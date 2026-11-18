#pragma once

#include "selection_monitor.h"

namespace selectum {

class NoopSelectionMonitor final : public SelectionMonitor
{
    Q_OBJECT

public:
    explicit NoopSelectionMonitor(QObject *parent = nullptr)
        : SelectionMonitor(parent)
    {
    }

    void start() override {}
    void stop() override {}
    AccessibilityPermission accessibilityPermission() const override
    {
        return AccessibilityPermission::Unknown;
    }
};

} // namespace selectum
