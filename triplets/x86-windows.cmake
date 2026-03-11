# Include the default x86-windows triplet
set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# Exclude compiler version from ABI hash so that weekly GitHub runner image
# updates don't invalidate the binary cache. Minor MSVC version bumps do not
# cause ABI incompatibilities for this project.
set(VCPKG_DISABLE_COMPILER_TRACKING ON)
