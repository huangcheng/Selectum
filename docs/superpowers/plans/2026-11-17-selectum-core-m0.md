# Selectum Core M0 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship an M0 host core that discovers JSON plugin packages, matches on `SelectionContext`, loads a native stub via `QPluginLoader`, and returns a serializable `Result` — with zero action plugins still a valid run.

**Architecture:** Header-only/interface `selectum_abi` (POD + `ISelectumPlugin`) linked by `selectum_core` (manifest, match, PluginHost) and by plugins. The Widgets shell stays thin: fake selection injector + toast stub; no Flomo/Cubox in this plan.

**Tech Stack:** Qt 6.5+ (Core, Widgets, Test), CMake 3.19+, C++17 (or the Qt kit default), Qt Test + CTest.

**Spec:** `docs/superpowers/specs/2026-11-17-selectum-core-design.md`

## Global Constraints

- Everything is a plugin — no provider logic in host/core.
- Manifest: JSON `plugin.json`, `schemaVersion: 1`, `apiVersion: 1`.
- Native only: `QPluginLoader`; no script runners.
- `Invocation` / `Result` / `SelectionContext` must JSON round-trip (IPC-ready).
- Never call `invoke` on the UI thread; marshal `Result` to UI before side effects.
- `PluginHost` returns `Result` only; shell applies toast / `openUrl` / `replaceText`.
- Secrets: OS keychain later; M0 uses an injectable stub store.
- Plan cutoff: **M0 only** (stub plugin + fake injector). No Flomo/Cubox packages.
- Keep app icon wiring (`assets/brand/`, `:/icons/selectum.png`).

## Review Focus

- Empty / missing `plugins/` directory → host starts; matched actions list is empty (not a crash).
- Invalid `plugin.json` (bad schemaVersion / missing `id`) → package skipped; other plugins still load.
- Disabled plugin (host settings) → never appears in match results.
- `editableOnly: true` with `editable: false` → no match; `replaceText` ignored by shell when not editable.
- Worker-thread invoke → UI thread never blocks; `Result` side effects run on UI thread only.

---

## File structure

```
include/selectum/abi/
  types.h                 # SelectionContext, Invocation, Result, enums
  plugin_interface.h      # ISelectumPlugin + IID
  json.h                  # toJson / fromJson declarations

src/abi/                  # optional .cpp if json helpers not header-only
  json.cpp

src/core/
  plugin_manifest.h/.cpp  # parse + validate plugin.json
  match_engine.h/.cpp
  secrets_store.h         # abstract
  stub_secrets_store.h/.cpp
  plugin_host.h/.cpp

src/host/
  main.cpp                # moved from repo root
  mainwindow.h/.cpp/.ui   # moved; thin shell + injector hook
  fake_selection_injector.h/.cpp

plugins/com.selectum.stub/
  plugin.json
  stub_plugin.h/.cpp
  CMakeLists.txt

tests/
  test_types_json.cpp
  test_match_engine.cpp
  test_plugin_manifest.cpp
  test_plugin_host.cpp
  CMakeLists.txt

CMakeLists.txt            # root: abi, core, app, plugins, tests
```

---

### Task 1: Build scaffold + ABI types

**Files:**
- Create: `include/selectum/abi/types.h`
- Create: `include/selectum/abi/plugin_interface.h`
- Create: `include/selectum/abi/json.h`
- Create: `src/abi/json.cpp`
- Create: `tests/test_types_json.cpp`
- Create: `tests/CMakeLists.txt`
- Modify: `CMakeLists.txt` (add `selectum_abi`, Qt Test, `enable_testing`, move toward `src/host` layout — host move may complete in Task 5 if keeping compile green)

**Interfaces:**
- Consumes: nothing
- Produces:
  - `selectum::Trigger` enum: `Selection`, `Palette`, `Hotkey`, `Clipboard`
  - `selectum::ResultStatus` enum: `Ok`, `Error`, `Cancelled`
  - `selectum::ScreenRect { double x,y,w,h; }`
  - `selectum::SelectionContext` fields per spec §12.1
  - `selectum::Invocation` fields per spec §12.2
  - `selectum::Result` fields per spec §12.3
  - `QJsonObject selectum::toJson(const T&)`, `bool selectum::fromJson(const QJsonObject&, T*)`
  - `struct ISelectumPlugin` with IID `"dev.selectum.ISelectumPlugin/1.0"`, `virtual QString id() const = 0`, `virtual Result invoke(const Invocation&) = 0`
  - CMake target `selectum_abi` (PUBLIC include `include/`)

- [ ] **Step 1: Write the failing test**

In `tests/test_types_json.cpp`, Qt Test cases:

- `roundTripSelectionContext` — fill all fields including `trigger = Selection`, assert `fromJson(toJson(ctx), &out)` equal
- `roundTripInvocation` — includes `config` and `secrets` maps
- `roundTripResult` — `status = Error`, `message`, `openUrl`, `replaceText`

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir <build> -R test_types_json --output-on-failure`  
(or build `test_types_json` first)  
Expected: FAIL (target/types missing)

- [ ] **Step 3: Implement ABI types + JSON helpers + CMake `selectum_abi` / test target**

Keep maps as `QVariantMap` or `QMap<QString,QString>` (stringly config per spec). No GUI types.

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest --test-dir <build> -R test_types_json --output-on-failure`  
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add include/selectum/abi src/abi tests CMakeLists.txt
git commit -m "feat(abi): add Invocation POD types and JSON round-trip"
```

---

### Task 2: Match engine

**Files:**
- Create: `src/core/match_engine.h`
- Create: `src/core/match_engine.cpp`
- Create: `tests/test_match_engine.cpp`
- Modify: `CMakeLists.txt` / `src/core` target `selectum_core` (static lib linking `selectum_abi` + Qt::Core)

**Interfaces:**
- Consumes: `selectum::SelectionContext`, `selectum::Trigger`
- Produces:
  - `struct selectum::MatchRules { int minLength = 0; bool editableOnly = false; QStringList appAllow; QStringList appDeny; QString regex; QStringList triggers; }`
  - `struct selectum::PluginRecord { QString id; MatchRules match; bool enabled = true; }`
  - `QVector<PluginRecord> selectum::matchPlugins(const QVector<PluginRecord>& plugins, const SelectionContext& ctx, Trigger trigger)`

- [ ] **Step 1: Write the failing test**

Cases in `tests/test_match_engine.cpp`:

- `emptyPlugins_returnsEmpty`
- `minLength_filtersShortText`
- `editableOnly_requiresEditable`
- `appDeny_excludes`, `appAllow_restricts`
- `disabledPlugin_excluded`
- `trigger_mustBeListed` (manifest triggers non-empty)

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir <build> -R test_match_engine --output-on-failure`  
Expected: FAIL

- [ ] **Step 3: Implement `matchPlugins` in `src/core/match_engine.cpp`**

Apply rules in the order listed in spec §10. Invalid regex → treat as no match for that plugin (log later; test that it does not throw).

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest --test-dir <build> -R test_match_engine --output-on-failure`  
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/core/match_engine.* tests/test_match_engine.cpp CMakeLists.txt
git commit -m "feat(core): add host-side plugin match engine"
```

---

### Task 3: Manifest parser

**Files:**
- Create: `src/core/plugin_manifest.h`
- Create: `src/core/plugin_manifest.cpp`
- Create: `tests/test_plugin_manifest.cpp`
- Create: `tests/fixtures/valid_stub/plugin.json` (minimal valid)
- Create: `tests/fixtures/bad_schema/plugin.json` (`schemaVersion: 2` or missing `id`)

**Interfaces:**
- Consumes: MatchRules shape from Task 2
- Produces:
  - `struct selectum::PluginManifest { QString id, name, version, icon, entryNative; int schemaVersion; int apiVersion; MatchRules match; /* config entries + capabilities + triggers */ }`
  - `struct selectum::ConfigEntry { QString key, type, label; bool required; }`
  - `std::optional<PluginManifest> selectum::parsePluginManifest(const QByteArray& json, QString* error)`
  - `std::optional<PluginManifest> selectum::loadPluginManifest(const QString& pluginDir, QString* error)`

- [ ] **Step 1: Write the failing test**

- `parsesValidFixture` — assert `id == "com.selectum.stub"`, `entryNative`, `match.minLength`
- `rejectsMissingId` / `rejectsWrongSchemaVersion`
- `parsesSecretConfigEntry` — `type == "secret"`, `required == true`

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir <build> -R test_plugin_manifest --output-on-failure`  
Expected: FAIL

- [ ] **Step 3: Implement parser**

Require fields from spec §9.1; ignore unknown fields. `schemaVersion` must be `1`; `apiVersion` must be `1` for M0.

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest --test-dir <build> -R test_plugin_manifest --output-on-failure`  
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/core/plugin_manifest.* tests/test_plugin_manifest.cpp tests/fixtures
git commit -m "feat(core): parse and validate plugin.json v1"
```

---

### Task 4: Stub plugin + PluginHost discover/load/invoke

**Files:**
- Create: `plugins/com.selectum.stub/plugin.json`
- Create: `plugins/com.selectum.stub/stub_plugin.h`
- Create: `plugins/com.selectum.stub/stub_plugin.cpp`
- Create: `plugins/com.selectum.stub/CMakeLists.txt`
- Create: `src/core/secrets_store.h`
- Create: `src/core/stub_secrets_store.h/.cpp`
- Create: `src/core/plugin_host.h/.cpp`
- Create: `tests/test_plugin_host.cpp`
- Modify: root `CMakeLists.txt` (`add_subdirectory(plugins/com.selectum.stub)`, copy/plugin output dir next to tests/app)

**Interfaces:**
- Consumes: ABI `ISelectumPlugin`, manifest parser, match engine, secrets store
- Produces:
  - `class selectum::SecretsStore { virtual QString get(pluginId, key) const = 0; virtual void set(...) = 0; }`
  - `class selectum::StubSecretsStore : public SecretsStore`
  - `class selectum::PluginHost` with:
    - `void setSecretsStore(SecretsStore*)`
    - `void setEnabled(const QString& pluginId, bool enabled)`
    - `bool scan(const QString& pluginsRoot, QString* error)`
    - `QVector<PluginManifest> manifests() const`
    - `QVector<PluginManifest> match(const SelectionContext&, Trigger) const`
    - `Result invoke(const QString& pluginId, const SelectionContext&, Trigger, const QMap<QString,QString>& config)` — loads plugin if needed, fills `Invocation` (uuid, secrets from store for `type==secret` keys), calls `ISelectumPlugin::invoke` **synchronously on the calling thread** (async wrapper is Task 5)
  - Stub plugin `id()` returns `com.selectum.stub`; `invoke` returns `Result{Ok, message: "stub:" + context.text}`

`plugin.json` for stub:

```json
{
  "schemaVersion": 1,
  "id": "com.selectum.stub",
  "name": "Stub",
  "version": "0.1.0",
  "apiVersion": 1,
  "entry": { "native": "stub" },
  "triggers": ["selection"],
  "capabilities": [],
  "match": { "minLength": 1, "editableOnly": false, "appAllow": [], "appDeny": [], "regex": null },
  "config": []
}
```

- [ ] **Step 1: Write the failing test**

In `tests/test_plugin_host.cpp` (use a temp plugins dir containing built stub package):

- `scanEmptyDir_okNoManifests`
- `scanLoadsStubManifest`
- `matchAndInvokeStub_returnsOkMessage`
- `invokeMissingPlugin_returnsError`
- `disabledStub_notMatched`

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir <build> -R test_plugin_host --output-on-failure`  
Expected: FAIL

- [ ] **Step 3: Implement stub Qt plugin + PluginHost**

Use `Q_PLUGIN_METADATA(IID "dev.selectum.ISelectumPlugin/1.0")`. Resolve library name from `entry.native` with platform prefixes/suffixes. Prefer lazy `QPluginLoader`.

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest --test-dir <build> -R test_plugin_host --output-on-failure`  
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/core plugins/com.selectum.stub tests/test_plugin_host.cpp CMakeLists.txt
git commit -m "feat(core): PluginHost discover/load/invoke with stub plugin"
```

---

### Task 5: Host shell — fake injector + off-UI invoke + empty-plugin smoke

**Files:**
- Move: `main.cpp`, `mainwindow.*` → `src/host/`
- Create: `src/host/fake_selection_injector.h/.cpp`
- Modify: `src/host/mainwindow.*` — button/action “Fake selection” builds `SelectionContext`, shows matched plugin names, invokes selected/first match
- Modify: `src/host/main.cpp` — keep icon/`QApplication` setup; construct `PluginHost`, scan `plugins/` beside app (and/or build tree path)
- Modify: `CMakeLists.txt` — executable sources under `src/host/`; link `selectum_core`; ensure stub plugin deployed next to app (`$<TARGET_FILE_DIR:Selectum>/plugins/...`)

**Interfaces:**
- Consumes: `PluginHost::match`, `PluginHost::invoke`
- Produces:
  - `FakeSelectionInjector::makeContext(QString text, …) -> SelectionContext` (defaults: `trigger=Selection`, `editable=false`)
  - UI helper: run invoke via `QtConcurrent::run` or `QThreadPool`, deliver `Result` with `QMetaObject::invokeMethod`/`QTimer` to UI thread; show `QMessageBox` or status tip for `message` (toast stub OK for M0)

- [ ] **Step 1: Write the failing test (smoke / integration)**

Prefer an automated check if feasible: extend `test_plugin_host` already covers invoke. For shell, add a lightweight test or document manual smoke:

- Automated: `tests/test_plugin_host.cpp` already asserts empty scan.  
- Manual smoke checklist (run after build): launch app with empty plugins dir → starts; with stub deployed → Fake selection “hello” → message contains `stub:hello`.

Add `tests/test_invoke_off_ui_contract.cpp` only if you can assert thread id without flakiness; otherwise verify in manual smoke that UI stays responsive (spec Review Focus — pin with a unit test that `PluginHost::invoke` is documented as sync-on-caller, and shell wrapper uses worker: implement `selectum::invokeAsync(PluginHost*, …, std::function<void(Result)>)` in `src/core/invoke_async.h/.cpp` and test that the callback is not invoked before schedule returns).

Minimal automated pin:

- Create `src/core/invoke_async.h/.cpp` with `void invokeAsync(PluginHost&, pluginId, ctx, trigger, config, QObject* receiver, std::function<void(Result)> done)` posting `done` to `receiver`'s thread.
- Test: `callbackRunsOnReceiverThread` using a worker `QThread` as caller and a `QObject` living on main test thread.

- [ ] **Step 2: Run new async test to verify it fails**

Run: `ctest --test-dir <build> -R test_invoke_async --output-on-failure`  
Expected: FAIL

- [ ] **Step 3: Implement `invokeAsync` + move host + fake injector wiring**

Ensure app runs with zero plugins (scan missing dir → empty, no crash).

- [ ] **Step 4: Verify**

Run: `ctest --test-dir <build> --output-on-failure` (all tests PASS)  
Manual: launch `Selectum.app`, Fake selection with stub present.

- [ ] **Step 5: Commit**

```bash
git add src/host src/core/invoke_async.* CMakeLists.txt tests
git commit -m "feat(host): fake selection injector and async invoke wiring"
```

---

### Task 6: M0 verification gate

**Files:**
- Modify: none required unless gaps found
- Optional: `docs/superpowers/plans/2026-11-17-selectum-core-m0.md` checkboxes updated by executor

**Interfaces:**
- Consumes: all prior tasks
- Produces: green `ctest` + manual smoke notes in the commit message body

- [ ] **Step 1: Run full test suite**

Run: `ctest --test-dir <build> --output-on-failure`  
Expected: all PASS

- [ ] **Step 2: Manual smoke**

1. Empty plugins → app starts, match list empty  
2. Stub present → fake text → `ok` + `stub:…`  
3. Disable stub via `PluginHost::setEnabled` (temporary UI checkbox or debug hook) → no match  

- [ ] **Step 3: Commit** (only if fixes landed)

```bash
git commit -m "test(m0): verify core PluginHost smoke path"
```

---

## Self-review notes (author)

| Spec area | Task |
|---|---|
| ABI POD + JSON | T1 |
| Match rules + enabled + editableOnly | T2 |
| Manifest schema v1 | T3 |
| QPluginLoader + stub + secrets stub | T4 |
| Host thin shell + fake injector + off-UI invoke | T5 |
| Empty plugins success / M0 cutoff | T4–T6 |
| OOP later | ABI serializable only — no OOP code in M0 |
| Flomo/Cubox | Explicitly out of plan |

---

## Execution handoff

Plan complete and saved to `docs/superpowers/plans/2026-11-17-selectum-core-m0.md`. Please review the plan. Which execution approach would you prefer?

- **Subagent-driven** — A fresh subagent implements each task and a fresh reviewer checks it before the next one starts, then a whole-branch review at the end. Most thorough; costs a fresh context per task and per review.
- **Native** — I implement every task myself in this session, then one fresh reviewer on the most capable model checks the whole branch. Cheapest and fastest; no independent review until the end.

**For this plan I recommend Subagent-driven**, because the tasks hinge on a shared ABI/PluginHost surface and an early interface mistake would poison every later plugin. Does the plan capture what you want, and which approach should we use?
