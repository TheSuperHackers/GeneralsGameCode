# ActiveBody

Status: AI-generated, 0/2 reviews

## Overview

The `ActiveBody` module manages health and damage systems for objects that can take damage, be healed, and be destroyed through combat. It handles health tracking, damage states, healing, subdual damage (Zero Hour only), and visual damage effects. Objects with ActiveBody can be damaged by weapons, healed by repair systems, and transition through different damage states that affect their appearance and functionality. This is a module added inside `Object` entries.

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
- **Description**: Maximum health points the object can have. Higher values make objects more durable and resistant to damage. This determines the total damage capacity before destruction. If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) when the first health change occurs. Health is automatically clamped to [MaxHealth](#maxhealth) as the upper limit and `0.0` as the lower limit during damage and healing operations.
- **Default**: `0.0`
- **Example**: `MaxHealth = 500.0`

#### `InitialHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Starting health points when the object is created. Higher values allow objects to spawn with more health than their maximum, providing temporary damage buffer. Lower values spawn objects at reduced health. The initial health value is set directly during object creation; if it exceeds [MaxHealth](#maxhealth), it will be clamped to [MaxHealth](#maxhealth) when the first health change occurs. Health cannot go below `0.0`.
- **Default**: `0.0`
- **Example**: `InitialHealth = 500.0`

### Subdual Damage Settings

Available only in: *(v1.04, Zero Hour only)*

#### `SubdualDamageCap`
Available only in: *(v1.04, Zero Hour only)*

- **Type**: `Real`
- **Description**: Maximum subdual damage that can accumulate before the object is subdued (disabled). Higher values allow objects to absorb more subdual damage before becoming incapacitated. At 0 (default), objects cannot be subdued (disabled). Subdual damage accumulates separately from normal health damage and can disable objects without destroying them. When subdual damage equals or exceeds [MaxHealth](#maxhealth), the object becomes subdued (disabled).
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

- **`PRISTINE`** *(v1.04)* - Unit should appear in pristine condition
- **`DAMAGED`** *(v1.04)* - Unit has been damaged
- **`REALLYDAMAGED`** *(v1.04)* - Unit is extremely damaged / nearly destroyed
- **`RUBBLE`** *(v1.04)* - Unit has been reduced to rubble/corpse/exploded-hulk, etc

## Examples

```ini
Body = ActiveBody ModuleTag_02
  MaxHealth = 200.0
  InitialHealth = 200.0
  SubdualDamageCap = 350.0
  SubdualDamageHealRate = 500
  SubdualDamageHealAmount = 50.0
End
```

```ini
Body = ActiveBody ModuleTag_02
  MaxHealth = 75.0
  InitialHealth = 75.0
End
```

```ini
Body = ActiveBody ModuleTag_02
  MaxHealth = 400
  InitialHealth = 400
  SubdualDamageCap = 700
  SubdualDamageHealRate = 500
  SubdualDamageHealAmount = 50
End
```

```ini
Body = ActiveBody ModuleTag_02
  MaxHealth = 200
  InitialHealth = 200
  SubdualDamageCap = 350
  SubdualDamageHealRate = 500
  SubdualDamageHealAmount = 50
End
```

## Usage

Place under `Body = ActiveBody ModuleTag_XX` inside [Object](../Object.md) entries. ActiveBody can only be added to [Object](../Object.md) entries in Retail (ObjectExtend does not exist in Retail). See [Template](#template) for correct syntax.

Only one body module (ActiveBody, InactiveBody, StructureBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- ActiveBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) when the first health change occurs. Health cannot go below `0.0` or above [MaxHealth](#maxhealth).
- [SubdualDamageCap](#subdualdamagecap) can disable objects without destroying them when subdual damage equals or exceeds [MaxHealth](#maxhealth). Subdual damage properties are available only in Zero Hour.
- Objects automatically heal subdual damage over time if [SubdualDamageHealRate](#subdualdamagehealrate) and [SubdualDamageHealAmount](#subdualdamagehealamount) are set. The healing is handled by helper systems that run at the specified intervals.

**Conditions**:
- Objects with [ActiveBody](#overview) can be targeted by [Weapon](../Weapon.md) attacks and affected by [DamageType](../DamageType.md). Health is reduced when weapons deal damage, and damage states are updated based on health percentage.
- Veterancy levels can modify maximum health and healing rates through upgrade systems.
- ActiveBody integrates with [Armor](../Armor.md) systems and damage effects. Armor modifies incoming damage before it is applied to health.
- Damage states are calculated based on global thresholds defined in [GameData](../GameData.md) and affect visual appearance and particle systems.
- **ObjectReskin**: [ObjectReskin](../ObjectReskin.md) uses the same module system as [Object](../Object.md). Adding ActiveBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- Requires proper [Armor](../Armor.md) and [DamageType](../DamageType.md) definitions to function correctly. ActiveBody relies on armor systems to modify incoming damage before it is applied to health.

## Template

```ini
Body = ActiveBody ModuleTag_XX
  MaxHealth = 0.0                 ; // maximum health points *(v1.04)*
  InitialHealth = 0.0             ; // starting health points *(v1.04)*

  SubdualDamageCap = 0.0          ; // maximum subdual damage before subdual *(v1.04, Zero Hour only)*
  SubdualDamageHealRate = 0       ; // milliseconds between subdual damage healing *(v1.04, Zero Hour only)*
  SubdualDamageHealAmount = 0.0   ; // amount of subdual damage healed per interval *(v1.04, Zero Hour only)*
End
```

## Notes

- ActiveBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- Damage states are calculated based on global thresholds and updated automatically when [MaxHealth](#maxhealth) changes.
- Subdual damage can disable objects without destroying them (Zero Hour only). Subdual damage accumulates separately from normal health damage and is limited by [SubdualDamageCap](#subdualdamagecap).
- Objects automatically heal subdual damage over time if [SubdualDamageHealRate](#subdualdamagehealrate) and [SubdualDamageHealAmount](#subdualdamagehealamount) are set. The healing is handled by helper systems that run at the specified intervals.
- Objects with [ActiveBody](#overview) can be targeted by weapons and affected by damage types. Armor modifies incoming damage before it is applied to health.
- Veterancy levels can modify maximum health and healing rates through upgrade systems.
- ActiveBody integrates with armor systems and damage effects.

## Modder Recommended Use Scenarios

(pending modder review)

## Source Files

**Base Class:** `BodyModule`

- Header (Retail Zero Hour): [ActiveBody.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h)
- Source (Retail Zero Hour): [ActiveBody.cpp](../../GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Body/ActiveBody.cpp)
- Header (Retail Generals): [ActiveBody.h](../../Generals/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h)
- Source (Retail Generals): [ActiveBody.cpp](../../Generals/Code/GameEngine/Source/GameLogic/Object/Body/ActiveBody.cpp)

## Changes History

*(No changes - ActiveBody exists in Retail 1.04)*

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)

