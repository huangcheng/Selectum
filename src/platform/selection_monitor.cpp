#include "selection_monitor.h"

#include "noop_selection_monitor.h"

#if defined(Q_OS_MACOS)
#include "macos/selection_monitor_mac.h"
#endif

namespace selectum {

std::unique_ptr<SelectionMonitor> createSelectionMonitor(QObject *parent)
{
#if defined(Q_OS_MACOS)
    return std::make_unique<SelectionMonitorMac>(parent);
#else
    return std::make_unique<NoopSelectionMonitor>(parent);
#endif
}

} // namespace selectum
