# InactiveBody

Status: AI-generated, 0/2 reviews

## Overview

The `InactiveBody` module provides an indestructible body system for objects that cannot be damaged or affected by normal game mechanics. Objects with InactiveBody are marked as "effectively dead" and bypass most health and damage calculations. These objects have no health system and ignore all damage and healing attempts except unresistable damage, which can trigger death modules. This behavior is commonly used for decorative objects, terrain features, and objects that should not interact with the damage system.

Available in: *(v1.04)* (Generals, Zero Hour)

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
  - [Limitations](#limitations)
  - [Conditions](#conditions)
  - [Dependencies](#dependencies)
- [Properties](#properties)
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

Place under `Body = InactiveBody ModuleTag_XX` inside [Object](../Object.md) entries. InactiveBody can only be added to [Object](../Object.md) entries in Retail. See [Template](#template) for correct syntax.

**Placement**:
- **Retail**: InactiveBody can only be added to `Object` entries.

Only one body module (ActiveBody, InactiveBody, StructureBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- InactiveBody objects have no health system and cannot be damaged or healed through normal means.
- All damage and healing attempts are ignored except unresistable damage type.
- Objects are automatically marked as "effectively dead" on construction, which affects targeting, weapon attacks, and UI interactions.
- Cannot be used on prerequisite objects (`IsPrerequisite = Yes`); the game will crash with an assertion if attempted.
- No veterancy interactions - veterancy level changes are ignored.
- No armor interactions - armor set flags cannot be modified or tested.
- Health modifications from upgrades, veterancy, or difficulty scaling have no effect.

**Conditions**:
- Objects with InactiveBody are automatically marked as "effectively dead" on construction, which affects weapon targeting (weapons will not attack), UI interactions (may be excluded from selection), player systems (excluded from gameplay interactions), and contain systems (status checked when processing contained objects).
- All damage attempts are ignored except unresistable damage type. When unresistable damage is applied, death modules are processed (object can be removed), but health is not modified.
- All healing attempts are ignored and have no effect.
- Script actions can remove InactiveBody objects using unresistable damage type.
- Objects can be removed via selling if death modules are present.
- Contain systems (garrisons, transports) can remove contained InactiveBody objects with unresistable damage when containers are destroyed, if death modules are present.
- **ObjectReskin (Retail)**: ObjectReskin uses the same module system as [Object](../Object.md). Adding InactiveBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- Requires the object system to function correctly.
- Compatible with death modules - unresistable damage can trigger death modules for object removal.
- Objects with InactiveBody cannot be healed by [AutoHealBehavior](../ObjectBehaviorsModules/AutoHealBehavior.md). InactiveBody has no health system and all healing attempts are ignored.

## Properties

InactiveBody has no INI-parsable properties. All health, damage, and armor functionality is disabled or bypassed.

## Examples

### Decorative Object
```ini
Body = InactiveBody ModuleTag_Decorative
End
```

### Terrain Feature
```ini
Body = InactiveBody ModuleTag_Terrain
End
```

### Special Indestructible Object
```ini
Body = InactiveBody ModuleTag_Indestructible
End
```

## Template

```ini
Body = InactiveBody ModuleTag_XX
  ; No additional properties - InactiveBody has no INI-parsable properties
End
```

## Notes

- InactiveBody objects have no health system and cannot be damaged or healed through normal means.
- Objects are automatically marked as "effectively dead" on construction, which affects targeting, weapon attacks, UI interactions, player systems, and contain systems.
- All damage and healing attempts are ignored except unresistable damage type, which can trigger death modules for object removal.
- Cannot be used on prerequisite objects (`IsPrerequisite = Yes`); the game will crash with an assertion if attempted.
- No veterancy, armor, or component interactions.
- Health modifications from upgrades, veterancy, or difficulty scaling have no effect.
- Commonly used for decorative objects, terrain features, and objects that should not interact with the damage system.
- Objects can still be removed through other means (like special powers, script commands using unresistable damage, or death modules triggered by unresistable damage).
- Only one body module is allowed per object; multiple bodies cause a startup assertion.

## Modder Recommended Use Scenarios

- InactiveBody is used by objects that are not damageable like FireFieldSmall and MultiplayerBeacon which should not be killed.

(pending modder review)

## Source Files

**Base Class:** [BodyModule](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/BodyModule.h) (Retail Zero Hour), [BodyModule](../../Generals/Code/GameEngine/Include/GameLogic/Module/BodyModule.h) (Retail Generals)

- Header (Retail Zero Hour): [InactiveBody.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/InactiveBody.h)
- Source (Retail Zero Hour): [InactiveBody.cpp](../../GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Body/InactiveBody.cpp)
- Header (Retail Generals): [InactiveBody.h](../../Generals/Code/GameEngine/Include/GameLogic/Module/InactiveBody.h)
- Source (Retail Generals): [InactiveBody.cpp](../../Generals/Code/GameEngine/Source/GameLogic/Object/Body/InactiveBody.cpp)

## Changes History

- v1.04 — Adds InactiveBody (indestructible body system for decorative objects).

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)
