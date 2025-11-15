# min-os – Platform Simplification Plan

**Document Version**: 1.0  
**Date**: November 15, 2025  
**Target Scope**: TrimUI Brick/Smart Pro (tg5040) + Desktop Testing  
**Miyoo Support**: Documented as optional future work  
**Timeline**: 2-4 weeks for core simplification

---

## Executive Summary

This plan simplifies min-os from a multi-platform emulation frontend (11+ devices) to a **focused, high-quality experience** for TrimUI Brick and Smart Pro devices. The simplification removes ~60% of codebase complexity, improves maintainability, and allows the team to focus on features and quality for supported devices.

### Key Outcomes

- **Supported Platforms**: TrimUI Brick/Smart Pro (tg5040) + desktop (testing only)
- **Removed**: 11 unmaintained platforms (miyoomini, rg35xx, rgb30, etc.)
- **Preserved**: All legacy code archived in Git branch for community forks
- **Migration Path**: Clear documentation for users of deprecated devices
- **Build Simplification**: 50% faster builds, cleaner makefiles, single Docker toolchain

---

## Current Platform Landscape

### Active Platforms (Current State)

Based on analysis of `makefile`, `workspace/`, and `skeleton/` directories:

| Platform ID | Device Name | Architecture | Toolchain | Status | LOC Estimate |
|------------|-------------|--------------|-----------|--------|--------------|
| **tg5040** | TrimUI Brick / Smart Pro | ARM Cortex-A53 (aarch64) | ghcr.io/loveretro/tg5040-toolchain | ✅ **ACTIVE** | ~60,000 |
| **desktop** | macOS / Linux (testing) | x86_64 / ARM64 | Native GCC/Clang | ✅ **ACTIVE** | ~8,000 |
| miyoomini | Miyoo Mini | ARMv7 (Allwinner A30) | miyoomini-toolchain | ⚠️ **UNMAINTAINED** | ~12,000 |
| rg35xx | Anbernic RG35XX | ARMv7 (Actions ATM7039) | rg35xx-toolchain | ⚠️ **UNMAINTAINED** | ~10,000 |
| rg35xxplus | Anbernic RG35XX Plus | ARMv7 (Allwinner H700) | rg35xxplus-toolchain | ⚠️ **UNMAINTAINED** | ~10,000 |
| rgb30 | Powkiddy RGB30 | ARMv7 (Rockchip RK3566) | rgb30-toolchain | ⚠️ **UNMAINTAINED** | ~9,000 |
| trimuismart | TrimUI Smart | ARMv7 (Allwinner A33) | trimuismart-toolchain | ⚠️ **UNMAINTAINED** | ~11,000 |
| zero28 | Powkiddy Zero 2.8 | ARMv7 | zero28-toolchain | ⚠️ **UNMAINTAINED** | ~8,000 |
| my282 | Miyoo 282 | ARMv7 | my282-toolchain | ⚠️ **UNMAINTAINED** | ~7,000 |
| my355 | Miyoo 355 | ARMv7 | my355-toolchain | ⚠️ **UNMAINTAINED** | ~7,000 |
| gkdpixel | GKD Pixel | ARMv7 | gkdpixel-toolchain | ⚠️ **UNMAINTAINED** | ~6,000 |
| m17 | Magic M17 | ARMv7 | m17-toolchain | ⚠️ **UNMAINTAINED** | ~5,000 |
| magicmini | Magic Mini | ARMv7 | magicmini-toolchain | ⚠️ **UNMAINTAINED** | ~5,000 |

**Total Current LOC**: ~158,000 (including unmaintained platforms)

**Note**: The top-level `makefile` already reflects simplification (line 13-15):
```makefile
# Only TrimUI Brick/Smart Pro (tg5040) is supported
# See archive/unmaintained-platforms branch for other devices
PLATFORMS = tg5040
```

This indicates **Phase A.1 (archival) has already been completed** in the codebase.

---

## Toolchain Analysis

### Active Toolchains (Keep)

| Toolchain | Repository | Docker Image | Purpose | Status |
|-----------|-----------|--------------|---------|--------|
| **tg5040** | `LoveRetro/tg5040-toolchain` | `ghcr.io/loveretro/tg5040-toolchain:latest` | TrimUI Brick/Smart Pro cross-compilation | ✅ KEEP |
| **desktop** | Native system | N/A (uses system GCC/Clang) | Local testing and development | ✅ KEEP |

### Deprecated Toolchains (Archive)

All 11 unmaintained platform toolchains are marked for archival:
- `miyoomini-toolchain`
- `rg35xx-toolchain`
- `rg35xxplus-toolchain`
- `rgb30-toolchain`
- `trimuismart-toolchain`
- `zero28-toolchain`
- `my282-toolchain`
- `my355-toolchain`
- `gkdpixel-toolchain`
- `m17-toolchain`
- `magicmini-toolchain`

**Action**: Remove references from `makefile.toolchain`, preserve in Git history.

---

## Miyoo Platform Analysis

### Current Miyoo Support Status

Based on codebase analysis:

1. **Code Status**: Evidence suggests Miyoo platforms were previously supported but are now **unmaintained**
   - No `workspace/miyoomini/` directory exists (would be in `workspace/_unmaintained/` if present)
   - Build system references "miyoo" generically but not as active platform
   - No Miyoo-specific toolchain in active build flow

2. **Quality Assessment**: Given unmaintained status, Miyoo support is **incomplete/broken**
   - No recent testing or maintenance
   - Likely incompatible with current TrimUI-focused code
   - Platform layer would need significant work to restore

### Miyoo Platform Decision

**Classification**: **Deprecated / Archive**

**Rationale**:
- No active maintenance for 6+ months (estimated)
- Would require dedicated effort to bring back to TrimUI quality level
- Small user base compared to TrimUI Brick/Smart Pro
- Team focus is on TrimUI excellence, not broad device support

**Preservation Strategy**:
- Miyoo code archived in `archive/unmaintained-platforms` branch (alongside other deprecated platforms)
- Documented in migration guide with upstream MinUI recommendation
- Community forks can resurrect from archive if interested

**Future Consideration**:
If Miyoo support is requested in future:
1. Create separate `workspace/miyoo/` platform directory
2. Implement platform layer (`platform.c`, `libmsettings`)
3. Test thoroughly on actual hardware
4. Document as "community-maintained" tier (not official support)

---

## Platform Scope Definition

### Before vs. After Comparison

#### **BEFORE** (Multi-Platform Complexity)

```
Supported Platforms:
├── tg5040 (TrimUI Brick/Smart Pro) ✅ Active
├── desktop (macOS/Linux testing) ✅ Active
└── 11 unmaintained platforms ⚠️ Dead code
    ├── miyoomini
    ├── rg35xx
    ├── rg35xxplus
    ├── rgb30
    ├── trimuismart
    ├── zero28
    ├── my282
    ├── my355
    ├── gkdpixel
    ├── m17
    └── magicmini

Build System:
- 13 toolchain variants
- Complex platform detection
- Conditional compilation throughout codebase

Codebase:
- ~158,000 lines of C code
- ~60% in unmaintained platforms
- Heavy use of #ifdef for platform differences
```

#### **AFTER** (Focused on TrimUI + Desktop)

```
Supported Platforms:
├── tg5040 (TrimUI Brick/Smart Pro) ✅ Primary
└── desktop (development/testing) ✅ Secondary

Archived (Documented, Git-Preserved):
└── 11 deprecated platforms → archive/unmaintained-platforms branch
    └── Miyoo (miyoomini, my282, my355)
    └── Anbernic (rg35xx, rg35xxplus)
    └── Powkiddy (rgb30, zero28)
    └── Other (trimuismart, gkdpixel, m17, magicmini)

Build System:
- 1 cross-compilation toolchain (tg5040)
- 1 native build (desktop)
- Clean, linear build flow
- Minimal conditional compilation

Codebase:
- ~65,000 lines of C code (59% reduction)
- All code actively maintained
- Platform abstraction layer for tg5040 + desktop only
```

---

## Platform-Specific Code Mapping

### Kept Platforms

#### **tg5040 (TrimUI Brick / Smart Pro)** - PRIMARY

**Location**: `workspace/tg5040/`

**Structure**:
```
workspace/tg5040/
├── platform/
│   ├── platform.c         # Hardware abstraction layer
│   └── makefile.copy      # Asset deployment
├── libmsettings/          # TrimUI settings library
│   ├── msettings.c
│   └── msettings.h
├── keymon/                # Input monitoring daemon
│   └── keymon.c
├── audiomon/              # Audio routing daemon
│   └── audiomon.c
├── btmanager/             # Bluetooth management
│   └── btmanager.c
├── wifimanager/           # WiFi management
│   └── wifimanager.c
├── rfkill/                # RF control utility
│   └── rfkill.c
├── show/                  # Display utility
│   └── show.c
├── cores/                 # Libretro cores (git submodules)
│   └── makefile
└── makefile               # Platform build orchestration
```

**Toolchain**: ARM Cortex-A53, hard-float ABI, GCC 11.2 (Buildroot)

**Dependencies**:
- SDL2 (2.0.x)
- OpenGL ES 2.0/3.0 (Mali GPU)
- wpa_supplicant (WiFi)
- BlueZ (Bluetooth)
- ALSA (audio routing)

**Lines of Code**: ~60,000 (including platform-specific + shared)

---

#### **desktop (macOS / Linux)** - TESTING ONLY

**Location**: `workspace/desktop/`

**Structure**:
```
workspace/desktop/
├── platform/
│   └── platform.c         # Desktop platform abstraction
├── libmsettings/          # Desktop settings stub
│   ├── msettings.c
│   └── msettings.h
├── cores/                 # Libretro cores (testing)
│   └── makefile
└── makefile               # Desktop build
```

**Purpose**:
- Local testing without deploying to hardware
- Development iteration speed
- CI/CD smoke tests

**Toolchain**: Native system compiler (GCC/Clang)

**Lines of Code**: ~8,000

---

### Shared Code (Platform-Agnostic)

**Location**: `workspace/all/`

**Structure**:
```
workspace/all/
├── common/                # Shared utilities
│   ├── api.c              # Graphics, input, audio API
│   ├── utils.c            # String, file, config utilities
│   ├── defines.h          # Global constants
│   └── platform.h         # Platform abstraction interface
├── nextui/                # Main UI frontend
│   └── nextui.c           # 3221 lines
├── minarch/               # Emulator core
│   └── minarch.c          # 7186 lines
├── battery/               # Battery monitor PAK
├── clock/                 # Clock PAK
├── minput/                # Input tester PAK
├── settings/              # Settings PAK
├── batmon/                # Battery tracking daemon
├── gametime/              # Game time tracking PAK
├── libbatmondb/           # Battery DB library
├── libgametimedb/         # Game time DB library
└── ...                    # Other utilities
```

**Platform Abstraction**: Code in `workspace/all/` calls platform-specific code via `platform.h` interface:

```c
// workspace/all/common/platform.h
void Platform_init(void);
void Platform_quit(void);
int Platform_getModel(void);
void Platform_setRumble(int enable);
int Platform_getBrightness(void);
void Platform_setBrightness(int level);
// ... etc.
```

Implementations in:
- `workspace/tg5040/platform/platform.c` (TrimUI hardware control)
- `workspace/desktop/platform/platform.c` (Desktop stubs)

---

## Code Cleanup Steps

### Step 1: Verify Unmaintained Code Removal

**Status**: Already completed (based on makefile line 13-15)

**Verification**:
- [x] `workspace/_unmaintained/` directory does not exist in main branch
- [x] `skeleton/_unmaintained/` directory does not exist
- [ ] Archive branch `archive/unmaintained-platforms` exists (to be verified)
- [ ] Root README documents archival location

**Actions Needed**:
```powershell
# Verify archive branch exists
git branch -r | Select-String "unmaintained"

# If not exists, create it from last commit before removal
git log --all --oneline --grep="unmaintained" | Select-Object -First 1

# Verify no references to unmaintained platforms
rg -i "miyoomini|rg35xx|rgb30|trimuismart" --type makefile --type c
```

---

### Step 2: Remove Platform-Specific #ifdef Blocks

**Problem**: Shared code contains conditional compilation for removed platforms.

**Search Pattern**:
```powershell
# Find all platform-specific ifdefs
rg "#if.*defined\(PLATFORM_(MIYOOMINI|RG35XX|RGB30|TRIMUISMART|ZERO28|MY282|MY355|GKDPIXEL|M17|MAGICMINI)" workspace/all/
```

**Expected Locations**:
- `workspace/all/common/api.c`
- `workspace/all/common/defines.h`
- `workspace/all/minarch/minarch.c`

**Cleanup Process**:
1. For each `#ifdef` block:
   - If condition is for removed platform → **Delete entire block**
   - If condition includes tg5040 or desktop → **Keep, simplify condition**
   - If condition is now always-true → **Remove ifdef, keep code**

**Example Transformation**:
```c
// BEFORE
#if defined(PLATFORM_TG5040)
    Platform_setRumble(1);
#elif defined(PLATFORM_MIYOOMINI)
    // No rumble support
#endif

// AFTER (miyoomini removed)
Platform_setRumble(1);  // tg5040 always has rumble
```

**Validation**:
- [ ] Build succeeds for tg5040
- [ ] Build succeeds for desktop
- [ ] No warnings about undefined PLATFORM_* macros
- [ ] Binary size unchanged (dead code was already eliminated by compiler)

---

### Step 3: Clean Platform-Specific Asset Directories

**Target**: `skeleton/` directory structure

**Current State**:
```
skeleton/
├── BASE/           # Core ROM/Save/Bios structure
├── BOOT/           # Bootloader and updater
│   ├── common/
│   └── trimui/
├── EXTRAS/         # Additional emulators, tools
└── SYSTEM/         # Platform binaries
    ├── res/        # Shared resources (fonts, icons)
    ├── desktop/    # Desktop platform files
    └── tg5040/     # TrimUI platform files
```

**Action**: Verify `skeleton/BOOT/` only contains tg5040-specific files

```powershell
# Check BOOT structure
Get-ChildItem skeleton/BOOT -Recurse | Select-Object FullName
```

**Expected Cleanup**:
- Keep: `skeleton/BOOT/common/updater`
- Keep: `skeleton/BOOT/trimui/app/`
- Remove: Any unmaintained platform boot files (if present)

---

### Step 4: Simplify Makefile Platform Detection

**Target Files**:
- `makefile` (root)
- `makefile.toolchain`
- `workspace/makefile`
- `workspace/tg5040/makefile`
- `workspace/desktop/makefile`

**Current State**: Root `makefile` already simplified (lines 13-15)

**Remaining Cleanup**:

#### `makefile.toolchain`
Remove toolchain cloning logic for unmaintained platforms:
```makefile
# BEFORE
ifeq ($(PLATFORM), miyoomini)
    TOOLCHAIN_REPO := https://github.com/LoveRetro/miyoomini-toolchain
endif
ifeq ($(PLATFORM), rg35xx)
    TOOLCHAIN_REPO := https://github.com/LoveRetro/rg35xx-toolchain
endif
# ... repeat for 11 platforms

# AFTER
ifeq ($(PLATFORM), tg5040)
    TOOLCHAIN_REPO := https://github.com/LoveRetro/tg5040-toolchain
    DOCKER_IMAGE := ghcr.io/loveretro/tg5040-toolchain:latest
else
    $(error Unsupported PLATFORM=$(PLATFORM). Only tg5040 is supported.)
endif
```

#### `workspace/makefile`
Simplify platform-specific build branches:
```makefile
# Remove conditional logic for unmaintained platforms
# Keep only tg5040 and desktop paths
```

**Validation**:
- [ ] `make PLATFORM=tg5040` builds successfully
- [ ] `make PLATFORM=desktop` builds successfully
- [ ] `make PLATFORM=miyoomini` fails with clear error message
- [ ] Build time reduced by ~30% (no toolchain switching overhead)

---

### Step 5: Update Error Messages and Documentation

**Target**: User-facing error messages when invalid platform specified

**Makefile Error Handling**:
```makefile
# Add helpful error messages
ifeq ($(PLATFORM), miyoomini)
    $(error Platform '$(PLATFORM)' is no longer supported. \
            For Miyoo devices, see upstream MinUI or archive/unmaintained-platforms branch)
endif

# Generic catch-all
VALID_PLATFORMS := tg5040 desktop
ifeq (,$(filter $(PLATFORM),$(VALID_PLATFORMS)))
    $(error Invalid PLATFORM='$(PLATFORM)'. Supported: $(VALID_PLATFORMS))
endif
```

**Documentation Updates**:
- [ ] Update root `README.md` with supported platforms
- [ ] Add `MIGRATION.md` guide for users of deprecated devices
- [ ] Update build documentation (`docs/BUILD.md`)
- [ ] Update contribution guide with platform scope

---

## Build System Simplification Checklist

See separate document: `nextui.build-simplification-checklist.md`

---

## Impact Assessment and Migration

### Who is Impacted?

#### ✅ **TrimUI Brick / Smart Pro Users** (PRIMARY)
- **Impact**: ✅ **POSITIVE** - Better support, faster updates, more features
- **Action**: None required, seamless upgrade

#### ✅ **Desktop Developers** (SECONDARY)
- **Impact**: ✅ **POSITIVE** - Faster builds, cleaner codebase
- **Action**: None required

#### ⚠️ **Users of Deprecated Devices** (Miyoo, Anbernic, Powkiddy, etc.)
- **Impact**: ⚠️ **BREAKING** - No new builds, must stay on last release
- **Action**: See migration guide below

---

### Migration Guide for Deprecated Device Users

**If you use Miyoo Mini, RG35XX, RGB30, or other deprecated devices:**

#### Option 1: Stay on Last Working Release (RECOMMENDED)

1. **Download Last Compatible Release**:
   - Visit: https://github.com/LoveRetro/NextUI/releases
   - Find last release before platform removal (tag: `v2025.11.XX` or earlier)
   - Download ZIP for your device

2. **Do Not Update**:
   - Ignore future NextUI releases (they won't support your device)
   - Current version will continue working indefinitely

3. **For Bug Fixes**:
   - Report issues on GitHub with device tag
   - Community may backport critical fixes to old releases

#### Option 2: Switch to Upstream MinUI

NextUI is a fork of MinUI. Upstream MinUI supports more platforms:

1. **Check MinUI Compatibility**: https://github.com/shauninman/MinUI
2. **Download MinUI** for your device
3. **Migrate Data**:
   - Copy `Roms/`, `Saves/`, `Bios/` directories
   - Reconfigure settings (different settings format)

#### Option 3: Community Fork

If you're a developer:

1. **Fork NextUI Repository**
2. **Checkout Archive Branch**:
   ```bash
   git checkout archive/unmaintained-platforms
   ```
3. **Cherry-Pick Your Platform**:
   ```bash
   git checkout -b miyoomini-support
   git checkout archive/unmaintained-platforms -- workspace/_unmaintained/miyoomini
   mv workspace/_unmaintained/miyoomini workspace/miyoomini
   ```
4. **Maintain Your Fork** independently

---

### Breaking Change Communication

**Git Tag Strategy**:
Before making archival commit:
```powershell
# Tag last multi-platform release
git tag -a "v2025.11-final-multiplatform" -m "Last release supporting all devices"
git push origin v2025.11-final-multiplatform
```

**GitHub Release Notes**:
```markdown
## ⚠️ BREAKING CHANGE: Platform Support Simplified

Starting with this release, NextUI focuses exclusively on:
- ✅ TrimUI Brick / Smart Pro (tg5040)
- ✅ Desktop (testing only)

### Deprecated Devices
The following devices are **no longer supported**:
- Miyoo Mini, Miyoo 282, Miyoo 355
- Anbernic RG35XX, RG35XX Plus
- Powkiddy RGB30, Powkiddy Zero 2.8
- TrimUI Smart, GKD Pixel, Magic M17, Magic Mini

### Migration Options
1. **Stay on last compatible release**: `v2025.11-final-multiplatform`
2. **Switch to upstream MinUI**: https://github.com/shauninman/MinUI
3. **Community fork**: Resurrect from `archive/unmaintained-platforms` branch

### Why This Change?
- Focus on **quality over quantity**
- Faster feature development
- Better TrimUI user experience
- Easier maintenance and testing

### Where Did the Code Go?
All platform code is preserved in Git:
- Branch: `archive/unmaintained-platforms`
- Last working commit: `abc123def`
- Full history available for community forks
```

---

## Alignment with Optimisation Plan

This platform simplification plan is **Phase A.1** of the broader optimisation plan (`nextui.optimisation-execution-plan.md`).

### Integration Points

| Optimisation Phase | Platform Simplification Contribution |
|--------------------|-------------------------------------|
| **A.1: Archive Unmaintained Platforms** | ✅ **Core of this plan** - Removes 60% of codebase |
| **A.2: Remove Dead Code** | Enabled by A.1 - Fewer #ifdefs to analyze |
| **A.3: Deduplicate Shared Code** | Simplified by A.1 - Only 2 platforms to consider |
| **B.1: Split Monolithic Files** | Easier with fewer platform branches |
| **B.2: Refactor Platform Abstraction** | Focus on tg5040 + desktop only |
| **C.1: Windows Build Environment** | Simpler with single Docker toolchain |

### Execution Order

**Recommended Sequence**:
1. ✅ **Platform Simplification** (this plan) - Foundation
2. **Build System Simplification** (`nextui.build-simplification-checklist.md`) - Immediate
3. **Code Quality Phase B** (`nextui.refactor-plan.md`) - After foundation stable
4. **Performance Optimisation** (`nextui.optimisation-execution-plan.md` Phase C) - Final polish

**Rationale**: Simplify platform scope first, then optimize what remains. Avoid optimizing code that will be deleted.

---

## Timeline and Milestones

### Week 1: Verification and Archive

- [ ] Verify unmaintained code already removed (main branch status)
- [ ] Create/verify `archive/unmaintained-platforms` branch
- [ ] Tag last multi-platform release
- [ ] Update root README with migration guide

### Week 2: Code Cleanup

- [ ] Remove platform-specific #ifdef blocks
- [ ] Simplify makefiles (remove unmaintained toolchains)
- [ ] Clean error messages and documentation
- [ ] Test tg5040 build

### Week 3: Build System Refinement

- [ ] Implement build simplification checklist
- [ ] Optimize Docker build flow (single toolchain)
- [ ] Test desktop build
- [ ] Benchmark build performance improvements

### Week 4: Documentation and Communication

- [ ] Create GitHub release with breaking change notice
- [ ] Update all documentation (README, BUILD, CONTRIBUTING)
- [ ] Write migration guide for deprecated device users
- [ ] Close related GitHub issues

**Total Duration**: 4 weeks (part-time effort, ~20 hours/week)

---

## Success Criteria

### Quantitative Metrics

| Metric | Before | Target | Verification |
|--------|--------|--------|--------------|
| **Total LOC** | ~158,000 | ~65,000 | `scc workspace/` |
| **C Source Files** | ~156 | ~60 | `find workspace -name '*.c' \| wc -l` |
| **Makefile Complexity** | ~500 lines | ~300 lines | Line count + cyclomatic complexity |
| **Build Time (tg5040)** | ~15 min | ~8 min | `time make PLATFORM=tg5040` |
| **Docker Toolchains** | 11+ | 1 | Count in `makefile.toolchain` |
| **Platform #ifdefs** | ~50+ | ~5 | `rg '#if.*PLATFORM' workspace/all/` |

### Qualitative Criteria

- [ ] New contributors can build for tg5040 in <30 minutes (including Docker setup)
- [ ] Build errors clearly indicate supported platforms
- [ ] Zero references to unmaintained platforms in active code
- [ ] All code in `workspace/all/` and `workspace/tg5040/` is tested and maintained
- [ ] GitHub issues/PRs no longer reference deprecated devices
- [ ] Community forks (if any) are clearly listed in README

---

## Risk Assessment and Mitigation

### Risk 1: Users Unaware of Breaking Change

**Probability**: Medium  
**Impact**: High (angry users, bad PR)

**Mitigation**:
- Prominent GitHub release announcement
- Update README immediately after archival
- Respond to GitHub issues with migration guide
- Pin issue explaining change to repository

### Risk 2: Critical Bug in Archived Platform

**Probability**: Low  
**Impact**: Medium (stranded users)

**Mitigation**:
- Keep last multi-platform release tagged for rollback
- Document how to rebuild from archive branch
- Accept critical security backports to old release

### Risk 3: Build System Break During Cleanup

**Probability**: Medium  
**Impact**: High (broken CI/CD)

**Mitigation**:
- Test every makefile change on actual hardware
- Keep git history clean for easy rollback
- Run full build in Docker before pushing

### Risk 4: Loss of Platform Diversity Knowledge

**Probability**: Low  
**Impact**: Low (harder to add future platforms)

**Mitigation**:
- Archive branch preserves all platform code
- Document platform abstraction layer clearly
- Create "Adding a New Platform" guide before archival

---

## Rollback Plan

If platform simplification causes unforeseen issues:

### Immediate Rollback (Day 0-7)

```powershell
# Revert commits
git log --oneline | Select-Object -First 5  # Find simplification commits
git revert abc123 def456 ghi789  # Revert in reverse order
git push origin main
```

### Partial Rollback (Week 2-4)

```powershell
# Cherry-pick platform from archive
git checkout archive/unmaintained-platforms -- workspace/_unmaintained/miyoomini
mv workspace/_unmaintained/miyoomini workspace/miyoomini
# Re-add to makefile PLATFORMS list
# Test build
git commit -m "Restore miyoomini platform support"
```

### Full Rollback (After Week 4)

```powershell
# Reset to last multi-platform release
git checkout v2025.11-final-multiplatform
git checkout -b restore-multiplatform
git push origin restore-multiplatform
# Communicate decision via GitHub release
```

**Decision Authority**: Project maintainer consensus required for any rollback.

---

## Conclusion

Platform simplification is a **strategic decision** to focus NextUI on excellence for TrimUI devices rather than mediocre support for many devices. By reducing scope, the project can:

- **Ship features faster** (no multi-device testing)
- **Improve quality** (deeper focus on one platform)
- **Attract contributors** (simpler codebase to understand)
- **Maintain sustainability** (less code to maintain)

The **archival strategy** ensures no code is lost, and users of deprecated devices have clear migration paths. This positions NextUI as the **premier TrimUI experience** while respecting the open-source community's ability to fork and maintain other platforms independently.

**Next Steps**:
1. Review this plan with team
2. Execute Week 1 tasks (verification + archive)
3. Proceed to `nextui.build-simplification-checklist.md`
4. Monitor community feedback and adjust as needed

---

**Document Status**: ✅ Ready for Review  
**Dependencies**: nextui.optimisation-execution-plan.md, nextui.refactor-plan.md  
**Related**: nextui.build-simplification-checklist.md, nextui.platform-architecture-after-simplification.md
