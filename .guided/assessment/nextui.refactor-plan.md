# NextUI – Refactor & Improvement Plan

This document provides a **phased, actionable roadmap** for refactoring and improving the NextUI codebase. Recommendations are organized by impact, effort, and risk, with clear priorities for short-term (0-3 months), medium-term (3-9 months), and long-term (9+ months) initiatives.

---

## Guiding Principles

1. **Incremental, not revolutionary**: Avoid "big rewrites"—focus on small, testable changes.
2. **User-facing stability first**: Never break existing functionality for users.
3. **Prioritize maintainability**: Simplify code for future contributors, not just current features.
4. **Hardware constraints matter**: Optimize for TrimUI's limited CPU, RAM, and battery.
5. **Measure before optimizing**: Profile first, then optimize based on data.

---

## Phase 1: Quick Wins (0-3 months, Low Effort, High Impact)

### Goal
Clean up technical debt, improve robustness, and remove cruft without major architectural changes.

---

### 1.1 Archive Unmaintained Platforms

**Problem**: `workspace/_unmaintained/` contains ~60% of C source code for retired devices.

**Action**:
1. Move `workspace/_unmaintained/` to a separate Git branch (`archive/unmaintained-platforms`).
2. Add `README.md` in root noting platforms are deprecated and pointing to archive branch.
3. Remove all `#ifdef` for unmaintained platforms from active code (`api.c`, `minarch.c`, etc.).

**Impact**: Reduces clutter, speeds up searches, simplifies onboarding.

**Effort**: 1-2 days

**Risk**: None (archived code preserved in Git).

**Deliverables**:
- [ ] Create `archive/unmaintained-platforms` branch
- [ ] Move `workspace/_unmaintained/` to archive branch
- [ ] Remove unmaintained `#ifdef` blocks from active code
- [ ] Update root `README.md` with archive link

---

### 1.2 Add Null Checks & Defensive Programming

**Problem**: Missing null checks after `malloc`, `fopen`, `dlopen` lead to crashes.

**Action**:
1. Audit all `malloc`, `calloc`, `realloc` calls and add null checks:
   ```c
   void* ptr = malloc(size);
   if (!ptr) {
       LOG_error("Out of memory (%zu bytes)\n", size);
       return NULL; // Or exit gracefully
   }
   ```
2. Audit all `fopen`, `fread`, `fwrite` and validate returns.
3. Audit all `dlopen` and handle failures gracefully (exit to menu instead of crash).

**Impact**: Significantly improves stability on low-memory conditions.

**Effort**: 3-5 days (systematic audit of ~150 allocation sites).

**Risk**: Low (defensive checks only).

**Deliverables**:
- [ ] Null-check wrapper macros (`XMALLOC`, `XFOPEN`, etc.)
- [ ] Apply wrappers to all allocation sites
- [ ] Test on low-memory scenarios (run multiple emus simultaneously)

---

### 1.3 Remove Dead Code & Commented-Out Blocks

**Problem**: ~50+ commented-out code blocks and unused functions clutter codebase.

**Action**:
1. Use `grep -r '//' workspace/all/ | grep -v 'http://'` to find commented code.
2. Remove all commented-out code (Git history preserves it).
3. Use `clang -Wunused-function` to find unused static functions.
4. Remove unused functions or mark with `__attribute__((unused))` if intentionally kept.

**Impact**: Cleaner, more readable code.

**Effort**: 1-2 days.

**Risk**: None (only removes commented code).

**Deliverables**:
- [ ] Remove all commented-out code
- [ ] Remove or annotate unused functions
- [ ] Document any "dead but intentional" code

---

### 1.4 Consolidate TODOs into GitHub Issues

**Problem**: 100+ `TODO` comments scattered across code.

**Action**:
1. Extract all TODOs via `grep -rn 'TODO' workspace/`.
2. Create GitHub issues for each TODO (label as "tech-debt").
3. Replace inline TODOs with issue references:
   ```c
   // TODO: Revisit this logic
   // → 
   // See issue #123: Revisit scaling logic
   ```
4. Remove stale or irrelevant TODOs.

**Impact**: Centralizes task tracking, easier to prioritize.

**Effort**: 1 day.

**Risk**: None.

**Deliverables**:
- [ ] Create GitHub issues for all TODOs
- [ ] Update code comments to reference issues
- [ ] Close or repurpose stale TODOs

---

### 1.5 Add GitHub Actions for CI

**Problem**: No automated testing or continuous integration.

**Action**:
1. Create `.github/workflows/build.yml`:
   ```yaml
   name: Build
   on: [push, pull_request]
   jobs:
     build:
       runs-on: ubuntu-latest
       steps:
         - uses: actions/checkout@v3
         - run: make PLATFORM=tg5040
   ```
2. Add basic smoke tests (compile-only for now).
3. Add automated release builds on Git tags.

**Impact**: Catch build breaks early, streamline releases.

**Effort**: 1 day.

**Risk**: Low.

**Deliverables**:
- [ ] GitHub Actions workflow for builds
- [ ] Automated releases on tags
- [ ] Badge in README showing build status

---

### 1.6 Document Build System

**Problem**: Build system is opaque to new contributors.

**Action**:
1. Create `docs/BUILD.md` documenting:
   - Prerequisites (Docker, Make)
   - Build commands and targets
   - Toolchain architecture
   - How to add a new platform
2. Add inline comments to complex Makefile sections.

**Impact**: Lowers contributor friction.

**Effort**: 1 day.

**Risk**: None.

**Deliverables**:
- [ ] `docs/BUILD.md` with step-by-step guide
- [ ] Makefile comments explaining non-obvious logic

---

### 1.7 Implement Battery Saver Mode

**Problem**: Battery drains quickly on WiFi, BT, high brightness.

**Action**:
1. Add "Battery Saver" toggle in Settings:
   - Reduce brightness to 50%
   - Disable WiFi and Bluetooth
   - Cap CPU governor at `ondemand`
   - Disable shaders
2. Store preference in `libmsettings`.
3. Apply automatically when battery < 20%.

**Impact**: +20-30% battery life in demanding scenarios.

**Effort**: 2-3 days.

**Risk**: Low.

**Deliverables**:
- [ ] Battery Saver setting in Settings.pak
- [ ] Auto-enable at low battery
- [ ] Test battery life improvement

---

## Phase 2: Medium-Effort Refactors (3-9 months)

### Goal
Address core architectural issues without breaking compatibility.

---

### 2.1 Split Monolithic Files

**Problem**: `minarch.c` (7186 lines), `api.c` (4219 lines), `nextui.c` (3221 lines) are too large.

**Action**:

#### Split `minarch.c` into:
- `minarch_core.c` (core loading, dlopen, function pointers)
- `minarch_video.c` (video callback, scaling, shaders)
- `minarch_audio.c` (audio callback, resampling, batching)
- `minarch_input.c` (input polling, binding, turbo)
- `minarch_menu.c` (in-game menu, options, controls)
- `minarch_state.c` (save states, auto-resume, screenshots)
- `minarch_main.c` (initialization, main loop, cleanup)

#### Split `api.c` into:
- `api_gfx.c` (graphics rendering, blitting, assets)
- `api_input.c` (input polling, PAD_* functions)
- `api_audio.c` (audio initialization, batching)
- `api_power.c` (battery, sleep, CPU scaling, LEDs)
- `api_network.c` (WiFi, Bluetooth)

#### Split `nextui.c` into:
- `nextui_directory.c` (directory scanning, Entry/Directory structures)
- `nextui_menu.c` (menu rendering, navigation)
- `nextui_recents.c` (recent game tracking)
- `nextui_main.c` (initialization, event loop)

**Impact**: Easier to navigate, test, and maintain.

**Effort**: 2-3 weeks (gradual refactor, one file at a time).

**Risk**: Medium (requires careful testing to avoid regressions).

**Deliverables**:
- [ ] Split each file into logical modules
- [ ] Update makefiles
- [ ] Verify no functional changes (git diff on binaries)

---

### 2.2 Refactor God Functions

**Problem**: `nextui.c:main()` is 1120 lines; `minarch.c:environment_callback()` is 360 lines.

**Action**:

#### For `nextui.c:main()`:
```c
int main(int argc, char* argv[]) {
    Context ctx = {0};
    if (initialize(&ctx, argc, argv) != 0) {
        return EXIT_FAILURE;
    }
    eventLoop(&ctx);
    cleanup(&ctx);
    return EXIT_SUCCESS;
}

static int initialize(Context* ctx, int argc, char* argv[]) {
    // SDL, fonts, settings, args parsing
}

static void eventLoop(Context* ctx) {
    while (!ctx->quit) {
        handleInput(ctx);
        updateState(ctx);
        render(ctx);
    }
}

static void cleanup(Context* ctx) {
    // Free resources
}
```

#### For `minarch.c:environment_callback()`:
Extract each case into a handler:
```c
static bool env_set_pixel_format(void* data) { ... }
static bool env_get_overscan(void* data) { ... }
static bool env_set_hw_render(void* data) { ... }

bool environment_callback(unsigned cmd, void* data) {
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
            return env_set_pixel_format(data);
        case RETRO_ENVIRONMENT_GET_OVERSCAN:
            return env_get_overscan(data);
        // ...
    }
}
```

**Impact**: Improves readability and testability.

**Effort**: 1 week.

**Risk**: Low (extract functions without changing logic).

**Deliverables**:
- [ ] Refactor `nextui.c:main()` into phases
- [ ] Extract `environment_callback()` handlers
- [ ] Unit test extracted functions

---

### 2.3 Introduce Context Structures (Reduce Global State)

**Problem**: Heavy use of global variables makes code untestable and non-reentrant.

**Action**:

#### Define context structs:
```c
typedef struct MinarchContext {
    SDL_Surface* screen;
    GFX_Renderer renderer;
    Core core;
    Game game;
    Config config;
    bool quit;
    bool show_menu;
} MinarchContext;

typedef struct NextUIContext {
    Array* recents;
    Array* directories;
    Directory* current_dir;
    int selected_index;
    bool quit;
} NextUIContext;
```

#### Pass contexts through functions:
```c
void minarch_run(MinarchContext* ctx) {
    while (!ctx->quit) {
        core_run(&ctx->core);
        handle_input(ctx);
        render(ctx);
    }
}
```

**Impact**: Enables unit testing, improves encapsulation.

**Effort**: 3-4 weeks (gradual conversion).

**Risk**: Medium (requires careful refactoring).

**Deliverables**:
- [ ] Define context structures
- [ ] Convert global variables to context members
- [ ] Pass contexts through call chains
- [ ] Verify no regressions

---

### 2.4 Migrate to CMake or Meson

**Problem**: Makefiles are brittle and hard to extend.

**Action**:
1. Choose build system (recommend **Meson** for speed and simplicity).
2. Write `meson.build` for each module:
   ```meson
   project('NextUI', 'c')
   
   sdl2 = dependency('SDL2')
   opengl = dependency('gl')
   
   nextui = executable('nextui',
       'workspace/all/nextui/nextui.c',
       'workspace/all/common/api.c',
       dependencies: [sdl2, opengl]
   )
   ```
3. Migrate platform-specific builds.
4. Update documentation.

**Impact**: Faster builds, better IDE integration, easier to maintain.

**Effort**: 1-2 weeks.

**Risk**: Medium (learning curve for contributors).

**Deliverables**:
- [ ] Meson build files for all modules
- [ ] Docker images updated with Meson
- [ ] Update `docs/BUILD.md`

---

### 2.5 Add Save State Checksums

**Problem**: Corrupted save states crash emulation.

**Action**:
1. Add CRC32 checksum to save state header.
2. Validate on load; reject if CRC mismatch.
3. Keep backup of last known-good state.

**Impact**: Prevents crashes from corrupted saves.

**Effort**: 1 week.

**Risk**: Low.

**Deliverables**:
- [ ] Update save state format with CRC32
- [ ] Validate on load
- [ ] Add migration for old saves (skip CRC check)

---

### 2.6 Replace Custom Data Structures with GLib

**Problem**: Reinvented `Array`, `Hash` are bug-prone and lack features.

**Action**:
1. Add `glib-2.0` dependency to toolchains.
2. Replace `Array` with `GArray`:
   ```c
   GArray* entries = g_array_new(FALSE, FALSE, sizeof(Entry*));
   g_array_append_val(entries, entry);
   ```
3. Replace `Hash` with `GHashTable`:
   ```c
   GHashTable* options = g_hash_table_new(g_str_hash, g_str_equal);
   g_hash_table_insert(options, "key", "value");
   ```
4. Test thoroughly.

**Impact**: More robust, fewer bugs, easier to maintain.

**Effort**: 1-2 weeks.

**Risk**: Medium (binary size increases ~500 KB; glib dependency).

**Deliverables**:
- [ ] Add glib to toolchains
- [ ] Migrate data structures
- [ ] Benchmark memory usage

---

## Phase 3: Long-Term Initiatives (9+ months)

### Goal
Strategic improvements for long-term sustainability.

---

### 3.1 Modularize Platform Abstraction

**Problem**: Platform code duplicated across devices.

**Action**:
1. Extract common platform code to `workspace/all/platform_common/`:
   - `platform_cpu.c` (CPU scaling)
   - `platform_power.c` (battery, sleep)
   - `platform_input.c` (input polling)
2. Platforms implement only device-specific overrides.
3. Use weak symbols or function pointers for extensibility.

**Impact**: Easier to add new platforms, less duplication.

**Effort**: 4-6 weeks.

**Risk**: High (requires deep platform knowledge).

**Deliverables**:
- [ ] Extract common platform functions
- [ ] Refactor tg5040 to use common code
- [ ] Test on hardware

---

### 3.2 Implement Automated Testing

**Problem**: No unit tests, integration tests, or CI/CD.

**Action**:
1. Choose testing framework (recommend **Unity** for embedded C).
2. Write unit tests for:
   - `utils.c` (string handling, file I/O)
   - `config.c` (INI parsing)
   - Data structures (if not migrated to glib)
3. Write integration tests for:
   - Core loading
   - ROM extraction
   - Save state loading
4. Add hardware-in-the-loop tests (actual device testing).

**Impact**: Catch regressions early, improve code quality.

**Effort**: 2-3 months.

**Risk**: Medium (learning curve for writing tests).

**Deliverables**:
- [ ] Unity test framework integrated
- [ ] 50+ unit tests
- [ ] Integration tests in GitHub Actions

---

### 3.3 Optimize GPU Rendering with Zero-Copy

**Problem**: Copying framebuffers from CPU to GPU is slow (~5-11 ms on high-res games).

**Action**:
1. Use `RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER` to render directly to GPU texture.
2. Fall back to current method if core doesn't support it.
3. Benchmark performance gains.

**Impact**: -30-50% frame time on high-res games.

**Effort**: 2-3 weeks.

**Risk**: High (requires core cooperation).

**Deliverables**:
- [ ] Implement zero-copy rendering
- [ ] Benchmark on PlayStation cores
- [ ] Submit patches to libretro cores if needed

---

### 3.4 Comprehensive Error Handling Refactor

**Problem**: Inconsistent error handling throughout codebase.

**Action**:
1. Define error types:
   ```c
   typedef enum {
       ERR_NONE = 0,
       ERR_OUT_OF_MEMORY,
       ERR_FILE_NOT_FOUND,
       ERR_INVALID_FORMAT,
       ERR_CORE_LOAD_FAILED,
   } ErrorCode;
   ```
2. Use result types for fallible functions:
   ```c
   typedef struct {
       bool ok;
       ErrorCode error;
       char message[256];
   } Result;
   
   Result core_load(const char* path) {
       void* handle = dlopen(path, RTLD_LAZY);
       if (!handle) {
           return (Result){.ok = false, .error = ERR_CORE_LOAD_FAILED, .message = dlerror()};
       }
       return (Result){.ok = true};
   }
   ```
3. Propagate errors up call chain instead of logging and continuing.

**Impact**: Graceful degradation, better diagnostics.

**Effort**: 2-3 months.

**Risk**: High (pervasive change).

**Deliverables**:
- [ ] Define error types and result structs
- [ ] Refactor all fallible functions
- [ ] Add error recovery paths

---

### 3.5 Structured Logging & Diagnostics

**Problem**: Plain text logs, no rotation, log spam.

**Action**:
1. Use structured logging (JSON):
   ```c
   LOG_json("{\"level\":\"error\",\"msg\":\"Core load failed\",\"path\":\"%s\"}", path);
   ```
2. Implement log rotation (keep last 5 logs, max 1 MB each).
3. Add runtime log level control (debug, info, warn, error).
4. Add telemetry (optional, opt-in): Send anonymous crash reports.

**Impact**: Better diagnostics, easier bug reports.

**Effort**: 2-3 weeks.

**Risk**: Low.

**Deliverables**:
- [ ] JSON logging framework
- [ ] Log rotation
- [ ] Runtime log level control

---

## Phased Roadmap Summary

### Phase 1: Quick Wins (0-3 months)
| Task | Effort | Impact | Priority |
|------|--------|--------|----------|
| Archive unmaintained platforms | 2 days | High | P0 |
| Add null checks | 5 days | High | P0 |
| Remove dead code | 2 days | Medium | P1 |
| GitHub issues for TODOs | 1 day | Medium | P1 |
| GitHub Actions CI | 1 day | High | P0 |
| Document build system | 1 day | Medium | P1 |
| Battery saver mode | 3 days | High | P0 |

**Total Effort**: ~3 weeks

---

### Phase 2: Medium Refactors (3-9 months)
| Task | Effort | Impact | Priority |
|------|--------|--------|----------|
| Split monolithic files | 3 weeks | High | P1 |
| Refactor god functions | 1 week | High | P1 |
| Introduce context structs | 4 weeks | High | P2 |
| Migrate to CMake/Meson | 2 weeks | Medium | P2 |
| Save state checksums | 1 week | High | P1 |
| Migrate to GLib | 2 weeks | Medium | P3 |

**Total Effort**: ~13 weeks

---

### Phase 3: Long-Term (9+ months)
| Task | Effort | Impact | Priority |
|------|--------|--------|----------|
| Modularize platform abstraction | 6 weeks | High | P2 |
| Automated testing | 12 weeks | High | P1 |
| Zero-copy rendering | 3 weeks | Medium | P3 |
| Comprehensive error handling | 12 weeks | High | P2 |
| Structured logging | 3 weeks | Medium | P3 |

**Total Effort**: ~36 weeks

---

## Risk Mitigation

1. **Always branch**: Never refactor directly on `main`.
2. **Test on hardware**: Every change must be tested on actual TrimUI devices.
3. **Incremental PRs**: Small, focused pull requests for review.
4. **Backward compatibility**: Old save states, configs must still work.
5. **Performance benchmarks**: Measure before/after for performance changes.

---

## Success Metrics

| Metric | Baseline | Phase 1 Target | Phase 2 Target | Phase 3 Target |
|--------|----------|----------------|----------------|----------------|
| **Lines of code** | ~140,000 | ~80,000 (archive unmaintained) | ~75,000 (split files) | ~70,000 (modularize) |
| **Largest file** | 7186 lines | Same | <2000 lines | <1500 lines |
| **Largest function** | 1120 lines | Same | <200 lines | <150 lines |
| **TODO count** | 100+ | 0 (migrated to issues) | 0 | 0 |
| **Crash rate** | Unknown | -50% (null checks) | -80% (error handling) | -95% (testing) |
| **Build time** | ~10 min | ~8 min (parallel) | ~5 min (CMake) | ~3 min (caching) |
| **Battery life (PS)** | ~3 hours | ~4 hours (battery saver) | ~4.5 hours (CPU tuning) | ~5 hours (GPU opt) |

---

## Conclusion

This refactor plan is **ambitious but achievable** with a small, dedicated team. The key is to **prioritize high-impact, low-risk changes** in Phase 1 to build momentum, then tackle deeper architectural issues in Phases 2 and 3.

NextUI is a **technically impressive project** that has achieved a lot in a short time. With strategic refactoring, it can become a **model of maintainable, performant embedded software** while continuing to delight users with new features and polish.

**Next Steps**:
1. Review this plan with the core team.
2. Assign owners to Phase 1 tasks.
3. Create GitHub project board to track progress.
4. Begin with P0 tasks (archive unmaintained, null checks, CI).

Good luck! 🚀
