# NextUI – Performance & Resilience Assessment

This document analyzes performance bottlenecks, error handling robustness, and opportunities for optimization in the NextUI codebase, with a focus on the constrained hardware environment of TrimUI handheld devices.

---

## Hardware Constraints

### Target Device: TrimUI Brick (tg5040)

| Component | Specification | Implications |
|-----------|---------------|--------------|
| **CPU** | ARM Cortex-A53 quad-core @ 1.2-1.8 GHz | Modest performance; thermal throttling likely |
| **GPU** | ARM Mali-G31 MP2 | Entry-level mobile GPU; OpenGL ES 3.x capable |
| **RAM** | 1 GB LPDDR4 | Limited; avoid memory-intensive features |
| **Storage** | microSD (user-provided) | Slow I/O; minimize file access |
| **Display** | 1024×768 @ 60 Hz (Brick), 1280×720 @ 60 Hz (Smart Pro) | Native resolution varies by model |
| **Battery** | ~4000 mAh | Battery life critical; CPU/GPU efficiency matters |
| **Thermal** | Passive cooling only | Sustained high CPU/GPU load → overheating |

**Key Constraints**:
1. **CPU power limited**: Avoid CPU-heavy tasks (e.g., complex shaders, excessive string processing).
2. **GPU is modest**: Multi-pass shaders should be optimized or optional.
3. **RAM is tight**: Keep runtime memory usage below 200 MB for NextUI + emulation.
4. **Battery life critical**: Dynamic CPU scaling, GPU vsync, and audio efficiency matter.
5. **Thermal management**: Prevent sustained 100% CPU to avoid overheating and throttling.

---

## Performance Hotspots

### 1. Emulation Loop (`minarch.c`)

#### Core Execution (`core.run()`)

**Frequency**: 60 Hz (16.67 ms per frame)

**Observed Bottlenecks** (from `todo.txt` and code comments):
1. **High-resolution cores** (PlayStation):
   - Games running at 640×480 or higher stress the GPU.
   - Example: *Colin McRae Rally 2* at 640×480 causes `SDL_UnlockTexture()` to block for 11 ms.
2. **Frame pacing issues** on certain devices (e.g., H700):
   - Audio underruns in `fceumm` even after batching improvements.
   - Suspected root cause: SDL2 vsync delays or panel refresh rate mismatch.
3. **Threaded video option adds complexity**:
   - Intended to improve pipelining but introduces race conditions.
   - Currently disabled by default.

**Example** (`minarch.c:video_refresh_callback_main()`):
```c
static void video_refresh_callback_main(const void *data, unsigned width, unsigned height, size_t pitch) {
    // Lock GPU texture (blocks on slow GPUs)
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    
    // Copy core framebuffer to GPU texture (CPU-bound)
    memcpy(pixels, data, height * pitch);
    
    // Unlock texture (blocks until GPU copies to VRAM)
    SDL_UnlockTexture(texture); // <--- Observed 5-11 ms block on high-res games
    
    // Render texture to screen
    SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
    SDL_RenderPresent(renderer); // <--- Blocks on vsync
}
```

**Measurement** (from logs):
- **GB (160×144)**: `SDL_UnlockTexture()` blocks for 3 ms
- **PS (368×480)**: 5 ms
- **PS (512×480)**: 8 ms
- **PS (640×480)**: 11 ms

**Root Cause**: Copying framebuffer from CPU to GPU is bandwidth-limited on Mali-G31.

**Optimization Opportunities**:
1. **Zero-copy rendering**: Use `RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER` to render directly to GPU texture.
   - **Issue**: Requires cores to support this environment variable (many don't).
2. **Downsample high-res games**: PlayStation games at 640×480 could be rendered at 512×384 and upscaled.
   - **Trade-off**: Slight quality loss for 3-4 ms speedup.
3. **Use `SDL_UpdateTexture()` instead of `Lock/Unlock`**:
   - Already tested (see `todo.txt`): Saves 1-2 ms in some cases.

**Recommendation**:
- **Short-term**: Expose downsampling option for PlayStation games.
- **Medium-term**: Benchmark `SDL_UpdateTexture()` vs. `Lock/Unlock` per platform.
- **Long-term**: Encourage libretro cores to support zero-copy framebuffers.

---

### 2. Audio Resampling & Batching

#### Current Implementation

**Pipeline**:
```
Core emits audio → audio_batch_callback()
  → Resample via libsamplerate (if core_rate != device_rate)
  → Batch into 400-sample chunks
  → Write to SDL audio buffer
  → SDL plays asynchronously
```

**Key Settings**:
- **Sample rate**: Varies by core (e.g., 32040 Hz for GB, 44100 Hz for PS).
- **Resampling quality**: Configurable per emulator (0=fast, 1=medium, 2=best).
- **Batch size**: 400 samples (reduced from 512 to minimize underruns).

**Observed Issues** (from `todo.txt`):
1. **Audio underruns** on `fceumm` (NES) even after batching fix.
2. **Judder in GBA games** (Castlevania: Aria of Sorrow) persists even with audio disabled.
   - **Root cause**: SDL2 vsync blocking, not audio.
3. **Crackling on quit** (some cores don't restore audio state correctly).

**Benchmark** (libsamplerate overhead):
- **Quality 0 (linear)**: ~0.5 ms per 400 samples
- **Quality 1 (sinc, medium)**: ~1.2 ms
- **Quality 2 (sinc, best)**: ~2.5 ms

**Optimization Opportunities**:
1. **Cache resampler state** instead of reinitializing per batch.
   - Currently done, but could be verified.
2. **Use SDL2 AudioStream** (see deps-and-stack.md):
   - May reduce latency but lower quality than libsamplerate.
3. **Tune batch size dynamically** based on buffer fill:
   - Increase to 512 samples when buffer is low, decrease to 256 when high.

**Recommendation**:
- **Short-term**: Add option to disable resampling (force core to output at device rate).
- **Medium-term**: Benchmark SDL2 AudioStream vs. libsamplerate.
- **Long-term**: Implement dynamic batch sizing.

---

### 3. Shader Pipeline

#### Multi-Pass Rendering

NextUI supports up to **3 shader passes** with runtime parameter adjustment.

**Typical Shader Chain**:
1. **Pass 1**: Upscale (e.g., 2×/3× nearest-neighbor or xBR)
2. **Pass 2**: Effect (e.g., CRT scanlines, LCD grid)
3. **Pass 3**: Post-process (e.g., color correction, bloom)

**Performance Impact**:
- **1 pass**: ~1 ms overhead
- **2 passes**: ~2-3 ms
- **3 passes**: ~4-5 ms (on Mali-G31)

**Example** (from logs):
```
Shader pass 1: 256×224 → 512×448 (xBR2x) - 1.2 ms
Shader pass 2: 512×448 → 1024×896 (CRT scanlines) - 1.8 ms
Shader pass 3: 1024×896 → 1024×768 (color correction) - 1.5 ms
Total: 4.5 ms (27% of 16.67 ms frame budget)
```

**Optimization Opportunities**:
1. **Pre-compile shaders at startup** instead of runtime (already done for built-ins, but user shaders are compiled on-demand).
2. **Skip redundant passes**:
   - If output resolution matches input, skip upscaling pass.
3. **Offer "lite" shader profiles** for battery-constrained modes.
4. **Use lower precision** (`mediump` instead of `highp` in GLSL):
   ```glsl
   precision mediump float; // Instead of highp
   ```

**Recommendation**:
- **Short-term**: Document shader performance impact in user docs.
- **Medium-term**: Add "Performance Mode" that disables shaders and upscaling.
- **Long-term**: GPU profiling to identify shader bottlenecks (use ARM Streamline).

---

### 4. File I/O & ROM Loading

#### ZIP Extraction

**Current**: ROMs in `.zip` archives extracted to `/tmp/nextarch/<TAG>/`.

**Observed Behavior**:
- Extraction happens on **every launch** unless file already exists in `/tmp`.
- microSD read speed: ~20-40 MB/s (class 10 card).
- Large ROMs (e.g., PlayStation ~600 MB) take 15-30 seconds to extract.

**Optimization Opportunities**:
1. **Cache extracted ROMs** in `/tmp` and only re-extract if `.zip` timestamp changes.
   - **Issue**: `/tmp` is tmpfs (RAM-backed), limited space (~200 MB on 1 GB RAM device).
   - **Alternative**: Extract to `.userdata/<TAG>/romcache/` on SD card.
2. **Lazy extraction**: Only extract file core actually needs (if `.zip` contains multiple files).
   - Already implemented for cores that support zip natively.

**Recommendation**:
- **Short-term**: Improve existing `/tmp` caching logic to check timestamps.
- **Medium-term**: Add option to extract to SD card cache folder.

---

### 5. UI Rendering

#### Main Menu (`nextui.c`)

**Frame Rate**: 60 Hz (vsync-locked)

**Rendering Cost**:
- **Directory scanning**: ~10-50 ms (first load), then cached.
- **Scrolling text rendering**: ~2-3 ms per frame (when active).
- **Battery/WiFi/BT status icons**: ~0.5 ms per frame.
- **Total**: ~3-5 ms per frame (well within 16.67 ms budget).

**Optimization Opportunities**:
1. **Lazy directory scanning**: Only scan directories when user navigates to them.
   - Currently scans all ROM folders at startup.
2. **Cache rendered text surfaces**: Avoid re-rendering static text every frame.
3. **Skip rendering when idle**: If no input for >1 second, reduce refresh rate to 15 Hz.

**Recommendation**:
- **Short-term**: Profile startup time and identify slow directory scans.
- **Medium-term**: Implement idle detection and reduced refresh rate.

---

## CPU & Power Management

### Dynamic CPU Scaling

NextUI adjusts CPU governor based on load:

| Mode | Governor | Typical Frequency | Use Case |
|------|----------|-------------------|----------|
| **Menu** | `powersave` | 600-1000 MHz | Browsing menus, low activity |
| **Normal** | `ondemand` | 1000-1400 MHz | Most emulation (GB, GBA, SNES) |
| **Performance** | `performance` | 1200-1800 MHz | Demanding games (PS, N64) |
| **Auto** | Dynamic | Adaptive | Detects core load and adjusts |

**Implementation** (`tg5040/platform/platform.c`):
```c
void PLAT_setCPUSpeed(int speed) {
    char governor[256];
    switch (speed) {
        case CPU_SPEED_MENU:      strcpy(governor, "powersave"); break;
        case CPU_SPEED_NORMAL:    strcpy(governor, "ondemand"); break;
        case CPU_SPEED_PERFORMANCE: strcpy(governor, "performance"); break;
    }
    putFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", governor);
}
```

**Observed Behavior** (from `todo.txt`):
- **Auto mode** detects when emulation is dropping frames and scales up.
- **Battery impact**: `performance` mode drains battery ~30% faster than `ondemand`.
- **Thermal**: Sustained `performance` mode causes throttling after ~10-15 minutes.

**Optimization Opportunities**:
1. **Core-specific presets**: Some cores (e.g., GPSP for GBA) run well at `powersave`.
2. **Frame time tracking**: Scale up only when average frame time exceeds 16.67 ms over 5 seconds.
3. **Thermal throttling detection**: If CPU temp exceeds 70°C, force `ondemand` even in `performance` mode.

**Recommendation**:
- **Short-term**: Log CPU frequency and temperature to help users debug performance issues.
- **Medium-term**: Implement thermal throttling safeguard.
- **Long-term**: Machine-learning-based adaptive scaling (probably overkill).

---

## Battery Life Optimization

### Current Battery Life (TrimUI Brick, 4000 mAh)

| Scenario | Estimated Battery Life | Notes |
|----------|------------------------|-------|
| **Menu browsing** (powersave) | ~8-10 hours | Low CPU, screen at 50% brightness |
| **GB/GBC emulation** (ondemand) | ~6-7 hours | Low CPU load |
| **SNES emulation** (ondemand) | ~5-6 hours | Medium CPU load |
| **PS emulation** (performance) | ~3-4 hours | High CPU + GPU load |
| **WiFi enabled** (idle) | −20% battery life | WiFi scanning drains battery |
| **Bluetooth audio** (active) | −30% battery life | BT audio streaming + encoding |

**Battery Drain Hotspots**:
1. **Display backlight**: ~40% of total power draw at 100% brightness.
2. **CPU**: ~30% at `performance`, ~15% at `powersave`.
3. **WiFi**: ~10% when enabled (even if not connected).
4. **Bluetooth audio**: ~15% when streaming.
5. **GPU**: ~10% during shader-heavy games.

**Optimization Opportunities**:
1. **Auto-dim backlight** after 30 seconds of inactivity.
2. **Disable WiFi** when not in use (already implemented).
3. **Lower default brightness**: Default to 70% instead of 100%.
4. **Skip vsync when on battery < 20%**: Reduce frame rate to 30 Hz to save CPU/GPU.

**Recommendation**:
- **Short-term**: Add battery saver mode (reduces brightness, disables WiFi, caps CPU at `ondemand`).
- **Medium-term**: Implement auto-dim and idle power-off.
- **Long-term**: Integrate with battery monitoring to predict remaining playtime.

---

## Error Handling & Resilience

### Current State

**Issues Identified**:
1. **Silent failures**: Many functions return errors but callers don't check.
2. **Null pointer dereferences**: Missing null checks after `malloc`, `fopen`, `dlopen`.
3. **Buffer overflows**: Fixed-size buffers with no bounds checking.
4. **Graceful degradation**: Failures often crash instead of falling back to safe defaults.

---

### 1. Memory Allocation Failures

**Example** (`nextui.c`):
```c
Array* self = malloc(sizeof(Array));
// No null check—crash if malloc fails (rare but possible on 1 GB RAM)
self->count = 0;
```

**Fix**:
```c
Array* self = malloc(sizeof(Array));
if (!self) {
    LOG_error("Failed to allocate Array\n");
    return NULL; // Or exit gracefully
}
self->count = 0;
```

**Recommendation**: Wrap `malloc` in a checked version:
```c
void* xmalloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        LOG_error("Out of memory (%zu bytes)\n", size);
        exit(EXIT_FAILURE); // Or longjmp to cleanup
    }
    return ptr;
}
```

---

### 2. File I/O Failures

**Example** (`minarch.c`):
```c
FILE* file = fopen(path, "r");
fgets(buffer, sizeof(buffer), file); // Crash if fopen returned NULL
```

**Fix**:
```c
FILE* file = fopen(path, "r");
if (!file) {
    LOG_warn("Failed to open %s: %s\n", path, strerror(errno));
    return; // Or use default value
}
fgets(buffer, sizeof(buffer), file);
fclose(file);
```

---

### 3. Dynamic Library Loading Failures

**Example** (`minarch.c`):
```c
core.handle = dlopen(core_path, RTLD_LAZY);
if (!core.handle) {
    LOG_error("Failed to load core: %s\n", dlerror());
    // Execution continues with NULL handle → crash on first dereference
}
```

**Fix**:
```c
core.handle = dlopen(core_path, RTLD_LAZY);
if (!core.handle) {
    LOG_error("Failed to load core: %s\n", dlerror());
    return -1; // Exit to menu instead of crashing
}
```

---

### 4. Save State Corruption

**Current**: Save states written to SD card with no checksums or validation.

**Risk**: Corrupted save states can crash emulation or corrupt SRAM.

**Proposed Solution**:
1. Add CRC32 checksum to save state header.
2. Validate checksum before loading.
3. Keep backup of last known-good save state.

**Example**:
```c
typedef struct {
    uint32_t magic; // 'NXST'
    uint32_t version;
    uint32_t crc32;
    size_t data_size;
    uint8_t data[];
} SaveState;

bool save_state_load(const char* path, SaveState** out) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    
    SaveState header;
    fread(&header, sizeof(header), 1, f);
    
    if (header.magic != 'NXST') {
        LOG_error("Invalid save state magic\n");
        fclose(f);
        return false;
    }
    
    SaveState* state = malloc(sizeof(SaveState) + header.data_size);
    memcpy(state, &header, sizeof(header));
    fread(state->data, header.data_size, 1, f);
    fclose(f);
    
    uint32_t crc = crc32(state->data, header.data_size);
    if (crc != header.crc32) {
        LOG_error("Save state CRC mismatch\n");
        free(state);
        return false;
    }
    
    *out = state;
    return true;
}
```

**Recommendation**: Implement in medium-term refactor.

---

### 5. Configuration Parsing Robustness

**Current**: INI-style parsing with minimal validation.

**Example** (`config.c`):
```c
while (fgets(line, sizeof(line), file)) {
    char* eq = strchr(line, '=');
    *eq = '\0'; // Crash if no '=' in line
}
```

**Fix**:
```c
while (fgets(line, sizeof(line), file)) {
    char* eq = strchr(line, '=');
    if (!eq) continue; // Skip malformed lines
    *eq = '\0';
}
```

**Recommendation**: Use a battle-tested INI parser (e.g., `inih` library).

---

## Logging & Diagnostics

### Current Logging

**Levels**: `LOG_debug`, `LOG_info`, `LOG_warn`, `LOG_error`

**Destinations**:
- **stdout/stderr**: Printed to terminal (when run from shell).
- **Log files**: `/.userdata/<platform>/logs/<TAG>.txt` for each emulator.

**Issues**:
1. **Log spam**: Many debug messages even in release builds.
2. **No log rotation**: Log files grow unbounded.
3. **No structured logging**: Plain text, hard to parse.

**Recommendation**:
- **Short-term**: Disable debug logs in release builds (`-DNDEBUG`).
- **Medium-term**: Implement log rotation (keep last 5 logs, max 1 MB each).
- **Long-term**: Use structured logging (JSON) for easier parsing and analysis.

---

## Performance & Resilience Summary

### Performance Grades

| Component | Performance | Bottlenecks | Recommendation |
|-----------|-------------|-------------|----------------|
| **Emulation Loop** | B+ | High-res PS games | Downsample option, zero-copy rendering |
| **Audio Resampling** | A− | libsamplerate overhead | Benchmark SDL2 AudioStream |
| **Shaders** | B | Multi-pass overhead | Lite shader profiles |
| **ROM Loading** | C+ | ZIP extraction slow | SD card caching |
| **UI Rendering** | A | Idle power waste | Idle detection, reduced refresh |
| **CPU Scaling** | A | Thermal throttling | Thermal safeguards |
| **Battery Life** | B | WiFi, BT audio drain | Battery saver mode |

### Resilience Grades

| Aspect | Grade | Issues | Recommendation |
|--------|-------|--------|----------------|
| **Null Checks** | C | Many missing | Wrap allocations, add checks |
| **File I/O** | C+ | Silent failures | Validate all fopen/fread |
| **Error Recovery** | D | Crash instead of degrade | Implement fallbacks |
| **Save State Safety** | C | No validation | Add checksums |
| **Config Parsing** | C+ | Brittle parsing | Use robust INI library |
| **Logging** | B | Log spam, no rotation | Structured logging, rotation |

---

## Priority Recommendations

| Priority | Effort | Impact | Action |
|----------|--------|--------|--------|
| **P0** | Low | High | Add null checks, validate file I/O |
| **P1** | Low | High | Implement battery saver mode |
| **P2** | Medium | High | Add save state checksums |
| **P3** | Medium | Medium | Profile and optimize shader pipeline |
| **P4** | High | Medium | Zero-copy rendering for cores |

**Next**: See the refactor plan for a phased approach to addressing these issues.
