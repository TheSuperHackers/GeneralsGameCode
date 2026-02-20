// MacOSDebugLog.h — macOS port debug logging utility
// 
// Usage:
//   #include "MacOSDebugLog.h"
//   DLOG_RFLOW(1, "PushButtonDraw pos=(%d,%d)", x, y);
//   DLOG_TEXTURE("loaded '%s' %dx%d", name, w, h);
//
// Features:
// - No-op on non-Apple platforms (safe to include everywhere)
// - Group-based filtering via DLOG_ACTIVE_GROUPS bitmask
// - All output goes to stdout with fflush for immediate visibility
// - Easy grep: all output starts with group tag (RFLOW[N], TEXTURE:, etc.)

#pragma once

#ifdef __APPLE__

#include <cstdio>

// ============================================================================
// Log groups — each is a bit flag
// ============================================================================
#define DLOG_GROUP_RFLOW       (1 << 0)   // Rendering flow (numbered pipeline stages)
#define DLOG_GROUP_TRANSITION  (1 << 1)   // TransitionHandler (menu animations)
#define DLOG_GROUP_INPUT       (1 << 2)   // Input handling / MainMenu events
#define DLOG_GROUP_TEXTURE     (1 << 3)   // Texture loading & binding
#define DLOG_GROUP_FONT        (1 << 4)   // Font loading & text rendering
#define DLOG_GROUP_WINDOW      (1 << 5)   // Window creation & management
#define DLOG_GROUP_INIT        (1 << 6)   // Initialization & startup
#define DLOG_GROUP_DRAW        (1 << 7)   // Low-level draw calls (Metal/DX8)

// ============================================================================
// Active groups — EDIT THIS to enable/disable output per group
// Comment out groups you've already verified to reduce noise
// ============================================================================
#define DLOG_ACTIVE_GROUPS ( \
    DLOG_GROUP_RFLOW       | \
    DLOG_GROUP_TRANSITION  | \
    DLOG_GROUP_INPUT       | \
    DLOG_GROUP_TEXTURE     | \
    DLOG_GROUP_FONT        | \
    DLOG_GROUP_WINDOW      | \
    DLOG_GROUP_INIT        | \
    DLOG_GROUP_DRAW          \
)

// ============================================================================
// Core macro — group filtering, no print limit
// ============================================================================
#define DLOG(group, tag, max_count, fmt, ...) \
    do { \
        if ((group) & DLOG_ACTIVE_GROUPS) { \
            printf(tag " " fmt "\n", ##__VA_ARGS__); \
            fflush(stdout); \
        } \
    } while(0)

// Variant with custom max count
#define DLOGN(group, tag, max_count, fmt, ...) \
    DLOG(group, tag, max_count, fmt, ##__VA_ARGS__)

// ============================================================================
// Convenience macros per group
// ============================================================================

// Rendering flow — numbered pipeline stages (RFLOW[1] through RFLOW[N])
// The number is part of the tag for ordering visibility
#define DLOG_RFLOW(n, fmt, ...)      DLOG(DLOG_GROUP_RFLOW, "RFLOW[" #n "]", DLOG_DEFAULT_MAX, fmt, ##__VA_ARGS__)
#define DLOG_RFLOW_N(n, max, fmt, ...) DLOG(DLOG_GROUP_RFLOW, "RFLOW[" #n "]", max, fmt, ##__VA_ARGS__)

// TransitionHandler
#define DLOG_TRANSITION(fmt, ...)    DLOG(DLOG_GROUP_TRANSITION, "TRANSITION:", DLOG_DEFAULT_MAX, fmt, ##__VA_ARGS__)

// Input
#define DLOG_INPUT(fmt, ...)         DLOG(DLOG_GROUP_INPUT, "INPUT:", DLOG_DEFAULT_MAX, fmt, ##__VA_ARGS__)

// Texture loading
#define DLOG_TEXTURE(fmt, ...)       DLOG(DLOG_GROUP_TEXTURE, "TEXTURE:", DLOG_DEFAULT_MAX, fmt, ##__VA_ARGS__)

// Font
#define DLOG_FONT(fmt, ...)          DLOG(DLOG_GROUP_FONT, "FONT:", DLOG_DEFAULT_MAX, fmt, ##__VA_ARGS__)

// Window
#define DLOG_WINDOW(fmt, ...)        DLOG(DLOG_GROUP_WINDOW, "WINDOW:", DLOG_DEFAULT_MAX, fmt, ##__VA_ARGS__)

// Init
#define DLOG_INIT(fmt, ...)          DLOG(DLOG_GROUP_INIT, "INIT:", DLOG_DEFAULT_MAX, fmt, ##__VA_ARGS__)

// Low-level draw
#define DLOG_DRAW(fmt, ...)          DLOG(DLOG_GROUP_DRAW, "DRAW:", DLOG_DEFAULT_MAX, fmt, ##__VA_ARGS__)

#else // !__APPLE__  — all macros are no-ops

#define DLOG(group, tag, max_count, fmt, ...)         ((void)0)
#define DLOGN(group, tag, max_count, fmt, ...)        ((void)0)
#define DLOG_RFLOW(n, fmt, ...)                       ((void)0)
#define DLOG_RFLOW_N(n, max, fmt, ...)                ((void)0)
#define DLOG_TRANSITION(fmt, ...)                     ((void)0)
#define DLOG_INPUT(fmt, ...)                          ((void)0)
#define DLOG_TEXTURE(fmt, ...)                        ((void)0)
#define DLOG_FONT(fmt, ...)                           ((void)0)
#define DLOG_WINDOW(fmt, ...)                         ((void)0)
#define DLOG_INIT(fmt, ...)                           ((void)0)
#define DLOG_DRAW(fmt, ...)                           ((void)0)

#endif // __APPLE__
