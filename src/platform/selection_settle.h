#pragma once

#include <QString>

namespace selectum {

// Returns true when a settled selection should be emitted to the host.
// Whitespace-only and unchanged text never emit; requires isSettleEvent.
bool shouldEmitSettledSelection(const QString &previous,
                                const QString &candidate,
                                bool isSettleEvent);

} // namespace selectum
