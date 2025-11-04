# HiveStructureBody

Status: AI-generated, 0/2 reviews

## Overview

`HiveStructureBody` redirects specified damage types from the structure to its slave objects (created by SpawnBehavior) or to contained objects (via ContainModule) when they are present. If no slaves/contained objects exist, it can optionally absorb (swallow) certain damage types instead of taking them normally.

Available in: *(v1.04)* (Generals, Zero Hour)

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
  - [Limitations](#limitations)
  - [Conditions](#conditions)
  - [Dependencies](#dependencies)
- [Properties](#properties)
  - [Damage Propagation](#damage-propagation)
- [Examples](#examples)
- [Template](#template)
- [Notes](#notes)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

Place under `Body = HiveStructureBody ModuleTag_XX` inside [Object](../Object.md) entries. HiveStructureBody can only be added to [Object](../Object.md) entries in Retail (ObjectExtend does not exist in Retail). See [Template](#template) for correct syntax.

Only one body module (ActiveBody, InactiveBody, StructureBody, ImmortalBody, HiveStructureBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- Requires [SpawnBehavior](../ObjectBehaviorsModules/SpawnBehavior.md) or [ContainModule](../ObjectModules/ContainModule.md) to redirect damage.
- Only redirects or swallows the damage types listed; other types behave normally.

**Conditions**:
- Always active once added to an object.
- Damage redirection requires at least one slave/contained object; otherwise, swallow rules may apply.
- Objects with HiveStructureBody can be targeted by [Weapon](../Weapon.md); armor (see [Armor](../Armor.md)) applies before redirection/swallowing.
- **ObjectReskin (Retail)**: Adding a body module with the same `ModuleTag` name causes a duplicate-module error (no automatic replacement).

**Dependencies**:
- [SpawnBehavior](../ObjectBehaviorsModules/SpawnBehavior.md) and/or [ContainModule](../ObjectModules/ContainModule.md) to supply slaves/contained objects.
- Valid damage type names (see [DamageType](../DamageType.md)).
- Inherits health/visual-state behavior from [StructureBody](./StructureBody.md) / [ActiveBody](./ActiveBody.md).

## Properties

### Damage Propagation

#### `PropagateDamageTypesToSlavesWhenExisting`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `DamageTypeFlags` (see [DamageType](../DamageType.md))
- **Description**: Redirect these damage types to the closest slave/contained object when present. Empty means no redirection. Use valid damage type names; invalid names will fail to parse.
- **Default**: `0` (none)
- **Example**: `PropagateDamageTypesToSlavesWhenExisting = EXPLOSION BALLISTIC`

#### `SwallowDamageTypesIfSlavesNotExisting`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `DamageTypeFlags` (see [DamageType](../DamageType.md))
- **Description**: When no slaves/contained objects exist, completely absorb these damage types (no effect on the structure). Empty means damage is taken normally. Use valid damage type names; invalid names will fail to parse.
- **Default**: `0` (none)
- **Example**: `SwallowDamageTypesIfSlavesNotExisting = EXPLOSION`

## Examples

```ini
Body = HiveStructureBody ModuleTag_01
  MaxHealth = 2000.0
  InitialHealth = 2000.0

  PropagateDamageTypesToSlavesWhenExisting = EXPLOSION BALLISTIC
  SwallowDamageTypesIfSlavesNotExisting = EXPLOSION
End
```

```ini
Body = HiveStructureBody ModuleTag_Transport
  MaxHealth = 1500.0
  InitialHealth = 1500.0

  PropagateDamageTypesToSlavesWhenExisting = BALLISTIC
  SwallowDamageTypesIfSlavesNotExisting = 0
End
```

## Template

```ini
Body = HiveStructureBody ModuleTag_XX
  MaxHealth = 100.0                       ; // maximum health *(v1.04)*
  InitialHealth = 100.0                   ; // starting health *(v1.04)*

  PropagateDamageTypesToSlavesWhenExisting = 0  ; // redirect these types to slaves/contained *(v1.04)*
  SwallowDamageTypesIfSlavesNotExisting = 0     ; // absorb these types if no slaves/contained *(v1.04)*
End
```

## Notes

- Redirection chooses the closest eligible slave/contained object for the listed damage types.
- Swallowing applies only when there are no available slave/contained objects.
- Armor and resistances apply before redirection/swallowing.
- Use valid [DamageType](../DamageType.md) names; invalid names will abort parsing.
- Only one body module is allowed per object; multiple bodies cause a startup assertion.
- Recommended for hive-like structures protected by spawned drones or garrisoned units.

## Source Files

**Base Class:** [StructureBody](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/StructureBody.h) (Retail Zero Hour), [StructureBody](../../Generals/Code/GameEngine/Include/GameLogic/Module/StructureBody.h) (Retail Generals)

- Header (Retail Zero Hour): [HiveStructureBody.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/HiveStructureBody.h)
- Source (Retail Zero Hour): [HiveStructureBody.cpp](../../GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Body/HiveStructureBody.cpp)
- Header (Retail Generals): [HiveStructureBody.h](../../Generals/Code/GameEngine/Include/GameLogic/Module/HiveStructureBody.h)
- Source (Retail Generals): [HiveStructureBody.cpp](../../Generals/Code/GameEngine/Source/GameLogic/Object/Body/HiveStructureBody.cpp)

## Changes History

- v1.04 — Adds HiveStructureBody (damage redirection/swallowing for structures).

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)
