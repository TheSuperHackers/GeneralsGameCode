# StructureBody

Status: AI-generated, 0/2 reviews

## Overview

The `StructureBody` module extends ActiveBody with additional functionality for structures, including constructor object tracking. It manages health, damage states, healing, subdual damage (Zero Hour only), and visual damage effects like ActiveBody, but also tracks which object built the structure. This allows structures to maintain relationships with their builders for gameplay systems like AI, player events, and script interactions. Objects with StructureBody can be damaged by weapons, healed by repair systems, and transition through different damage states that affect their appearance and functionality. This is a module added inside `Object` entries.

Available in: *(v1.04)* (Generals, Zero Hour)

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
  - [Limitations](#limitations)
  - [Conditions](#conditions)
  - [Dependencies](#dependencies)
- [Properties](#properties)
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

Used by structures that need active health management and constructor tracking, such as buildings, defensive structures, and constructed objects that need to maintain relationships with their builders. This is a **body module** that must be embedded within object definitions. Use the [Template](#template) below by copying it into your object definition. Then, customize it as needed, making sure to review any limitations, conditions, or dependencies related to its usage.

**Placement**:
- **Retail**: StructureBody can only be added to `Object` entries (ObjectExtend does not exist in Retail).

Only one body module (ActiveBody, InactiveBody, StructureBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- StructureBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) when the first health change occurs. Health cannot go below `0.0` or above [MaxHealth](#maxhealth).
- [SubdualDamageCap](#subdualdamagecap) can disable objects without destroying them when subdual damage equals or exceeds [MaxHealth](#maxhealth). Subdual damage properties are available only in Zero Hour.
- Objects automatically heal subdual damage over time if [SubdualDamageHealRate](#subdualdamagehealrate) and [SubdualDamageHealAmount](#subdualdamagehealamount) are set. The healing is handled by helper systems that run at the specified intervals.
- Constructor tracking is limited to a single constructor object ID. If the constructor object is destroyed or invalid, the constructor ID remains set but may reference an invalid object.

**Conditions**:
- Objects with StructureBody can be targeted by weapons (see [Weapon documentation](../Weapon.md)) and affected by damage types. Health is reduced when weapons deal damage, and damage states are updated based on health percentage.
- Veterancy levels can modify maximum health and healing rates through upgrade systems.
- StructureBody integrates with armor systems (see [Armor documentation](../Armor.md)) and damage effects. Armor modifies incoming damage before it is applied to health.
- Damage states are calculated based on global thresholds defined in game data and affect visual appearance and particle systems.
- **Constructor tracking**: StructureBody tracks which object built the structure. The constructor object is set automatically when structures are built via build systems. The constructor object ID can be retrieved and is used by gameplay systems for AI decisions, player events, and script interactions. If the constructor object is destroyed, the constructor ID remains set but may reference an invalid object.
- **ObjectReskin (Retail)**: ObjectReskin uses the same module system as Object. Adding StructureBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement like ObjectExtend.

**Dependencies**:
- Requires proper armor and damage type definitions to function correctly. StructureBody relies on armor systems to modify incoming damage before it is applied to health.
- Inherits all properties and functionality from [ActiveBody](ActiveBody.md).

## Properties

StructureBody inherits all properties from [ActiveBody](ActiveBody.md) with no additional INI-parsable properties. The constructor object tracking is handled internally and set automatically when structures are built. See [ActiveBody](ActiveBody.md) for complete property list.

### Health Settings

Available in: *(v1.04)* (Generals, Zero Hour)

#### `MaxHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Maximum health points the object can have. Higher values make objects more durable and resistant to damage. This determines the total damage capacity before destruction. If `InitialHealth` exceeds `MaxHealth`, the current health will be clamped to `MaxHealth` when the first health change occurs. Health is automatically clamped to `MaxHealth` as the upper limit and `0.0` as the lower limit during damage and healing operations.
- **Default**: `0.0`
- **Example**: `MaxHealth = 500.0`

#### `InitialHealth`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `Real`
- **Description**: Starting health points when the object is created. Higher values allow objects to spawn with more health than their maximum, providing temporary damage buffer. Lower values spawn objects at reduced health. The initial health value is set directly during object creation; if it exceeds `MaxHealth`, it will be clamped to `MaxHealth` when the first health change occurs. Health cannot go below `0.0`.
- **Default**: `0.0`
- **Example**: `InitialHealth = 500.0`

### Subdual Damage Settings

Available in: *(v1.04)* (Zero Hour)

#### `SubdualDamageCap`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Maximum subdual damage that can accumulate before the object is subdued (disabled). Higher values allow objects to absorb more subdual damage before becoming incapacitated. At 0 (default), objects cannot be subdued (disabled). Subdual damage accumulates separately from normal health damage and can disable objects without destroying them. When subdual damage equals or exceeds `MaxHealth`, the object becomes subdued (disabled).
- **Default**: `0.0`
- **Example**: `SubdualDamageCap = 350.0`

#### `SubdualDamageHealRate`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Time interval between subdual damage healing attempts. Lower values heal subdual damage more frequently, while higher values heal less often. At 0 (default), no automatic subdual healing occurs. Subdual damage is healed automatically by the subdual damage helper system at the specified interval.
- **Default**: `0`
- **Example**: `SubdualDamageHealRate = 500`

#### `SubdualDamageHealAmount`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Amount of subdual damage healed per healing interval. Higher values heal more subdual damage per tick, while lower values heal less. At 0 (default), no subdual healing occurs. This amount is subtracted from the current subdual damage each time the healing interval elapses.
- **Default**: `0.0`
- **Example**: `SubdualDamageHealAmount = 50.0`

## Enum Value Lists

#### `BodyDamageType` Values
Available in: *(v1.04)* (Generals, Zero Hour)

**Source:** [BodyModule.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/BodyModule.h#54) - `BodyDamageType` enum definition; string names from `TheBodyDamageTypeNames[]`

**Retail 1.04 Values** *(available in Retail Generals 1.04, Retail Zero Hour 1.04)*:

- **`PRISTINE`** *(v1.04)* - Unit should appear in pristine condition
- **`DAMAGED`** *(v1.04)* - Unit has been damaged
- **`REALLYDAMAGED`** *(v1.04)* - Unit is extremely damaged / nearly destroyed
- **`RUBBLE`** *(v1.04)* - Unit has been reduced to rubble/corpse/exploded-hulk, etc

## Examples

### Standard Structure Body
```ini
Body = StructureBody ModuleTag_Structure
  MaxHealth = 500.0
  InitialHealth = 500.0
End
```

### Defensive Structure with Subdual Damage
```ini
Body = StructureBody ModuleTag_Defense
  MaxHealth = 1000.0
  InitialHealth = 1000.0
  SubdualDamageCap = 350.0
  SubdualDamageHealRate = 500
  SubdualDamageHealAmount = 50.0
End
```

### Production Structure
```ini
Body = StructureBody ModuleTag_Production
  MaxHealth = 800.0
  InitialHealth = 800.0
End
```

## Template

```ini
Body = StructureBody ModuleTag_XX
  MaxHealth = 0.0                 ; // maximum health points *(v1.04)*
  InitialHealth = 0.0             ; // starting health points *(v1.04)*

  SubdualDamageCap = 0.0          ; // maximum subdual damage before subdual *(v1.04, Zero Hour only)*
  SubdualDamageHealRate = 0       ; // milliseconds between subdual damage healing *(v1.04, Zero Hour only)*
  SubdualDamageHealAmount = 0.0   ; // amount of subdual damage healed per interval *(v1.04, Zero Hour only)*
End
```

## Notes

- StructureBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- Damage states are calculated based on global thresholds and updated automatically when [MaxHealth](#maxhealth) changes.
- Subdual damage can disable objects without destroying them (Zero Hour only). Subdual damage accumulates separately from normal health damage and is limited by [SubdualDamageCap](#subdualdamagecap).
- Objects automatically heal subdual damage over time if [SubdualDamageHealRate](#subdualdamagehealrate) and [SubdualDamageHealAmount](#subdualdamagehealamount) are set. The healing is handled by helper systems that run at the specified intervals.
- Objects with StructureBody can be targeted by weapons (see [Weapon documentation](../Weapon.md)) and affected by damage types. Armor modifies incoming damage before it is applied to health.
- Veterancy levels can modify maximum health and healing rates through upgrade systems.
- StructureBody integrates with armor systems (see [Armor documentation](../Armor.md)) and damage effects.
- StructureBody tracks which object built the structure. The constructor object is set automatically when structures are built via build systems. The constructor object ID can be retrieved and is used by gameplay systems for AI decisions, player events, and script interactions. If the constructor object is destroyed, the constructor ID remains set but may reference an invalid object.
- Commonly used for buildings, defensive structures, and constructed objects that need builder tracking.
- Only one body module is allowed per object; multiple bodies cause a startup assertion.

## Source Files

**Base Class:** [ActiveBody](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h) (Retail Zero Hour), [ActiveBody](../../Generals/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h) (Retail Generals)

- Header (Retail Zero Hour): [StructureBody.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/StructureBody.h)
- Source (Retail Zero Hour): [StructureBody.cpp](../../GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Body/StructureBody.cpp)
- Header (Retail Generals): [StructureBody.h](../../Generals/Code/GameEngine/Include/GameLogic/Module/StructureBody.h)
- Source (Retail Generals): [StructureBody.cpp](../../Generals/Code/GameEngine/Source/GameLogic/Object/Body/StructureBody.cpp)

## Changes History

- v1.04 — Adds StructureBody (active body system for structures with constructor tracking).

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)

