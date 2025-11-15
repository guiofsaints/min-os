# NextUI – Dependencies, Tech Stack & Tooling

This document catalogs the full technology stack, external dependencies, build system, and toolchain used by NextUI, along with assessments of each dependency's role, maturity, and opportunities for simplification or modernization.

---

## Tech Stack Overview

### Programming Languages

| Language | Usage | % of Codebase | Notes |
|----------|-------|---------------|-------|
| **C (C99/C11)** | All binaries, libraries, cores | ~98% | System programming language for performance and hardware access |
| **Shell (Bash)** | PAK launch scripts, install/update scripts | ~1% | Simple automation for deployment |
| **Make** | Build system orchestration | ~1% | Makefiles for compilation and packaging |
| **GLSL** | OpenGL shaders | <1% | Fragment/vertex shaders for CRT effects, scanlines, etc. |
| **Markdown** | Documentation | N/A | READMEs, guides, online docs |

**Assessment**:
- **C is appropriate** for embedded Linux and performance-critical emulation.
- **Shell scripts are simple and effective** for PAK extensibility.
- **GLSL shaders are well-scoped** and easy to extend.

**Opportunities**:
- Consider **C++ for new modules** (better encapsulation, RAII for resource management).
- Use **Python** for build tooling instead of complex Makefiles (e.g., SCons, Meson).

---

## Build System

### Toolchain Architecture

```
Host Machine (macOS/Linux/Windows)
├── make (GNU Make)
├── docker (Docker Engine)
└── makefile.toolchain
    └── Pulls ghcr.io/loveretro/<platform>-toolchain:latest
        └── Runs workspace/makefile in container
            └── Invokes workspace/<platform>/makefile
                └── Compiles binaries and cores
```

### Build Flow

1. **Top-level `makefile`**:
   - Defines platforms, release naming, package assembly
   - Calls `makefile.toolchain` for each platform
   - Packages `skeleton/` + built binaries into `.zip` files

2. **`makefile.toolchain`**:
   - Clones toolchain repos (e.g., `github.com/LoveRetro/tg5040-toolchain`)
   - Pulls Docker images (e.g., `ghcr.io/loveretro/tg5040-toolchain:latest`)
   - Mounts `workspace/` into container at `/root/workspace`
   - Runs `workspace/makefile` inside container

3. **`workspace/makefile`**:
   - Builds cross-platform code (`workspace/all/`)
   - Calls `workspace/<platform>/makefile` for platform-specific code
   - Copies binaries to `build/SYSTEM/<platform>/`

4. **`workspace/<platform>/makefile`**:
   - Builds platform abstraction layer (`platform.c`, `libmsettings.so`)
   - Builds platform-specific daemons (`keymon`, `wifimanager`, etc.)
   - Optionally builds libretro cores (`cores/makefile`)

5. **Post-build**:
   - `makefile` copies binaries from `workspace/<platform>/build/` to `build/SYSTEM/`
   - Assembles `MinUI.zip` (`.system/` + `.tmp_update/` + `Tools/`)
   - Creates `BASE` and `EXTRAS` zip files for release

### Build Targets

| Target | Description |
|--------|-------------|
| `make all` | Build all platforms and package releases |
| `make PLATFORM=tg5040` | Build single platform |
| `make PLATFORM=tg5040 shell` | Open Docker shell for debugging |
| `make PLATFORM=tg5040 build-cores` | Build libretro cores |
| `make PLATFORM=tg5040 build-core CORE=gambatte` | Build single core |
| `make clean` | Clean build artifacts |
| `make package` | Assemble release zips |

**Assessment**:
- **Docker-based toolchains ensure reproducibility** across contributors.
- **Makefile complexity is high**—deeply nested targets, fragile dependencies.
- **No incremental build optimization**—frequent full rebuilds.

**Issues**:
1. **Brittle dependencies**: Easy to trigger unintended full rebuilds.
2. **No parallel builds by default**: `make -j` not consistently used.
3. **Complex variable passing**: `PLATFORM`, `COMPILE_CORES`, `CORE` passed through layers.
4. **No build caching**: Docker containers rebuilt every time.
5. **Platform-specific logic scattered**: Hard to add new platforms.

**Recommendations**:
- **Short-term**:
  - Add `make -j$(nproc)` to parallel builds.
  - Document build system in `docs/BUILD.md`.
  - Add `make help` target.
- **Medium-term**:
  - Migrate to **CMake** or **Meson** for better dependency tracking.
  - Cache Docker layers for faster builds.
  - Use **ccache** to cache compilation results.
- **Long-term**:
  - Consider **Buildroot** or **Yocto** for full firmware images.

---

## Toolchains (Cross-Compilation)

### Active Toolchains

| Platform | Toolchain Repo | Architecture | Compiler | Sysroot Source |
|----------|---------------|--------------|----------|----------------|
| **tg5040** | `LoveRetro/tg5040-toolchain` | ARMv8 (aarch64) | GCC 11.2 (Buildroot) | Stock TrimUI firmware |
| **desktop** | Native (macOS/Linux) | x86_64 or ARM64 | System GCC/Clang | System libraries |

### Unmaintained Toolchains (Archived)

- `miyoomini-toolchain` (ARMv7, Allwinner A30)
- `rg35xx-toolchain` (ARMv7, Actions ATM7039)
- `rg35xxplus-toolchain` (ARMv7, Allwinner H700)
- `trimuismart-toolchain` (ARMv7, Allwinner A33)
- ... and 6+ more

**Assessment**:
- **Toolchains are well-maintained** for active platforms.
- **Docker images are versioned and cached** (GitHub Container Registry).
- **Sysroots extracted from stock firmware** ensure ABI compatibility.

**Issues**:
- **No automated toolchain updates**—manual process to bump GCC versions.
- **No CI/CD**—no automated testing of toolchain changes.

**Recommendations**:
- Set up **GitHub Actions** to rebuild toolchains on dependency updates.
- Add **smoke tests** to verify compiled binaries run on target hardware.

---

## External Dependencies

### Core Libraries

| Library | Version | Purpose | Link Method | License | Notes |
|---------|---------|---------|-------------|---------|-------|
| **SDL2** | 2.0.x–2.30.x | Video, audio, input | Dynamic (system) | Zlib | Core rendering and I/O |
| **SDL2_ttf** | 2.x | TrueType font rendering | Dynamic (system) | Zlib | UI text rendering |
| **SDL2_image** | 2.x | PNG/JPG loading | Dynamic (system) | Zlib | Image assets, overlays |
| **OpenGL ES 2.0/3.0** | GLES 2.0/3.0 | GPU rendering, shaders | Dynamic (system) | N/A | Hardware acceleration |
| **libsamplerate** | 0.2.2 | Audio resampling | Bundled (static) | BSD-2-Clause | High-quality audio resampling |
| **libzip** | 1.10.x | ZIP file handling | Bundled (static) | BSD-3-Clause | Extracting zipped ROMs |
| **zlib** | 1.2.x | Compression | Dynamic (system) | Zlib | Required by libzip |
| **libbz2** | 1.0.x | Compression | Bundled (static) | BSD-like | Required by libzip |
| **liblzma** | 5.x | Compression | Bundled (static) | Public Domain | Required by libzip |
| **libzstd** | 1.x | Compression | Bundled (static) | BSD-2-Clause | Required by libzip |
| **pthread** | POSIX | Threading | Dynamic (system) | LGPL | Audio threading, CPU monitoring |
| **libdl** | POSIX | Dynamic loading | Dynamic (system) | LGPL | Load libretro cores at runtime |
| **libm** | POSIX | Math functions | Dynamic (system) | LGPL | Floating-point math |

### Platform-Specific Libraries

| Library | Platform | Purpose | Link Method | License | Notes |
|---------|----------|---------|-------------|---------|-------|
| **wpa_supplicant** | tg5040 | WiFi management | System binary | BSD | CLI tool for WPA/WPA2 |
| **bluez** | tg5040 | Bluetooth stack | System daemon + libs | GPL-2.0 | BlueZ 5.x for BT audio |
| **alsa-lib** | tg5040 | Audio routing | Dynamic (system) | LGPL-2.1 | Speaker/headphone switching |
| **mali** | tg5040 | GPU drivers | Dynamic (system) | Proprietary | ARM Mali OpenGL ES |

### Build-Time Tools

| Tool | Purpose | Notes |
|------|---------|-------|
| **GCC** | C compiler | Cross-compilation via Buildroot |
| **Make** | Build orchestration | GNU Make 4.x |
| **Docker** | Containerized builds | Ensures reproducibility |
| **pkg-config** | Library detection | Used by makefiles to find SDL2, etc. |
| **git** | Version control | Submodules for libretro cores |

### Libretro Cores (Git Submodules)

NextUI bundles 20+ libretro cores as Git submodules in `workspace/<platform>/cores/`. Each core is a separate Git repository.

**Core Examples**:
- `gambatte_libretro` (Game Boy, Game Boy Color)
- `gpsp_libretro` (Game Boy Advance)
- `snes9x_libretro` (SNES)
- `fceumm_libretro` (NES)
- `picodrive_libretro` (Genesis, Sega CD)
- `pcsx_rearmed_libretro` (PlayStation)
- `mgba_libretro` (GBA, alternate core)
- `fbneo_libretro` (Arcade)
- ... and 15+ more

**Assessment**:
- Cores are **well-maintained by libretro community**.
- **Submodules ensure version pinning** but complicate updates.

**Issues**:
- **No automated core updates**—manual process to bump submodule refs.
- **Patches required for some cores** (e.g., FBNeo screen rotation, GPSP performance).

**Recommendations**:
- Automate core updates via GitHub Actions (weekly check for upstream changes).
- Upstream patches to libretro repos where possible.

---

## Dependency Analysis

### Strengths

1. **Minimal external dependencies**: Relies mostly on system libraries (SDL2, OpenGL).
2. **Bundled critical libs**: libsamplerate, libzip ensure consistency.
3. **Mature libraries**: SDL2, OpenGL, wpa_supplicant, BlueZ are battle-tested.
4. **Libretro cores**: Benefit from community testing and fixes.

### Weaknesses

1. **No dependency versioning**: System libraries version-dependent (SDL2 2.0.x vs. 2.30.x differences).
2. **Multiple compression libs**: libbz2, liblzma, libzstd all bundled for libzip—overkill.
3. **Platform-specific tools fragile**: wpa_supplicant, bluetoothctl CLIs have unstable output formats.
4. **No package manager**: Dependencies manually managed in toolchain sysroots.

### Opportunities for Simplification

#### 1. Replace Custom Data Structures with GLib

**Current**: Custom `Array`, `Hash` implementations in `nextui.c`.

**Proposed**: Use `glib` (part of GTK+, widely used in embedded Linux):
```c
#include <glib.h>

// Instead of Array
GArray* entries = g_array_new(FALSE, FALSE, sizeof(Entry*));
g_array_append_val(entries, entry);

// Instead of Hash
GHashTable* options = g_hash_table_new(g_str_hash, g_str_equal);
g_hash_table_insert(options, "key", "value");
```

**Benefits**:
- Mature, optimized, memory-safe.
- Widely used in open-source projects (e.g., GNOME, systemd).

**Risks**:
- Adds ~500KB to binary size (but glibc is already present on target).

---

#### 2. Replace Manual WiFi/BT Management with libnm/libbluetooth

**Current**: Shell-scripting `wpa_supplicant` CLI and `bluetoothctl` CLI.

**Proposed**: Use **libnm** (NetworkManager) and **libbluetooth** (BlueZ D-Bus API):
```c
#include <NetworkManager.h>
#include <bluetooth/bluetooth.h>
```

**Benefits**:
- Stable APIs instead of brittle CLI parsing.
- Better error handling and state management.

**Risks**:
- Requires NetworkManager and BlueZ D-Bus daemons (may not be present on stock firmware).

**Alternative**: Use D-Bus directly (lightweight).

---

#### 3. Consolidate Compression Libraries

**Current**: libzip depends on zlib, libbz2, liblzma, libzstd.

**Proposed**: Configure libzip to use **only zlib** (most ROMs are plain `.zip`):
```bash
./configure --without-bzip2 --without-liblzma --without-libzstd
```

**Benefits**:
- Reduces binary size by ~300KB.
- Simplifies dependency management.

**Risks**:
- Users with `.7z`, `.tar.xz` archives need to re-compress.

---

#### 4. Migrate to SDL2 2.30+ Consistently

**Current**: Supports SDL2 2.0.x (old stock firmware) and 2.30.x (newer builds).

**Proposed**: Require SDL2 2.30+ and backport to older devices if needed.

**Benefits**:
- Access to new features (better vsync, improved joystick handling).
- Cleaner codebase without version-specific ifdefs.

**Risks**:
- Requires rebuilding stock firmware libraries (or bundling SDL2).

---

#### 5. Replace Custom Audio Batching with SDL_AudioStream (SDL2 2.0.7+)

**Current**: Manual audio batching and resampling via libsamplerate.

**Proposed**: Use SDL2's built-in `SDL_AudioStream` for resampling:
```c
SDL_AudioStream* stream = SDL_NewAudioStream(
    src_format, src_channels, src_rate,
    dst_format, dst_channels, dst_rate
);
SDL_AudioStreamPut(stream, audio_data, audio_len);
SDL_AudioStreamGet(stream, output_buffer, output_len);
```

**Benefits**:
- Simpler code (no libsamplerate dependency).
- SDL2's resampler is fast and tunable.

**Risks**:
- libsamplerate is higher quality (audiophile-grade).
- SDL2's resampler may introduce artifacts.

**Recommendation**: Benchmark both; offer as a compile-time option.

---

## Technology Modernization Opportunities

### 1. Migrate to CMake or Meson

**Current**: GNU Make with deeply nested, brittle Makefiles.

**Proposed**: Use **CMake** (industry-standard) or **Meson** (modern, fast):

**CMake Example**:
```cmake
cmake_minimum_required(VERSION 3.20)
project(NextUI LANGUAGES C)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(nextui
    workspace/all/nextui/nextui.c
    workspace/all/common/api.c
    workspace/all/common/utils.c
)

target_link_libraries(nextui SDL2::SDL2 OpenGL::GL)
```

**Meson Example**:
```meson
project('NextUI', 'c')
sdl2 = dependency('SDL2')
opengl = dependency('gl')

executable('nextui',
    'workspace/all/nextui/nextui.c',
    'workspace/all/common/api.c',
    dependencies: [sdl2, opengl]
)
```

**Benefits**:
- Better dependency tracking (faster incremental builds).
- Cross-platform support (Windows, macOS, Linux).
- Modern tooling (integration with IDEs, clangd, etc.).

**Risks**:
- Learning curve for contributors familiar with Make.

---

### 2. Add Automated Testing (GitHub Actions)

**Proposed**:
- Unit tests for `utils.c`, `config.c`, data structures.
- Integration tests for core loading, ROM extraction.
- Hardware-in-the-loop tests on actual TrimUI devices (via self-hosted runners).

**Example** (GitHub Actions workflow):
```yaml
name: Build and Test
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - run: make PLATFORM=tg5040
      - run: make test
```

**Benefits**:
- Catch regressions early.
- Validate pull requests automatically.

---

### 3. Add Continuous Deployment

**Proposed**:
- Automatically build and publish releases on Git tags.
- Upload to GitHub Releases with changelogs.

**Example**:
```yaml
on:
  push:
    tags:
      - 'v*'
jobs:
  release:
    runs-on: ubuntu-latest
    steps:
      - run: make all
      - uses: softprops/action-gh-release@v1
        with:
          files: releases/*.zip
```

---

### 4. Replace Shell Scripts with Python

**Current**: PAK launch scripts, install/update scripts in Bash.

**Proposed**: Use Python for complex logic (e.g., WiFi setup, Bluetooth pairing).

**Benefits**:
- Better error handling, logging.
- Easier to test and maintain.

**Risks**:
- Python not available on all stock firmwares (but BusyBox is).

---

### 5. Modernize C Code with C11/C17 Features

**Current**: C99 with manual memory management.

**Proposed**:
- Use `_Generic` for type-safe macros.
- Use `_Static_assert` for compile-time checks.
- Use `stdatomic.h` for lock-free concurrency.

**Example**:
```c
_Static_assert(sizeof(int) == 4, "Requires 32-bit ints");

#include <stdatomic.h>
atomic_int frame_count = ATOMIC_VAR_INIT(0);
atomic_fetch_add(&frame_count, 1);
```

---

## Summary

### Current Stack Assessment

| Aspect | Grade | Notes |
|--------|-------|-------|
| **Core Libraries** | A− | Mature, stable (SDL2, OpenGL, libsamplerate) |
| **Build System** | C | Functional but brittle Makefiles, no incremental builds |
| **Toolchains** | B+ | Docker-based, reproducible, but manual updates |
| **Dependency Management** | C+ | No versioning, scattered system libraries |
| **Testing** | F | No automated tests |
| **CI/CD** | F | No continuous integration or deployment |
| **Code Modernization** | C | Stuck on C99, manual memory management |

### Recommendations Priority Matrix

| Priority | Effort | Impact | Action |
|----------|--------|--------|--------|
| **P0** | Low | High | Add GitHub Actions for builds, basic testing |
| **P1** | Medium | High | Migrate to CMake or Meson |
| **P2** | Low | Medium | Consolidate compression libs, update SDL2 |
| **P3** | High | High | Replace custom data structures with glib |
| **P4** | High | Medium | Use libnm/libbluetooth instead of CLI tools |

**Next**: See the performance and resilience assessment for runtime analysis.
