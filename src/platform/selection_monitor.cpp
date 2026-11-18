#include "selection_monitor.h"

#include "noop_selection_monitor.h"

namespace selectum {

std::unique_ptr<SelectionMonitor> createSelectionMonitor(QObject *parent)
{
    // Task 2 replaces this with SelectionMonitorMac on Q_OS_MACOS.
    return std::make_unique<NoopSelectionMonitor>(parent);
}

} // namespace selectum
