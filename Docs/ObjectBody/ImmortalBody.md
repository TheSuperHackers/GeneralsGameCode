# ImmortalBody

Status: AI-generated, 0/2 reviews

## Overview

The `ImmortalBody` module is a specialized body module that prevents objects from ever dying by ensuring their health never drops below 1 point. Unlike ActiveBody which allows objects to die when health reaches 0, ImmortalBody overrides the health change system to prevent health from falling below 1, making objects truly immortal. Objects can still take damage, show damage states, and be affected by all other body systems, but they can never be destroyed through health loss. This is a module added inside `Object` entries.

Available in: *(v1.04)* (Generals, Zero Hour)

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
  - [Limitations](#limitations)
  - [Conditions](#conditions)
  - [Dependencies](#dependencies)
- [Properties](#properties)
  - [Health Settings](#health-settings)
  - [Subdual Damage Settings](#subdual-damage-settings)
- [Enum Value Lists](#enum-value-lists)
- [Examples](#examples)
- [Template](#template)
- [Notes](#notes)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

Place under `Body = ImmortalBody ModuleTag_XX` inside [Object](../Object.md) entries. ImmortalBody can only be added to [Object](../Object.md) entries in Retail (ObjectExtend does not exist in Retail). See [Template](#template) for correct syntax.

Only one body module (ActiveBody, ImmortalBody, StructureBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- Health is automatically clamped to minimum 1 point - objects can never die from any type of damage, regardless of how much damage is dealt.
- Objects can still take damage normally and show damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) when the first health change occurs. Health cannot go below `1.0` or above [MaxHealth](#maxhealth).
- [SubdualDamageCap](#subdualdamagecap) can disable objects without destroying them when subdual damage equals or exceeds [MaxHealth](#maxhealth). Subdual damage properties are available only in Zero Hour.

**Conditions**:
- Objects with ImmortalBody can be targeted by weapons (see [Weapon documentation](../Weapon.md)) and affected by damage types. Health is reduced when weapons deal damage, but cannot fall below 1 point. Damage states are updated based on health percentage.
- **MaxHealthUpgrade**: When upgrades are applied that increase maximum health, `MaxHealthUpgrade` calls `setMaxHealth()`, which internally calls `internalChangeHealth()`. ImmortalBody's override ensures health never drops below 1, even when health changes occur during upgrade application. The health change type (PRESERVE_RATIO, ADD_CURRENT_HEALTH_TOO, FULLY_HEAL, SAME_CURRENTHEALTH) determines how current health is adjusted when maximum health changes.
- **Veterancy system**: When objects gain veterancy levels (Veteran, Elite, Heroic), the system calls `onVeterancyLevelChanged()`, which modifies maximum health using `setMaxHealth()` with `PRESERVE_RATIO` to scale health proportionally. ImmortalBody's health protection ensures objects remain at minimum 1 health even after veterancy changes.
- **Player difficulty scaling**: In single-player games, difficulty settings modify object maximum health via `setMaxHealth()` with `PRESERVE_RATIO`. ImmortalBody's protection applies during these scaling operations.
- **Healing systems**: Healing behaviors and repair systems call `attemptHealing()`, which uses `internalChangeHealth()`. ImmortalBody allows healing to work normally, but ensures health never goes below 1.
- **Script actions**: Scripts can damage objects using `doNamedDamage()` and `doDamageTeamMembers()`, which call `attemptDamage()` and eventually `internalChangeHealth()`. ImmortalBody prevents script-based damage from killing objects. Scripts can also set maximum health via object properties (`objectMaxHPs`), which calls `setMaxHealth()`.
- ImmortalBody integrates with armor systems (see [Armor documentation](../Armor.md)) and damage effects. Armor modifies incoming damage before it is applied to health, but the final health value is clamped to minimum 1.
- Damage states are calculated based on global thresholds defined in game data and affect visual appearance and particle systems.
- **ObjectReskin (Retail)**: ObjectReskin uses the same module system as Object. Adding ImmortalBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- Requires proper armor and damage type definitions to function correctly. ImmortalBody relies on armor systems to modify incoming damage before it is applied to health.
- Inherits all properties and functionality from ActiveBody while overriding the health change system to prevent death.
- Works with upgrade systems that modify maximum health.
- Compatible with veterancy system health scaling through global data health bonuses.
- Compatible with healing and repair systems that restore health over time.

## Properties

ImmortalBody inherits all properties from ActiveBody with no additional INI-parsable properties. All properties listed below are inherited from ActiveBody and function identically, except that health is prevented from dropping below 1 point.

### Health Settings

Available in: *(v1.04)* (Generals, Zero Hour)

#### `MaxHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Maximum health points the object can have. Higher values make objects more durable and resistant to damage. This determines the total damage capacity before destruction. If `InitialHealth` exceeds `MaxHealth`, the current health will be clamped to `MaxHealth` when the first health change occurs. Health is automatically clamped to `MaxHealth` as the upper limit and `1.0` as the lower limit (instead of `0.0` for ActiveBody) during damage and healing operations.
- **Default**: `0.0`
- **Example**: `MaxHealth = 500.0`

#### `InitialHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Starting health points when the object is created. Higher values allow objects to spawn with more health than their maximum, providing temporary damage buffer. Lower values spawn objects at reduced health. The initial health value is set directly during object creation; if it exceeds `MaxHealth`, it will be clamped to `MaxHealth` when the first health change occurs. Health cannot go below `1.0` (instead of `0.0` for ActiveBody).
- **Default**: `0.0`
- **Example**: `InitialHealth = 500.0`

### Subdual Damage Settings

Available in: *(v1.04)* (Zero Hour only)

#### `SubdualDamageCap`
Available in: *(v1.04)* (Zero Hour only)

- **Type**: `Real`
- **Description**: Maximum subdual damage that can accumulate before the object is subdued (disabled). Higher values allow objects to absorb more subdual damage before becoming incapacitated. At 0 (default), objects cannot be subdued (disabled). Subdual damage accumulates separately from normal health damage and can disable objects without destroying them. When subdual damage equals or exceeds `MaxHealth`, the object becomes subdued (disabled).
- **Default**: `0.0`
- **Example**: `SubdualDamageCap = 350.0`

#### `SubdualDamageHealRate`
Available in: *(v1.04)* (Zero Hour only)

- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Time interval between subdual damage healing attempts. Lower values heal subdual damage more frequently, while higher values heal less often. At 0 (default), no automatic subdual healing occurs. Subdual damage is healed automatically by the subdual damage helper system at the specified interval.
- **Default**: `0`
- **Example**: `SubdualDamageHealRate = 500`

#### `SubdualDamageHealAmount`
Available in: *(v1.04)* (Zero Hour only)

- **Type**: `Real`
- **Description**: Amount of subdual damage healed per healing interval. Higher values heal more subdual damage per tick, while lower values heal less. At 0 (default), no subdual healing occurs. This amount is subtracted from the current subdual damage each time the healing interval elapses.
- **Default**: `0.0`
- **Example**: `SubdualDamageHealAmount = 50.0`

## Enum Value Lists

#### `BodyDamageType` Values
Available in: *(v1.04)* (Generals, Zero Hour)

**Source:** [BodyModule.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/BodyModule.h#54) - `BodyDamageType` enum definition; string names from `TheBodyDamageTypeNames[]`

**Retail 1.04 Values** *(available in v1.04)*:

- **`PRISTINE`** *(v1.04)* - Unit should appear in pristine condition
- **`DAMAGED`** *(v1.04)* - Unit has been damaged
- **`REALLYDAMAGED`** *(v1.04)* - Unit is extremely damaged / nearly destroyed
- **`RUBBLE`** *(v1.04)* - Unit has been reduced to rubble/corpse/exploded-hulk, etc

## Examples

### Truly Immortal Hero Unit
```ini
Body = ImmortalBody ModuleTag_01
  MaxHealth = 1000.0
  InitialHealth = 1000.0
  SubdualDamageCap = 0.0
  SubdualDamageHealRate = 0
  SubdualDamageHealAmount = 0.0
End
```

### Indestructible Objective Marker
```ini
Body = ImmortalBody ModuleTag_02
  MaxHealth = 500.0
  InitialHealth = 500.0
  SubdualDamageCap = 0.0
  SubdualDamageHealRate = 0
  SubdualDamageHealAmount = 0.0
End
```

### Immortal Unit with Subdual Damage (Zero Hour only)
```ini
Body = ImmortalBody ModuleTag_03
  MaxHealth = 800.0
  InitialHealth = 800.0
  SubdualDamageCap = 700.0
  SubdualDamageHealRate = 500
  SubdualDamageHealAmount = 50.0
End
```

## Template

```ini
Body = ImmortalBody ModuleTag_XX
  MaxHealth = 0.0                 ; // maximum health points *(v1.04)*
  InitialHealth = 0.0             ; // starting health points *(v1.04)*

  SubdualDamageCap = 0.0          ; // maximum subdual damage before subdual *(v1.04, Zero Hour only)*
  SubdualDamageHealRate = 0       ; // milliseconds between subdual damage healing *(v1.04, Zero Hour only)*
  SubdualDamageHealAmount = 0.0   ; // amount of subdual damage healed per interval *(v1.04, Zero Hour only)*
End
```

## Notes

- ImmortalBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- Health is automatically clamped to minimum 1 point - objects can never die from any type of damage, regardless of how much damage is dealt. This is the key difference from ActiveBody.
- Damage states are calculated based on global thresholds and updated automatically when [MaxHealth](#maxhealth) changes.
- Subdual damage can disable objects without destroying them (Zero Hour only). Subdual damage accumulates separately from normal health damage and is limited by [SubdualDamageCap](#subdualdamagecap). Even when subdued, objects with ImmortalBody cannot die.
- Objects automatically heal subdual damage over time if [SubdualDamageHealRate](#subdualdamagehealrate) and [SubdualDamageHealAmount](#subdualdamagehealamount) are set. The healing is handled by helper systems that run at the specified intervals.
- Objects with ImmortalBody can be targeted by weapons and affected by damage types. Armor modifies incoming damage before it is applied to health, but the final health value is clamped to minimum 1.
- **MaxHealthUpgrade interactions**: When MaxHealthUpgrade modules are applied, they call `setMaxHealth()` which internally uses `internalChangeHealth()`. ImmortalBody's override ensures health never drops below 1 during upgrade application, regardless of the health change type (PRESERVE_RATIO, ADD_CURRENT_HEALTH_TOO, FULLY_HEAL, SAME_CURRENTHEALTH).
- **Veterancy interactions**: Veterancy level changes modify maximum health using `setMaxHealth()` with `PRESERVE_RATIO`, scaling health proportionally based on global veterancy health bonuses. ImmortalBody ensures objects remain at minimum 1 health even after veterancy scaling.
- **Healing and repair interactions**: Healing behaviors and repair systems call `attemptHealing()`, which uses `internalChangeHealth()`. ImmortalBody allows healing to restore health normally, but prevents health from ever falling below 1.
- **Script interactions**: Script actions can damage objects via `doNamedDamage()` and `doDamageTeamMembers()`, which eventually call `internalChangeHealth()`. ImmortalBody prevents script-based damage from killing objects. Scripts can also modify maximum health via object properties, which calls `setMaxHealth()`.
- **Player difficulty interactions**: Single-player difficulty settings modify object maximum health via `setMaxHealth()` with `PRESERVE_RATIO`. ImmortalBody's protection applies during difficulty-based health scaling.
- ImmortalBody integrates with armor systems and damage effects.
- Objects can still be removed through other means (like special powers or script commands that directly remove objects), but they cannot die from health loss through any damage, healing, upgrade, or veterancy system.
- Recommended for creating truly indestructible objectives, markers, or special hero units that must never die from combat damage, regardless of upgrades, veterancy, healing, or script interactions.

## Source Files

**Base Class:** [ActiveBody](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h) (Retail Zero Hour), [ActiveBody](../../Generals/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h) (Retail Generals)

- Header (Retail Zero Hour): [ImmortalBody.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/ImmortalBody.h)
- Source (Retail Zero Hour): [ImmortalBody.cpp](../../GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Body/ImmortalBody.cpp)
- Header (Retail Generals): [ImmortalBody.h](../../Generals/Code/GameEngine/Include/GameLogic/Module/ImmortalBody.h)
- Source (Retail Generals): [ImmortalBody.cpp](../../Generals/Code/GameEngine/Source/GameLogic/Object/Body/ImmortalBody.cpp)

## Changes History

- v1.04 — Adds ImmortalBody (prevents health from dropping below 1 point, making objects truly immortal).

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)

