# HighlanderBody

Status: AI-generated, 0/2 reviews

## Overview

The `HighlanderBody` module makes objects effectively unkillable by normal damage: their health is clamped to never drop below 1 point from resistable damage. Only unresistable damage can finish them. This is useful for special objectives or units that can only be destroyed by specific weapons. This is a module added inside `Object` entries.

Available in: *(v1.04)* (Generals, Zero Hour)

## Table of Contents

- [Overview](#overview)
- [Properties](#properties)
  - [Health Settings](#health-settings)
  - [Subdual Damage Settings](#subdual-damage-settings)
- [Enum Value Lists](#enum-value-lists)
- [Examples](#examples)
- [Usage](#usage)
- [Template](#template)
- [Notes](#notes)
- [Modder Recommended Use Scenarios](#modder-recommended-use-scenarios)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Properties

### Health Settings

Available in: *(v1.04)* (Generals, Zero Hour)

#### `MaxHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Maximum health points the object can have. Higher values make objects more durable and resistant to damage. This determines the total damage capacity before destruction. If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) during health operations. Health is automatically clamped to [MaxHealth](#maxhealth) as the upper limit and `0.0` as the lower limit during damage and healing operations.
- **Default**: `0.0`
- **Example**: `MaxHealth = 500.0`

#### `InitialHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Starting health points when the object is created. Lower values spawn objects at reduced health. If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the health will be clamped to [MaxHealth](#maxhealth) during health operations. Health cannot go below `1.0` for normal damage (or `0.0` for unresistable damage).
- **Default**: `0.0`
- **Example**: `InitialHealth = 500.0`

### Subdual Damage Settings

Available only in: *(v1.04, Zero Hour only)*

#### `SubdualDamageCap`
Available only in: *(v1.04, Zero Hour only)*

- **Type**: `Real`
- **Description**: Maximum subdual damage that can accumulate before the object is subdued (disabled). Higher values allow objects to absorb more subdual damage before becoming incapacitated. At 0 (default), objects cannot be subdued (disabled). Subdual damage accumulates separately from normal health damage and can disable objects without destroying them. When subdual damage equals or exceeds [MaxHealth](#maxhealth), the object becomes subdued (disabled). Only subdual damage types affect this cap: `SUBDUAL_MISSILE`, `SUBDUAL_VEHICLE`, `SUBDUAL_BUILDING`, and `SUBDUAL_UNRESISTABLE` (see [DamageType documentation](../DamageType.md)).
- **Default**: `0.0`
- **Example**: `SubdualDamageCap = 350.0`

#### `SubdualDamageHealRate`
Available only in: *(v1.04, Zero Hour only)*

- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Time interval between subdual damage healing attempts. Lower values heal subdual damage more frequently, while higher values heal less often. At 0 (default), no automatic subdual healing occurs. Subdual damage is healed automatically by the subdual damage helper system at the specified interval.
- **Default**: `0`
- **Example**: `SubdualDamageHealRate = 500`

#### `SubdualDamageHealAmount`
Available only in: *(v1.04, Zero Hour only)*

- **Type**: `Real`
- **Description**: Amount of subdual damage healed per healing interval. Higher values heal more subdual damage per tick, while lower values heal less. At 0 (default), no subdual healing occurs. This amount is subtracted from the current subdual damage each time the healing interval elapses.
- **Default**: `0.0`
- **Example**: `SubdualDamageHealAmount = 50.0`

## Enum Value Lists

#### `BodyDamageType` Values
Available in: *(v1.04)* (Generals, Zero Hour)

**Source:** `BodyModule.h` - `BodyDamageType` enum definition

Damage states are calculated based on health percentage thresholds defined in [GameData](../GameData.md) (typically `UnitDamagedThreshold` = 50-70% and `UnitReallyDamagedThreshold` = 10-35%):

- **`PRISTINE`** *(v1.04)* - Unit appears in pristine condition (health > `UnitDamagedThreshold`, typically > 50-70%)
- **`DAMAGED`** *(v1.04)* - Unit has been damaged (health between `UnitReallyDamagedThreshold` and `UnitDamagedThreshold`, typically 10-35% to 50-70%)
- **`REALLYDAMAGED`** *(v1.04)* - Unit is extremely damaged / nearly destroyed (health between 0% and `UnitReallyDamagedThreshold`, typically 0% to 10-35%)
- **`RUBBLE`** *(v1.04)* - Unit has been reduced to rubble/corpse/exploded-hulk (health = 0%)

## Examples

```ini
Body = HighlanderBody ModuleTag_01
  MaxHealth = 1000.0
  InitialHealth = 1000.0
End
```

```ini
Body = HighlanderBody ModuleTag_02
  MaxHealth = 400.0
  InitialHealth = 400.0
  SubdualDamageCap = 350.0
  SubdualDamageHealRate = 500
  SubdualDamageHealAmount = 50.0
End
```

```ini
Body = HighlanderBody ModuleTag_03
  MaxHealth = 200.0
  InitialHealth = 200.0
End
```

## Usage

Place under `Body = HighlanderBody ModuleTag_XX` inside [Object](../Object.md) entries. HighlanderBody can only be added to [Object](../Object.md) entries in Retail. See [Template](#template) for correct syntax.

**Placement**:
- **Retail**: HighlanderBody can only be added to `Object` entries.

Only one body module (ActiveBody, InactiveBody, StructureBody, HighlanderBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- HighlanderBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values)) based on health percentage thresholds defined in [GameData](../GameData.md). Damage states affect visual appearance and particle systems.
- HighlanderBody prevents death from normal damage types by clamping health to a minimum of 1 point. Only unresistable damage can reduce health below 1 point and cause death.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the health will be clamped to [MaxHealth](#maxhealth) during health operations. Health cannot go below `1.0` for normal damage (or `0.0` for unresistable damage).
- [SubdualDamageCap](#subdualdamagecap) can disable objects without destroying them when subdual damage equals or exceeds [MaxHealth](#maxhealth). Subdual damage properties are available only in Zero Hour.
- Objects automatically heal subdual damage over time if healing properties are set. The healing is handled by helper systems that run at the specified intervals.

**Conditions**:
- Objects with HighlanderBody can be targeted by [Weapon](../Weapon.md) attacks and affected by [DamageType](../DamageType.md). Health is reduced when weapons deal damage, but normal damage cannot reduce health below 1 point. Only unresistable damage can kill HighlanderBody objects.
- Veterancy levels can modify maximum health and healing rates through the veterancy bonus system (see [GameData](../GameData.md) for veterancy health bonuses).
- **ObjectReskin (Retail)**: ObjectReskin uses the same module system as [Object](../Object.md). Adding HighlanderBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- Requires proper [Armor](../Armor.md) and [DamageType](../DamageType.md) definitions to function correctly. HighlanderBody relies on armor systems to modify incoming damage before it is applied to health. The unresistable damage type must be properly defined for the death mechanism to work.
- Objects with HighlanderBody can be healed by [AutoHealBehavior](../ObjectBehaviorsModules/AutoHealBehavior.md). AutoHealBehavior heals main health of objects with HighlanderBody.

## Template

```ini
Body = HighlanderBody ModuleTag_XX
  MaxHealth = 0.0                 ; // maximum health points *(v1.04)*
  InitialHealth = 0.0             ; // starting health points *(v1.04)*

  SubdualDamageCap = 0.0          ; // maximum subdual damage before subdual *(v1.04, Zero Hour only)*
  SubdualDamageHealRate = 0       ; // milliseconds between subdual healing *(v1.04, Zero Hour only)*
  SubdualDamageHealAmount = 0.0   ; // subdual damage healed per interval *(v1.04, Zero Hour only)*
End
```

## Notes

- Damage states ([BodyDamageType Values](#bodydamagetype-values)) are updated automatically when [MaxHealth](#maxhealth) changes.
- The name references the "Highlander" concept of near-immortality ("there can be only one"). Health is clamped to minimum 1 point for normal damage types, but unresistable damage can reduce health below 1 and cause death.
- Subdual damage types that affect this system: `SUBDUAL_MISSILE`, `SUBDUAL_VEHICLE`, `SUBDUAL_BUILDING`, and `SUBDUAL_UNRESISTABLE` (see [DamageType documentation](../DamageType.md)).

## Modder Recommended Use Scenarios

- HighlanderBody is commonly used by TreeStumps objects, some laser objects, and bomb-like objects such as C4Charge.


## Source Files

**Base Class:** [ActiveBody](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h) (Retail Zero Hour), [ActiveBody](../../Generals/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h) (Retail Generals)

- Header (Retail Zero Hour): [HighlanderBody.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/HighlanderBody.h)
- Source (Retail Zero Hour): [HighlanderBody.cpp](../../GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Body/HighlanderBody.cpp)
- Header (Retail Generals): [HighlanderBody.h](../../Generals/Code/GameEngine/Include/GameLogic/Module/HighlanderBody.h)
- Source (Retail Generals): [HighlanderBody.cpp](../../Generals/Code/GameEngine/Source/GameLogic/Object/Body/HighlanderBody.cpp)

## Changes History

*(No changes - HighlanderBody exists in Retail 1.04)*

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)


