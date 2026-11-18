#pragma once

#include <platform/selection_monitor.h>
#include <selectum/abi/types.h>

namespace selectum {

// Reads the focused UI element's selected text via Accessibility.
// Returns false if AX is unavailable, nothing focused, or no selection.
bool readFocusedSelection(SelectionContext *out);

AccessibilityPermission queryAccessibilityPermission();
void requestAccessibilityPermissionPrompt();

} // namespace selectum

