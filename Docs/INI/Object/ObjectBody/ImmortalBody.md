# ImmortalBody

Status: AI-generated, 0/2 reviews

## Overview

The `ImmortalBody` module is a specialized body module that prevents objects from ever dying by ensuring their health never drops below 1 point. Unlike ActiveBody which allows objects to die when health reaches 0, ImmortalBody prevents health from falling below 1, making objects truly immortal. Objects can still take damage, show damage states, and be affected by all other body systems, but they can never be destroyed through health loss. This is a module added inside `Object` entries.

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

Place under `Body = ImmortalBody ModuleTag_XX` inside [Object](../Object.md) entries. ImmortalBody can only be added to [Object](../Object.md) entries in Retail. See [Template](#template) for correct syntax.

**Placement**:
- **Retail**: ImmortalBody can only be added to `Object` entries.

Only one body module (ActiveBody, ImmortalBody, StructureBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- ImmortalBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values)) based on health percentage thresholds defined in [GameData](../GameData.md). Damage states affect visual appearance and particle systems.
- Health is automatically clamped to minimum 1 point - objects can never die from any type of damage, regardless of how much damage is dealt.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the health will be clamped to [MaxHealth](#maxhealth) during health operations. Health cannot go below `1.0` or above [MaxHealth](#maxhealth).
- [SubdualDamageCap](#subdualdamagecap) can disable objects without destroying them when subdual damage equals or exceeds [MaxHealth](#maxhealth). Subdual damage properties are available only in Zero Hour.

**Conditions**:
- Objects with ImmortalBody can be targeted by [Weapon](../Weapon.md) attacks and affected by [DamageType](../DamageType.md). Health is reduced when weapons deal damage, but cannot fall below 1 point.
- When upgrades are applied that increase maximum health, ImmortalBody ensures health never drops below 1 point, even when health changes occur during upgrade application.
- When objects gain veterancy levels (Veteran, Elite, Heroic), veterancy bonuses modify maximum health and scale health proportionally. ImmortalBody's health protection ensures objects remain at minimum 1 health even after veterancy changes.
- In single-player games, difficulty settings modify object maximum health. ImmortalBody's protection applies during these scaling operations.
- Healing behaviors and repair systems can heal ImmortalBody objects normally. ImmortalBody allows healing to work normally, but ensures health never goes below 1 point.
- Scripts can damage ImmortalBody objects, but ImmortalBody prevents script-based damage from killing objects. Scripts can also modify maximum health, and ImmortalBody's protection applies to these changes.
- **ObjectReskin (Retail)**: ObjectReskin uses the same module system as [Object](../Object.md). Adding ImmortalBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- Requires proper [Armor](../Armor.md) and [DamageType](../DamageType.md) definitions to function correctly. ImmortalBody relies on armor systems to modify incoming damage before it is applied to health.
- Has all the same properties as [ActiveBody](ActiveBody.md) while preventing health from falling below 1 point.
- Objects with ImmortalBody can be healed by [AutoHealBehavior](../ObjectBehaviorsModules/AutoHealBehavior.md). AutoHealBehavior heals main health of objects with ImmortalBody. Even though ImmortalBody prevents death, healing still increases health up to [MaxHealth](#maxhealth).

## Properties

ImmortalBody has all the same properties as [ActiveBody](ActiveBody.md) with no additional INI-parsable properties. All properties listed below function identically to ActiveBody, except that health is prevented from dropping below 1 point.

### Health Settings

Available in: *(v1.04)* (Generals, Zero Hour)

#### `MaxHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Maximum health points the object can have. Higher values make objects more durable and resistant to damage. This determines the total damage capacity before destruction. If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) during health operations. Health is automatically clamped to [MaxHealth](#maxhealth) as the upper limit and `1.0` as the lower limit (instead of `0.0` for ActiveBody) during damage and healing operations.
- **Default**: `0.0`
- **Example**: `MaxHealth = 500.0`

#### `InitialHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Starting health points when the object is created. Lower values spawn objects at reduced health. If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the health will be clamped to [MaxHealth](#maxhealth) during health operations. Health cannot go below `1.0` for normal damage (or `0.0` for unresistable damage) (instead of `0.0` for ActiveBody).
- **Default**: `0.0`
- **Example**: `InitialHealth = 500.0`

### Subdual Damage Settings

Available in: *(v1.04)* (Zero Hour only)

#### `SubdualDamageCap`
Available in: *(v1.04)* (Zero Hour only)

- **Type**: `Real`
- **Description**: Maximum subdual damage that can accumulate before the object is subdued (disabled). Higher values allow objects to absorb more subdual damage before becoming incapacitated. At 0 (default), objects cannot be subdued (disabled). Subdual damage accumulates separately from normal health damage and can disable objects without destroying them. When subdual damage equals or exceeds [MaxHealth](#maxhealth), the object becomes subdued (disabled). Only subdual damage types affect this cap: `SUBDUAL_MISSILE`, `SUBDUAL_VEHICLE`, `SUBDUAL_BUILDING`, and `SUBDUAL_UNRESISTABLE` (see [DamageType documentation](../DamageType.md)).
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

Damage states are calculated based on health percentage thresholds defined in [GameData](../GameData.md) (typically `UnitDamagedThreshold` = 50-70% and `UnitReallyDamagedThreshold` = 10-35%):

- **`PRISTINE`** *(v1.04)* - Unit appears in pristine condition (health > `UnitDamagedThreshold`, typically > 50-70%)
- **`DAMAGED`** *(v1.04)* - Unit has been damaged (health between `UnitReallyDamagedThreshold` and `UnitDamagedThreshold`, typically 10-35% to 50-70%)
- **`REALLYDAMAGED`** *(v1.04)* - Unit is extremely damaged / nearly destroyed (health between 0% and `UnitReallyDamagedThreshold`, typically 0% to 10-35%)
- **`RUBBLE`** *(v1.04)* - Unit has been reduced to rubble/corpse/exploded-hulk (health = 0%)

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

- Damage states ([BodyDamageType Values](#bodydamagetype-values)) are updated automatically when [MaxHealth](#maxhealth) changes.
- Health is automatically clamped to minimum 1 point for all damage types. Objects can take damage normally but cannot die from it, regardless of how much damage is dealt.
- All damage calculations, armor, and status effects work normally. Objects still show damage states and visual effects based on health percentage.
- Subdual damage types that affect this system: `SUBDUAL_MISSILE`, `SUBDUAL_VEHICLE`, `SUBDUAL_BUILDING`, and `SUBDUAL_UNRESISTABLE` (see [DamageType documentation](../DamageType.md)).

## Modder Recommended Use Scenarios

- ImmortalBody is commonly used by neutral objects such as SupplyDock and SupplyPileSmall.

(pending modder review)

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

