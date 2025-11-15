# NextUI – Optimization Goals and Current State

**Document Version**: 1.0  
**Date**: November 15, 2025  
**Target Platforms**: TrimUI Brick, TrimUI Smart Pro (tg5040)  
**Future Vision**: Rebranding to MinOS with platform consolidation

---

## Executive Summary

NextUI is a high-performance custom firmware for TrimUI handheld gaming devices that has achieved impressive technical capabilities through rapid, organic development. However, this growth has resulted in significant technical debt, code duplication, and maintenance challenges. This document defines three core optimization goals and establishes the baseline state from which improvements will be measured.

**Current State**: ~160,000 LOC, 33.71% code duplication, 459 unsafe function calls, monolithic architecture  
**Target State**: ~70,000 LOC, <10% duplication, zero unsafe patterns, modular architecture  
**Timeline**: 9-12 months phased optimization

---

## Three Core Optimization Goals

### Goal 1: Reduce Project Size Without Losing Core Features and Purpose

**Objective**: Shrink the codebase from ~160,000 lines to ~70,000 lines (56% reduction) by eliminating unmaintained platforms, dead code, and duplicated logic while preserving all functionality for TrimUI Brick and Smart Pro users.

**Rationale**:
- 60% of C source code resides in `workspace/_unmaintained/` for obsolete devices
- 33.71% of the codebase is duplicated code (7,440 lines directly removable)
- Large codebase creates cognitive burden for contributors and slows development velocity
- Smaller footprint improves build times, reduces binary size, and simplifies maintenance

**Non-Goals**:
- ❌ Do not remove any features currently used by TrimUI Brick/Smart Pro users
- ❌ Do not compromise emulation performance or compatibility
- ❌ Do not break existing user configurations or save states

**Success Metrics**:
- ✅ Total LOC reduced to <80,000 (Phase 1) and <70,000 (Phase 2+)
- ✅ All TrimUI Brick/Smart Pro features remain functional
- ✅ Build time reduced by 30%+
- ✅ Binary size reduced by 20%+

---

### Goal 2: Reduce Complexity, Duplication, and Dead Code

**Objective**: Transform the codebase from a monolithic, duplicated structure into a modular, maintainable architecture with clear separation of concerns, minimal code duplication (<10%), and no dead code.

**Rationale**:
- **Monolithic files**: `minarch.c` (7,186 lines), `api.c` (4,219 lines), `nextui.c` (3,221 lines) are unmaintainable
- **God functions**: `nextui.c:main()` (1,120 lines) is impossible to test or reason about
- **Code duplication**: 33.71% duplication indicates missing abstractions and copy-paste development
- **Dead code**: ~50+ commented-out blocks, unused functions, and conditional compilation for retired platforms
- **Technical debt**: 275+ TODO/FIXME comments, 100+ HACK markers, inconsistent error handling

**Non-Goals**:
- ❌ Do not perform "big bang" rewrites—favor incremental, testable refactoring
- ❌ Do not introduce new dependencies without careful evaluation
- ❌ Do not sacrifice performance for modularity (measure before/after)

**Success Metrics**:
- ✅ No source file exceeds 1,500 lines
- ✅ No function exceeds 150 lines
- ✅ Code duplication reduced to <10%
- ✅ All TODO/FIXME converted to tracked GitHub issues or resolved
- ✅ Zero commented-out code blocks
- ✅ Cyclomatic complexity average <50 per file

---

### Goal 3: Enable Running or Emulating NextUI on Windows for Validation

**Objective**: Establish Docker-based and/or WSL-based development workflows that allow Windows contributors to build, test, and validate NextUI changes without requiring physical TrimUI hardware or switching to Linux/macOS.

**Rationale**:
- Windows is the dominant development platform for many contributors
- Current build system requires Docker (supported) but lacks clear Windows documentation
- Refactoring work requires frequent validation to avoid regressions
- Emulation or desktop builds enable faster iteration cycles
- Continuous integration on Windows reduces platform-specific bugs

**Non-Goals**:
- ❌ Do not attempt to run ARM binaries natively on Windows (cross-compilation only)
- ❌ Do not replace the official Linux/Docker toolchain
- ❌ Do not compromise build reproducibility

**Success Metrics**:
- ✅ Docker Desktop + PowerShell build workflow documented and tested
- ✅ WSL 2 build workflow documented and tested
- ✅ Desktop build (`PLATFORM=desktop`) runs on Windows (via WSL or native toolchain)
- ✅ Smoke tests validate core functionality without hardware
- ✅ CI/CD pipeline includes Windows builds (GitHub Actions)

---

## Current State Assessment

### Codebase Metrics (Baseline)

| Metric | Current Value | Target (Phase 1) | Target (Final) |
|--------|---------------|------------------|----------------|
| **Total Lines of Code** | 159,901 | 80,000 | 70,000 |
| **Active Code (C)** | 41,185 | 30,000 | 28,000 |
| **Unmaintained Platforms** | 60% of C code | 0% (archived) | 0% (archived) |
| **Code Duplication** | 33.71% | 20% | <10% |
| **Largest File** | 7,186 lines (`minarch.c`) | <2,000 lines | <1,500 lines |
| **Largest Function** | 1,120 lines (`nextui.c:main()`) | <300 lines | <150 lines |
| **Cyclomatic Complexity** | 9,097 total (avg 92/file) | <60/file | <50/file |
| **Comment Ratio** | 3.3% | 10% | 15% |
| **TODO/FIXME Count** | 275 | 100 | 0 |
| **Unsafe Functions** | 459 calls | 0 | 0 |
| **malloc w/o NULL check** | ~150 | 0 | 0 |
| **Build Time (full)** | ~15 min (Docker) | ~10 min | ~5 min |

### Key Findings from Assessment Documents

#### From `nextui.overview.md`:

**Strengths**:
- ✅ High-performance emulation (20ms input latency, 60fps)
- ✅ Rich feature set (WiFi, Bluetooth, shaders, game tracking, LED control)
- ✅ Active community and responsive maintainers
- ✅ Docker-based reproducible builds
- ✅ Extensive libretro core support

**Weaknesses**:
- 🔴 Large, monolithic C files with deeply nested logic
- 🔴 Significant technical debt (100+ TODO/FIXME/HACK comments)
- 🔴 Code duplication in platform abstraction layers
- 🔴 Unmaintained platform code bloats repository
- 🔴 Hard-to-follow control flow (function pointers, callbacks, macros)
- 🔴 Inconsistent error handling
- 🔴 No automated testing or CI/CD
- 🔴 Sparse documentation (3.3% comments vs. recommended 15-25%)

#### From `nextui.code-quality-report.md`:

**Critical Issues**:

1. **Monolithic Source Files**:
   - `minarch.c`: 7,186 lines (emulation loop, menu, input, audio, video, shaders)
   - `api.c`: 4,219 lines (graphics, input, audio, power, WiFi, Bluetooth, battery, LEDs)
   - `nextui.c`: 3,221 lines (file browsing, menu, game selection, recent tracking)
   - `scaler.c`: 3,073 lines (multiple scaling algorithms)

2. **God Functions**:
   - `nextui.c:main()`: 1,120 lines (initialization, event loop, rendering, cleanup)
   - `minarch.c:environment_callback()`: 360 lines (50+ libretro environment requests)
   - `minarch.c:Menu_options()`: 409 lines (render, input, settings)

3. **Code Duplication** (33.71%):
   - Platform-specific code duplicated across 10+ unmaintained devices
   - `libmsettings/` duplicated per platform (~600-800 lines each)
   - `platform.c` duplicated with 90%+ identical code
   - ROM scanning logic duplicated 4+ times in `nextui.c`

4. **Unsafe Patterns** (459 instances):
   - `strcpy`, `strcat`, `sprintf` without bounds checking
   - `gets()` (exploitable buffer overflow)
   - `malloc`/`fopen` without NULL checks (~150 instances)

5. **Global State**:
   - 50+ global variables in `minarch.c`, `nextui.c`, `api.c`
   - No encapsulation or context structures
   - Thread-unsafe design

#### From `nextui.deps-and-stack.md`:

**Build System Issues**:
- ❌ Brittle makefiles with fragile dependencies
- ❌ No incremental build optimization (frequent full rebuilds)
- ❌ Complex variable passing through nested makefiles
- ❌ No parallel builds by default
- ❌ No build caching

**Dependency Concerns**:
- ⚠️ Custom data structures (Array, Hash) instead of mature libraries (glib)
- ⚠️ Multiple compression libraries bundled (zlib, libbz2, liblzma, libzstd) for libzip
- ⚠️ Shell-script-based WiFi/Bluetooth management (brittle CLI parsing)

#### From `nextui.performance-and-resilience.md`:

**Performance Bottlenecks**:
- 🐌 High-res PlayStation games: `SDL_UnlockTexture()` blocks 5-11ms (frame budget is 16.67ms)
- 🐌 Audio resampling overhead: 0.5-2.5ms per batch (depends on quality setting)
- 🐌 Multi-pass shaders: 4-5ms overhead on Mali-G31 GPU
- 🐌 ZIP extraction: 15-30 seconds for large ROMs (600MB PlayStation games)

**Resilience Issues**:
- 🔴 Missing null checks after `malloc`, `fopen`, `dlopen` → crashes on failure
- 🔴 Corrupted save states crash emulation (no checksums or validation)
- 🔴 Configuration parsing brittle (crashes on malformed INI files)
- 🔴 No structured logging (plain text, no rotation, log spam)

#### From `nextui.windows-tooling-report.md`:

**Analysis Findings** (ANALYSIS-REPORT.md):
- 📊 159,901 total LOC
- 📊 33.71% duplication (7,440 duplicated lines)
- 📊 459 unsafe function calls
- 📊 358 memory allocations (150+ without null checks)
- 📊 275 TODO/FIXME/NOTE comments
- 📊 1,576 magic numbers (hex constants)

---

## Top 10 Critical Issues and Risks

### Priority 0 (Critical – Security & Stability)

#### 1. Unsafe Function Calls (459 instances)

**Issue**: Widespread use of `strcpy`, `strcat`, `sprintf`, `gets` without bounds checking  
**Risk**: Buffer overflow vulnerabilities, potential code execution exploits  
**Impact**: 🔴 **Critical** – Security vulnerability, crash risk  
**Effort**: Medium (systematic replacement, ~1-2 weeks)  
**Example**:
```c
// workspace/desktop/platform/platform.c:331
strcpy(combined, replacement_version);  // ❌ No size check
strcat(combined, define);              // ❌ No size check
```
**Fix**: Replace with `strncpy`, `strncat`, `snprintf`

#### 2. Missing NULL Checks (150+ instances)

**Issue**: Memory allocations and file operations without error checking  
**Risk**: Null pointer dereference → crash  
**Impact**: 🔴 **Critical** – Stability, user data loss  
**Effort**: Medium (systematic audit, ~1 week)  
**Example**:
```c
Array* self = malloc(sizeof(Array));  // ❌ No NULL check
self->count = 0;  // Crash if malloc failed
```
**Fix**: Add null checks after all allocations

#### 3. Code Duplication (33.71%)

**Issue**: 7,440 lines of duplicated code, primarily in platform abstraction  
**Risk**: Bugs fixed in one place persist elsewhere, maintenance burden  
**Impact**: 🔴 **High** – Maintainability, consistency  
**Effort**: High (requires architectural refactoring, ~2-3 months)  
**Example**: `platform.c` duplicated across 10+ devices with 90%+ identical code  
**Fix**: Extract common platform code to shared library

#### 4. Unmaintained Platform Code (60% of C source)

**Issue**: `workspace/_unmaintained/` contains code for 10+ obsolete devices  
**Risk**: Confusion for new contributors, bloated repository, wasted maintenance effort  
**Impact**: 🟡 **Medium** – Code navigation, repository size  
**Effort**: Low (archive to separate branch, ~1-2 days)  
**Fix**: Move to `archive/unmaintained-platforms` Git branch

#### 5. Monolithic Files (7,186 lines in `minarch.c`)

**Issue**: Single files contain multiple responsibilities (emulation, rendering, input, audio, menu)  
**Risk**: Merge conflicts, hard to test, difficult to onboard new contributors  
**Impact**: 🟡 **High** – Maintainability, development velocity  
**Effort**: High (requires careful refactoring, ~3-4 weeks)  
**Fix**: Split into logical modules (core, video, audio, input, menu, state)

### Priority 1 (High – Quality & Maintainability)

#### 6. God Functions (1,120 lines in `nextui.c:main()`)

**Issue**: Single functions with excessive responsibilities and line counts  
**Risk**: Impossible to unit test, high cyclomatic complexity, error-prone  
**Impact**: 🟡 **High** – Testability, code quality  
**Effort**: Medium (extract helper functions, ~1-2 weeks)  
**Fix**: Break into initialization, event loop, rendering, and cleanup phases

#### 7. Global State (50+ global variables)

**Issue**: Heavy reliance on global variables instead of encapsulation  
**Risk**: Thread-unsafe, hard to test, hidden dependencies  
**Impact**: 🟡 **Medium** – Testability, future threading  
**Effort**: High (requires context structures, ~3-4 weeks)  
**Fix**: Introduce `MinarchContext`, `NextUIContext` structs

#### 8. No Automated Testing

**Issue**: Zero unit tests, integration tests, or CI/CD  
**Risk**: Regressions undetected until users report bugs  
**Impact**: 🟡 **Medium** – Quality assurance, confidence in changes  
**Effort**: High (requires testing framework, ~2-3 months)  
**Fix**: Implement Unity test framework + GitHub Actions CI

#### 9. Technical Debt (275 TODO/FIXME comments)

**Issue**: Significant incomplete work and deferred decisions  
**Risk**: Features half-implemented, known bugs unfixed  
**Impact**: 🟢 **Low** – Code quality, developer experience  
**Effort**: Medium (triage and convert to issues, ~1 week)  
**Fix**: Create GitHub issues for all TODOs, remove stale comments

#### 10. Sparse Documentation (3.3% comments)

**Issue**: Complex code lacks inline explanations  
**Risk**: Onboarding friction, knowledge silos  
**Impact**: 🟢 **Low** – Contributor experience  
**Effort**: Medium (ongoing effort, ~2-3 weeks initial pass)  
**Fix**: Document public APIs, complex algorithms, platform-specific workarounds

---

## Constraints and Invariants

### What MUST NOT Change

#### User-Facing Functionality
- ✅ All emulation cores must continue to work (GB, GBA, SNES, NES, PS, etc.)
- ✅ All UI features must remain accessible (game switcher, battery monitor, WiFi, Bluetooth, LED control)
- ✅ Save states must remain compatible (or provide migration path)
- ✅ User settings must remain compatible
- ✅ PAK system must continue to work

#### Performance Targets
- ✅ Input latency must remain ≤20ms (1 frame at 60fps)
- ✅ Emulation must maintain 60fps for all currently supported cores
- ✅ Battery life must not regress (target: 3-4 hours for PlayStation games)

#### Supported Devices (Current Focus)
- ✅ TrimUI Brick (tg5040)
- ✅ TrimUI Smart Pro (tg5040)

#### Build System Requirements
- ✅ Docker-based cross-compilation must remain the official build method
- ✅ Builds must be reproducible across Windows, macOS, and Linux
- ✅ Release packages must maintain backward compatibility with stock firmware

### What CAN Change

#### Code Organization
- ✅ File structure can be reorganized (preserving Git history)
- ✅ Function signatures can change (internal APIs only)
- ✅ Build system can be modernized (Make → CMake/Meson)

#### Platform Support
- ✅ Unmaintained platforms can be archived (not deleted, preserved in Git)
- ✅ Future platform additions can be rejected if they increase maintenance burden

#### Internal Implementation
- ✅ Data structures can be replaced (custom Array → glib GArray)
- ✅ Global variables can be converted to context structures
- ✅ Unsafe functions can be replaced with safe alternatives

---

## Alignment with Future Vision: MinOS Rebranding

### Long-Term Direction

NextUI is planned to rebrand as **MinOS** with a strategic focus on:
1. **Platform consolidation**: Support only TrimUI Brick and Smart Pro (retire all other platforms)
2. **Simplification**: Minimal UI philosophy, reduced feature bloat
3. **Modularity**: Clean separation between OS layer, emulation engine, and UI
4. **Community extensibility**: Robust PAK system for user-contributed cores and tools

### How Optimization Goals Align

| Optimization Goal | MinOS Alignment |
|-------------------|-----------------|
| **Reduce project size** | ✅ Essential for focused platform support |
| **Reduce complexity** | ✅ Core to "minimal OS" philosophy |
| **Windows emulation** | ✅ Broadens contributor base, accelerates development |

### Strategic Decisions Informed by Optimization

1. **Archive unmaintained platforms**: Directly supports platform consolidation
2. **Modularize architecture**: Prepares for future MinOS layered architecture
3. **Reduce duplication**: Enables cleaner platform abstraction for TrimUI-only focus
4. **Improve testing**: Essential for maintaining quality during transition

---

## Risks and Mitigation

### Risk: Functional Regressions

**Probability**: High (any refactoring carries risk)  
**Impact**: Critical (broken emulation, lost saves, user frustration)  
**Mitigation**:
- ✅ Never refactor directly on `main` branch (use feature branches)
- ✅ Test every change on actual TrimUI hardware before merging
- ✅ Maintain backward compatibility for save states and configs
- ✅ Implement smoke tests for core functionality
- ✅ Use Git bisect to identify regression sources quickly

### Risk: Performance Degradation

**Probability**: Medium (abstraction can introduce overhead)  
**Impact**: High (emulation stutters, battery life decreases)  
**Mitigation**:
- ✅ Benchmark before and after every performance-sensitive change
- ✅ Profile with `perf` or `gprof` to identify hotspots
- ✅ Keep tight inner loops optimized (emulation core)
- ✅ Use compiler optimizations (`-O3`, LTO)

### Risk: Build System Breakage

**Probability**: Medium (Makefile complexity is high)  
**Impact**: Medium (contributors cannot build)  
**Mitigation**:
- ✅ Document all build commands clearly
- ✅ Test builds on Windows (Docker), macOS, and Linux
- ✅ Add CI/CD to catch build breaks immediately
- ✅ Provide Docker images for consistent environment

### Risk: Contributor Friction

**Probability**: Low-Medium (change can be disruptive)  
**Impact**: Medium (reduced contributions, community frustration)  
**Mitigation**:
- ✅ Communicate optimization plan clearly (this document)
- ✅ Provide migration guides for contributors
- ✅ Keep changes incremental and well-documented
- ✅ Solicit feedback at major milestones

### Risk: Scope Creep

**Probability**: Medium (optimizations can expand indefinitely)  
**Impact**: Medium (timeline slips, incomplete work)  
**Mitigation**:
- ✅ Define clear phase boundaries (see execution plan)
- ✅ Limit Phase 1 to size reduction and safety fixes only
- ✅ Defer nice-to-have improvements to later phases
- ✅ Track progress with GitHub project board

---

## Success Criteria

### Phase 1 (0-3 months)

- ✅ Unmaintained platforms archived (LOC reduction to ~80,000)
- ✅ All unsafe functions replaced (459 → 0)
- ✅ All allocations have null checks (150 → 0)
- ✅ GitHub Actions CI/CD running
- ✅ Windows build workflow documented and tested
- ✅ No functional regressions on TrimUI hardware

### Phase 2 (3-9 months)

- ✅ Monolithic files split (no file >1,500 lines)
- ✅ God functions refactored (no function >150 lines)
- ✅ Code duplication reduced to <15%
- ✅ Context structures introduced (global state reduced)
- ✅ Build system migrated to CMake or Meson
- ✅ Save state checksums implemented

### Phase 3 (9-12 months)

- ✅ Code duplication reduced to <10%
- ✅ Platform abstraction modularized (common + device-specific overrides)
- ✅ Automated testing with 50+ unit tests
- ✅ Zero TODO/FIXME comments (all converted to issues or resolved)
- ✅ Comment ratio increased to 15%
- ✅ Cyclomatic complexity average <50/file

### Final Success (12+ months)

- ✅ Total LOC reduced to ~70,000
- ✅ Code quality metrics match industry best practices
- ✅ Windows contributors can build and test without barriers
- ✅ Zero critical bugs or security vulnerabilities
- ✅ Positioned for MinOS rebranding and platform consolidation

---

## Conclusion

NextUI has achieved remarkable technical capabilities but faces significant maintainability challenges due to rapid organic growth. The three optimization goals—reducing project size, eliminating complexity and duplication, and enabling Windows development—are achievable through a phased, incremental approach over 9-12 months.

**Key Takeaways**:
1. **60% of code is removable** (unmaintained platforms + duplication)
2. **Safety issues are fixable** in weeks (null checks, safe string functions)
3. **Architectural improvements require patience** but are high-value (modularization, testing)
4. **Windows workflows are already 80% there** (Docker Desktop works, needs documentation)

The optimization work directly supports the future MinOS vision by:
- Creating a lean, focused codebase for TrimUI-only support
- Establishing quality standards for long-term sustainability
- Broadening the contributor base through improved tooling and documentation

**Next Steps**:
1. Review this document with core team
2. Assign owners to optimization phases
3. Create GitHub project board
4. Begin Phase 1: Archive unmaintained platforms, fix safety issues, document Windows workflows

---

**Document Owner**: NextUI Core Team  
**Last Updated**: November 15, 2025  
**Next Review**: January 2026 (post-Phase 1)
