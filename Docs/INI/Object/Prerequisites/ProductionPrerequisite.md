# ProductionPrerequisite

Status: AI-generated, 0/2 reviews

## Overview

The `ProductionPrerequisite` block defines checks used by production/build systems to decide whether a player may produce a unit/structure. It supports unit presence/absence, sciences via simple key lines. Used as a child block inside features that validate production availability.

Available in:
*(v1.04)* (Generals, Zero Hour)

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
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

Place as a child block under systems that decide whether production is allowed (e.g., factories/build menus). Lines inside the block are evaluated according to their semantics (presence/absence requirements, OR grouping for repeated unit entries as supported by the parser).

**Limitations**:
- Requires a valid owning player context.

**Conditions**:
- Multiple unit lines may form OR groups across sequential entries.

**Dependencies**:
- References must match actual entity names: units/objects use [Object](../Object.md); upgrades use [Upgrade](../Upgrade.md); sciences use [Science](../Science.md).

## Properties

- `Object`
  - Type: Object name or list of object names (see [Object](../Object.md))
  - Description: Player must own the listed object(s). Multiple names on the same line are OR; repeating this line creates additional checks combined by AND. If an object name is misspelled or doesn't exist, the game can behave unpredictably — always use exact object names.
  - Default: none
  - Example: `Object = AmericaWarFactory Barracks`

- `Science`
  - Type: Science name or list of science names (see [Science documentation](../Science.md))
  - Description: Player must own all listed sciences (AND within the same line). If a science name is wrong or doesn't exist, the game will stop at load with an error — use exact science names.
  - Default: none
  - Example: `Science = SCIENCE_TechLevel2`


## Examples

```ini
ProductionPrerequisite ; block name varies by parent; under objects use 'Prerequisite' (follows parent property name)
  Object = WarFactory Barracks
  Science = SCIENCE_TechLevel2
End
```

```ini
Prerequisite ; block name varies by parent; under objects use 'Prerequisite' (follows parent property name)
  Science = ChinaNuclearMissile
End
```

## Template

```ini
ProductionPrerequisite ; block name varies by parent; under objects use 'Prerequisite' (follows parent property name)
  Object = ObjectName                 ; player must own (OR within line; AND across lines)
  Science = ScienceName               ; player must own all listed sciences
End
```

## Notes

- Where lines repeat, groups may be evaluated with OR semantics across sequential entries.
- Use exact names as defined in their respective docs.


## Source Files

- Header: [Code/GameEngine/Include/Common/ProductionPrerequisite.h](../../Code/GameEngine/Include/Common/ProductionPrerequisite.h)
- Source: [Code/GameEngine/Source/Common/RTS/ProductionPrerequisite.cpp](../../Code/GameEngine/Source/Common/RTS/ProductionPrerequisite.cpp)

## Changes History

- 2003 — Retail 1.04 — ProductionPrerequisite used for production gating.

## Status

- Documentation Status: AI-generated
- Last Updated: 15/01/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)


