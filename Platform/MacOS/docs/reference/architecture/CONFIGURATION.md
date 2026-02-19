# Configuration and Data Management

Generals is famously "data-driven." Almost every aspect of the game—from unit stats and weapon damage to UI layouts and particle effects—is controlled by text-based **INI files**.

## INI System Overview

The `INI` class (`Core/GameEngine/Source/Common/INI/INI.cpp`) is responsible for parsing these files. The system supports:
- **Hierarchical Loading**: Files in `Data\INI\Default\` define base values, which are then overridden by files in `Data\INI\`.
- **Inheritance**: Objects can inherit properties from other templates (using `Designator` or `InheritFrom`).
- **Dynamic Reloading**: In build configurations, some values can be reloaded without restarting the engine.

## Key Configuration Areas

### 1. Object Definitions (`Data\INI\Object\`)
Defines all units, buildings, and projectiles. This is the primary way to balance the game.
```ini
Object AmericaVehiclePaladin
  DisplayName        = OBJECT:AmericaVehiclePaladin
  EditorSorting      = VEHICLE
  Health             = 400.0
  VisionRange        = 150.0
  ArmorSet
    Armor            = TankArmor
  End
End
```

### 2. Game Rules (`GameData.ini`)
Global constants like starting money, building build times, and damage multipliers.

### 3. User Interface (`Window\*.wnd` and `GUI.ini`)
Visual layout of menus and the HUD. Uses a proprietary format that defines gadgets (buttons, sliders, text boxes).

### 4. Particles and FX (`FXList.ini`, `ParticleSystem.ini`)
Defines explosions, smoke, tracers, and other visual effects.

## Data Packaging (BIG Files)

In production, thousands of INI and asset files are packed into **.BIG** archives (a custom uncompressed container format).
- **TheArchiveFileSystem**: Handles the mounting of BIG files.
- **Priority**: Files inside a BIG archive can be overridden by a loose file with the same path if the developer/modder places it in the `Data/` directory.

## Translation (CSF Files)

Strings shown to the user are stored in **.CSF** (Compiled String File) binary files. This separates game logic from localized text, allowing for easy translation into multiple languages.
- **TheStringDB**: The global cache for all localized strings.
