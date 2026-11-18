# macOS Selection Monitor — Design Spec

**Date:** 2026-11-18  
**Status:** Approved by go-ahead (brainstorming opted out)  
**Parent:** `PRD.md` M1, `docs/superpowers/specs/2026-11-17-selectum-core-design.md`

## Intent

Add `src/platform/` with a **SelectionMonitor** that, on macOS, turns a settled text selection into a `selectum::SelectionContext` and feeds the existing `PluginHost` match/invoke path. No Flomo/Cubox in this slice. Selection **bar chrome** stays minimal (reuse/extend host UI list + status); full pill bar is a follow-on.

## Goals

1. Abstract `SelectionMonitor` API under `src/platform/` (OS backends plug in later).
2. macOS backend via **Accessibility (AX)** — preferred over clipboard.
3. Emit only after selection **settles** (mouse-up / short debounce) — no flicker while dragging.
4. Fill `text`, `appId`, `editable`, `screenRect`, `trigger=Selection` when available; `pageUrl`/`title` best-effort.
5. Surface Accessibility permission state; host can prompt user to open System Settings.
6. Keep Fake selection for tests/dev when AX unavailable.

## Non-goals

- Windows / Linux backends (stubs OK).
- Clipboard synthesis fallback (document as later).
- Full PopClip-style frameless pill bar.
- Flomo/Cubox plugins; real keychain.
- OOP plugin host.

## Approaches

| Approach | Notes |
|---|---|
| **A. AX + settle (chosen)** | Matches PRD; low flicker |
| B. Poll AX continuously | Noisy, battery/CPU, flicker |
| C. Clipboard-only | Last resort; not for M1 first cut |

## Design

```
src/platform/
  selection_monitor.h          # abstract QObject API
  selection_monitor.cpp        # factory createSelectionMonitor()
  macos/selection_monitor_mac.mm
  macos/ax_selection.h/.mm     # AX helpers (testable where possible)
```

**API (conceptual):**

- `start()` / `stop()`
- `signal selectionSettled(SelectionContext)`
- `PermissionStatus accessibilityPermission()` + `requestAccessibilityPermission()` (open settings)
- Ignore selections originating from Selectum’s own UI (bundle id filter)

**Settle policy:** on left mouse-up (global event tap or AX observer + debounce ~50–100ms), read focused element’s selected text; if non-empty and changed, emit.

**Host wiring:** `main` creates monitor on macOS, connects to MainWindow slot that runs `PluginHost::match` and updates the matched list (same as Fake selection path).

## Success

- With AX granted: select text in TextEdit/Safari → Selectum shows stub (or matched plugins) without using Fake button.
- With AX denied: app still runs; Fake selection works; UI indicates permission needed.
- Existing `ctest` suite stays green.
