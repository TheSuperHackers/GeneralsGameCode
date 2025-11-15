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
- [Modder Recommended Use Scenarios](#modder-recommended-use-scenarios)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

Place under `Body = HiveStructureBody ModuleTag_XX` inside [Object](../Object.md) entries. HiveStructureBody can only be added to [Object](../Object.md) entries in Retail. See [Template](#template) for correct syntax.

**Placement**:
- **Retail**: HiveStructureBody can only be added to `Object` entries.

Only one body module (ActiveBody, InactiveBody, StructureBody, ImmortalBody, HiveStructureBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- HiveStructureBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values)) based on health percentage thresholds defined in [GameData](../GameData.md). HiveStructureBody uses the same damage state system as StructureBody and ActiveBody (see [ActiveBody documentation](ActiveBody.md#bodydamagetype-values) for complete damage state enum values). Damage states affect visual appearance and particle systems.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the health will be clamped to [MaxHealth](#maxhealth) during health operations. Health cannot go below `0.0` or above [MaxHealth](#maxhealth).
- Only redirects or swallows the damage types listed in [PropagateDamageTypesToSlavesWhenExisting](#propagatedamagetypestoslaveswhenexisting) and [SwallowDamageTypesIfSlavesNotExisting](#swallowdamagetypesifslavesnotexisting); other types behave normally.

**Conditions**:
- Objects with HiveStructureBody can be targeted by [Weapon](../Weapon.md) attacks; [Armor](../Armor.md) applies before redirection/swallowing.
- When [SpawnBehavior](../ObjectBehaviorsModules/SpawnBehavior.md) or [ContainModule](../ObjectModules/ContainModule.md) supplies slave/contained objects, damage types listed in [PropagateDamageTypesToSlavesWhenExisting](#propagatedamagetypestoslaveswhenexisting) are redirected to the closest slave/contained object. When no slaves/contained objects exist, damage types listed in [SwallowDamageTypesIfSlavesNotExisting](#swallowdamagetypesifslavesnotexisting) are completely absorbed (no effect on the structure). Without SpawnBehavior or ContainModule, no damage redirection or swallowing occurs.
- **ObjectReskin (Retail)**: ObjectReskin uses the same module system as [Object](../Object.md). Adding HiveStructureBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- [SpawnBehavior](../ObjectBehaviorsModules/SpawnBehavior.md) and/or [ContainModule](../ObjectModules/ContainModule.md) to supply slaves/contained objects.
- Valid [DamageType](../DamageType.md) names.
- Has all the same health and visual-state properties as [StructureBody](./StructureBody.md) and [ActiveBody](./ActiveBody.md).
- Objects with HiveStructureBody can be healed by [AutoHealBehavior](../ObjectBehaviorsModules/AutoHealBehavior.md). AutoHealBehavior heals main health of objects with HiveStructureBody.

## Properties

### Damage Propagation

#### `PropagateDamageTypesToSlavesWhenExisting`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `DamageTypeFlags` (see [DamageType](../DamageType.md) documentation)
- **Description**: Redirect these damage types to the closest slave/contained object when present. Empty means no redirection. Use valid damage type names; invalid names will fail to parse.
- **Default**: `0` (none)
- **Example**: `PropagateDamageTypesToSlavesWhenExisting = EXPLOSION BALLISTIC`

#### `SwallowDamageTypesIfSlavesNotExisting`
Available in: *(v1.04)* (Generals, Zero Hour)

- **Type**: `DamageTypeFlags` (see [DamageType](../DamageType.md) documentation)
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

- Damage states ([BodyDamageType Values](#bodydamagetype-values)) are updated automatically when [MaxHealth](#maxhealth) changes. HiveStructureBody uses the same damage state system as StructureBody and ActiveBody (see [ActiveBody documentation](ActiveBody.md#bodydamagetype-values) for complete damage state enum values).
- HiveStructureBody requires either [SpawnBehavior](../ObjectBehaviorsModules/SpawnBehavior.md) or [ContainModule](../ObjectModules/ContainModule.md) to redirect or swallow damage. Without these modules, no damage redirection or swallowing occurs.
- The closest slave/contained object to the damage source receives the redirected damage. Damage absorption (swallowing) only occurs when no slaves exist and the damage type matches the swallow criteria.
- If no slaves exist and the damage type doesn't match swallow criteria, damage is taken normally by the structure.

## Modder Recommended Use Scenarios

- HiveStructureBody is used by objects like StingerSite which has slaved objects.

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
