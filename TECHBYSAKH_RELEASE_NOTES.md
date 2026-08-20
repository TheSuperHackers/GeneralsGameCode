# TECHBYSAKH Modified - Community Edition

## Release summary

This fork now includes a branded, compatibility-preserving upgrade slice for Generals/Zero Hour. The rebuilt Zero Hour executable is a **32-bit Windows PE binary** produced with the repository’s MinGW target and the TECHBYSAKH profile enabled.

The executable keeps the original lockstep simulation and existing LAN transport. It does not replace the W3D/Direct3D 8-compatible renderer with a new D3D11 or Vulkan backend; instead, it adds an explicit GPU-aware quality profile on top of the renderer that the project already ships, while preserving a conservative fallback when the hardware is not recognized.

## Completed changes

| Area | Result |
| --- | --- |
| Branding | Product title is `TECHBYSAKH Modified - Community Edition`, with an in-game credits identity and build switches. |
| Frame pacing | The limiter now schedules against a high-resolution deadline, avoids accumulating sleep drift, re-anchors after stalls or limit changes, and handles uncapped mode without a synthetic wait. |
| LAN responsiveness | The branded LAN lobby pump runs at 50 ms instead of 200 ms, making discovery, chat, and ready-state updates feel more immediate without changing synchronized in-game transport. |
| LAN smoothness | Branded LAN matches no longer forcibly disable the user’s render FPS cap. The configured cap remains available so presentation does not unnecessarily compete with lockstep work. |
| GPU quality | Known Direct3D-capable chipsets can use the enhanced high-quality LOD profile. The profile is opt-out through CMake. |
| New content surface | Added a mode manifest for Co-op Mission, Team Assault, and Free-for-All Blitz, plus map briefs, deterministic scripting rules, and LAN instructions. |
| Visual identity | Added an original TECHBYSAKH shield-and-circuit emblem at `TECHBYSAKH_Mod/Branding/techbysakh_emblem.png`. |

## Build verification

The native Unix configure path was intentionally not used for the final binary because the project targets Windows APIs. After installing the MinGW-w64 and Wine IDL prerequisites, the following target completed successfully:

```text
cmake -S . -B build/mingw-techbysakh -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-i686.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DRTS_BUILD_TECHBYSAKH=ON \
  -DRTS_BUILD_TECHBYSAKH_HIGH_QUALITY=ON \
  -DRTS_BUILD_CORE_TOOLS=OFF \
  -DRTS_BUILD_ZEROHOUR=ON \
  -DRTS_BUILD_GENERALS=OFF \
  -DRTS_BUILD_ZEROHOUR_TOOLS=OFF \
  -DRTS_BUILD_ZEROHOUR_EXTRAS=OFF
cmake --build build/mingw-techbysakh --parallel 2
```

The result is `build/mingw-techbysakh/GeneralsMD/generalszh.exe`. The build reached `1118/1118` and linked successfully. Repository whitespace validation also passed with `git diff --check`. Warnings emitted by the legacy codebase were non-fatal and unrelated to the new TECHBYSAKH changes.

## Runtime requirements

The binary still requires a legally obtained Generals/Zero Hour installation and compatible game data. The repository does not redistribute EA assets. The release ZIP now includes a root-level `Install-TECHBYSAKH.cmd` installer. Run it after installing Generals/Zero Hour from your legitimate source; it detects common install locations or prompts for the folder, backs up `generalszh.exe` as `generalszh.exe.original`, installs the rebuilt executable and TECHBYSAKH package, and creates `Launch-TECHBYSAKH.cmd`. For multiplayer, install the same executable, map, and data-pack version on every machine, allow the game through the private-network firewall, and use LAN discovery or direct connect with the host’s private IPv4 address.

## Content status

The mode and map files in `TECHBYSAKH_Mod/` are a safe data-driven authoring surface. They describe the intended gameplay and scripting contract but are not falsely presented as finished binary `.map` assets. Final mission maps and INI data should be created in the companion data/assets project and tested with replay, save/load, transfer, and observer cases before public release.
