# macOS Selection Monitor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `src/platform` SelectionMonitor with a macOS AX backend that emits settled `SelectionContext` into the host match UI.

**Architecture:** Abstract `SelectionMonitor` (QObject) + macOS `.mm` backend; factory returns platform impl. Host connects `selectionSettled` to the same match path as Fake selection. Settle after mouse-up + short debounce.

**Tech Stack:** Qt 6 Widgets/Core, CMake, macOS ApplicationServices/Cocoa/Carbon for AX + event tap as needed, Objective-C++.

**Spec:** `docs/superpowers/specs/2026-11-18-macos-selection-monitor-design.md`

## Global Constraints

- Prefer AX over clipboard; no clipboard fallback in this plan.
- Emit only after settle (mouse-up / ~80ms debounce); `trigger = Selection`.
- Filter out Selectum’s own bundle id.
- App must run without AX permission; Fake selection remains.
- No Flomo/Cubox; no full pill bar; keep existing ctest green.
- Everything-is-a-plugin: platform code only assembles `SelectionContext`, never invokes providers.

## Review Focus

- Dragging selection must not spam emits (settle only).
- Empty / whitespace-only selection → no emit.
- AX denied → no crash; permission status queryable.
- Selecting inside Selectum window → ignored.
- Existing PluginHost tests still pass.

---

## File structure

```
src/platform/selection_monitor.h
src/platform/selection_monitor.cpp          # factory
src/platform/macos/ax_selection.h
src/platform/macos/ax_selection.mm
src/platform/macos/selection_monitor_mac.h
src/platform/macos/selection_monitor_mac.mm
tests/test_selection_settle.cpp             # debounce/dedupe helper tests if extracted
```

---

### Task 1: Platform API + settle helper + CMake

**Files:**
- Create: `src/platform/selection_monitor.h`
- Create: `src/platform/selection_monitor.cpp` (factory stub returning nullptr or Noop on non-Mac)
- Create: `src/platform/selection_settle.h/.cpp` — pure helper: given previous text + candidate + settled flag → shouldEmit
- Create: `tests/test_selection_settle.cpp`
- Modify: `CMakeLists.txt` — `selectum_platform` lib; link into Selectum; `.mm` only on APPLE

**Interfaces:**
- Produces:
  - `enum class AccessibilityPermission { Unknown, Granted, Denied }`
  - `class SelectionMonitor : public QObject` with `start()`, `stop()`, `accessibilityPermission()`, signals `void selectionSettled(const selectum::SelectionContext &)`
  - `std::unique_ptr<SelectionMonitor> createSelectionMonitor(QObject *parent = nullptr)`
  - `bool shouldEmitSettledSelection(const QString &previous, const QString &candidate, bool isSettleEvent)` — empty/whitespace → false; unchanged text → false; require isSettleEvent

- [ ] **Step 1: Write failing test** `test_selection_settle` cases: empty rejected; whitespace rejected; same text rejected; new text on settle accepted; new text without settle rejected

- [ ] **Step 2: Run RED** — `ctest -R test_selection_settle`

- [ ] **Step 3: Implement helper + SelectionMonitor headers + noop factory + CMake**

- [ ] **Step 4: GREEN** — all ctest including new test

- [ ] **Step 5: Commit** — `feat(platform): add SelectionMonitor API and settle helper`

---

### Task 2: macOS AX read + SelectionMonitorMac

**Files:**
- Create: `src/platform/macos/ax_selection.h/.mm` — `readFocusedSelection(SelectionContext *out)` using AX focused UI element / selected text + selected text range bounds; fill appId via NSWorkspace frontmost
- Create: `src/platform/macos/selection_monitor_mac.h/.mm` — event tap or NSEvent local/global monitor for left mouse up → debounce → readFocusedSelection → shouldEmit → signal
- Modify: factory to return `SelectionMonitorMac` on `__APPLE__`
- Modify: CMake link ApplicationServices, Cocoa, optionally Carbon

**Interfaces:**
- Consumes: settle helper, SelectionContext
- Produces: working `createSelectionMonitor()` on macOS

- [ ] **Step 1: Write a narrow test or manual checklist** — AX hard to unit test; add `tests/test_ax_selection_smoke.cpp` only if injectable fake; else document manual: TextEdit select → log/context

- [ ] **Step 2: Implement AX read + monitor with ~80ms QTimer debounce on mouse-up**

- [ ] **Step 3: Build Selectum.app; verify compile**

- [ ] **Step 4: Commit** — `feat(platform): macOS AX SelectionMonitor with settle-on-mouseup`

---

### Task 3: Host wiring + permission UX

**Files:**
- Modify: `src/host/main.cpp` — create monitor, start it
- Modify: `src/host/mainwindow.h/.cpp` — slot `onSelectionSettled`; show permission hint label/button “Open Accessibility Settings”; keep Fake selection
- Info.plist / CMake: `NSAppleEventsUsageDescription` or accessibility usage string if required for the tap; ensure hardened runtime notes in README snippet not required for local debug

**Interfaces:**
- Consumes: `selectionSettled` → same match UI path as Fake

- [ ] **Step 1: Wire signal to match list update**

- [ ] **Step 2: Manual smoke** — grant AX, select in TextEdit, see stub match; deny AX, Fake still works

- [ ] **Step 3: Full ctest green**

- [ ] **Step 4: Commit** — `feat(host): wire SelectionMonitor into match UI`

---

## Self-review

| Spec item | Task |
|---|---|
| src/platform abstract API | T1 |
| Settle / no drag spam | T1–T2 |
| macOS AX | T2 |
| Permission + Fake remains | T3 |
| No clipboard / no Flomo | all |
