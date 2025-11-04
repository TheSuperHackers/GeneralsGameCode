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

- `PRISTINE` *(v1.04)* - Unit should appear in pristine condition
- `DAMAGED` *(v1.04)* - Unit has been damaged
- `REALLYDAMAGED` *(v1.04)* - Unit is extremely damaged / nearly destroyed
- `RUBBLE` *(v1.04)* - Unit has been reduced to rubble/corpse/exploded-hulk, etc

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

Place under `Body = HighlanderBody ModuleTag_XX` inside [Object](../Object.md) entries. See [Template](#template) for correct syntax.

Only one body module (ActiveBody, InactiveBody, StructureBody, HighlanderBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- HighlanderBody prevents death from normal damage types by clamping health to a minimum of 1 point. Only unresistable damage can reduce health below 1 point and cause death.
- ActiveBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) when the first health change occurs. Health cannot go below `1.0` for normal damage (or `0.0` for unresistable damage).
- [SubdualDamageCap](#subdualdamagecap) can disable objects without destroying them when subdual damage equals or exceeds [MaxHealth](#maxhealth). Subdual damage properties are available only in Zero Hour.
- Objects automatically heal subdual damage over time if [SubdualDamageHealRate](#subdualdamagehealrate) and [SubdualDamageHealAmount](#subdualdamagehealamount) are set. The healing is handled by helper systems that run at the specified intervals.

**Conditions**:
- Objects with HighlanderBody can be targeted by weapons (see [Weapon documentation](../Weapon.md)) and affected by damage types. Health is reduced when weapons deal damage, but normal damage cannot reduce health below 1 point. Only unresistable damage can kill HighlanderBody objects.
- Veterancy levels can modify maximum health and healing rates through upgrade systems.
- HighlanderBody integrates with armor systems (see [Armor documentation](../Armor.md)) and damage effects. Armor modifies incoming damage before it is applied to health.
- Damage states are calculated based on global thresholds defined in game data and affect visual appearance and particle systems. Objects can still show damage states even though they cannot die from normal damage.

**Dependencies**:
- Requires proper armor and damage type definitions to function correctly. HighlanderBody relies on armor systems to modify incoming damage before it is applied to health. The unresistable damage type must be properly defined for the death mechanism to work.

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

- HighlanderBody objects can take damage normally but cannot die from normal damage types. Health is automatically clamped to minimum 1 point for normal damage.
- Only unresistable damage can reduce health below 1 point and cause death. This creates "immortal" objects that can only be destroyed by special weapons or abilities.
- All damage calculations, armor, and status effects work normally. Objects still show damage states ([BodyDamageType Values](#bodydamagetype-values)) and visual effects even though they cannot die from normal damage.
- HighlanderBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- Damage states are calculated based on global thresholds and updated automatically when [MaxHealth](#maxhealth) changes.
- Subdual damage can disable objects without destroying them (Zero Hour only). Subdual damage accumulates separately from normal health damage and is limited by [SubdualDamageCap](#subdualdamagecap).
- Objects automatically heal subdual damage over time if [SubdualDamageHealRate](#subdualdamagehealrate) and [SubdualDamageHealAmount](#subdualdamagehealamount) are set. The healing is handled by helper systems that run at the specified intervals.
- Objects with HighlanderBody can be targeted by weapons (see [Weapon documentation](../Weapon.md)) and affected by damage types. Armor modifies incoming damage before it is applied to health.
- Veterancy levels can modify maximum health and healing rates through upgrade systems.
- HighlanderBody integrates with armor systems (see [Armor documentation](../Armor.md)) and damage effects.
- Useful for creating hero units, special objectives, or indestructible objects that require special weapons to destroy.
- The name references the "Highlander" concept of near-immortality ("there can be only one").

## Modder Recommended Use Scenarios

(pending modder review)

## Source Files

**Base Class:** `ActiveBody`

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


