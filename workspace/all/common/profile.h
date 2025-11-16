/**
 * @file profile.h
 * @brief Performance profiling utilities for min-os
 * 
 * Provides lightweight timing instrumentation for identifying performance bottlenecks.
 * 
 * Usage:
 *   #include "profile.h"
 *   
 *   void some_function() {
 *       PROFILE_START(my_operation);
 *       // ... code to profile ...
 *       PROFILE_END(my_operation);
 *   }
 * 
 * Build with profiling enabled:
 *   make PROFILE=1
 * 
 * Output format:
 *   PROFILE [operation_name]: 1.234 ms
 */

#ifndef PROFILE_H
#define PROFILE_H

#include <stdint.h>
#include <stdio.h>

#ifdef PROFILE_ENABLED

#include <SDL/SDL.h>

/**
 * Start timing a named operation
 * Creates a local variable to store the start timestamp
 */
#define PROFILE_START(name) \
    uint64_t _profile_start_##name = SDL_GetPerformanceCounter()

/**
 * End timing and print elapsed time in milliseconds
 * Calculates elapsed time since corresponding PROFILE_START
 */
#define PROFILE_END(name) do { \
    uint64_t _profile_end = SDL_GetPerformanceCounter(); \
    uint64_t _profile_elapsed = _profile_end - _profile_start_##name; \
    double _profile_ms = (_profile_elapsed * 1000.0) / SDL_GetPerformanceFrequency(); \
    printf("PROFILE [%s]: %.3f ms\n", #name, _profile_ms); \
} while(0)

/**
 * Profile a single statement or block
 * Usage: PROFILE_BLOCK(operation_name, { code; });
 */
#define PROFILE_BLOCK(name, block) do { \
    PROFILE_START(name); \
    block; \
    PROFILE_END(name); \
} while(0)

/**
 * Profile a function call
 * Usage: PROFILE_CALL(function_name, function(args));
 */
#define PROFILE_CALL(name, call) do { \
    PROFILE_START(name); \
    call; \
    PROFILE_END(name); \
} while(0)

/**
 * Conditional profiling - only profile if condition is true
 */
#define PROFILE_IF(condition, name) \
    uint64_t _profile_start_##name = (condition) ? SDL_GetPerformanceCounter() : 0

#define PROFILE_END_IF(condition, name) do { \
    if (condition && _profile_start_##name != 0) { \
        uint64_t _profile_end = SDL_GetPerformanceCounter(); \
        uint64_t _profile_elapsed = _profile_end - _profile_start_##name; \
        double _profile_ms = (_profile_elapsed * 1000.0) / SDL_GetPerformanceFrequency(); \
        printf("PROFILE [%s]: %.3f ms\n", #name, _profile_ms); \
    } \
} while(0)

/**
 * Frame profiling helpers - track per-frame timing
 */
typedef struct {
    const char* name;
    uint64_t start;
    uint64_t total;
    uint32_t count;
} ProfileZone;

#define PROFILE_ZONE_DECLARE(name) \
    static ProfileZone _profile_zone_##name = { #name, 0, 0, 0 }

#define PROFILE_ZONE_START(name) do { \
    _profile_zone_##name.start = SDL_GetPerformanceCounter(); \
} while(0)

#define PROFILE_ZONE_END(name) do { \
    uint64_t end = SDL_GetPerformanceCounter(); \
    _profile_zone_##name.total += (end - _profile_zone_##name.start); \
    _profile_zone_##name.count++; \
} while(0)

#define PROFILE_ZONE_REPORT(name) do { \
    if (_profile_zone_##name.count > 0) { \
        double avg_ms = (_profile_zone_##name.total * 1000.0) / \
                        (SDL_GetPerformanceFrequency() * _profile_zone_##name.count); \
        printf("PROFILE_ZONE [%s]: avg=%.3f ms, count=%u\n", \
               _profile_zone_##name.name, avg_ms, _profile_zone_##name.count); \
    } \
} while(0)

#define PROFILE_ZONE_RESET(name) do { \
    _profile_zone_##name.total = 0; \
    _profile_zone_##name.count = 0; \
} while(0)

#else // !PROFILE_ENABLED

// No-op macros when profiling is disabled
#define PROFILE_START(name)
#define PROFILE_END(name)
#define PROFILE_BLOCK(name, block) block
#define PROFILE_CALL(name, call) call
#define PROFILE_IF(condition, name)
#define PROFILE_END_IF(condition, name)
#define PROFILE_ZONE_DECLARE(name)
#define PROFILE_ZONE_START(name)
#define PROFILE_ZONE_END(name)
#define PROFILE_ZONE_REPORT(name)
#define PROFILE_ZONE_RESET(name)

#endif // PROFILE_ENABLED

#endif // PROFILE_H
