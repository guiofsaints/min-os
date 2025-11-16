# NextUI – Optimization Execution Plan

**Document Version**: 1.0  
**Date**: November 15, 2025  
**Target Timeline**: 9-12 months  
**Phased Approach**: Incremental, testable, low-risk

---

## Overview

This execution plan provides a detailed, step-by-step roadmap for optimizing the NextUI codebase across three phases:

- **Phase A**: Size Reduction and Archival (0-3 months)
- **Phase B**: Code Refactoring and Cleanup (3-9 months)
- **Phase C**: Windows Emulation and Validation (parallel with A & B)

Each phase is broken down into concrete, actionable tasks with clear dependencies, verification criteria, and rollback strategies.

---

## Guiding Principles

1. **Incremental Progress**: Small, testable changes over big-bang rewrites
2. **User Safety**: Never break existing functionality for TrimUI Brick users
3. **Measure Everything**: Benchmark before/after, track metrics, verify with real hardware
4. **Parallel Work Streams**: Size reduction, refactoring, and tooling can progress simultaneously
5. **Clear Checkpoints**: Each task has defined "done" criteria and validation steps

---

# Phase A: Size Reduction and Archival

**Duration**: 0-3 months  
**Goal**: Reduce codebase from ~160,000 LOC to ~80,000 LOC  
**Focus**: Remove unmaintained platforms, dead code, and low-hanging duplication

## A.1: Archive Unmaintained Platforms

### A.1.1: Create Archive Branch

**Objective**: Preserve obsolete platform code in a separate Git branch for historical reference.

**Tasks**:
- [ ] Create new branch `archive/unmaintained-platforms` from `main`
- [ ] Document branch purpose in README
- [ ] Add metadata file listing all archived platforms with last-working commit hash

**Commands**:
```powershell
git checkout main
git checkout -b archive/unmaintained-platforms
echo "# Unmaintained Platforms Archive" > workspace/_unmaintained/README.md
echo "" >> workspace/_unmaintained/README.md
echo "This branch preserves platform code for devices no longer actively supported:" >> workspace/_unmaintained/README.md
echo "- miyoomini, rg35xx, rg35xxplus, rgb30, trimuismart, zero28, my282, my355, gkdpixel, m17, magicmini" >> workspace/_unmaintained/README.md
echo "" >> workspace/_unmaintained/README.md
echo "Last maintained: $(Get-Date -Format 'yyyy-MM-dd')" >> workspace/_unmaintained/README.md
echo "Archived from commit: $(git rev-parse HEAD)" >> workspace/_unmaintained/README.md
git add workspace/_unmaintained/README.md
git commit -m "docs: Archive unmaintained platforms with README"
git push origin archive/unmaintained-platforms
```

**Verification**:
- ✅ Branch `archive/unmaintained-platforms` exists on remote
- ✅ README documents all archived platforms and commit hash
- ✅ Branch is visible in GitHub under "Branches"

**Effort**: 30 minutes  
**Risk**: None (preservation only, no deletion)

---

### A.1.2: Remove Unmaintained Directories from Main

**Objective**: Remove `workspace/_unmaintained/` from active codebase.

**Pre-Removal Checklist**:
- [ ] Verify archive branch exists and is pushed
- [ ] Confirm no active code references unmaintained platforms
- [ ] Document removal in changelog

**Tasks**:
- [ ] Checkout `main` branch
- [ ] Remove `workspace/_unmaintained/` directory
- [ ] Remove `skeleton/_unmaintained/` directory
- [ ] Update root `README.md` to point to archive branch
- [ ] Commit and push

**Commands**:
```powershell
git checkout main
Remove-Item -Recurse -Force workspace/_unmaintained
Remove-Item -Recurse -Force skeleton/_unmaintained
# Update README
Add-Content README.md "`n## Archived Platforms`n"
Add-Content README.md "Code for unmaintained devices (miyoomini, rg35xx, etc.) is preserved in the \``archive/unmaintained-platforms\`` branch.`n"
git add .
git commit -m "chore: Archive unmaintained platforms to separate branch

- Removed workspace/_unmaintained/ (11 platforms, ~80KB C code)
- Removed skeleton/_unmaintained/
- Archive preserved in archive/unmaintained-platforms branch
- Reduces active LOC by ~60%

BREAKING CHANGE: Builds for miyoomini, rg35xx, etc. no longer supported.
Users of these devices should use earlier releases or upstream MinUI."
git push origin main
```

**Verification**:
- ✅ `workspace/_unmaintained/` does not exist in `main`
- ✅ `make PLATFORM=tg5040` builds successfully
- ✅ No broken references in makefiles or source code
- ✅ Archive branch still accessible via Git

**Expected Impact**:
- 📉 Total LOC: 159,901 → ~80,000 (50% reduction)
- 📉 C source files: 156 → ~60 (61% reduction)
- 📉 Build time: ~15 min → ~10 min (33% reduction)
- 📉 Repository size: TBD (Git history preserves old commits)

**Effort**: 2 hours (including testing)  
**Risk**: Low (reversible via Git, archive preserved)

**Rollback**:
```powershell
git revert HEAD  # Undo removal commit
git cherry-pick archive/unmaintained-platforms  # Restore files
```

---

### A.1.3: Clean Up Conditional Compilation

**Objective**: Remove `#ifdef` blocks for unmaintained platforms from active code.

**Affected Files** (estimated):
- `workspace/all/common/api.c`
- `workspace/all/minarch/minarch.c`
- `workspace/all/common/defines.h`
- `workspace/all/common/platform.h`
- Various makefiles

**Tasks**:
- [ ] Search for all platform-specific `#ifdef` blocks
- [ ] Remove blocks for unmaintained platforms (preserve `PLATFORM_TG5040`, `PLATFORM_DESKTOP`)
- [ ] Verify remaining ifdefs are necessary
- [ ] Test build on tg5040 and desktop platforms

**Search Pattern**:
```powershell
# Find all platform ifdefs
rg "#ifdef PLATFORM_" workspace/all/ --type c -n

# Find all elif/else chains with unmaintained platforms
rg "PLATFORM_(MIYOOMINI|RG35XX|RGB30|TRIMUISMART|ZERO28|MY282|MY355|GKDPIXEL|M17|MAGICMINI)" workspace/all/ --type c -n
```

**Example Refactoring**:

**Before**:
```c
#if defined(PLATFORM_MIYOOMINI)
    // Mini-specific code
#elif defined(PLATFORM_RG35XX)
    // RG35XX-specific code
#elif defined(PLATFORM_TG5040)
    // TG5040-specific code (KEEP)
#else
    #error "Unsupported platform"
#endif
```

**After**:
```c
#if defined(PLATFORM_TG5040)
    // TG5040-specific code
#elif defined(PLATFORM_DESKTOP)
    // Desktop fallback
#else
    #error "Unsupported platform"
#endif
```

**Verification**:
- ✅ No references to unmaintained `PLATFORM_*` macros
- ✅ `make PLATFORM=tg5040` builds without warnings
- ✅ `make PLATFORM=desktop` builds without warnings (if applicable)
- ✅ Grep returns zero results: `rg "PLATFORM_MIYOOMINI|PLATFORM_RG35XX" workspace/`

**Effort**: 4-6 hours  
**Risk**: Medium (potential to break builds if conditionals are complex)

**Rollback**: Git revert each file individually

---

### A.1.4: Update Documentation

**Objective**: Reflect archival in user-facing and developer-facing docs.

**Tasks**:
- [ ] Update `README.md` with supported platforms (TrimUI Brick only)
- [ ] Add section on accessing archived platforms
- [ ] Update build documentation (`docs/BUILD.md`)
- [ ] Update online documentation (if hosted separately)

**Verification**:
- ✅ README clearly states supported platforms
- ✅ Link to archive branch is prominent
- ✅ Build docs removed references to unmaintained platforms

**Effort**: 1 hour  
**Risk**: None

---

## A.2: Remove Dead Code

### A.2.1: Remove Commented-Out Code Blocks

**Objective**: Delete all commented-out code (Git history preserves it).

**Search Pattern**:
```powershell
# Find large commented-out blocks
rg "^\s*//" workspace/ --type c --count-matches | Sort-Object -Descending

# Manual review required to distinguish comments from commented code
```

**Tasks**:
- [ ] Identify commented-out code blocks (manual review)
- [ ] Remove blocks that are clearly old code
- [ ] Preserve actual documentation comments
- [ ] Commit in small batches for easy revert

**Verification**:
- ✅ Code builds successfully after each batch
- ✅ No functional regressions on hardware

**Effort**: 4-6 hours (manual review intensive)  
**Risk**: Low (Git preserves history)

---

### A.2.2: Remove Unused Functions

**Objective**: Delete static functions never called.

**Detection**:
```powershell
# Use clang compiler to detect unused functions
# In Docker container or WSL:
make PLATFORM=tg5040 CFLAGS="-Wunused-function"
```

**Tasks**:
- [ ] Compile with `-Wunused-function`
- [ ] Review warnings for false positives (callbacks, weak symbols)
- [ ] Remove confirmed unused functions
- [ ] Annotate intentionally unused functions with `__attribute__((unused))`

**Verification**:
- ✅ Builds without unused function warnings
- ✅ No regressions

**Effort**: 2-3 hours  
**Risk**: Low (static analysis is reliable)

---

### A.2.3: Remove Empty or Stub Files

**Objective**: Delete files with no substantive code.

**Search Pattern**:
```powershell
# Find tiny files
fd -e c -e h . workspace/ -x wc -l | Where-Object { $_ -match "^\s*[0-9]{1,2}\s" } | Sort-Object
```

**Tasks**:
- [ ] Identify files <20 lines
- [ ] Check if they're included elsewhere
- [ ] Remove if not required
- [ ] Update makefiles if necessary

**Verification**:
- ✅ Builds successfully
- ✅ No missing symbols during linking

**Effort**: 1 hour  
**Risk**: Low

---

## A.3: Reduce Low-Hanging Duplication

### A.3.1: Consolidate Platform Utility Functions

**Objective**: Extract duplicated utility functions into `workspace/all/platform_common/`.

**Target Files**:
- `workspace/tg5040/platform/platform.c`
- `workspace/desktop/platform/platform.c`

**Duplicated Functions** (examples):
- `putFile()` / `getFile()` (sysfs read/write)
- `PLAT_setCPUSpeed()`
- `PLAT_getBatteryLevel()`
- `PLAT_enableBacklight()`

**Tasks**:
- [ ] Create `workspace/all/platform_common/` directory
- [ ] Create `platform_utils.c` and `platform_utils.h`
- [ ] Extract common functions (50-100 lines each)
- [ ] Update platform-specific files to call common functions
- [ ] Add to build system

**Example Refactoring**:

**Before** (in `workspace/tg5040/platform/platform.c`):
```c
static void putFile(const char* path, const char* value) {
    FILE* f = fopen(path, "w");
    if (!f) return;
    fputs(value, f);
    fclose(f);
}
```

**After** (in `workspace/all/platform_common/platform_utils.c`):
```c
bool PLAT_putFile(const char* path, const char* value) {
    FILE* f = fopen(path, "w");
    if (!f) {
        LOG_warn("Failed to write %s: %s\n", path, strerror(errno));
        return false;
    }
    fputs(value, f);
    fclose(f);
    return true;
}
```

**Verification**:
- ✅ Reduced duplication in jscpd report (33.71% → <30%)
- ✅ Builds successfully on tg5040 and desktop
- ✅ No functional changes (test on hardware)

**Effort**: 1 week  
**Risk**: Medium (requires careful testing)

---

### A.3.2: Unify `libmsettings` Implementations

**Objective**: Merge platform-specific `libmsettings/` into a single library with device-specific overrides.

**Current State**:
- `workspace/tg5040/libmsettings/` (~600-800 lines)
- `workspace/desktop/libmsettings/` (~600-800 lines)
- ~70%+ code duplication

**Approach**:
1. Create `workspace/all/libmsettings/` with core logic
2. Use function pointers or weak symbols for platform-specific overrides
3. Move platform-specific parts to `workspace/<platform>/libmsettings_platform.c`

**Tasks**:
- [ ] Analyze differences between tg5040 and desktop `libmsettings`
- [ ] Extract common interface and implementation
- [ ] Create override mechanism (function pointers or weak symbols)
- [ ] Migrate tg5040 and desktop to use unified library
- [ ] Test settings persistence on both platforms

**Verification**:
- ✅ Settings load and save correctly on hardware
- ✅ Platform-specific settings (e.g., LED brightness) work
- ✅ Duplication reduced by ~1,000 lines

**Effort**: 2 weeks  
**Risk**: High (settings are critical, test thoroughly)

---

## A.4: Safety and Stability Fixes

### A.4.1: Replace Unsafe String Functions

**Objective**: Eliminate all 459 unsafe function calls.

**Replacement Map**:
| Unsafe | Safe Alternative |
|--------|------------------|
| `strcpy(dst, src)` | `strncpy(dst, src, sizeof(dst) - 1); dst[sizeof(dst)-1] = '\0';` |
| `strcat(dst, src)` | `strncat(dst, src, sizeof(dst) - strlen(dst) - 1);` |
| `sprintf(buf, fmt, ...)` | `snprintf(buf, sizeof(buf), fmt, ...)` |
| `gets(buf)` | `fgets(buf, sizeof(buf), stdin)` |
| `scanf("%s", buf)` | `scanf("%127s", buf)` (with explicit width) |

**Tasks**:
- [ ] Create helper script to find and replace patterns
- [ ] Review each instance manually (automated tools can introduce bugs)
- [ ] Test after each file is modified
- [ ] Use static analyzers (cppcheck, clang-tidy) to verify

**Automation Script** (PowerShell):
```powershell
# Example: Find all strcpy calls
rg "\bstrcpy\s*\(" workspace/ --type c -n > strcpy-locations.txt

# Manual replacement in each file (use multi_replace_string_in_file tool)
```

**Verification**:
- ✅ Zero unsafe function calls: `rg "\b(strcpy|strcat|sprintf|gets)\s*\(" workspace/ --type c` returns nothing
- ✅ cppcheck reports zero buffer overflow warnings
- ✅ No regressions on hardware

**Effort**: 1-2 weeks (459 instances, ~30-50/day with testing)  
**Risk**: Medium (string handling is error-prone, need careful testing)

---

### A.4.2: Add NULL Checks to All Allocations

**Objective**: Add error checking after all `malloc`, `calloc`, `fopen`, `dlopen` calls.

**Pattern**:
```c
// Before
void* ptr = malloc(size);
ptr->field = value;  // Crash if malloc failed

// After
void* ptr = malloc(size);
if (!ptr) {
    LOG_error("Out of memory allocating %zu bytes\n", size);
    return ERROR_NOMEM;  // Or cleanup and exit gracefully
}
ptr->field = value;
```

**Tasks**:
- [ ] Search for all allocation sites: `rg "\b(malloc|calloc|realloc|fopen|dlopen)\s*\(" workspace/ --type c -n`
- [ ] Add null checks systematically (file by file)
- [ ] Define error handling strategy (return error code, goto cleanup, etc.)
- [ ] Test on low-memory scenarios (limit RAM in VM or Docker)

**Helper Macros** (optional):
```c
// In defines.h
#define XMALLOC(size) ({ \
    void* _ptr = malloc(size); \
    if (!_ptr) { \
        LOG_error("malloc failed: %zu bytes\n", size); \
        exit(EXIT_FAILURE); \
    } \
    _ptr; \
})

// Usage
void* ptr = XMALLOC(sizeof(MyStruct));
```

**Verification**:
- ✅ All allocations followed by null check or XMALLOC macro
- ✅ Test with `ulimit -v 100000` (limit virtual memory) to trigger failures
- ✅ No crashes during stress testing

**Effort**: 1 week (150+ allocations)  
**Risk**: Low (defensive programming, always safe)

---

## A.5: Establish Continuous Integration

### A.5.1: Create GitHub Actions Workflow

**Objective**: Automate builds and basic validation on every commit.

**Tasks**:
- [ ] Create `.github/workflows/build.yml`
- [ ] Configure Ubuntu runner with Docker
- [ ] Add build job for `PLATFORM=tg5040`
- [ ] Add optional build job for `PLATFORM=desktop`
- [ ] Upload build artifacts
- [ ] Add basic smoke tests (binaries exist, execute without crashing)

**Workflow Example**:
```yaml
name: Build and Test

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build-tg5040:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: recursive

      - name: Build TG5040
        run: make PLATFORM=tg5040

      - name: Verify Binaries
        run: |
          test -f build/SYSTEM/tg5040/bin/nextui.elf
          test -f build/SYSTEM/tg5040/bin/minarch.elf

      - name: Upload Artifacts
        uses: actions/upload-artifact@v3
        with:
          name: tg5040-build
          path: build/
```

**Verification**:
- ✅ Workflow runs on every push and PR
- ✅ Build status badge in README
- ✅ Failed builds block PRs (optional setting)

**Effort**: 1 day  
**Risk**: Low

---

### A.5.2: Add Static Analysis to CI

**Objective**: Run cppcheck and clang-tidy on every commit.

**Tasks**:
- [ ] Add cppcheck job to GitHub Actions
- [ ] Add clang-tidy job (if compilation database available)
- [ ] Configure to fail on `[error]` severity
- [ ] Generate reports as artifacts

**Workflow Addition**:
```yaml
  static-analysis:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Install cppcheck
        run: sudo apt-get install -y cppcheck

      - name: Run cppcheck
        run: |
          cppcheck --enable=all --error-exitcode=1 \
            --suppress=missingIncludeSystem \
            workspace/all/ workspace/tg5040/ \
            2> cppcheck-report.txt

      - name: Upload cppcheck Report
        if: always()
        uses: actions/upload-artifact@v3
        with:
          name: cppcheck-report
          path: cppcheck-report.txt
```

**Verification**:
- ✅ Static analysis runs on every commit
- ✅ Reports available as artifacts
- ✅ Errors fail the build

**Effort**: 4 hours  
**Risk**: Low

---

## Phase A Summary

**Timeline**: 0-3 months (can be parallelized)

**Key Milestones**:
1. ✅ Week 1-2: Archive unmaintained platforms, clean up ifdefs
2. ✅ Week 3-4: Remove dead code, identify duplication targets
3. ✅ Week 5-8: Replace unsafe functions, add null checks
4. ✅ Week 9-10: Consolidate platform utilities, unify libmsettings
5. ✅ Week 11-12: Establish CI/CD, final testing and documentation

**Expected Outcomes**:
- 📉 LOC: 159,901 → ~80,000 (50% reduction)
- 📉 Unsafe functions: 459 → 0
- 📉 Unprotected allocations: 150 → 0
- 📉 Duplication: 33.71% → ~25%
- ✅ CI/CD running on GitHub Actions
- ✅ All changes tested on TrimUI hardware

**Dependencies for Phase B**: None (Phase B can begin in parallel)

---

# Phase B: Code Refactoring and Cleanup

**Duration**: 3-9 months (overlaps with Phase A)  
**Goal**: Transform monolithic architecture into modular, maintainable code  
**Focus**: Split large files, refactor god functions, introduce context structures

## B.1: Split Monolithic Files

### B.1.1: Split `minarch.c` (7,186 lines)

**Objective**: Break `minarch.c` into logical modules.

**Target Structure**:
```
workspace/all/minarch/
├── minarch_core.c      (core loading, dlopen, symbols)
├── minarch_video.c     (video callbacks, scaling, shaders)
├── minarch_audio.c     (audio callbacks, resampling, batching)
├── minarch_input.c     (input polling, binding, turbo)
├── minarch_menu.c      (in-game menu, options, shortcuts)
├── minarch_state.c     (save states, auto-resume, screenshots)
├── minarch_main.c      (initialization, main loop, cleanup)
└── minarch.h           (shared declarations)
```

**Approach**:
1. **Week 1**: Extract headers and identify module boundaries
2. **Week 2**: Create separate files, move functions (start with smallest module)
3. **Week 3**: Update build system, test each module incrementally
4. **Week 4**: Refactor remaining global state into structures

**Detailed Tasks**:

**B.1.1.1: Extract Core Loading Module**
- [ ] Create `minarch_core.c` and `minarch_core.h`
- [ ] Move core loading/unloading functions: `Core_open()`, `Core_close()`, `Core_loadSymbols()`
- [ ] Move core initialization: `retro_init()`, `retro_set_environment()`, etc.
- [ ] Update `minarch_main.c` to call core module functions
- [ ] Test core loading on multiple emulator cores

**Verification**:
- ✅ Cores load successfully (test GB, GBA, SNES, PS)
- ✅ No change in binary behavior (diff outputs if possible)

**Effort**: 3 days

---

**B.1.1.2: Extract Video Rendering Module**
- [ ] Create `minarch_video.c` and `minarch_video.h`
- [ ] Move video callbacks: `video_refresh_callback_*()`, `get_current_framebuffer_callback()`
- [ ] Move scaling logic: `selectScaler()`, `GFX_*()` calls
- [ ] Move shader pipeline: `Shader_*()` functions
- [ ] Update main loop to call video module

**Verification**:
- ✅ Rendering works correctly on hardware
- ✅ Shaders apply correctly
- ✅ Scaling modes work (aspect, native, fullscreen)

**Effort**: 5 days

---

**B.1.1.3: Extract Audio Module**
- [ ] Create `minarch_audio.c` and `minarch_audio.h`
- [ ] Move audio callbacks: `audio_sample_callback()`, `audio_sample_batch_callback()`
- [ ] Move resampling logic: libsamplerate integration
- [ ] Move audio batching: `audioBuffer[]`, batching state
- [ ] Update main loop

**Verification**:
- ✅ Audio plays without crackling or underruns
- ✅ Resampling quality settings work (0, 1, 2)

**Effort**: 4 days

---

**B.1.1.4: Extract Input Module**
- [ ] Create `minarch_input.c` and `minarch_input.h`
- [ ] Move input callbacks: `input_poll_callback()`, `input_state_callback()`
- [ ] Move input mapping: button bindings, analog stick handling
- [ ] Move turbo logic: `turbo_mode`, `turbo_frames`

**Verification**:
- ✅ Input responsive and accurate (test on multiple games)
- ✅ Turbo mode works

**Effort**: 3 days

---

**B.1.1.5: Extract Menu Module**
- [ ] Create `minarch_menu.c` and `minarch_menu.h`
- [ ] Move in-game menu: `Menu_loop()`, `Menu_options()`, `Menu_controls()`
- [ ] Move menu rendering: text, backgrounds, navigation
- [ ] Move shortcut handling: quick save, quick load, reset

**Verification**:
- ✅ Menu opens and navigates correctly
- ✅ Options apply correctly (scaling, audio, etc.)
- ✅ Shortcuts work

**Effort**: 5 days

---

**B.1.1.6: Extract Save State Module**
- [ ] Create `minarch_state.c` and `minarch_state.h`
- [ ] Move save state logic: `SaveState_*()` functions
- [ ] Move auto-resume: `autoResume_*()` functions
- [ ] Move screenshot: `Screenshot_*()` functions

**Verification**:
- ✅ Save/load states work
- ✅ Auto-resume works
- ✅ Screenshots capture correctly

**Effort**: 3 days

---

**B.1.1.7: Refactor Main Module**
- [ ] Create `minarch_main.c` (if not already separate)
- [ ] Simplify `main()` to call initialization/loop/cleanup from modules
- [ ] Move global state into `MinarchContext` struct (see B.3)

**Example**:
```c
// minarch_main.c
int main(int argc, char** argv) {
    MinarchContext ctx = {0};
    
    if (Minarch_init(&ctx, argc, argv) != 0) {
        return EXIT_FAILURE;
    }
    
    Minarch_run(&ctx);
    Minarch_cleanup(&ctx);
    
    return EXIT_SUCCESS;
}
```

**Verification**:
- ✅ `main()` is <100 lines
- ✅ All modules integrate correctly

**Effort**: 2 days

---

**Total Effort for B.1.1**: 3-4 weeks  
**Risk**: High (requires extensive testing on hardware)  
**Rollback**: Each module is added incrementally; revert each commit if issues arise

---

### B.1.2: Split `api.c` (4,219 lines)

**Objective**: Separate mixed responsibilities into focused modules.

**Target Structure**:
```
workspace/all/common/
├── api_gfx.c       (graphics rendering, blitting, assets)
├── api_input.c     (input polling, PAD_* functions)
├── api_audio.c     (audio initialization, batching)
├── api_power.c     (battery, sleep, CPU scaling, LEDs)
├── api_network.c   (WiFi, Bluetooth)
└── api.h           (public interface)
```

**Approach**: Similar to `minarch.c` split (incremental, module-by-module)

**Tasks**:
- [ ] Extract graphics functions to `api_gfx.c`
- [ ] Extract input functions to `api_input.c`
- [ ] Extract audio functions to `api_audio.c`
- [ ] Extract power/battery functions to `api_power.c`
- [ ] Extract network functions to `api_network.c`
- [ ] Update build system
- [ ] Test each module thoroughly

**Verification**:
- ✅ All API functions work correctly
- ✅ No regressions in UI, input, battery monitoring, WiFi, etc.

**Effort**: 2-3 weeks  
**Risk**: Medium

---

### B.1.3: Split `nextui.c` (3,221 lines)

**Objective**: Separate UI concerns from directory management and game tracking.

**Target Structure**:
```
workspace/all/nextui/
├── nextui_directory.c  (directory scanning, Entry/Directory)
├── nextui_menu.c       (menu rendering, navigation)
├── nextui_recents.c    (recent game tracking)
├── nextui_main.c       (initialization, event loop)
└── nextui.h
```

**Tasks**:
- [ ] Extract directory scanning to `nextui_directory.c`
- [ ] Extract menu rendering to `nextui_menu.c`
- [ ] Extract recent tracking to `nextui_recents.c`
- [ ] Simplify `main()` in `nextui_main.c`

**Verification**:
- ✅ UI navigates correctly
- ✅ Recent games load and save
- ✅ Game launching works

**Effort**: 1-2 weeks  
**Risk**: Medium

---

## B.2: Refactor God Functions

### B.2.1: Refactor `nextui.c:main()` (1,120 lines)

**Objective**: Break `main()` into initialization, event loop, and cleanup.

**Target Pattern**:
```c
int main(int argc, char* argv[]) {
    NextUIContext ctx = {0};
    
    if (NextUI_init(&ctx, argc, argv) != 0) {
        return EXIT_FAILURE;
    }
    
    NextUI_eventLoop(&ctx);
    NextUI_cleanup(&ctx);
    
    return EXIT_SUCCESS;
}
```

**Tasks**:
- [ ] Extract initialization logic to `NextUI_init()`
- [ ] Extract event loop to `NextUI_eventLoop()`
- [ ] Extract cleanup to `NextUI_cleanup()`
- [ ] Break event loop into smaller helpers:
  - `NextUI_handleInput()`
  - `NextUI_updateState()`
  - `NextUI_render()`

**Verification**:
- ✅ `main()` is <50 lines
- ✅ Event loop is <200 lines
- ✅ UI works identically to before

**Effort**: 1 week  
**Risk**: Medium

---

### B.2.2: Refactor `minarch.c:environment_callback()` (360 lines)

**Objective**: Extract each `RETRO_ENVIRONMENT_*` case into a handler function.

**Pattern**:
```c
// Before
bool environment_callback(unsigned cmd, void* data) {
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
            // 30 lines of logic
            return true;
        }
        case RETRO_ENVIRONMENT_GET_OVERSCAN: {
            // 5 lines
            return true;
        }
        // ... 50+ more cases
    }
}

// After
static bool env_set_pixel_format(void* data) {
    // 30 lines
    return true;
}

static bool env_get_overscan(void* data) {
    // 5 lines
    return true;
}

bool environment_callback(unsigned cmd, void* data) {
    switch (cmd) {
        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
            return env_set_pixel_format(data);
        case RETRO_ENVIRONMENT_GET_OVERSCAN:
            return env_get_overscan(data);
        // ... clean switch
    }
    return false;
}
```

**Tasks**:
- [ ] Extract each case into a named function
- [ ] Update `environment_callback()` to dispatch to handlers
- [ ] Test with multiple cores to ensure all cases still work

**Verification**:
- ✅ `environment_callback()` is <100 lines (mostly switch cases)
- ✅ All cores initialize correctly

**Effort**: 3-4 days  
**Risk**: Low

---

### B.2.3: Refactor Other Large Functions

**Targets**:
- `minarch.c:Menu_options()` (409 lines)
- `minarch.c:PLAT_pollInput()` (418 lines)
- Any function >200 lines

**Approach**: Extract logical sections into helper functions

**Effort**: 1-2 weeks total  
**Risk**: Low-Medium

---

## B.3: Introduce Context Structures

### B.3.1: Define Context Structures

**Objective**: Replace global variables with encapsulated context structs.

**Structures**:

**MinarchContext**:
```c
typedef struct {
    SDL_Surface* screen;
    GFX_Renderer renderer;
    
    Core core;
    Game game;
    Config config;
    
    bool quit;
    bool show_menu;
    bool fast_forward;
    int overclock;
    int screen_scaling;
    int resampling_quality;
    
    // Audio state
    uint8_t audioBuffer[AUDIO_BUFFER_SIZE];
    int audioBufferIndex;
    
    // Shader state
    Shader shader;
} MinarchContext;
```

**NextUIContext**:
```c
typedef struct {
    Array* recents;
    Array* stack;
    Array* root;
    Directory* current_dir;
    int selected_index;
    bool quit;
    
    SDL_Surface* screen;
    SDL_Surface* font;
    SDL_Surface* icons;
} NextUIContext;
```

**Tasks**:
- [ ] Define context structures in headers
- [ ] Allocate context at `main()` entry
- [ ] Pass context through function call chains
- [ ] Incrementally convert global variables to context members
- [ ] Test after each conversion

**Verification**:
- ✅ Zero global variables in `minarch.c` and `nextui.c`
- ✅ Code is thread-safe (no shared mutable state)
- ✅ No regressions

**Effort**: 3-4 weeks  
**Risk**: High (pervasive change)

---

## B.4: Reduce Remaining Duplication

### B.4.1: Eliminate Duplicate ROM Scanning Logic

**Objective**: Unify `hasRoms()`, `getDiscs()`, `getCollection()`, `addEntries()` in `nextui.c`.

**Approach**: Create generic `scanDirectory(path, filter_fn, sort_fn)` function

**Tasks**:
- [ ] Analyze differences between scanning functions
- [ ] Extract common pattern into `Directory_scan()`
- [ ] Use function pointers for filtering and sorting
- [ ] Replace all scanning calls with unified function

**Verification**:
- ✅ All game lists populate correctly
- ✅ Filtering works (e.g., hide .txt files)
- ✅ Sorting works (alphabetical, recent)

**Effort**: 3-5 days  
**Risk**: Medium

---

### B.4.2: Unify Menu Rendering Logic

**Objective**: Extract common menu rendering code from `minarch.c` and `nextui.c`.

**Tasks**:
- [ ] Identify shared rendering patterns (text, backgrounds, navigation)
- [ ] Create `menu_common.c` with reusable functions
- [ ] Refactor both files to use common functions

**Verification**:
- ✅ Menus render identically
- ✅ Navigation works

**Effort**: 1 week  
**Risk**: Medium

---

### B.4.3: Consolidate Input Handling

**Objective**: Extract common input event loop pattern.

**Tasks**:
- [ ] Create `inputLoop(render_fn, update_fn)` helper
- [ ] Migrate `nextui.c`, `minarch.c`, `battery.c`, `settings.c` to use helper
- [ ] Reduce code duplication in event handling

**Verification**:
- ✅ Input works correctly in all modules
- ✅ Reduced duplication by ~500 lines

**Effort**: 1 week  
**Risk**: Low

---

## B.5: Technical Debt Cleanup

### B.5.1: Convert TODO Comments to GitHub Issues

**Objective**: Remove all 275 TODO/FIXME/NOTE comments from code.

**Process**:
1. Extract all TODOs: `rg "TODO|FIXME|NOTE|XXX|HACK" workspace/ --type c -n > todos.txt`
2. Categorize by priority (P0: FIXME/XXX, P1: TODO, P2: NOTE)
3. Create GitHub issues for each (use script or manual)
4. Replace inline comments with issue references: `// TODO: Fix this → // See issue #123`
5. Remove stale/irrelevant TODOs

**Tasks**:
- [ ] Extract and categorize TODOs
- [ ] Create GitHub issues (label: `tech-debt`)
- [ ] Update code comments to reference issues
- [ ] Remove or complete trivial TODOs

**Verification**:
- ✅ All TODOs tracked in GitHub
- ✅ Code comments reference specific issues
- ✅ No orphaned TODOs

**Effort**: 2 days  
**Risk**: None

---

### B.5.2: Improve Code Documentation

**Objective**: Increase comment ratio from 3.3% to 15%.

**Focus Areas**:
1. **Public APIs**: Document all exported functions
2. **Complex algorithms**: Explain scaling, resampling, shader math
3. **Platform-specific workarounds**: Document why certain code exists
4. **Module headers**: Add overview comments to each file

**Style Guide**:
```c
/**
 * Initialize the MinArch emulation engine.
 * 
 * @param ctx Context structure to initialize
 * @param argc Command-line argument count
 * @param argv Command-line arguments (ROM path, core path, etc.)
 * @return 0 on success, non-zero error code on failure
 * 
 * This function:
 * - Loads the libretro core
 * - Initializes SDL subsystems (video, audio, input)
 * - Sets up rendering pipeline (shaders, scaling)
 * - Loads game ROM and applies patches
 */
int Minarch_init(MinarchContext* ctx, int argc, char** argv);
```

**Tasks**:
- [ ] Document all public functions in `minarch.h`, `api.h`, `nextui.h`
- [ ] Add file-level overview comments
- [ ] Document complex sections (>50 lines without comments)
- [ ] Run `scc` to track comment ratio progress

**Verification**:
- ✅ Comment ratio >10% (Phase B), >15% (Phase C)
- ✅ All public APIs documented

**Effort**: 2-3 weeks (ongoing)  
**Risk**: None

---

## B.6: Migrate to Modern Build System (Optional)

### B.6.1: Evaluate CMake vs. Meson

**Objective**: Choose and implement a modern build system.

**Comparison**:
| Feature | CMake | Meson |
|---------|-------|-------|
| Learning curve | Medium | Low |
| Speed | Medium | Fast |
| IDE integration | Excellent | Good |
| Cross-compilation | Good | Excellent |
| Community | Large | Growing |

**Recommendation**: **Meson** (faster builds, simpler syntax, excellent for cross-compilation)

**Tasks**:
- [ ] Create proof-of-concept `meson.build` for one module (e.g., `minarch`)
- [ ] Benchmark build time vs. Make
- [ ] Get team feedback
- [ ] Decision: proceed or stick with Make

**Effort**: 1 week (proof-of-concept)  
**Risk**: Medium (learning curve)

---

### B.6.2: Implement Meson Build (if approved)

**Tasks**:
- [ ] Write top-level `meson.build`
- [ ] Write per-module `meson.build` files
- [ ] Configure cross-compilation for tg5040
- [ ] Integrate with Docker toolchain
- [ ] Update documentation

**Verification**:
- ✅ Build time <10 minutes (vs. 15 with Make)
- ✅ Incremental builds work correctly
- ✅ All platforms build successfully

**Effort**: 2-3 weeks  
**Risk**: Medium

---

## Phase B Summary

**Timeline**: 3-9 months (overlaps with Phase A and C)

**Key Milestones**:
1. ✅ Month 1-2: Split `minarch.c` into modules
2. ✅ Month 2-3: Split `api.c` and `nextui.c`
3. ✅ Month 3-4: Refactor god functions, extract helpers
4. ✅ Month 4-6: Introduce context structures, eliminate global state
5. ✅ Month 6-8: Reduce remaining duplication, unify common code
6. ✅ Month 8-9: Technical debt cleanup, improve documentation

**Expected Outcomes**:
- ✅ No file exceeds 1,500 lines
- ✅ No function exceeds 150 lines
- ✅ Duplication reduced to <10%
- ✅ Zero global variables
- ✅ Comment ratio >15%
- ✅ All TODO/FIXME tracked in GitHub

**Dependencies for Phase C**: None (parallel work streams)

---

# Phase C: Windows Emulation and Validation

**Duration**: Parallel with A & B (0-9 months)  
**Goal**: Enable Windows contributors to build and validate changes  
**Focus**: Docker workflows, desktop builds, smoke tests

## C.1: Docker Desktop Workflow

### C.1.1: Document Windows + Docker Desktop Build

**Objective**: Provide clear, step-by-step instructions for Windows users.

**Tasks**:
- [ ] Create `docs/BUILD-WINDOWS.md`
- [ ] Document prerequisites (Docker Desktop, PowerShell 5.1+)
- [ ] Provide example build commands
- [ ] Document troubleshooting common issues
- [ ] Add screenshots or video walkthrough (optional)

**Content Outline**:
1. Prerequisites
   - Docker Desktop installation
   - Verify Docker: `docker --version`
2. Clone repository
3. Build TrimUI firmware: `make all PLATFORM=tg5040`
4. Build libretro cores (optional): `make all PLATFORM=tg5040 COMPILE_CORES=true`
5. Verify output: `build/SYSTEM/tg5040/bin/*.elf`
6. Troubleshooting:
   - Docker not found
   - Permission errors
   - Slow builds (WSL 2 backend recommended)

**Verification**:
- ✅ Fresh Windows 11 machine can follow instructions and build successfully
- ✅ Build completes in <15 minutes (Docker)

**Effort**: 1 day  
**Risk**: None

---

### C.1.2: Optimize Docker Build Performance

**Objective**: Reduce Docker build time on Windows from 15 minutes to <10 minutes.

**Optimizations**:
1. **Use WSL 2 backend** (faster than Hyper-V)
2. **Enable Docker layer caching** (cache toolchain images)
3. **Parallel builds**: Add `-j$(nproc)` to makefiles
4. **ccache integration**: Cache compiled objects

**Tasks**:
- [ ] Document WSL 2 setup in build guide
- [ ] Add `make -j` to makefiles (if not already present)
- [ ] Test ccache integration (optional, may require toolchain changes)
- [ ] Benchmark before/after

**Verification**:
- ✅ Build time reduced by 30%+
- ✅ Incremental builds work correctly

**Effort**: 2-3 days  
**Risk**: Low

---

## C.2: WSL 2 Workflow

### C.2.1: Document WSL 2 + Native Docker Build

**Objective**: Provide alternative build workflow using WSL 2.

**Tasks**:
- [ ] Add WSL 2 section to `docs/BUILD-WINDOWS.md`
- [ ] Document WSL 2 installation: `wsl --install`
- [ ] Document Docker installation in WSL: `sudo apt install docker.io`
- [ ] Provide build commands (same as Linux)

**Advantages**:
- ✅ Faster builds (native ext4 filesystem)
- ✅ More Linux-native experience
- ✅ No Docker Desktop license required (free for personal use)

**Verification**:
- ✅ Build completes in <10 minutes (WSL 2 + Docker)
- ✅ Instructions clear for non-technical users

**Effort**: 1 day  
**Risk**: None

---

## C.3: Desktop Platform Build

### C.3.1: Document Desktop Build on Windows

**Objective**: Enable building `PLATFORM=desktop` for testing on Windows.

**Current Status**: Desktop build targets macOS/Linux only

**Tasks**:
- [ ] Investigate desktop build requirements (SDL2, OpenGL)
- [ ] Document native Windows build (if possible with MSVC or MinGW)
- [ ] Document WSL 2 desktop build (fallback)
- [ ] Create smoke test script to verify desktop build works

**Challenges**:
- Windows graphics subsystem differs (DirectX vs. OpenGL)
- SDL2 on Windows may have different behavior

**Approach**:
1. **Preferred**: Build `PLATFORM=desktop` in WSL 2 with X11 forwarding (run GUI apps from WSL)
2. **Alternative**: Port desktop platform to use SDL2 on Windows natively (significant effort)

**Verification**:
- ✅ Desktop build runs on Windows (via WSL + X11 or native)
- ✅ Can test basic emulation without hardware

**Effort**: 1-2 weeks (investigation + documentation)  
**Risk**: High (may not be feasible without significant porting)

---

### C.3.2: Create Smoke Tests for Desktop Build

**Objective**: Automate validation of core functionality without hardware.

**Tests**:
1. **Core loading**: Verify libretro core loads without errors
2. **ROM loading**: Verify ROM extraction and loading
3. **Rendering**: Verify SDL window opens and renders (even if just a test pattern)
4. **Input**: Verify keyboard input is detected
5. **Audio**: Verify audio subsystem initializes (mute output)

**Implementation**:
```powershell
# smoke-test.ps1
.\build\SYSTEM\desktop\bin\nextui.elf --test-mode
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Smoke test passed"
} else {
    Write-Host "❌ Smoke test failed"
    exit 1
}
```

**Tasks**:
- [ ] Add `--test-mode` flag to nextui and minarch
- [ ] Implement basic smoke tests (load core, load ROM, render 1 frame, exit)
- [ ] Add to CI/CD pipeline

**Verification**:
- ✅ Smoke tests pass on Windows desktop build
- ✅ CI/CD runs smoke tests on every commit

**Effort**: 1 week  
**Risk**: Medium

---

## C.4: Continuous Integration on Windows

### C.4.1: Add Windows Runner to GitHub Actions

**Objective**: Run builds and tests on Windows in CI/CD.

**Tasks**:
- [ ] Add `windows-latest` runner to GitHub Actions
- [ ] Configure Docker Desktop in runner (or use WSL 2)
- [ ] Run builds on Windows in parallel with Linux builds
- [ ] Compare artifacts (binaries should be identical)

**Workflow Example**:
```yaml
  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: recursive

      - name: Setup Docker Desktop
        run: |
          choco install docker-desktop -y
          Start-Service docker

      - name: Build TG5040 on Windows
        run: make PLATFORM=tg5040

      - name: Upload Windows Build
        uses: actions/upload-artifact@v3
        with:
          name: tg5040-windows-build
          path: build/
```

**Verification**:
- ✅ Windows build passes in CI
- ✅ Artifacts match Linux build (binary-identical or diff minimal)

**Effort**: 1 day  
**Risk**: Medium (Windows runners slower and may have compatibility issues)

---

## C.5: Validation Workflow

### C.5.1: Define Validation Checklist

**Objective**: Standardize testing after each refactoring phase.

**Checklist**:
- [ ] **Build**: All platforms build without errors or warnings
- [ ] **Static Analysis**: cppcheck and clang-tidy report zero errors
- [ ] **Smoke Tests**: Desktop build passes automated tests
- [ ] **Hardware Testing**: TrimUI Brick manual testing
  - [ ] Launch nextui, navigate menus
  - [ ] Launch each emulator core (GB, GBA, SNES, NES, PS)
  - [ ] Test save states (save, load, auto-resume)
  - [ ] Test in-game menu (options, controls, reset)
  - [ ] Test WiFi (connect, disconnect, NTP sync)
  - [ ] Test Bluetooth audio (pair, connect, disconnect)
  - [ ] Test battery monitoring and LED control
  - [ ] Play for 30+ minutes to check stability
- [ ] **Performance**: Measure frame time, input latency (should not regress)
- [ ] **Binary Size**: Check size of executables (should not increase significantly)

**Tasks**:
- [ ] Create `TESTING.md` with detailed checklist
- [ ] Assign testing responsibilities (manual hardware testing)
- [ ] Automate what's possible (build, static analysis, smoke tests)

**Verification**:
- ✅ All checklist items pass after each major change

**Effort**: 1 day (documentation)  
**Risk**: None

---

### C.5.2: Benchmark Performance Metrics

**Objective**: Ensure refactoring does not degrade performance.

**Metrics to Track**:
1. **Build Time**: Track on each platform (goal: reduce by 30%)
2. **Binary Size**: `nextui.elf`, `minarch.elf` (goal: reduce by 20%)
3. **Input Latency**: Measure on hardware (goal: maintain ≤20ms)
4. **Frame Time**: Measure in demanding games (goal: maintain 60fps)
5. **Memory Usage**: RSS on device (goal: maintain <200MB)
6. **Battery Life**: Measured in real-world usage (goal: no regression)

**Tools**:
- Build time: `Measure-Command { make PLATFORM=tg5040 }` (PowerShell)
- Binary size: `ls -lh build/SYSTEM/tg5040/bin/*.elf`
- Performance: Manual testing with logging (see `performance-and-resilience.md`)

**Tasks**:
- [ ] Establish baseline metrics (before Phase A)
- [ ] Measure after each phase
- [ ] Document in `BENCHMARKS.md`

**Verification**:
- ✅ No metric regresses >5%
- ✅ Ideally, improvements across all metrics

**Effort**: Ongoing (1-2 hours per phase)  
**Risk**: None

---

## Phase C Summary

**Timeline**: Parallel with A & B (0-9 months)

**Key Milestones**:
1. ✅ Month 1: Document Docker Desktop and WSL 2 workflows
2. ✅ Month 2: Optimize Docker build performance
3. ✅ Month 3: Investigate and document desktop build
4. ✅ Month 4: Create smoke tests for desktop build
5. ✅ Month 5: Add Windows runner to GitHub Actions
6. ✅ Month 6+: Ongoing validation and benchmarking

**Expected Outcomes**:
- ✅ Clear Windows build documentation (Docker + WSL 2)
- ✅ Desktop build works on Windows (via WSL or native)
- ✅ Smoke tests validate core functionality
- ✅ CI/CD includes Windows builds
- ✅ Performance benchmarks tracked across all phases

**Dependencies**: None (can start immediately)

---

# Cross-Phase Dependencies and Ordering

## Recommended Sequence

### Weeks 1-4 (Phase A Start)
1. A.1: Archive unmaintained platforms
2. A.4.1: Replace unsafe functions (start)
3. C.1: Document Windows build workflows

### Weeks 5-8 (Phase A + C)
4. A.4.1: Replace unsafe functions (continue)
5. A.4.2: Add NULL checks
6. A.5: Establish CI/CD
7. C.2: Document WSL 2 workflow

### Weeks 9-12 (Phase A Complete, Phase B Start)
8. A.2: Remove dead code
9. A.3: Reduce low-hanging duplication
10. B.1.1: Start splitting `minarch.c`
11. C.3: Investigate desktop build

### Months 4-6 (Phase B Main Work)
12. B.1.1-1.3: Complete file splits
13. B.2: Refactor god functions
14. C.4: Add Windows CI runner

### Months 7-9 (Phase B Complete, Phase C Complete)
15. B.3: Introduce context structures
16. B.4: Reduce remaining duplication
17. B.5: Technical debt cleanup
18. C.5: Ongoing validation and benchmarking

## Parallel Work Streams

Multiple developers can work on:
- **Stream 1**: Size reduction (A.1-A.3)
- **Stream 2**: Safety fixes (A.4)
- **Stream 3**: CI/CD and tooling (A.5, C.1-C.4)
- **Stream 4**: Refactoring (B.1-B.3) - starts after A.1 complete

---

# Rollback and Recovery Strategies

## Per-Task Rollback

**Every task should**:
1. Be committed to a feature branch (never directly to `main`)
2. Have a corresponding PR for review
3. Be tested on hardware before merge
4. Be small enough to revert cleanly

**Rollback Process**:
```powershell
# Identify problematic commit
git log --oneline

# Revert specific commit
git revert <commit-hash>

# Or reset to previous state (destructive)
git reset --hard <previous-commit>
```

## Phase-Level Rollback

If an entire phase needs to be abandoned:
1. Tag `main` before starting phase: `git tag phase-A-start`
2. Work on phase in a long-lived branch: `feature/phase-A`
3. If phase fails, abandon branch and reset: `git checkout main`

## Data Migration Issues

**Save states and configs** must remain compatible:
1. Always version save state format
2. Provide migration code for old versions
3. Test migration with real user data

**Rollback**: Keep backup of user data before upgrading

---

# Success Metrics and Reporting

## Phase A Success Criteria

- [ ] LOC reduced to <80,000
- [ ] Unsafe functions eliminated (459 → 0)
- [ ] NULL checks added (150+ allocations)
- [ ] Duplication reduced to <25%
- [ ] CI/CD running on GitHub Actions
- [ ] Zero regressions on TrimUI hardware

## Phase B Success Criteria

- [ ] No file >1,500 lines
- [ ] No function >150 lines
- [ ] Duplication <10%
- [ ] Zero global variables
- [ ] Comment ratio >15%
- [ ] All TODO/FIXME tracked

## Phase C Success Criteria

- [ ] Windows build workflow documented
- [ ] Desktop build works on Windows
- [ ] Smoke tests pass in CI
- [ ] Performance benchmarks stable or improved

## Reporting Cadence

- **Weekly**: Progress updates on current tasks
- **Monthly**: Phase milestone reports
- **Per-Phase**: Comprehensive retrospective

---

# Risk Management

## High-Risk Tasks

1. **B.1.1: Split `minarch.c`** - Complex, requires extensive testing
2. **B.3.1: Introduce context structures** - Pervasive change, high bug risk
3. **C.3.1: Desktop build on Windows** - May not be feasible

**Mitigation**:
- Allocate extra time for testing
- Pair programming or code review
- Prototype before full implementation

## Medium-Risk Tasks

1. **A.4.1: Replace unsafe functions** - 459 instances, manual work
2. **B.1.2-1.3: Split `api.c` and `nextui.c`** - Moderate complexity
3. **B.4: Reduce remaining duplication** - Requires careful refactoring

**Mitigation**:
- Incremental changes
- Automated testing where possible

## Low-Risk Tasks

1. **A.1: Archive unmaintained platforms** - Simple, reversible
2. **A.2: Remove dead code** - Safe, no functional impact
3. **C.1: Document Windows workflows** - Documentation only

---

# Conclusion

This execution plan provides a concrete, actionable roadmap for transforming the NextUI codebase from a monolithic, duplicated structure into a clean, modular, maintainable architecture. By following the phased approach and adhering to the defined verification criteria, the team can achieve the three core optimization goals while minimizing risk and maintaining user-facing stability.

**Key Takeaways**:
- **Incremental progress** is safer and more sustainable than big-bang rewrites
- **Parallelization** across phases enables faster completion (9-12 months vs. 18+ sequential)
- **Clear verification** at each step ensures quality and prevents regressions
- **Windows workflows** can be established without major infrastructure changes

**Next Steps**:
1. Review and approve this plan with core team
2. Assign owners to each phase
3. Create GitHub project board with all tasks
4. Begin Phase A.1: Archive unmaintained platforms

---

**Document Owner**: NextUI Core Team  
**Last Updated**: November 15, 2025  
**Next Review**: Monthly during execution
