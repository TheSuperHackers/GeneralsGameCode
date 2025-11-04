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
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

Used by objects that should be indestructible and unaffected by damage, such as decorative objects, terrain features, or special objects that should not interact with the damage system. This is a **body module** that must be embedded within object definitions. Use the [Template](#template) below by copying it into your object definition. Then, customize it as needed, making sure to review any limitations, conditions, or dependencies related to its usage.

**Placement**:
- **Retail**: InactiveBody can only be added to `Object` entries (ObjectExtend does not exist in Retail).

Only one body module (ActiveBody, InactiveBody, StructureBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- InactiveBody objects have no health system and cannot be damaged or healed through normal means.
- All damage and healing attempts are ignored except unresistable damage type.
- Objects are automatically marked as "effectively dead" on construction, which affects targeting, weapon attacks, and UI interactions.
- Health queries always return `0.0`.
- Damage state is always `BODY_PRISTINE` and cannot be changed.
- Cannot be used on prerequisite objects (`IsPrerequisite = Yes`); the game will crash with an assertion if attempted.
- No veterancy interactions - veterancy level changes are ignored.
- No armor interactions - armor set flags cannot be modified or tested.
- Health modifications from upgrades, veterancy, or difficulty scaling have no effect.

**Conditions**:
- Objects with InactiveBody are automatically marked as "effectively dead" on construction, which affects:
  - **Weapon targeting**: Weapons will not attack effectively dead objects.
  - **UI interactions**: UI systems may exclude effectively dead objects from selection and display.
  - **Player systems**: Player systems (resource gathering, unit selection, etc.) exclude effectively dead objects from normal gameplay interactions.
  - **Contain systems**: Contain systems (garrisons, transports, etc.) check effectively dead status when processing contained objects.
  - **Script systems**: Script actions check effectively dead status when processing objects.
- All damage attempts are ignored except unresistable damage type. When unresistable damage is applied, DieModules are processed (object can be removed via death modules), but health is not modified and damage FX are not applied.
- All healing attempts are ignored and have no effect.
- Damage estimation returns `0.0` for all damage types except unresistable damage, allowing AI and weapon systems to recognize that InactiveBody objects cannot be damaged normally.
- Health queries always return `0.0`.
- Health modifications from upgrades, veterancy, difficulty scaling, or object properties have no effect.
- Damage state is always `BODY_PRISTINE` and cannot be changed.
- Veterancy bonuses and upgrades that modify health do not affect InactiveBody objects.
- Armor set flags cannot be modified or tested (no armor interactions).
- Script actions can remove InactiveBody objects using unresistable damage type. Scripts that check health will receive `0.0`.
- Objects can be removed via selling if DieModules are present.
- Contain systems (garrisons, transports) can remove contained InactiveBody objects with unresistable damage when containers are destroyed, if DieModules are present.
- **Prerequisite restriction**: Objects with `IsPrerequisite = Yes` cannot use InactiveBody. The game will crash with an assertion if this combination is attempted.
- **ObjectReskin (Retail)**: ObjectReskin uses the same module system as Object. Adding InactiveBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement like ObjectExtend.

**Dependencies**:
- Requires the object system to function correctly.
- Compatible with DieModules - unresistable damage can trigger death modules for object removal.

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
- Objects are automatically marked as "effectively dead" on construction, which affects targeting, weapon attacks, UI interactions, and player systems.
- All damage and healing attempts are ignored except unresistable damage type, which can trigger DieModules for object removal.
- Health queries always return `0.0`.
- Damage state is always `BODY_PRISTINE` and cannot be changed.
- Cannot be used on prerequisite objects (`IsPrerequisite = Yes`); the game will crash with an assertion if attempted.
- No veterancy, armor, or component interactions.
- Health modifications from upgrades, veterancy, or difficulty scaling have no effect.
- Damage estimation returns `0.0` for all damage types except unresistable damage, allowing AI and weapon systems to recognize that InactiveBody objects cannot be damaged normally.
- Commonly used for decorative objects, terrain features, and objects that should not interact with the damage system.
- Objects can still be removed through other means (like special powers, script commands using unresistable damage, or DieModules triggered by unresistable damage).
- Only one body module is allowed per object; multiple bodies cause a startup assertion.

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
