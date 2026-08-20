# TECHBYSAKH Upgrade Audit

## Baseline inspected

The repository is a C++20-compatible modernization of the open-source Generals/Zero Hour engine. It builds the original 32-bit Windows game path and keeps retail compatibility as a primary constraint. The active engine-device layer is still tied to the legacy W3D/Direct3D 8-compatible interface and links the `d3d8lib` abstraction. The source tree includes a separated `Core/GameNetwork` module, but the current project documentation describes a future upgraded lobby/server as unfinished work rather than a completed feature.

The repository has an existing CMake/vcpkg build layout, Windows 32-bit and MinGW presets, Tracy profiling support, Docker-based Linux build scripts, and a replay-checking workflow. The sandbox does not currently have CMake, a C++ compiler, or the Windows cross compiler installed, so validation will require static checks, targeted source-level tests, or dependency installation before a full build.

## Upgrade direction

The first implementation slice will be compatibility-preserving and low-risk: add a centralized TECHBYSAKH branding/build identity, make frame pacing and performance options explicit, improve LAN discovery/session setup through a small reusable networking utility where the existing code allows it, and add a documented mod/data integration surface for new maps and modes. More invasive renderer replacement is not safe to claim without the original game assets, the 32-bit toolchain, and the proprietary runtime-facing interfaces; the repository will instead gain an extensible graphics/performance foundation and clearly marked hooks for a future modern backend.

## Constraints

The original game installation and data files remain required to run the binary. The project is GPL-3.0, while original game assets and trademarks remain governed by their respective owners. Any bundled TECHBYSAKH artwork will be original project branding and will not replace required EA assets.
