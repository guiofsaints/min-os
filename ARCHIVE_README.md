# Unmaintained Platforms Archive

**Branch**: `archive/unmaintained-platforms`  
**Last Maintained**: November 15, 2025  
**Archived From Commit**: `1e1bb85`  
**Removal Commit**: `fd72fb6`

---

## Purpose

This branch preserves platform code for devices no longer actively supported in NextUI/min-os. All code remains accessible in Git history for community forks and historical reference.

---

## Archived Platforms

The following 11 platforms are preserved in `workspace/_unmaintained/`:

### Handheld Devices
1. **gkdpixel** - GKD Pixel (ARMv7)
2. **miyoomini** - Miyoo Mini (ARMv7, Allwinner A30)
3. **my282** - Miyoo 282 (ARMv7)
4. **my355** - Miyoo 355 (ARMv7)
5. **rg35xx** - Anbernic RG35XX (ARMv7, Actions ATM7039)
6. **rg35xxplus** - Anbernic RG35XX Plus (ARMv7, Allwinner H700)
7. **rgb30** - Powkiddy RGB30 (ARMv7, Rockchip RK3566)
8. **trimuismart** - TrimUI Smart (ARMv7, Allwinner A33)
9. **zero28** - Powkiddy Zero 2.8 (ARMv7)

### Mini Consoles
10. **m17** - Magic M17 (ARMv7)
11. **magicmini** - Magic Mini (ARMv7)

---

## Why Archived?

**Rationale for archival** (from commit `fd72fb6`):
- Reduces active codebase by ~50% (from 160K to 80K LOC)
- Eliminates 33.71% code duplication from platform-specific copies
- Focuses development resources on TrimUI Brick/Smart Pro (tg5040)
- Aligns with min-os vision: streamlined, single-platform custom firmware

**Quality Assessment**:
- These platforms were unmaintained with incomplete/broken support
- No recent testing or quality assurance
- Missing features compared to upstream MinUI

---

## Migration Paths for Users

If you own one of these devices, you have three options:

### Option 1: Use Earlier NextUI Releases
- Download NextUI v1.x releases that included platform support
- Check GitHub releases for last version supporting your device
- Stable but won't receive new features

### Option 2: Switch to Upstream MinUI
- Install [MinUI](https://github.com/shauninman/MinUI) instead
- Active maintenance for most archived platforms
- Official support and documentation

### Option 3: Fork for Community Maintenance
- Fork from this archive branch
- Continue development independently
- Merge upstream improvements selectively

---

## Code Structure

Platform-specific code preserved in:
```
workspace/_unmaintained/
├── gkdpixel/
│   ├── cores/           # Libretro cores configuration
│   ├── libmsettings/    # Platform settings library
│   └── platform/        # Hardware abstraction layer
├── miyoomini/
├── my282/
├── my355/
├── rg35xx/
├── rg35xxplus/
├── rgb30/
├── trimuismart/
├── zero28/
├── m17/
└── magicmini/

skeleton/_unmaintained/
├── BOOT/                # Boot scripts and launchers
└── EXTRAS/              # Additional emulator paks
```

---

## Toolchains

Archived platform toolchains:
- `gkdpixel-toolchain`
- `miyoomini-toolchain`
- `my282-toolchain`
- `my355-toolchain`
- `rg35xx-toolchain`
- `rg35xxplus-toolchain`
- `rgb30-toolchain`
- `trimuismart-toolchain`
- `zero28-toolchain`
- `m17-toolchain`
- `magicmini-toolchain`

Docker images and build configurations are preserved in Git history.

---

## Accessing Archived Code

### Clone This Branch
```bash
git clone -b archive/unmaintained-platforms https://github.com/guiofsaints/min-os.git
```

### Switch to Archive Branch
```bash
git checkout archive/unmaintained-platforms
```

### Extract Specific Platform
```bash
git checkout archive/unmaintained-platforms -- workspace/_unmaintained/miyoomini
```

---

## Building Archived Platforms

**Warning**: Build system may require modifications to work with archived code.

### Prerequisites
1. Install appropriate toolchain for target platform
2. Restore makefile references (check commit `fd72fb6^`)
3. Test build on compatible Docker image

### Example Build (Miyoo Mini)
```bash
# From archive branch
make PLATFORM=miyoomini

# Note: May require build system adjustments
```

---

## Contributing to Archives

This branch is **read-only** for historical preservation.

For community forks:
1. Fork this repository
2. Create new branch from `archive/unmaintained-platforms`
3. Maintain independently with your own release cycle

---

## Active Development

**Current supported platforms** (main branch):
- ✅ **tg5040** - TrimUI Brick / Smart Pro (primary focus)
- ✅ **desktop** - Native macOS/Linux builds (testing only)

See main branch README for current development status.

---

## Links

- **Main Repository**: [github.com/guiofsaints/min-os](https://github.com/guiofsaints/min-os)
- **Upstream MinUI**: [github.com/shauninman/MinUI](https://github.com/shauninman/MinUI)
- **Platform Simplification Plan**: `.guided/assessment/nextui.platform-simplification-plan.md`
- **Archival Commit**: [`fd72fb6`](https://github.com/guiofsaints/min-os/commit/fd72fb6)

---

**Archive Status**: ✅ Preserved  
**Last Updated**: November 15, 2025  
**Maintainer**: Community (unmaintained by core team)
