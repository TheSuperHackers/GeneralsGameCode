# Slang is intentionally built from a pinned source checkout. Do not replace
# this with a package-manager lookup: graphics owns the engine shader language and
# must use the compiler configured together with this build.
include(FetchContent)

set(SLANG_ENABLE_SLANG_RHI OFF CACHE BOOL "Do not build Slang's RHI" FORCE)
set(SLANG_ENABLE_GFX OFF CACHE BOOL "Do not build Slang's gfx layer" FORCE)
set(SLANG_ENABLE_SLANGC OFF CACHE BOOL "Do not build the Slang command line compiler" FORCE)
set(SLANG_ENABLE_SLANGD OFF CACHE BOOL "Do not build the Slang language server" FORCE)
set(SLANG_ENABLE_SLANGI OFF CACHE BOOL "Do not build the Slang interpreter" FORCE)
set(SLANG_ENABLE_SLANGRT OFF CACHE BOOL "Do not build SlangRT" FORCE)
set(SLANG_ENABLE_SLANG_GLSLANG OFF CACHE BOOL "Do not build glslang integration" FORCE)
set(SLANG_ENABLE_SLANG_PROXY OFF CACHE BOOL "Do not build Slang proxy support" FORCE)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "Do not build Slang tests" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "Do not build Slang examples" FORCE)
set(SLANG_ENABLE_REPLAYER OFF CACHE BOOL "Do not build Slang replayer" FORCE)
set(SLANG_EXCLUDE_DAWN ON CACHE BOOL "Do not build Dawn" FORCE)
set(SLANG_EXCLUDE_TINT ON CACHE BOOL "Do not build Tint" FORCE)
set(SLANG_ENABLE_MIMALLOC OFF CACHE BOOL "Use the host allocator" FORCE)
set(SLANG_LIB_TYPE STATIC CACHE STRING "Build Slang as a static library" FORCE)
set(SLANG_EMBED_CORE_MODULE ON CACHE BOOL "Embed Slang's core module" FORCE)
set(SLANG_EMBED_CORE_MODULE_SOURCE ON CACHE BOOL "Embed Slang core module source" FORCE)

# Keep the LLVM dependency disabled. The checked-in graphics shader sources use
# Slang's built-in target backends and do not require the optional LLVM path.
set(SLANG_SLANG_LLVM_FLAVOR DISABLE CACHE STRING "Slang LLVM flavor" FORCE)

FetchContent_Declare(slang
    GIT_REPOSITORY https://github.com/shader-slang/slang.git
    # v2026.1 is a released Slang checkout with the DXBC API used by GenMD.
    # Later 2026 tags currently add source-generator/record-replay changes
    # that are not needed by this x86 static consumer.
    GIT_TAG bab9b0d994ebb4d53f9030bc85da3f273b741b83 # v2026.1
    GIT_SHALLOW FALSE
    GIT_SUBMODULES_RECURSE TRUE
)

FetchContent_MakeAvailable(slang)

if(TARGET slang)
    # slang.h defaults to the DLL ABI unless a static consumer opts in.
    target_compile_definitions(slang PUBLIC SLANG_STATIC)
endif()
