#include "selection_settle.h"

namespace selectum {

bool shouldEmitSettledSelection(const QString &previous,
                                const QString &candidate,
                                bool isSettleEvent)
{
    if (!isSettleEvent) {
        return false;
    }
    const QString trimmed = candidate.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }
    if (trimmed == previous.trimmed()) {
        return false;
    }
    return true;
}

} // namespace selectum
