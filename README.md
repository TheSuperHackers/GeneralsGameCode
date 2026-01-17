[![GitHub Release](https://img.shields.io/github/v/release/TheSuperHackers/GeneralsGameCode?include_prereleases&sort=date&display_name=tag&style=flat&label=Release)](https://github.com/TheSuperHackers/GeneralsGameCode/releases)
![GitHub milestone details](https://img.shields.io/github/milestones/progress-percent/TheSuperHackers/GeneralsGameCode/3)
![GitHub milestone details](https://img.shields.io/github/milestones/progress-percent/TheSuperHackers/GeneralsGameCode/1)
![GitHub milestone details](https://img.shields.io/github/milestones/progress-percent/TheSuperHackers/GeneralsGameCode/4)
![GitHub milestone details](https://img.shields.io/github/milestones/progress-percent/TheSuperHackers/GeneralsGameCode/5)
![GitHub milestone details](https://img.shields.io/github/milestones/progress-percent/TheSuperHackers/GeneralsGameCode/6)

[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/bug?style=flat&label=Bug%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3ABug)
[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/enhancement?style=flat&label=Enhancement%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3AEnhancement)
[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/major?style=flat&label=Major%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3AMajor)
[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/critical?style=flat&label=Critical%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3ACritical)
[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/blocker?style=flat&label=Blocker%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3ABlocker)

# Welcome to the Generals Game Code Project

GeneralsGameCode is a community-driven project aimed at fixing and improving the classic RTS game, *Command &
Conquer: Generals* and its expansion *Zero Hour*. This repository contains the source code for both games, with a
primary focus on *Zero Hour*.

Additionally, there is a complementary project repository for fixing and improving game data and assets such as
INI scripts, GUI, AI, maps, models, textures, audio, localization. You can find it
[here](https://github.com/TheSuperHackers/GeneralsGamePatch/) and contribute to it as well.

## Project Overview

The game was originally developed using Visual Studio 6 and C++98. We've updated the code to be compatible with Visual
Studio 2022 and C++20.

The initial goal of this project is to fix critical bugs and implement improvements while maintaining compatibility with
the original *Generals* version 1.08 and *Zero Hour* version 1.04. Once we can break retail compatibility, more fixes
and features will be possible to implement.

## Current Focus and Future Plans

Here's an overview of our current focus and future plans

- **Modernizing the Codebase**: Transitioning to modern C++ standards and refactoring old code.
- **Critical Bug Fixes**: Fixing game-breaking issues (e.g., fullscreen crash).
- **Minor Bug Fixes**: Addressing minor bugs (e.g., UI issues, graphical glitches).
- **Cross-Platform Support**: Adding support for more platforms (e.g., Linux, macOS).
- **Engine Improvements**: Enhancing the game engine to improve performance and stability.
- **Client-Side Features**: Enhancing the game's client with features such as an improved replay viewer and UI updates.
- **Multiplayer Improvements**: Implementing a new game server and an upgraded matchmaking lobby.
- **Tooling Improvements**: Developing new or improving existing tools for modding and game development.
- **Community-Driven Improvements**: Once the community grows, we plan to incorporate more features, updates, and
  changes based on player feedback.

## Running the Game

To run *Generals* or *Zero Hour* using this project, you need to have the original *Command & Conquer: Generals and Zero Hour* game
installed. The easiest way to get it is through *Command & Conquer The Ultimate Collection*
on [Steam](https://store.steampowered.com/bundle/39394). Once the game is ready, download the latest version of the
project from [GitHub Releases](https://github.com/TheSuperHackers/GeneralsGameCode/releases), extract the necessary 
files, and follow the instructions in the [Wiki](https://github.com/TheSuperHackers/GeneralsGameCode/wiki).


## Joining the Community

You can chat and discuss the development of the project on our [Discord channel](https://www.community-outpost.com/discord) to get the latest updates,
report bugs, and contribute to the project!

## Building the Game Yourself

We provide support for building the project on Windows and Linux. For detailed build instructions, check the
[Wiki](https://github.com/TheSuperHackers/GeneralsGameCode/wiki/build_guides), which includes guides for VS6, VS2022,
Docker, CLion, and links to forks supporting additional versions.

### Quick Start

**Windows (Visual Studio 2022)**
```bash
cmake --preset win32
cmake --build build/win32 --config Release
```

**Linux (via Docker)**
```bash
./scripts/docker-build.sh              # Build using Docker
./scripts/docker-install.sh --detect # Install to your game
```

### Dependency management

The repository uses a vcpkg manifest (`vcpkg.json`) paired with a lockfile (`vcpkg-lock.json`). When you add or upgrade
dependencies, run `vcpkg install --x-manifest-root . --triplet <triplet>` with `VCPKG_FEATURE_FLAGS=versions` so the
lockfile picks up the new versions and include the updated lockfile in your change. GitHub Actions consumes these ports
through `VCPKG_BINARY_SOURCES=clear;files,<workspace>/vcpkg-bincache,readwrite` (paired with an `actions/cache` entry for
that folder), so the first CI build warms the cache and subsequent builds pull prebuilt binaries instead of
re-compiling everything.

## Contributing

We welcome contributions to the project! If you’re interested in contributing, you need to have knowledge of C++. Join
the developer chat on Discord for more information on how to get started. Please make sure to read our
[Contributing Guidelines](CONTRIBUTING.md) before submitting a pull request. You can also check out 
the [Wiki](https://github.com/TheSuperHackers/GeneralsGameCode/wiki) for more detailed documentation.


## License & Legal Disclaimer

EA has not endorsed and does not support this product. All trademarks are the property of their respective owners.

This project is licensed under the [GPL-3.0 License](https://www.gnu.org/licenses/gpl-3.0.html), which allows you to
freely modify and distribute the source code under the terms of this license. Please see [LICENSE.md](LICENSE.md) 
for details.
