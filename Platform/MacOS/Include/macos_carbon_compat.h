/**
 * macos_carbon_compat.h — Force-included BEFORE all macos_platform source files.
 *
 * Resolves type name conflicts between Carbon (macOS SDK) and game engine:
 *   WideChar    → BLOCK IntlResources.h (safe, no dependencies)
 *   ChunkHeader → BLOCK AIFF.h (safe, no dependencies)
 *   RGBColor    → Guarded in BaseType.h with #if !defined(__QUICKDRAW__)
 *   FileInfo    → Renamed via macro during Apple framework import, then undefed
 *   ToolUtils   → BLOCK (parse errors with game types)
 *
 * This header is force-included via CMake: -include macos_carbon_compat.h
 */

#pragma once

#ifdef __APPLE__

// ── Safely block Carbon headers that conflict with game types ──
#ifndef __INTLRESOURCES__
#define __INTLRESOURCES__  // Blocks: union WideChar (conflicts with typedef wchar_t)
#endif

#ifndef __AIFF__
#define __AIFF__           // Blocks: struct ChunkHeader (conflicts with chunkio.h)
#endif

#ifndef __TOOLUTILS__
#define __TOOLUTILS__      // Blocks: FixRatio etc. (parse errors with game types)
#endif

// ── Rename Carbon's FileInfo during Apple framework import ──
// Carbon Files.h defines `struct FileInfo` (HFS file metadata).
// Game's FileSystem.h defines `struct FileInfo` (size + timestamp).
// We rename Carbon's version during import, then restore the name
// so game code can define its own FileInfo normally.
#define FileInfo Carbon_FileInfo_

// ── Import Apple frameworks (in ObjC/ObjC++ context) ──
// QuickDraw and Files.h load normally — needed by other Carbon subsystems.
// QuickDraw's RGBColor is handled by the guard in BaseType.h.
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

// Restore FileInfo name for game code
#undef FileInfo

#endif // __APPLE__
