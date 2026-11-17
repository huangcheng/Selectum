# Selectum Core — Design Spec

**Date:** 2026-11-17  
**Status:** Approved — 2026-11-17  
**Product contract:** `PRD.md`  
**Agent invariants:** `AGENTS.md`

## 1. Intent

Build the **host core** first: discover → match → load → invoke plugins, with a serializable invocation ABI. Selection UI chrome and OS selection backends come after this spine is stable.

Selectum is a thin native command engine. Product features (Flomo, Cubox, copy, translate, …) are plugins on one ABI — never hardcoded into the host.

## 2. Why

Without a locked core ABI, M0/M1 scaffolding will fight later isolation and third-party plugins. Core-first lets Flomo/Cubox (and every later sink) land as packages with **zero host feature code**.

## 3. Goals

1. One native plugin ABI (`QPluginLoader` + C++ IID) shared by first- and third-party plugins.
2. Manifest-driven identity, UI labels/icons, match rules, config schema, capabilities.
3. Rich `SelectionContext` → `Invocation` → `Result` as POD/serializable types (IPC-ready).
4. Host-owned secrets via OS keychain; plugins declare needs only.
5. Host can run with **zero** action plugins loaded.
6. Design leaves a clean path to an out-of-process restricted plugin host later.

## 4. Non-goals / anti-goals

- Hardcoding Flomo / Cubox / translate / copy / todo into the host.
- Script/WASM plugin runners in v1.
- Claiming plugins are “sandboxed” while loading in-process.
- Custom vault product (OS keychain only for v1).
- Selection bar visual polish, palette, clipboard fronts (separate specs after core).
- Perfect Wayland selection parity.
- Plugin marketplace / signing requirement for MVP.

**Anti-goal:** A “god” `MainWindow` that grows provider logic.

## 5. Constraints

| Constraint | Choice |
|---|---|
| UI / app | Qt 6 Widgets |
| Build | CMake |
| Plugins (v1) | `QPluginLoader` + declared C++ interface |
| Manifest | JSON `plugin.json`, `schemaVersion: 1` |
| Secrets | OS keychain / Credential Manager / Secret Service |
| Process model (M0/M1) | In-process load |
| Process model (later) | OOP plugin host over same serializable ABI |

## 6. Approaches considered

### A — Native `QPluginLoader` first (chosen)

C++ interface + JSON manifest. Matches Albert-like Qt architecture, one ABI story, best fit for a thin Widgets host.

- **Pros:** Type-safe invoke, no script runtime, clear path to OOP later if types stay POD.  
- **Cons:** Higher cost to author first plugins than scripts.

### B — Script/manifest plugins first

Faster Flomo/Cubox prototypes; native ABI deferred.

- **Pros:** Ship sinks quickly.  
- **Cons:** Two worlds later; fights “one ABI”; weak story for restricted host.

### C — Dual runners from day one

Native + script behind one façade.

- **Pros:** Flexibility.  
- **Cons:** Doubles core surface before M1; rejected for “build core first.”

**Decision:** Approach **A**. Scripts only if a later design adds them explicitly.

## 7. Architecture (target layering)

```
src/host/       # entry, tray, settings shell
src/core/       # Invocation types, PluginHost, match engine (no providers)
src/platform/   # SelectionMonitor backends (after core)
src/ui/         # SelectionBar, toasts (after core)
plugins/        # first-party plugins (flomo, cubox, …)
```

Dependency direction: `platform` → `core` ← `ui`/`host`; actions only under `plugins/`.

### 7.1 Process model

| Phase | Model |
|---|---|
| M0/M1 | In-process `QPluginLoader` |
| Post-M1 | Optional restricted helper process; same `Invocation`/`Result` over IPC |

Until OOP ships, document trust: first-party + user-local installs; capabilities are policy hints, not a sandbox.

## 8. Package layout

```
plugins/<plugin-id>/
  plugin.json          # required
  icon.png             # optional; path from manifest
  lib<name>.dylib      # macOS
  <name>.dll           # Windows
  lib<name>.so         # Linux
```

Search paths (host-resolved, order TBD by builder within reason):

1. App-bundle / install-relative `plugins/`
2. User data `plugins/` (user-installed)

## 9. Manifest schema (v1)

File: `plugin.json`. Unknown fields are ignored; missing required fields → plugin skipped with a log/toast-friendly reason.

### 9.1 Required

| Field | Type | Notes |
|---|---|---|
| `schemaVersion` | number | `1` for this spec |
| `id` | string | Reverse-DNS, stable (`com.selectum.collect.flomo`) |
| `name` | string | Display name |
| `version` | string | Semver-ish string |
| `apiVersion` | number | Host plugin IID generation; `1` for this spec |
| `entry.native` | string | Library basename without prefix/suffix (host maps to `lib<entry>.dylib` etc.) |

### 9.2 Optional

| Field | Type | Notes |
|---|---|---|
| `icon` | string | Relative path inside package |
| `triggers` | string[] | `selection` \| `palette` \| `hotkey` \| `clipboard` |
| `capabilities` | string[] | e.g. `network`, `clipboard.write` — policy/UI only in v1 |
| `match` | object | See §10 |
| `config` | array | Settings + secret declarations |

Host-side (not in the package manifest): per-plugin **enabled** flag in settings (default `true`). Match engine skips disabled plugins.

### 9.3 Config entries

```json
{
  "key": "apiToken",
  "type": "secret",
  "label": "Flomo API token",
  "required": true
}
```

| `type` | Storage |
|---|---|
| `secret` | OS keychain; never written to plain settings JSON |
| `string` / `bool` / `number` / `enum` | Host settings store (non-secret) |

### 9.4 Example

```json
{
  "schemaVersion": 1,
  "id": "com.selectum.collect.flomo",
  "name": "Save to Flomo",
  "version": "0.1.0",
  "apiVersion": 1,
  "icon": "icon.png",
  "entry": { "native": "flomo" },
  "triggers": ["selection", "palette", "clipboard"],
  "capabilities": ["network"],
  "match": {
    "minLength": 1,
    "editableOnly": false,
    "appAllow": [],
    "appDeny": [],
    "regex": null
  },
  "config": [
    {
      "key": "apiToken",
      "type": "secret",
      "label": "Flomo API token",
      "required": true
    }
  ]
}
```

## 10. Match engine (host)

Host filters **before** invoke using manifest `match` + `triggers`:

| Rule | Behavior |
|---|---|
| enabled (host settings) | Disabled plugins never match |
| `triggers` | Invocation `trigger` must be listed (if array non-empty) |
| `minLength` | `context.text` length ≥ N |
| `editableOnly` | If `true`, require `context.editable` |
| `appAllow` | If non-empty, `appId` must match one entry |
| `appDeny` | If `appId` matches, exclude |
| `regex` | If set, `text` must match |

Empty allow/deny lists mean no restriction. Matching is host-side; a C++ `matches()` override is optional and may only further restrict (not broaden) host results — **builder call:** start without C++ override; add if a real plugin needs it.

## 11. C++ plugin interface (apiVersion 1)

Conceptual IID (exact Qt IID string left to builder, must be stable and versioned):

```text
ISelectumPlugin (apiVersion 1)
  id() -> string                 # must equal manifest id
  invoke(Invocation) -> Result   # required
```

- No `QWidget*`, no host singletons, no access to other plugins.
- Plugin may use Qt networking / its own code; host does not inject HTTP clients as a required ABI in v1.
- Library export: standard Qt plugin metadata (`Q_PLUGIN_METADATA` / IID).

## 12. Invocation & Result (POD)

Serializable (JSON or equivalent) for future IPC. No Qt GUI types on the boundary.

### 12.1 SelectionContext

| Field | Type | Notes |
|---|---|---|
| `text` | string | Selected / clipboard / query text |
| `appId` | string | May be empty |
| `pageUrl` | string | Optional; critical for Cubox when known |
| `title` | string | Optional |
| `editable` | bool | Replace-in-place allowed |
| `screenRect` | `{x,y,w,h}` | Logical px; may be zero/empty |
| `trigger` | enum | `selection` \| `palette` \| `hotkey` \| `clipboard` \| … |

### 12.2 Invocation

| Field | Type | Notes |
|---|---|---|
| `id` | string | UUID per invoke |
| `pluginId` | string | Manifest id |
| `context` | SelectionContext | |
| `config` | map\<string,string\> | Non-secret settings |
| `secrets` | map\<string,string\> | From keychain; in-memory for this call only |
| `locale` | string | Host UI locale |

### 12.3 Result

| Field | Type | Notes |
|---|---|---|
| `status` | enum | `ok` \| `error` \| `cancelled` |
| `message` | string | User-visible toast / error |
| `openUrl` | string | Optional |
| `replaceText` | string | Optional; host applies only if `editable` |

**v1 omissions (explicit):** streaming progress, cancel tokens, binary payloads.

### 12.4 Threading & Result application

- **Threading:** never call `invoke` on the UI thread. Host runs invoke on a worker; marshals `Result` back to the UI thread before side effects.
- **Side effects:** `PluginHost` returns `Result` only. The **host/UI shell** applies toast, `openUrl`, and `replaceText` (replace only when `editable`). Core does not own QWidget toasts.

## 13. Secrets

- Plugins declare `type: "secret"` config keys in the manifest.
- Host stores values in OS secure storage keyed by `pluginId` + config `key`.
- Host injects secrets into `Invocation.secrets` at invoke time.
- Settings UI shows configured / not configured; do not re-display raw secrets after save.
- Never commit tokens; examples use placeholders only.

## 14. PluginHost responsibilities (core)

1. Scan search paths for `plugin.json`.
2. Validate schema; skip invalid packages with reason.
3. Resolve native library path from `entry.native`.
4. Load via `QPluginLoader` when first needed (lazy) or at startup (builder choice; prefer lazy for M0).
5. Given an assembled `SelectionContext` + trigger, compute matched plugin list.
6. On user action: resolve config + secrets → `Invocation` → `invoke` → surface `Result` to host/UI (toast / open URL / replace).

Core does **not** implement selection capture UI; tests may use a fake injector that builds `SelectionContext`.

## 15. Brand / UI notes (out of core implementation scope)

- Selection bar chrome references (pill, overflow menu, settings toggle) are **visual only**; actions come from matched plugins.
- Do not ship Yuanbao-like default actions (search/translate/copy/speak) as host features.
- App icon: `assets/brand/selectum-icon.png` (+ `.icns` / `.ico`) already wired for the shell.

## 16. Milestones mapping & plan cutoff

| Milestone | Core relevance |
|---|---|
| **M0** | PluginHost, fake `SelectionContext` injector, zero plugins ⇒ empty actions, load ≥1 stub plugin, JSON round-trip test |
| **M1** | Flomo/Cubox **plugin packages** + toast wiring; consume this ABI (separate implementation plan) |
| **M2+** | Platform selection backends; optional OOP plugin host |

**Plan cutoff for implementing this spec:** **M0 only** (core + stub plugin + fake injector). M1 sinks are out of this plan’s delivery scope but must not require ABI breaks.

**Secrets UI:** M0 may use a stub/test keychain injector. A minimal settings shell can exist as host chrome; real per-plugin secret forms land with M1.

**ABI shipping (call):** provide an in-repo `selectum_abi` (INTERFACE or static) target — headers for `ISelectumPlugin` + POD types — linked by host and plugins. No separate published SDK for MVP.

## 17. Left to the builder

- Exact C++ header names, IID UUID string, and JSON field parsing helpers.
- Whether `config` map values are always strings or typed (stringly-typed map is acceptable for v1).
- Lazy vs eager `QPluginLoader` (prefer lazy).
- Logging framework.
- Unit-test layout for match engine.
- Concrete keychain wrapper API per OS.
- Worker-thread primitive (Qt thread pool vs dedicated queue) as long as UI thread never blocks on invoke.

## 18. Success criteria

- Empty plugin directory → host runs; no actions.
- Stub native plugin with manifest → appears in match list for a fake selection; invoke returns `Result`.
- Adding Flomo/Cubox requires only new packages under `plugins/` — no host feature branches.
- `Invocation`/`Result` round-trip through JSON without loss of required fields (prep for OOP).

---

## Document control

| Item | Value |
|---|---|
| Spec | Selectum Core |
| Path | `docs/superpowers/specs/2026-11-17-selectum-core-design.md` |
| Related | `PRD.md`, `AGENTS.md` |

Update this doc when ABI, manifest schema, or process model decisions change.
