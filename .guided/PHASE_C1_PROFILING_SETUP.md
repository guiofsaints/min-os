# Phase C.1: Profiling Infrastructure - Setup Guide

**Date**: November 15, 2025  
**Phase**: C.1.1 - Desktop Profiling Setup  
**Status**: ✅ **COMPLETE**

---

## Overview

This document describes the profiling infrastructure added to min-os for performance optimization work in Phase C.

---

## Profiling Macros

### Location
- **Header**: `workspace/all/common/profile.h`
- **Build Support**: `workspace/tg5040/platform/makefile.env`, `workspace/desktop/platform/makefile.env`

### Usage

#### Basic Timing
```c
#include "profile.h"

void some_function() {
    PROFILE_START(operation_name);
    // ... code to profile ...
    PROFILE_END(operation_name);
}
```

Output:
```
PROFILE [operation_name]: 1.234 ms
```

#### Block Profiling
```c
PROFILE_BLOCK(my_block, {
    // code to profile
    some_work();
    more_work();
});
```

#### Function Call Profiling
```c
PROFILE_CALL(expensive_function, expensive_function(arg1, arg2));
```

#### Conditional Profiling
```c
PROFILE_IF(debug_mode, conditional_op);
// ... code ...
PROFILE_END_IF(debug_mode, conditional_op);
```

#### Zone Profiling (Frame Tracking)
```c
// At file scope
PROFILE_ZONE_DECLARE(render_frame);

void game_loop() {
    while (running) {
        PROFILE_ZONE_START(render_frame);
        render();
        PROFILE_ZONE_END(render_frame);
    }
    
    // Print average over all frames
    PROFILE_ZONE_REPORT(render_frame);
}
```

---

## Building with Profiling

### Desktop (macOS/Linux)
```bash
cd /path/to/min-os
make desktop PROFILE=1
```

### TrimUI Brick (cross-compilation)
```bash
make PLATFORM=tg5040 PROFILE=1
```

### Verification
```bash
# Check that PROFILE_ENABLED is defined
grep -r "PROFILE_ENABLED" workspace/*/platform/makefile.env
```

---

## Profiling Workflow

### 1. Add Instrumentation

Identify hot paths to profile:
- Rendering: `GFX_blitScreen()`, `SDL_Flip()`, shader processing
- Audio: `audio_callback()`, libsamplerate resampling
- Frame loop: `retro_run()`, `video_refresh()`
- Memory: allocation-heavy functions

Add profiling macros:
```c
// In workspace/all/minarch/minarch.c
#include "profile.h"

static void video_refresh_callback(...) {
    PROFILE_START(video_refresh);
    
    // existing code
    
    PROFILE_END(video_refresh);
}
```

### 2. Build and Run

```bash
# Desktop testing
make desktop PROFILE=1
./build/desktop/bin/minarch.elf /path/to/rom.gb

# Watch profiling output
# PROFILE [video_refresh]: 0.123 ms
# PROFILE [audio_callback]: 0.045 ms
# ...
```

### 3. Collect Data

Redirect output to file for analysis:
```bash
./build/desktop/bin/minarch.elf rom.gb 2>&1 | tee profile.log

# Run for 10 minutes, then analyze
grep "PROFILE" profile.log | awk '{print $2, $3}' | sort | uniq -c
```

### 4. Analyze Results

Look for:
- **High frequency** operations (called many times per frame)
- **Long duration** operations (>5ms per call)
- **Variability** (inconsistent timings indicate issues)

Example analysis:
```bash
# Count occurrences of each profiled operation
grep "PROFILE" profile.log | awk '{print $2}' | sort | uniq -c | sort -rn

# Average time per operation
grep "PROFILE" profile.log | awk '{sum[$2]+=$3; count[$2]++} END {for(op in sum) print op, sum[op]/count[op] "ms"}'
```

---

## macOS-Specific: Instruments

For deeper profiling on macOS:

### 1. Build with Debug Symbols
```bash
make desktop PROFILE=1 DEBUG=1
```

### 2. Launch Instruments
```bash
# Open Instruments app
open /Applications/Xcode.app/Contents/Applications/Instruments.app

# Or command line
instruments -t "Time Profiler" -D profile.trace ./build/desktop/bin/minarch.elf rom.gb
```

### 3. Profilers to Use

- **Time Profiler**: CPU hot paths, call tree
- **Allocations**: Memory allocation tracking
- **System Trace**: System-level performance
- **Counters**: Custom performance counters

### 4. Analyze in Instruments

1. Run game for 2-3 minutes
2. Stop recording
3. Look at Call Tree (heaviest stack first)
4. Identify functions consuming >5% CPU time
5. Drill down into hot functions

---

## TrimUI Brick On-Device Profiling

### Challenges
- No native profiler support
- Limited debugging tools
- Need custom logging

### Approach

#### 1. Add File Logging
```c
#ifdef PROFILE_ENABLED
#include <stdio.h>
static FILE* profile_log = NULL;

void profile_init() {
    profile_log = fopen("/mnt/SDCARD/.userdata/profile.log", "w");
}

void profile_close() {
    if (profile_log) fclose(profile_log);
}

#define PROFILE_LOG(name, ms) \
    if (profile_log) fprintf(profile_log, "PROFILE [%s]: %.3f ms\n", name, ms)
#endif
```

#### 2. Modify Macros for File Output
```c
#define PROFILE_END(name) do { \
    uint64_t _profile_end = SDL_GetPerformanceCounter(); \
    uint64_t _profile_elapsed = _profile_end - _profile_start_##name; \
    double _profile_ms = (_profile_elapsed * 1000.0) / SDL_GetPerformanceFrequency(); \
    PROFILE_LOG(#name, _profile_ms); \
} while(0)
```

#### 3. Run on Device
```bash
# Build for TrimUI Brick
make PLATFORM=tg5040 PROFILE=1

# Copy to SD card
cp -r build/SYSTEM /mnt/SDCARD/.system

# Run game on device for 10 minutes
# Profile data written to /mnt/SDCARD/.userdata/profile.log
```

#### 4. Analyze Logs
```bash
# Copy log from SD card
cp /mnt/SDCARD/.userdata/profile.log ./

# Analyze
grep "PROFILE" profile.log | awk '{sum[$2]+=$3; count[$2]++} END {for(op in sum) print op, sum[op]/count[op] "ms", count[op] "calls"}'
```

---

## Next Steps

1. **Baseline Measurements** (C.1.3):
   - Profile 5 cores (GB, GBA, SNES, MD, PS)
   - Collect FPS, frame time, hot functions
   - Generate baseline report

2. **Hot Path Identification**:
   - Identify top 10 hot functions
   - Prioritize by time × frequency
   - Focus optimization efforts

3. **Optimization**:
   - Optimize top hot paths
   - Benchmark before/after
   - Document improvements

---

## Tips

### Minimize Profiling Overhead
- Use zone profiling for frame-level stats
- Avoid profiling in tight loops
- Aggregate data, don't print every call

### Profile Representatively
- Use typical gameplay scenarios
- Profile for 5-10 minutes minimum
- Test multiple cores/games

### Focus on Impact
- Optimize functions with highest `time × frequency`
- Low-frequency operations (<1/frame) rarely matter
- Target operations >1ms or called >100/frame

---

## Profiling Checklist

- [ ] profile.h header created
- [ ] Makefile support added (PROFILE=1)
- [ ] Desktop profiling tested
- [ ] Example profiling added to minarch.c
- [ ] Logs parsing scripts created
- [ ] On-device profiling planned
- [ ] Ready for baseline measurements

---

**Status**: ✅ Profiling infrastructure complete  
**Next**: C.1.3 Baseline Measurements  
**Date**: November 15, 2025
