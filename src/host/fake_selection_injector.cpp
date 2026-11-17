#include "fake_selection_injector.h"

namespace selectum {

SelectionContext FakeSelectionInjector::makeContext(const QString &text,
                                                    const QString &appId,
                                                    bool editable,
                                                    Trigger trigger)
{
    SelectionContext ctx;
    ctx.text = text;
    ctx.appId = appId;
    ctx.editable = editable;
    ctx.trigger = trigger;
    return ctx;
}

} // namespace selectum
