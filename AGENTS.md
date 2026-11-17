# Selectum — agent instructions

Selectum is a **native cross-platform productivity command engine** (Qt 6
Widgets): selection toolbar + (later) Alfred-like palette, with pluggable
actions that send text to services (Flomo, Cubox, todos, search, AI, …).

Product contract: **`PRD.md`**. Read it before changing scope, surfaces, or
milestones. Update `PRD.md` when a product decision changes.

## Non-negotiable: everything is a plugin

- The host stays thin: lifecycle/tray, selection capture, plugin
  discover/load/match/invoke, shared chrome (bar, palette, toast/settings).
- **Do not** hardcode product features into the core (no built-in Flomo/Cubox/
  translate/todo/copy logic in the host).
- First-party actions ship as plugins on the **same ABI** as third-party
  plugins. If the host can run with zero action plugins loaded, the design is
  healthy.
- Prefer extending a plugin or adding a plugin over growing `MainWindow` /
  core “god objects.”

## Stack

| Piece | Choice |
|---|---|
| UI / app | Qt 6 Widgets (not Quick/QML for the shell unless PRD changes) |
| Build | CMake |
| OS | macOS, Windows, Linux (X11 full; Wayland limited — see PRD) |
| Plugins | `QPluginLoader` / declared interfaces first; scripts only if designed |

- No Electron, no WebView-based primary shell.
- Do not add heavy dependencies without saying why against the PRD.

## Architecture map (target)

```
src/host/          # app entry, tray, settings shell
src/core/          # Invocation, PluginHost, match engine (no providers)
src/platform/      # SelectionMonitor backends (mac/win/linux)
src/ui/            # SelectionBar, toasts, shared widgets
plugins/           # first-party plugins (flomo, cubox, …)
```

Names may evolve; the **layering** must not: platform → core → ui/host, with
actions only under `plugins/`.

## Selection & UX rules

- Prefer OS accessibility APIs (AX / UIA / X11) over clipboard synthesis.
  Clipboard fallback is last resort; restore clipboard when used.
- Selection toolbar must not clear the selection when clicked (no careless
  focus steal).
- Pass rich `SelectionContext` into plugins (`text`, `appId`, `pageUrl`,
  `title`, `editable`, `screenRect`, `trigger`) — not a bare `QString`.

## Docs & process

- Product: `PRD.md`
- Future design/ABI specs: prefer `docs/` (create when locking plugin ABI)
- Do not invent Electron alternatives, marketplace backends, or Wayland
  “full parity” claims without updating the PRD
- Keep secrets out of the repo (API tokens for Flomo/Cubox etc. only in
  user settings / keychain; examples use placeholders)

## Verification

- Build with the project’s Qt 6 CMake setup; fix compile errors you introduce.
- Prefer **live smoke checks** for UI/selection/plugins (run the app, select
  text or use a fake injector, invoke a plugin).
- If you did not run a check, say so. Do not claim cross-OS behavior you did
  not exercise on that OS.

## Commits

- Only commit when the user asks.
- English imperative subjects; body explains **why**.
- Do not commit secrets, local Qt Creator user files clutter, or build trees.
