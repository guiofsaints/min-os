# Phase C: Performance Optimization - Execution Plan

**Date**: November 15, 2025  
**Phase**: Phase C - Performance Optimization  
**Status**: 🚀 **READY TO START**  
**Duration**: 1-2 months  
**Prerequisites**: Phase A ✅ Complete, Phase B ✅ Complete

---

## Overview

Phase C focuses on **performance optimization** for TrimUI Brick. With the codebase simplified (Phase A) and hardened (Phase B), we can now profile and optimize critical paths to achieve maximum performance on the target hardware.

---

## Objectives

### Primary Goals

1. **Reduce frame latency** to <16ms (60 FPS target)
2. **Optimize audio pipeline** to maintain <20ms latency
3. **Improve rendering performance** by 20%+ 
4. **Reduce memory allocations** in hot paths
5. **Optimize CPU usage** for better battery life

### Success Metrics

| Metric | Current (Baseline) | Target | Measurement |
|--------|-------------------|--------|-------------|
| **Average FPS** | TBD | 60 FPS stable | Real hardware |
| **Frame time** | TBD | <16.67ms | Profiler |
| **Audio latency** | ~20ms | <20ms | Measurement tool |
| **CPU usage** | TBD | -15% | System monitor |
| **Memory allocs/frame** | TBD | -50% | Profiler |

---

## Phase C Sub-Phases

### C.1: Profiling Infrastructure (Week 1)

**Objective**: Set up profiling tools and establish performance baselines.

#### C.1.1: Desktop Profiling Setup

**Tools for macOS**:
- Instruments (Time Profiler, Allocations)
- `perf` (if available via Homebrew)
- Custom timing macros in code

**Tasks**:
- [ ] Add `PROFILE=1` build flag to makefile
- [ ] Add timing instrumentation macros (`PROFILE_START`, `PROFILE_END`)
- [ ] Create profiling scripts for common scenarios
- [ ] Document profiling workflow

**Example Macro**:
```c
#ifdef PROFILE
#define PROFILE_START(name) \
    uint64_t _profile_start_##name = SDL_GetPerformanceCounter()
#define PROFILE_END(name) do { \
    uint64_t _profile_end = SDL_GetPerformanceCounter(); \
    double ms = (_profile_end - _profile_start_##name) * 1000.0 / SDL_GetPerformanceFrequency(); \
    printf("PROFILE [%s]: %.3f ms\n", #name, ms); \
} while(0)
#else
#define PROFILE_START(name)
#define PROFILE_END(name)
#endif
```

**Verification**:
- ✅ Desktop build with profiling enabled
- ✅ Can measure frame time, render time, audio time
- ✅ Profiling has <1% overhead

**Effort**: 2 days

---

#### C.1.2: TrimUI Brick Profiling Setup

**Challenges**:
- Limited debugging tools on device
- No native profiler support
- Need custom logging/timing

**Approach**:
- Add performance counters to critical sections
- Log timing data to file on SD card
- Analyze logs on desktop

**Tasks**:
- [ ] Add on-device timing instrumentation
- [ ] Create log parser script
- [ ] Verify timing accuracy on real hardware
- [ ] Document on-device profiling workflow

**Verification**:
- ✅ Can measure frame time on real hardware
- ✅ Logs are readable and accurate
- ✅ Minimal performance impact from logging

**Effort**: 3 days

---

#### C.1.3: Baseline Measurements

**Objective**: Establish "before optimization" baselines for comparison.

**Measurements Needed**:
1. **Rendering**:
   - Average FPS across different cores (GB, GBA, SNES, etc.)
   - Frame time distribution (min, max, p50, p99)
   - SDL blit performance
   - Shader processing time

2. **Audio**:
   - Resampling latency (libsamplerate)
   - Audio buffer underruns per minute
   - Audio callback overhead

3. **Memory**:
   - Allocations per frame (malloc/free count)
   - Peak memory usage
   - Allocation hot spots

4. **CPU**:
   - CPU usage % during gameplay
   - Hot functions (top 10 by time)
   - Function call frequency

**Tasks**:
- [ ] Profile 5 representative cores (GB, GBA, SNES, MD, PS)
- [ ] Collect 10-minute samples for each
- [ ] Generate baseline report with graphs
- [ ] Identify top 10 hot paths

**Deliverable**: `BASELINE_PERFORMANCE_REPORT.md` with:
- FPS charts
- CPU flame graphs
- Memory allocation heatmaps
- Top hot functions list

**Verification**:
- ✅ Baseline data collected for all target cores
- ✅ Hot paths identified and prioritized
- ✅ Report is reproducible

**Effort**: 1 week

---

### C.2: Rendering Optimization (Weeks 2-3)

**Objective**: Optimize frame rendering pipeline for consistent 60 FPS.

#### C.2.1: Optimize SDL Blitting

**Hot Path**: `GFX_blitScreen()`, `SDL_BlitSurface()`, `SDL_Flip()`

**Profiling Questions**:
- How much time is spent in SDL blit operations?
- Are we doing redundant blits?
- Can we use hardware acceleration?
- Are surface formats optimal?

**Optimizations**:
1. **Eliminate Redundant Blits**:
   - Cache previous frame state
   - Only blit changed regions (dirty rectangles)

2. **Optimize Surface Formats**:
   - Ensure source/dest formats match (avoid conversion)
   - Use RGBA8888 consistently

3. **Reduce Blit Count**:
   - Batch multiple blits
   - Minimize intermediate surfaces

**Tasks**:
- [ ] Profile current blit operations
- [ ] Implement dirty rectangle tracking
- [ ] Optimize surface format conversions
- [ ] Benchmark before/after
- [ ] Commit with performance metrics

**Expected Improvement**: 15-25% faster rendering

**Verification**:
- ✅ FPS increased by 10%+ in profiling
- ✅ No visual artifacts
- ✅ Works across all cores

**Effort**: 4-5 days

---

#### C.2.2: Optimize Shader Processing

**Hot Path**: Shader application in `platform.c` (if using shaders)

**Optimizations**:
1. **Cache Shader Compilation**:
   - Pre-compile shaders at startup
   - Avoid runtime compilation

2. **Reduce Shader Complexity**:
   - Profile shader execution time
   - Simplify expensive shaders (e.g., CRT effects)

3. **Conditional Shader Application**:
   - Skip shaders when not needed
   - Optimize shader parameter updates

**Tasks**:
- [ ] Profile shader overhead
- [ ] Optimize shader loading/compilation
- [ ] Simplify expensive shaders (if needed)
- [ ] Add shader on/off toggle for benchmarking

**Expected Improvement**: 5-10% faster rendering (if shaders used)

**Verification**:
- ✅ Shader performance improved
- ✅ Visual quality maintained
- ✅ User-configurable shader complexity

**Effort**: 3 days

---

#### C.2.3: Reduce Frame Overhead

**Hot Path**: Frame loop in `minarch.c` and `nextui.c`

**Optimizations**:
1. **Minimize Allocations in Frame Loop**:
   - Pre-allocate frame buffers
   - Reuse temporary buffers
   - Avoid malloc/free in hot path

2. **Optimize Function Calls**:
   - Inline hot functions
   - Reduce virtual dispatch overhead
   - Cache frequently accessed data

3. **Improve Cache Locality**:
   - Group related data structures
   - Align buffers to cache lines
   - Reduce pointer chasing

**Tasks**:
- [ ] Profile allocations per frame
- [ ] Eliminate/reduce hot path allocations
- [ ] Inline critical functions
- [ ] Benchmark memory bandwidth usage

**Expected Improvement**: 10-20% faster frame processing

**Verification**:
- ✅ Allocations per frame reduced by 50%+
- ✅ Frame time variability reduced
- ✅ CPU cache efficiency improved

**Effort**: 1 week

---

### C.3: Audio Optimization (Week 4)

**Objective**: Maintain <20ms audio latency while reducing CPU overhead.

#### C.3.1: Optimize Resampling

**Hot Path**: libsamplerate resampling in `api.c`

**Profiling Questions**:
- How much time is spent in resampling?
- Can we use a faster algorithm?
- Are we resampling unnecessarily?

**Optimizations**:
1. **Algorithm Selection**:
   - Profile different libsamplerate algorithms
   - Choose fastest with acceptable quality
   - Make algorithm configurable per-core

2. **Reduce Resampling**:
   - Match sample rates when possible
   - Cache resampled buffers (if applicable)

3. **Optimize Buffer Sizes**:
   - Find optimal buffer size for latency vs. overhead
   - Reduce buffer copies

**Tasks**:
- [ ] Profile resampling overhead
- [ ] Benchmark libsamplerate algorithms
- [ ] Implement configurable resampling quality
- [ ] Optimize buffer management

**Expected Improvement**: 20-30% faster audio processing

**Verification**:
- ✅ Audio latency <20ms maintained
- ✅ CPU usage reduced
- ✅ No audio glitches or pops

**Effort**: 3-4 days

---

#### C.3.2: Reduce Audio Callback Overhead

**Hot Path**: Audio callback in `api.c` and `minarch.c`

**Optimizations**:
1. **Minimize Locking**:
   - Use lock-free ring buffers if possible
   - Reduce mutex contention

2. **Batch Processing**:
   - Process larger chunks less frequently
   - Reduce context switches

3. **Eliminate Allocations**:
   - Pre-allocate all audio buffers
   - Reuse buffers across callbacks

**Tasks**:
- [ ] Profile audio callback frequency and duration
- [ ] Implement lock-free audio buffers (if beneficial)
- [ ] Optimize buffer management
- [ ] Benchmark latency and throughput

**Expected Improvement**: 10-15% lower CPU usage

**Verification**:
- ✅ Audio remains glitch-free
- ✅ Latency not increased
- ✅ CPU usage measurably reduced

**Effort**: 4 days

---

### C.4: Core Callback Optimization (Week 5)

**Objective**: Reduce overhead in libretro core interface.

#### C.4.1: Optimize Video Refresh Callback

**Hot Path**: `video_refresh_callback()` in `minarch.c`

**Optimizations**:
1. **Reduce Pixel Format Conversions**:
   - Match core output format to display format
   - Eliminate unnecessary conversions

2. **Optimize Buffer Copies**:
   - Use memcpy optimizations
   - Consider DMA if available

3. **Reduce Overhead**:
   - Cache frame dimensions
   - Avoid redundant calculations

**Tasks**:
- [ ] Profile video_refresh overhead
- [ ] Optimize pixel format handling
- [ ] Benchmark buffer copy performance
- [ ] Implement optimizations

**Expected Improvement**: 5-10% faster frame delivery

**Verification**:
- ✅ Video quality unchanged
- ✅ FPS improved
- ✅ Compatible with all cores

**Effort**: 3 days

---

#### C.4.2: Optimize retro_run Overhead

**Hot Path**: Core execution loop in `minarch.c`

**Optimizations**:
1. **Reduce Call Overhead**:
   - Minimize wrapper functions
   - Inline where beneficial

2. **Optimize State Management**:
   - Cache core state
   - Reduce state queries

3. **Improve Input Processing**:
   - Batch input updates
   - Reduce polling frequency if safe

**Tasks**:
- [ ] Profile retro_run call overhead
- [ ] Optimize wrapper functions
- [ ] Benchmark core execution time
- [ ] Verify compatibility with cores

**Expected Improvement**: 5-10% faster core execution

**Verification**:
- ✅ All cores work correctly
- ✅ Save states still function
- ✅ Performance improved

**Effort**: 3-4 days

---

### C.5: Memory Optimization (Week 6)

**Objective**: Reduce memory allocations and improve cache efficiency.

#### C.5.1: Eliminate Hot Path Allocations

**Profiling**: Identify all malloc/free calls in frame/audio loops

**Optimizations**:
1. **Pre-allocate Buffers**:
   - Allocate at startup, not per-frame
   - Use static or persistent buffers

2. **Pool Allocators**:
   - Create memory pools for fixed-size objects
   - Reduce fragmentation

3. **Stack Allocation**:
   - Use stack for small temporary buffers
   - Avoid heap allocation when possible

**Tasks**:
- [ ] Profile allocation hotspots
- [ ] Replace hot path allocations with pre-allocated buffers
- [ ] Implement memory pools where beneficial
- [ ] Benchmark allocation overhead

**Expected Improvement**: 30-50% fewer allocations

**Verification**:
- ✅ Allocation count per frame reduced significantly
- ✅ No memory leaks introduced
- ✅ Memory usage remains reasonable

**Effort**: 1 week

---

#### C.5.2: Improve Cache Locality

**Objective**: Organize data structures for better CPU cache utilization.

**Optimizations**:
1. **Data Structure Alignment**:
   - Align to cache line boundaries (64 bytes typical)
   - Pack hot data together

2. **Array of Structures → Structure of Arrays**:
   - For frequently accessed fields
   - Improves sequential access patterns

3. **Reduce Pointer Indirection**:
   - Flatten data structures where possible
   - Embed instead of pointing

**Tasks**:
- [ ] Analyze cache miss rates (if tooling available)
- [ ] Reorganize hot data structures
- [ ] Benchmark memory access patterns
- [ ] Verify performance improvement

**Expected Improvement**: 10-15% better memory performance

**Verification**:
- ✅ Cache efficiency improved (if measurable)
- ✅ FPS improved
- ✅ Code remains maintainable

**Effort**: 4-5 days

---

### C.6: Battery Life Optimization (Week 7)

**Objective**: Reduce CPU usage to extend battery life on TrimUI Brick.

#### C.6.1: Dynamic CPU Frequency Scaling

**Current**: May run at max frequency unnecessarily

**Optimization**:
- Monitor frame time
- Reduce CPU frequency when not needed (e.g., menu, simple cores)
- Scale up when needed (demanding cores, shader effects)

**Tasks**:
- [ ] Research TrimUI Brick CPU frequency control
- [ ] Implement dynamic scaling based on load
- [ ] Measure battery life impact
- [ ] Make configurable (user preference)

**Expected Improvement**: 10-20% longer battery life

**Verification**:
- ✅ Battery life increased measurably
- ✅ No performance degradation during gameplay
- ✅ Smooth frequency transitions

**Effort**: 4-5 days

---

#### C.6.2: Idle Optimization

**Objective**: Reduce CPU usage when paused or in menu.

**Optimizations**:
1. **Pause Rendering**:
   - Stop frame updates when paused
   - Reduce refresh rate in menu (30 FPS sufficient)

2. **Sleep Appropriately**:
   - Use proper sleep/wait functions
   - Don't busy-wait

3. **Suspend Background Work**:
   - Pause unnecessary timers
   - Reduce polling frequency

**Tasks**:
- [ ] Measure CPU usage in menu vs. gameplay
- [ ] Implement idle optimizations
- [ ] Verify wake-up latency acceptable
- [ ] Benchmark battery impact

**Expected Improvement**: 5-10% overall battery life

**Verification**:
- ✅ CPU usage near zero when idle
- ✅ Responsive wake-up from pause
- ✅ Battery life improved

**Effort**: 2-3 days

---

## Measurement and Validation

### Performance Benchmarking Suite

Create automated benchmarks for regression testing:

**Benchmark Scenarios**:
1. **Frame Rate Test**: Run 5 cores for 10 minutes each, measure average FPS
2. **Latency Test**: Measure input-to-display and audio latency
3. **Battery Test**: Run standard gameplay for 2 hours, measure battery drain
4. **Memory Test**: Monitor peak memory usage across cores
5. **Stress Test**: Run demanding cores (PS1) for extended periods

**Automation**:
- Script to run all benchmarks
- Collect results in CSV format
- Generate comparison reports
- Track trends over time

**Tasks**:
- [ ] Create benchmark scripts
- [ ] Establish baseline metrics
- [ ] Run benchmarks after each optimization
- [ ] Document improvements

**Effort**: 1 week (spread across phase)

---

## Risk Mitigation

### Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| **Performance regression** | High | Benchmark before/after every change |
| **Introduced bugs** | High | Extensive testing on real hardware |
| **Optimization complexity** | Medium | Start with low-hanging fruit |
| **Incompatibility with cores** | Medium | Test with diverse core set |
| **Diminishing returns** | Low | Profile continuously, stop when gains <5% |

---

## Success Criteria

### Must-Have (P0)

- ✅ FPS improved by 10%+ on average
- ✅ Audio latency remains <20ms
- ✅ No new bugs or regressions
- ✅ All cores tested and working
- ✅ Performance report documenting improvements

### Should-Have (P1)

- ✅ FPS improved by 20%+ on average
- ✅ Battery life improved by 10%+
- ✅ Memory allocations reduced by 50%+
- ✅ CPU usage reduced by 15%+
- ✅ Automated benchmark suite

### Nice-to-Have (P2)

- ✅ 60 FPS stable on all cores
- ✅ Battery life improved by 20%+
- ✅ User-configurable performance profiles
- ✅ Flame graphs and performance visualizations

---

## Timeline

```
Week 1: Profiling Infrastructure & Baselines
Week 2-3: Rendering Optimization
Week 4: Audio Optimization
Week 5: Core Callback Optimization
Week 6: Memory Optimization
Week 7: Battery Life Optimization
Week 8: Final Benchmarking & Documentation
```

**Total**: 8 weeks (~2 months)

---

## Deliverables

1. **BASELINE_PERFORMANCE_REPORT.md** - Initial performance analysis
2. **PHASE_C_COMPLETION_SUMMARY.md** - Final results and metrics
3. **Optimized codebase** - All optimizations committed with benchmarks
4. **Benchmark suite** - Automated performance testing
5. **Performance guide** - Document optimization techniques for future work

---

## Next Steps (After Phase C)

1. **Continuous Profiling**: Monitor performance in production
2. **Community Feedback**: Gather user reports on performance improvements
3. **Future Optimizations**: Identify new hot paths as usage evolves
4. **Platform-Specific Features**: Leverage TrimUI Brick hardware capabilities

---

**Document Status**: ✅ Ready to Execute  
**Phase**: Phase C - Performance Optimization  
**Prerequisites**: Phase A ✅ Complete, Phase B ✅ Complete  
**Author**: GitHub Copilot  
**Date**: November 15, 2025
