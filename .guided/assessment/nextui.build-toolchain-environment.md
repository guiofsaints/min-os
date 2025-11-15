# NextUI Build Toolchain and Environment

This document captures the build toolchain, environment requirements, and build commands for NextUI on Windows.

---

## Overview

NextUI uses a **Docker-based cross-compilation toolchain** targeting ARM Cortex-A53 processors (TrimUI Brick/Smart Pro). The build system is orchestrated by GNU Make and requires Linux-based ARM toolchains that run inside Docker containers.

**Key Insight**: Building NextUI natively on Windows is **not supported** by the official build system. However, Windows users can:
1. Use **Docker Desktop** to run the Linux-based build
2. Use **WSL 2** (Windows Subsystem for Linux) for native Linux builds
3. Perform **analysis and documentation** without building (this guide's focus)

---

## Toolchain Architecture

### Target Platforms
- **TrimUI Brick** (`PLATFORM=tg5040`)
- **TrimUI Smart Pro** (`PLATFORM=tg5040`)
- **Desktop** (`PLATFORM=desktop`) - macOS/Linux testing only

### Build Layers

```
┌─────────────────────────────────────────────────┐
│ Host System: Windows 10/11 / macOS / Linux     │
├─────────────────────────────────────────────────┤
│ Docker Desktop / WSL 2                          │
├─────────────────────────────────────────────────┤
│ Docker Image: ghcr.io/loveretro/tg5040-toolchain│
├─────────────────────────────────────────────────┤
│ ARM Cross-Compiler: arm-linux-gnueabihf-gcc     │
│ Build Tools: make, git, bash                    │
├─────────────────────────────────────────────────┤
│ NextUI Source: /root/workspace                  │
├─────────────────────────────────────────────────┤
│ Output: ./build/SYSTEM/tg5040/bin/*.elf         │
└─────────────────────────────────────────────────┘
```

---

## Build System Components

### 1. Makefiles

#### Root Makefile: `makefile`
- **Purpose**: Top-level orchestration, packaging, release management
- **Key Targets**:
  - `all` - Full build: setup → platforms → package
  - `setup` - Initialize build directories, copy skeleton
  - `PLATFORM` - Build a specific platform (e.g., `make tg5040`)
  - `package` - Create distribution ZIPs
  - `clean` - Remove build artifacts
- **Environment Variables**:
  - `PLATFORM` - Target platform (e.g., `tg5040`, `desktop`)
  - `COMPILE_CORES` - Enable core compilation (slow)
  - `BUILD_HASH` - Git commit hash (auto-detected)
  - `RELEASE_NAME` - Release version string

#### Toolchain Makefile: `makefile.toolchain`
- **Purpose**: Docker integration, toolchain invocation
- **Key Actions**:
  - Clone platform-specific toolchain repos
  - Pull Docker images from GitHub Container Registry
  - Mount workspace into Docker container
  - Execute builds inside container

#### Workspace Makefile: `workspace/makefile`
- **Purpose**: Coordinate builds across all modules
- **Builds** (in order):
  1. Platform-specific tools (rfkill, btmanager, audiomon, wifimanager, keymon)
  2. libmsettings (platform settings library)
  3. Core apps (nextui, minarch, battery, clock, settings)
  4. Tracking libs (libbatmondb, libgametimedb)
  5. Utilities (syncsettings, nextval, ledcontrol, bootlogo)
  6. Libretro cores (if `COMPILE_CORES=true`)

#### Platform Makefile: `workspace/tg5040/makefile`
- **Purpose**: Platform-specific targets (TrimUI Brick/Smart Pro)
- **Dependencies**:
  - evtest (input device testing)
  - jstest (joystick testing)
  - unzip60 (ROM extraction)
  - NextCommander (file manager)

### 2. Docker Toolchain

#### Image: `ghcr.io/loveretro/tg5040-toolchain:latest`
- **Base**: Debian-based Linux
- **Compiler**: `arm-linux-gnueabihf-gcc` (ARM Cortex-A53 hard-float)
- **SDK**: TrimUI SDK with platform headers and libraries
- **Tools**: make, git, bash, pkg-config, autotools

#### Toolchain Repositories
- **Source**: `https://github.com/LoveRetro/tg5040-toolchain`
- **Content**:
  - Dockerfile for building toolchain image
  - Pre-built cross-compilers
  - Platform-specific libraries (SDL2, OpenGL ES, etc.)
  - Build scripts and configuration

---

## Build Commands

### Windows with Docker Desktop

#### Prerequisites
```powershell
# Install Docker Desktop
winget install Docker.DockerDesktop

# Verify installation
docker --version
docker info
```

#### Build Steps

```powershell
# Navigate to repository
cd C:\Workspace\NextUI

# Full build (no core compilation)
make all PLATFORM=tg5040

# Build with cores (slow, ~30-60 minutes)
make all PLATFORM=tg5040 COMPILE_CORES=true

# Build specific core
make build-core PLATFORM=tg5040 CORE=gambatte_libretro

# Clean build
make clean PLATFORM=tg5040
```

**What happens:**
1. `make` reads `makefile` and detects `PLATFORM=tg5040`
2. Invokes `makefile.toolchain` to set up Docker
3. Clones `tg5040-toolchain` repo if not present
4. Pulls `ghcr.io/loveretro/tg5040-toolchain:latest` image
5. Mounts `.\workspace` to `/root/workspace` in container
6. Runs `make` inside container with platform-specific toolchain
7. Builds all modules in `workspace/makefile` order
8. Copies artifacts to `./build/SYSTEM/tg5040/bin/`

#### Interactive Shell (for debugging)

```powershell
# Open shell in Docker container
make shell PLATFORM=tg5040

# Inside container:
# pwd → /root/workspace
# which arm-linux-gnueabihf-gcc → /path/to/cross-compiler
```

### Windows with WSL 2 (Alternative)

#### Prerequisites
```powershell
# Install WSL 2
wsl --install

# Install Ubuntu (or Debian)
wsl --install -d Ubuntu

# Enter WSL
wsl
```

#### Inside WSL

```bash
# Install dependencies
sudo apt update
sudo apt install -y git make docker.io

# Clone NextUI
git clone https://github.com/LoveRetro/NextUI.git
cd NextUI

# Build with Docker
make all PLATFORM=tg5040
```

**Advantage**: More Linux-native experience, better Docker performance.

---

## Environment Variables

### Build-Time Variables

| Variable | Purpose | Default | Set By |
|----------|---------|---------|--------|
| `PLATFORM` | Target platform | None (required) | User |
| `COMPILE_CORES` | Build libretro cores | `false` | User |
| `CORE` | Specific core to build | None | User (for `build-core`) |
| `BUILD_HASH` | Git commit SHA | Auto-detected | Makefile |
| `BUILD_BRANCH` | Git branch name | Auto-detected | Makefile |
| `RELEASE_TIME` | Build timestamp | Auto-detected | Makefile |
| `RELEASE_NAME` | Release version | Computed | Makefile |
| `HOST_WORKSPACE` | Host workspace path | `$(pwd)/workspace` | makefile.toolchain |
| `GUEST_WORKSPACE` | Container workspace path | `/root/workspace` | makefile.toolchain |

### Runtime Variables (Docker Container)

Set inside Docker container by toolchain:

| Variable | Purpose | Example |
|----------|---------|---------|
| `CROSS_COMPILE` | Cross-compiler prefix | `arm-linux-gnueabihf-` |
| `CC` | C compiler | `arm-linux-gnueabihf-gcc` |
| `CXX` | C++ compiler | `arm-linux-gnueabihf-g++` |
| `AR` | Archiver | `arm-linux-gnueabihf-ar` |
| `RANLIB` | Archive indexer | `arm-linux-gnueabihf-ranlib` |
| `PKG_CONFIG_PATH` | Library metadata | `/opt/trimui/lib/pkgconfig` |
| `SYSROOT` | Target root filesystem | `/opt/trimui/sysroot` |

---

## Compiler and Toolchain Details

### Cross-Compiler: arm-linux-gnueabihf-gcc

**Version**: Typically GCC 8.x - 10.x (check with `docker run ghcr.io/loveretro/tg5040-toolchain:latest arm-linux-gnueabihf-gcc --version`)

**Target Architecture**:
- **CPU**: ARM Cortex-A53 (ARMv8-A, 32-bit mode)
- **FPU**: VFPv4 with NEON (hard-float ABI)
- **Endianness**: Little-endian
- **Thumb**: Thumb-2 instruction set

**Common Flags** (from toolchain):
```bash
CFLAGS = -march=armv8-a -mtune=cortex-a53 -mfpu=neon-vfpv4 -mfloat-abi=hard
LDFLAGS = -L/opt/trimui/lib -Wl,-rpath-link=/opt/trimui/lib
```

### Key Dependencies (in Docker Image)

| Library | Version | Purpose |
|---------|---------|---------|
| SDL2 | 2.0.x | Graphics, input, audio |
| OpenGL ES | 2.0/3.0 | GPU rendering |
| libsamplerate | 0.2.x | Audio resampling |
| libzip | 1.8.x | ROM extraction |
| zlib | 1.2.x | Compression |
| freetype | 2.x | Font rendering |
| libpng | 1.6.x | Image handling |

---

## Build Verification (Without Full Build)

For analysis purposes, you can verify the build system without compiling:

### Check Docker Setup

```powershell
# Pull toolchain image
docker pull ghcr.io/loveretro/tg5040-toolchain:latest

# Inspect image
docker inspect ghcr.io/loveretro/tg5040-toolchain:latest

# Check compiler version
docker run --rm ghcr.io/loveretro/tg5040-toolchain:latest arm-linux-gnueabihf-gcc --version

# Check installed packages
docker run --rm ghcr.io/loveretro/tg5040-toolchain:latest dpkg -l
```

### Dry-Run Build

```powershell
# See what make would do (dry-run)
make -n all PLATFORM=tg5040

# Check toolchain setup
make -f makefile.toolchain PLATFORM=tg5040
```

### Extract Build Logs

```powershell
# Run build with verbose output
make all PLATFORM=tg5040 > .guided/assessment/reports/build-logs/build.log 2>&1

# Extract compiler commands
Select-String "arm-linux-gnueabihf-gcc" .guided/assessment/reports/build-logs/build.log > .guided/assessment/reports/build-logs/compiler-commands.txt
```

---

## PATH Requirements

### On Windows (Docker Desktop)

Docker Desktop handles PATH automatically. Ensure:
- Docker Desktop is running
- Docker CLI is in PATH (verify with `docker --version`)

### On WSL 2

```bash
# Inside WSL, ensure Docker is in PATH
export PATH="/usr/bin:$PATH"

# Or add to ~/.bashrc
echo 'export PATH="/usr/bin:$PATH"' >> ~/.bashrc
```

---

## Build Output Structure

After a successful build:

```
build/
├── BASE/
│   ├── MinUI.zip                # Installable package
│   ├── Bios/                    # BIOS files (empty templates)
│   ├── Roms/                    # ROM directories
│   ├── Saves/                   # Save directories
│   └── trimui/                  # TrimUI-specific boot files
├── EXTRAS/
│   ├── Emus/tg5040/             # Extra emulator cores
│   ├── Tools/tg5040/            # PAK tools
│   └── README.txt
├── PAYLOAD/
│   ├── .system/                 # System binaries
│   │   ├── tg5040/bin/          # ELF executables
│   │   │   ├── nextui.elf       # Main UI (3221 lines compiled)
│   │   │   ├── minarch.elf      # Emulator engine (7186 lines compiled)
│   │   │   ├── keymon.elf       # Input monitor
│   │   │   ├── batmon.elf       # Battery monitor
│   │   │   └── ...
│   │   └── tg5040/lib/          # Shared libraries
│   │       ├── libmsettings.so
│   │       ├── libsamplerate.so
│   │       └── ...
│   └── .tmp_update/             # Updater
└── SYSTEM/
    └── version.txt              # Build version and hash
```

---

## Differences: Windows vs. Linux vs. macOS

| Aspect | Windows (Docker) | Windows (WSL) | macOS | Linux |
|--------|------------------|---------------|-------|-------|
| **Docker** | Docker Desktop required | Native Docker in WSL | Docker Desktop recommended | Native Docker |
| **Filesystem** | NTFS → ext4 (mounted) | Native ext4 | APFS → ext4 (mounted) | Native ext4 |
| **Performance** | Slower (virtualization) | Fast (native kernel) | Slower (virtualization) | Fastest |
| **Build Speed** | ~15-20 min | ~10-12 min | ~12-15 min | ~8-10 min |
| **Line Endings** | CRLF → LF (Git handles) | Native LF | Native LF | Native LF |
| **Scripting** | PowerShell + bash in Docker | Native bash | Native bash | Native bash |

---

## Troubleshooting

### Docker Not Found

```powershell
# Check Docker Desktop is running
Get-Process Docker

# Restart Docker Desktop
Stop-Service -Name docker
Start-Service -Name docker
```

### Permission Denied in Docker

```bash
# Inside WSL, add user to docker group
sudo usermod -aG docker $USER
newgrp docker
```

### Slow Docker on Windows

- Use WSL 2 backend (Docker Desktop settings)
- Allocate more CPU/RAM to Docker (Settings → Resources)
- Store repository on WSL filesystem for better performance

### Build Fails with "Image not found"

```powershell
# Manually pull image
docker pull ghcr.io/loveretro/tg5040-toolchain:latest

# Verify image exists
docker images | Select-String "tg5040"
```

---

## Toolchain Versions (as of November 2025)

Based on available information:

| Component | Version/Details |
|-----------|-----------------|
| **Docker Image** | ghcr.io/loveretro/tg5040-toolchain:latest |
| **Compiler** | arm-linux-gnueabihf-gcc 8.x - 10.x |
| **Make** | GNU Make 4.x |
| **SDL2** | 2.0.x (cross-compiled for ARM) |
| **OpenGL ES** | 2.0/3.0 (Mali-G31 GPU) |
| **Git** | 2.x |
| **Target OS** | Linux (custom TrimUI kernel) |

**Note**: For exact versions, run:
```powershell
docker run --rm ghcr.io/loveretro/tg5040-toolchain:latest /bin/bash -c "arm-linux-gnueabihf-gcc --version && make --version && git --version"
```

---

## Recommended Setup for Windows Users

### For Build + Analysis:
1. Install **Docker Desktop**
2. Clone NextUI to `C:\Workspace\NextUI`
3. Use PowerShell for builds: `make all PLATFORM=tg5040`
4. Use analysis scripts for static analysis (no compilation needed)

### For Analysis Only (No Build):
1. Skip Docker installation
2. Install analysis tools (scc, ripgrep, cppcheck, etc.)
3. Run `.\tools\windows\run-analysis.ps1`
4. Review generated reports

### For Performance:
1. Use **WSL 2** instead of Docker Desktop
2. Clone NextUI inside WSL filesystem (`~/NextUI`)
3. Build from WSL: `make all PLATFORM=tg5040`
4. Access build output from Windows: `\\wsl$\Ubuntu\home\user\NextUI\build`

---

## Next Steps

1. **Document exact toolchain versions** by running Docker commands above
2. **Extract compilation database** for better clang-tidy results:
   ```powershell
   # Inside Docker container
   make PLATFORM=tg5040
   bear -- make clean PLATFORM=tg5040
   bear -- make PLATFORM=tg5040
   # Generates compile_commands.json
   ```
3. **Profile build performance** with timing metrics
4. **Test builds** on different Windows versions (10 vs 11, Home vs Pro)

---

## References

- [NextUI GitHub](https://github.com/LoveRetro/NextUI)
- [tg5040-toolchain](https://github.com/LoveRetro/tg5040-toolchain)
- [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop)
- [WSL 2 Documentation](https://docs.microsoft.com/en-us/windows/wsl/)
- [ARM GCC Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain)
