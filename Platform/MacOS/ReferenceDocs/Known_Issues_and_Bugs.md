# Known Issues and Historical Bugs
**Source:** `GeneralsX:old-multiplatform-attempt` (`docs/WORKDIR/phases/legacy/Known_Issues/README.md`)
**Time Context:** Late 2025

## 1. Wide Texture "Orange Tint" Bug
- **Symptoms**: Textures with width >= 1024 pixels (e.g., Victory screens) render correctly on the left half but appear solid orange on the right half.
- **Cause**: Potential Apple GPU driver limitation or incorrect `MTLTexture` descriptor setup for large non-power-of-two (NPOT) textures.
- **Workaround**: Shrink large textures to 512x256 or use BC3 (DXT5) decompression on the CPU (`bc3decompressor.cpp`) to convert them to basic RGBA8 before upload.

## 2. AGXMetal Driver Crashes
- **Symptoms**: Random segfaults when passing ASCII strings to certains system calls or driver paths.
- **Root Cause**: Apple Silicon (M1/M2) driver bugs related to memory pointer validation.
- **Fix**: Implemented `GameMemory.cpp::isValidMemoryPointer()` to verify all pointers before they reach the driver boundary.

## 3. Blank Screen / Black Viewport
- **Symptoms**: Game initialized, UI interactions occurred (based on logs), but the window remained black or magenta.
- **Root Cause**: Found to be related to `MTLRenderPassDescriptor` `loadAction` settings. If `MTLLoadActionClear` is used without subsequent drawing, the buffer remains empty.
- **Verification**: Check `MTLViewport` and `MTLScissorRect` scaling on Retina displays (2x scale).

## 4. Endianness (ARM64)
- **Status**: Mostly resolved.
- **Details**: Generals was written for Little-Endian (x86). Metal and ARM64 are also Little-Endian, but `.big` files and some binary assets might require byte-swapping if they use Big-Endian headers (notably Apple's older PPC heritage).
