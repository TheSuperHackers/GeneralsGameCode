[![Build Status](https://github.com/TheSuperHackers/GeneralsGameCode/actions/workflows/build-games.yml/badge.svg?branch=main)](https://github.com/TheSuperHackers/GeneralsGameCode/actions/workflows/build-games.yml)
[![GitHub Repo stars](https://img.shields.io/github/stars/TheSuperHackers/GeneralsGameCode?style=flat&logo=github&label=Stars&color=%23FFD700)](https://github.com/TheSuperHackers/GeneralsGameCode)
[![Discord](https://img.shields.io/discord/951133504605917224?style=flat&logo=discord&logoColor=6A5ACD&label=Discord&labelColor=D0CBEF&color=6A5ACD)](https://www.community-outpost.com/discord)
[![YouTube Channel Views](https://img.shields.io/youtube/channel/views/UCi0AO7Bzth2NN8A9z7Gi40Q?style=flat&logo=youtube&logoColor=red&label=Views&labelColor=FFBCBC&color=red)](https://www.youtube.com/@xezon0/videos)
[![Website](https://img.shields.io/website?url=https%3A%2F%2Fwww.gamereplays.org%2F&style=flat&label=GameReplays.org&labelColor=%233d77eb)](https://www.gamereplays.org/community/-cnc_zero_hour-Section.html)

[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/bug?style=flat&label=Bug%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3ABug)
[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/enhancement?style=flat&label=Enhancement%20Issues&labelColo=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3AEnhancement)
[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/blocker?style=flat&label=Blocker%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3ABlocker)
[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/critical?style=flat&label=Critical%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3ACritical)
[![GitHub issues by-label](https://img.shields.io/github/issues/TheSuperHackers/GeneralsGameCode/major?style=flat&label=Major%20Issues&labelColor=%23c4c4c4&color=%23424242)](https://github.com/TheSuperHackers/GeneralsGameCode/issues?q=label%3AMajor)

# Welcome to the Generals Game Code Project

GeneralsGameCode is a community-driven project aimed at maintaining and improving the classic RTS game, *Command &
Conquer: Generals* and its expansion *Zero Hour*. This repository contains the source code for both the original game
and *Zero Hour*, with a primary focus on *Zero Hour*.

Additionally, there is a separate repository for managing game assets such as graphics, localization files, and more.
You can find it [here](https://github.com/TheSuperHackers/GeneralsGamePatch/) and contribute to it as well.

## Project Overview

The game was originally developed using Visual Studio 6 and C++98. We've updated the code to be compatible with Visual
Studio 2022 and C++20 to improve the development experience and take advantage of modern C++ features.

The main goal of this project is to fix critical bugs and implement improvements while maintaining compatibility with
the original *Zero Hour* version 1.04 in the initial stages. We'll focus on resolving critical bugs, with future plans
to break compatibility with version 1.04 once the community around the project is established, allowing for further bug
fixes and the addition of new features.

## Current Focus and Future Plans

This project is in the early stages and heavily work in progress, with ongoing work on bug fixes and enhancements.
Here's an overview of our current focus and future plans

- **Modernizing the Codebase**: Transitioning to modern C++ standards and refactoring old code.
- **Critical Bug Fixes**: Fixing game-breaking issues (e.g., fullscreen crash).
- **Minor Bug Fixes**: Addressing minor bugs (e.g., UI issues, graphical glitches).
- **Cross-Platform Support**: Adding support for more platforms (e.g., Linux, macOS).
- **Engine Improvements**: Enhancing the game engine to improve performance and stability.
- **Client-Side Features**: Enhancing the game’s client with features such as an improved replay viewer and UI updates.
- **New Gameplay Features**: Adding new in-game features (e.g., new units, balance changes).
- **Multiplayer Improvements**: Implementing a new game server and an upgraded matchmaking lobby.
- **Tooling Improvements**: Developing new or improving existing tools for modding and game development.
- **Community-Driven Improvements**: Once the community grows, we plan to incorporate more features, updates, and
  changes based on player feedback.

## Running the Game

To run *Zero Hour* using this project, you need to have the original *Command & Conquer: Generals and Zero Hour* game
installed. The easiest way to get it is through *Command & Conquer The Ultimate Collection*
on [Steam](https://store.steampowered.com/bundle/39394). Once the game is ready, download the latest version of the
project from [GitHub Releases](https://github.com/TheSuperHackers/GeneralsGameCode/releases), extract the necessary 
files, and follow the instructions in the [Wiki](https://github.com/TheSuperHackers/GeneralsGameCode/wiki).

### Disclaimer

The current builds may have bugs or unexpected behavior. We will aim to release stable versions that are as free from
bugs as possible, but even those may have some unexpected behaviors.

### Antivirus Warning

Some antivirus software may mistakenly flag this project as a false positive (e.g., Windows Defender). We assure you
that the project is fully safe to use. However, if you have concerns, you can
always [build it yourself](#building-the-game-yourself).

## Joining the Community

You can chat and discuss the development of the project on our
[Discord channel](https://www.community-outpost.com/discord) or on 
our [GameReplays forum](https://www.gamereplays.org/community/index.php?showforum=132). Join us to get the latest
updates, report bugs, and contribute to the project!

## Building the Game Yourself

We provide support for building the project using Visual Studio 6 (VS6) and Visual Studio 2022. For detailed build
instructions, check the [Wiki](https://github.com/TheSuperHackers/GeneralsGameCode/wiki//build_guides), which also includes guides
for building with Docker, CLion, and links to forks supporting additional versions.

## Contributing

We welcome contributions to the project! If you’re interested in contributing, you should have some knowledge of C++ and
Visual Studio. We also allow this to be built without Visual Studio. Join the developer chat on Discord for more
information on how to get started. You can also check out
the [Wiki](https://github.com/TheSuperHackers/GeneralsGameCode/wiki) for more detailed documentation.

## Modding

Interested in creating your own mods? Check out
our [modding guide](https://github.com/TheSuperHackers/GeneralsGameCode/wiki/Modding).

## License & Legal Disclaimer

This project is not affiliated with, nor endorsed by, Electronic Arts in any way. All trademarks are the property of
their respective owners.

This project is licensed under the [GPL-3.0 License](https://www.gnu.org/licenses/gpl-3.0.html), which allows you to
freely modify and distribute the source code under the terms of this license. Please see [LICENSE.md](LICENSE.md) 
for details.
