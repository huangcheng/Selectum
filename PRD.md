# Selectum — Product Requirements Document

## 1. Vision

Selectum is a **native, cross-platform productivity command engine**.

It is closer to **Alfred / Raycast** than to a 划词翻译 tool: a thin host that
invokes **plugins**. Text selection is a first-class input surface (PopClip-style
floating bar), not the whole product. The same commands also run from a hotkey
palette, clipboard/universal actions, and deep links.

**Tagline:** Select text (or type a query) → digest → send to a service.

**Stack:** Qt 6 + Widgets, CMake. No Electron. No embedded browser UI for the
core shell.

**Platforms:** macOS, Windows, Linux (X11 full; Wayland best-effort).

## 2. Problem

Power users constantly move fragments of text into other tools: Flomo, Cubox,
todo apps, search, AI, notes. Existing options force a bad tradeoff:

| Option | Problem |
|---|---|
| PopClip / Selected | Excellent selection UX; **macOS only** |
| Alfred / Raycast | Strong command model; **macOS-centric** |
| uTools / similar | Plugin ecosystems, but **Electron** / heavy web stacks |
| Per-OS scripts | Works, but no shared ABI or UX across machines |

There is no **native** Win + macOS + Linux product that combines:

1. Floating selection action bar  
2. Alfred-like command/plugin engine  
3. Pluggable sinks (Flomo, Cubox, Todoist, …) on one ABI  

Selectum fills that gap.

## 3. Goals

1. **Everything is a plugin** — copy, search, translate, speak, collect (Flomo /
   Cubox), create todo, ask AI, etc. all ship as plugins. The host never grows
   feature logic.
2. **One ABI, three OSes** — the same plugin package works (or degrades
   gracefully) on macOS, Windows, and Linux.
3. **Selection as a first-class trigger** — select text → bar appears → action
   runs with rich context (text, app, URL when available, editable flag).
4. **Multiple fronts, one engine** — selection bar, hotkey palette, clipboard
   actions, and URL schemes all produce the same `Invocation`.
5. **Native and light** — Qt Widgets, tray-resident, no Electron; stay out of
   the way until invoked.
6. **Privacy-respecting defaults** — selection text goes only to the plugin /
   provider the user chose; no mandatory cloud account for the host.

## 4. Non-goals (v1)

- Replacing a full IDE, browser, or notes app
- Being a translation-first product (translate is a plugin, not the brand)
- Perfect Wayland global selection parity with X11/macOS/Windows
- Electron, Tauri, or WebView as the primary shell
- Hardcoding Flomo/Cubox/Todoist into the core binary
- A closed plugin marketplace requirement for MVP (local plugins first)

## 5. Core principle: everything is a plugin

The host is responsible only for:

- Lifecycle (tray, permissions prompts, settings shell)
- Selection capture (per-OS backends) and invocation assembly
- Plugin discovery, load, match, invoke
- Shared surfaces (selection bar, palette, result/toast chrome)

Built-in shipping plugins (same ABI as third-party):

- Collect → Flomo, Cubox, local Markdown, …
- Create Todo → Todoist / Things / system reminders / …  
- Search, Copy, Speak, Open URL  
- AI ask / summarize (user-configured providers)

If a feature can be expressed as a command + provider, it **must** be a plugin.

## 6. Primary use cases

### 6.1 Collect selection → knowledge / read-later

- Select text → **Save to Flomo** (memo + optional tags / source)
- Select text or URL → **Save to Cubox** (clip / later-read; prefer page URL when known)

### 6.2 Create work items

- Select `Buy milk tomorrow @errands` → **Create Todo** → chosen provider  
  Plugin digests title / due / tags; host only invokes.

### 6.3 Universal actions

- Same command list available from: selection bar, palette (typed query),
  clipboard history item
- Example: “Collect” with provider picker; toolbar may pin Flomo / Cubox shortcuts

### 6.4 Launcher-style commands (phase 2+)

- App launch, file search, web search templates, run shell, snippets  
  Still plugins; palette is just another front-end.

## 7. Product surfaces

| Surface | Role | MVP |
|---|---|---|
| Selection toolbar | Frameless pill near selection; plugin actions | Yes |
| System tray | Keep alive, settings, quit, permission status | Yes |
| Settings | Enable plugins, tokens, per-app action lists | Yes |
| Hotkey palette | Alfred-like query box | Phase 2 |
| Clipboard / universal actions | Act on clipboard like selection | Phase 2 |
| Result panel / toast | Confirm save, show errors, optional open-URL | Yes |

## 8. Selection context (host → plugin)

Plugins receive a structured context, not a bare string:

| Field | Purpose |
|---|---|
| `text` | Selected text |
| `appId` | Source app (bundle id / exe) |
| `pageUrl` / `title` | When available (browsers) — critical for Cubox |
| `screenRect` | Anchor the toolbar |
| `editable` | Whether replace-in-place is allowed |
| `trigger` | `selection` \| `palette` \| `hotkey` \| `clipboard` \| … |

Clipboard fallback for reading selection is a **policy** (prefer AX / UIA /
platform APIs; synthesize copy only as last resort; restore clipboard).

## 9. Plugin model (product requirements)

- Package format: directory + manifest + native library and/or script entry
  (exact format TBD in design spec; conceptually PopClip/Albert-like)
- Capabilities declared in manifest (triggers accepted, match rules, config
  schema for secrets/options)
- Host filters by match rules (regex, app allow/deny, min length, editable-only)
- Auth/secrets stored by host keychain/secure settings; plugins declare needs,
  never bake secrets into the package
- Failures surface as user-visible errors (e.g. Flomo token invalid) without
  crashing the host

## 10. Platform expectations

| OS | Selection bar | Notes |
|---|---|---|
| macOS | Full | Accessibility permission required |
| Windows | Full | UI Automation + input hooks |
| Linux X11 | Full | PRIMARY / accessibility as available |
| Linux Wayland | Limited | Hotkey / clipboard / explicit invoke; document limits |

Toolbar must not steal focus in a way that clears selection when clicked
(`Qt::Tool` / show-without-activating patterns).

## 11. Success metrics (qualitative MVP)

- Same Collect → Flomo and Collect → Cubox flow works on the developer’s
  macOS, Windows, and Linux (X11) machines with one plugin ABI
- Adding a new sink (e.g. Notion) requires **only** a new plugin — zero host
  feature code
- Idle memory and UX feel native (competitive with Albert/PopClip class, not
  Electron launchers)
- User can disable every action plugin and still have a running empty host

## 12. Milestones

### M0 — Host skeleton
- Tray app, settings shell, plugin loader stub, fake selection injector for UI
- Empty action set proves “everything is a plugin”

### M1 — Selection bar + two collect plugins
- Real selection on at least one OS (prefer macOS first)
- Plugins: **Flomo**, **Cubox**
- Toast/result on success/failure

### M2 — Cross-platform selection
- Windows + Linux X11 backends
- Per-app enable/disable lists
- Wayland documented limitations

### M3 — Command depth
- Create Todo plugin family
- Provider picker / pinned toolbar shortcuts
- Match rules + config UI polish

### M4 — Palette & universal actions
- Hotkey command palette
- Clipboard as invocation source
- Usage-based ranking (optional)

## 13. Competitive positioning

| Product | Selectum relationship |
|---|---|
| PopClip / Selected | UX reference for the bar; Mac-only — we go cross-OS |
| Alfred / Raycast | Command model reference; we stay native + multi-OS |
| Albert | Closest Qt/plugin architecture reference |
| Wox | Cross-OS launcher reference; we add selection-first |
| uTools / Quicker | Ecosystem inspiration; **not** the tech model |

**One-liner:** Native cross-platform command engine (Qt) — Raycast-like
extensions, with selection as a first-class input; everything is a plugin.

## 14. Open questions

- Exact plugin ABI (Qt `QPluginLoader` IID vs script plugins first)
- Manifest schema and signing (later)
- Whether palette ships in M3 or waits for M4
- Official vs community Flomo/Cubox API terms and rate limits documentation
- Auto-update strategy for host and plugins

## 15. Document control

| Item | Value |
|---|---|
| Product | Selectum |
| Repo | `/Users/huangcheng/Projects/Selectum` |
| Status | Draft PRD — guides implementation |
| Related | `AGENTS.md` (agent/engineering invariants) |

Update this PRD when product scope or non-goals change. Detailed technical
design belongs in a design spec under `docs/` once brainstorming locks ABI
details.
