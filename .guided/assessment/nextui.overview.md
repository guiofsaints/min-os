# NextUI – High-Level Overview

## What is NextUI?

**NextUI** is a custom firmware (CFW) for handheld retro gaming devices, specifically targeting the **TrimUI Brick** and **TrimUI Smart Pro**. It is based on **MinUI** – a minimal retro gaming interface originally created by Shaun Inman – but NextUI has significantly diverged from MinUI through a complete rebuild of the emulation engine and the addition of numerous features tailored to the TrimUI hardware ecosystem.

NextUI aims to deliver a polished, high-performance emulation experience with low latency, advanced audio resampling, GPU-accelerated rendering, shader support, and modern quality-of-life features such as WiFi integration, Bluetooth audio, game time tracking, LED control, and deep sleep management.

---

## Purpose & Main Features

### Core Purpose
NextUI exists to address performance, latency, and feature gaps in the stock firmware and vanilla MinUI for TrimUI devices. The project's primary goals are:

1. **Eliminate screen tearing and sync stutter** via a rebuilt emulation core
2. **Reduce input latency** to ~20ms (1 frame at 60fps)
3. **Provide high audio quality** through advanced resampling (libsamplerate)
4. **Enable GPU-accelerated rendering** (OpenGL/GLES) with shader support
5. **Add modern conveniences** WiFi, Bluetooth audio, game time tracking, LED customization, ambient mode, and more

### Feature Highlights

- **Rebuilt Emulation Engine**: Fixed sync and tearing issues from MinUI
- **Game Switcher Menu**: Onion OS-style quick switcher (by [@frysee](https://github.com/frysee))
- **Advanced Audio**: libsamplerate-based resampling with per-emulator quality/performance profiles
- **Low Latency**: Average 20ms latency, ~1 frame at 60fps
- **Shaders & Overlays**: CRT filters, pixel grids, scanlines, and custom overlays
- **OpenGL/GPU Rendering**: Fully GPU-based for improved performance
- **WiFi Support**: Native WiFi with WPA/WPA2, network time sync (NTP), and timezone management
- **Bluetooth Audio**: Wireless audio output via Bluetooth
- **Game Art & Media**: Support for game artwork and metadata
- **Game Time Tracker**: Tracks playtime and session history
- **Cheat Support**: Integrated cheat engine
- **LED Control**: Customizable LED colors, effects, brightness, and ambient mode (Brick only)
- **Haptic Feedback**: Vibration feedback in menus (by [@ExonakiDev](https://github.com/exonakidev))
- **Deep Sleep Mode**: Instant-on functionality and avoids overheating on the Brick (by [@zhaofengli](https://github.com/zhaofengli))
- **Battery Monitoring**: History graph and time-remaining predictions
- **Dynamic CPU Scaling**: Balances performance, battery life, and thermal management
- **Display Controls**: Brightness, color temperature, contrast, saturation, exposure
- **Custom Boot Logos**: Contributed by [@SolvalouArt](https://bsky.app/profile/solvalouart.bsky.social)
- **Scrolling Titles**: Animated scrolling for long game names
- **Updated Emulation Cores**: Optimized builds of libretro cores (FBNeo, SNES9x, MGBA, PCSX ReARMed, etc.)
- **Rumble Support**: Variable rumble strength matching game behavior
- **FBNeo Screen Rotation**: Arcade game rotation support
- **PAL Mode**: Proper PAL region emulation
- **CJK Font Support**: Japanese, Chinese, Korean ROM name rendering

---

## Supported Devices

- **TrimUI Brick** (tg5040)
- **TrimUI Smart Pro** (tg5040)

For other devices, NextUI recommends checking out the upstream [MinUI](https://github.com/shauninman/MinUI) project.

---

## How NextUI Extends & Diverges from MinUI

NextUI started as a fork of MinUI but has evolved into a distinct project with significant architectural and feature differences.

### What NextUI Inherits from MinUI

- **Core Philosophy**: Minimal UI, fast navigation, no-frills design
- **Folder Structure**: ROM organization by console with tagged folders (e.g., `Game Boy (GB)`)
- **PAK System**: Extensible emulator and tool paks (`.pak` folders with `launch.sh`)
- **Libretro Integration**: Leverages libretro cores for emulation
- **Settings Persistence**: Stored in `.userdata` folders per platform

### Where NextUI Diverges

| **Aspect**               | **MinUI**                                   | **NextUI**                                                   |
|--------------------------|---------------------------------------------|--------------------------------------------------------------|
| **Emulation Engine**     | Basic libretro wrapper (`minarch`)          | Completely rebuilt `minarch` with advanced sync, GPU rendering, OpenGL shaders, thread optimization |
| **Audio Resampling**     | Basic SDL audio                             | libsamplerate with quality tiers (performance vs. fidelity)  |
| **Rendering**            | Software blitting or basic SDL acceleration | Full OpenGL/GLES GPU pipeline with shader support            |
| **Latency**              | Varies (~2-3 frames typical)                | Optimized to ~20ms (1 frame)                                 |
| **WiFi & Bluetooth**     | Not supported                               | Full WiFi (WPA/WPA2, NTP sync) and Bluetooth audio           |
| **LED Control**          | Not applicable (device-specific)            | Custom LED effects, ambient mode, brightness profiles (Brick)|
| **Game Time Tracking**   | Not supported                               | Integrated database tracking playtime and sessions           |
| **Battery Monitoring**   | Basic battery icon                          | Battery history graph, time remaining prediction             |
| **Deep Sleep**           | Not supported                               | Instant-on deep sleep with power-off protection (Brick)      |
| **UI Enhancements**      | Static menus                                | Animated scrolling titles, game switcher, themed colors      |
| **Platform Focus**       | Multi-device (many handhelds)               | TrimUI Brick & Smart Pro optimized (other devices unmaintained) |

### Technical Divergence

NextUI's `minarch` emulator core has been heavily rewritten:

- **Frame Pacing & Vsync**: Custom vsync logic to eliminate tearing and stutter
- **Threaded Video**: Optional threaded rendering for certain cores
- **Shader Pipeline**: Multi-pass OpenGL shader support with runtime parameter adjustment
- **Overlay System**: Layered rendering for overlays, backgrounds, and UI elements
- **Audio Buffering**: Sophisticated batching and resampling with underrun mitigation
- **Input Polling**: Optimized input handling with analog stick support and turbo modes
- **State Management**: Enhanced save state management with auto-resume and quick-save slots

---

## Strengths

1. **Performance & Responsiveness**: Significantly lower latency and smoother gameplay than stock firmware and vanilla MinUI
2. **Feature-Rich**: Comprehensive feature set (WiFi, BT audio, game tracking, LED control, etc.) without bloat
3. **GPU Acceleration**: Modern OpenGL rendering enables shaders and efficient scaling
4. **Audio Quality**: libsamplerate provides audiophile-grade resampling
5. **Extensibility**: PAK system allows community-contributed emulators and tools
6. **Active Development**: Regular updates and responsive maintainers
7. **Deep Hardware Integration**: Excellent use of TrimUI-specific features (LEDs, deep sleep, haptics)
8. **Documentation**: Comprehensive online documentation and guides ([nextui.loveretro.games](https://nextui.loveretro.games))
9. **Build System**: Docker-based cross-compilation ensures reproducible builds
10. **Community**: Strong Discord community and growing ecosystem of paks

---

## Weaknesses

1. **Code Complexity**: Large, monolithic C files (`nextui.c` ~3200 lines, `minarch.c` ~7200 lines) with deeply nested logic
2. **Technical Debt**: Significant `TODO`, `FIXME`, and `HACK` comments throughout the codebase (~100+ occurrences)
3. **Duplicated Code**: Platform abstraction layers (`libmsettings`, `platform.c`) duplicated across unmaintained devices
4. **Unmaintained Platforms**: Large `_unmaintained/` directory with obsolete code that bloats the repository
5. **Hard-to-Follow Control Flow**: Extensive use of function pointers, callbacks, and macros obscures execution paths
6. **Monolithic Functions**: Several functions exceed 300+ lines (e.g., `nextui.c:main` is 1120 lines)
7. **Error Handling**: Inconsistent error handling; many failures logged but not gracefully recovered
8. **Build System Fragility**: Makefile dependencies brittle; easy to trigger unintended full rebuilds
9. **Testing**: No automated tests or CI/CD; manual testing only
10. **Documentation Gaps**: Inline comments sparse; many complex sections undocumented
11. **Memory Management**: Manual malloc/free with limited RAII-style cleanup; potential for leaks
12. **Dead Code**: Unused or legacy code paths (e.g., conditional compilation for unsupported devices)

---

## Summary

NextUI is a **highly capable, performance-focused CFW** for TrimUI devices that pushes the hardware to its limits. It successfully addresses the core issues of latency, tearing, and audio quality while adding a rich feature set that rivals commercial handheld UIs.

However, the codebase has grown **organically and rapidly**, resulting in maintainability challenges, technical debt, and code quality issues typical of fast-moving open-source hardware projects. A strategic refactoring effort focused on **modularization, dead code removal, and incremental cleanup** would significantly improve long-term sustainability without disrupting the current user experience.

NextUI is a testament to what a small, dedicated team can achieve with direct hardware access and community collaboration, but it would benefit from some **architectural consolidation** and **cleanup to ensure it can scale gracefully** as new features and devices are added.

---

**Next Steps**: See the detailed architecture, code quality, and refactor plan documents for actionable recommendations.
