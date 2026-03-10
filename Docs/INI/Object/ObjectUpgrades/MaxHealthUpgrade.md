# MaxHealthUpgrade

Status: AI-generated, 0/2 reviews

## Overview

The `MaxHealthUpgrade` upgrade module increases an object's maximum health when specific upgrades are applied. This upgrade allows objects to gain enhanced durability and survivability by adding a specified amount to their maximum health. The behavior supports different health change types to control how the current health is affected when the maximum health is increased.

Available in: *(v1.04)* (Generals, Zero Hour)

## Table of Contents

- [Overview](#overview)
- [Properties](#properties)
  - [Health Modification Settings](#health-modification-settings)
  - [Upgrade Integration Settings](#upgrade-integration-settings)
- [Enum Value Lists](#enum-value-lists)
  - [MaxHealthChangeType Values](#maxhealthchangetype-values)
- [Examples](#examples)
- [Usage](#usage)
  - [Limitations](#limitations)
  - [Conditions](#conditions)
  - [Dependencies](#dependencies)
- [Template](#template)
- [Notes](#notes)
- [Modder Recommended Use Scenarios](#modder-recommended-use-scenarios)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Status](#status)
  - [Reviewers](#reviewers)

## Properties

### Health Modification Settings

#### `AddMaxHealth`

Available in: *(v1.04)*

- **Type**: `Real`
- **Description**: The amount of maximum health to add to the object. Positive values increase maximum health, providing enhanced durability. The value is always treated as an absolute amount and is added directly to maximum health. The health increase is applied when the upgrade is activated and remains until the upgrade is removed.
- **Default**: `0.0`
- **Example**: `AddMaxHealth = 100.0`

#### `ChangeType`

Available in: *(v1.04)*

- **Type**: `MaxHealthChangeType` (see [MaxHealthChangeType Values](#maxhealthchangetype-values) section)
- **Description**: Controls how the current health is affected when maximum health is increased. Different types provide different behaviors for health management: `SAME_CURRENTHEALTH` keeps current health unchanged, `PRESERVE_RATIO` maintains the same health percentage, `ADD_CURRENT_HEALTH_TOO` adds the same amount to current health as maximum health, and `FULLY_HEAL` fully heals the object. This property works in conjunction with [AddMaxHealth](#addmaxhealth) to determine the final health state after the upgrade is applied.
- **Default**: `SAME_CURRENTHEALTH`
- **Example**: `ChangeType = PRESERVE_RATIO`
- **Available Values**: see [MaxHealthChangeType Values](#maxhealthchangetype-values)

### Upgrade Integration Settings

#### `TriggeredBy`

Available in: *(v1.04)*

- **Type**: List of upgrade names (see [Upgrade documentation](../Upgrade.md))
- **Description**: List of upgrade names that must be active for this upgrade module to activate. When any of the listed upgrades (or all, if [RequiresAllTriggers](#requiresalltriggers) is `Yes`) become active, this upgrade module will activate and increase the object's maximum health. The upgrade names must reference valid `Upgrade` entries defined in the game. Multiple upgrade names can be specified by listing them multiple times or using a space-separated list, depending on the parser implementation.
- **Default**: Empty (no trigger required)
- **Example**: `TriggeredBy = Upgrade_HealthTraining`

#### `ConflictsWith`

Available in: *(v1.04)*

- **Type**: List of upgrade names (see [Upgrade documentation](../Upgrade.md))
- **Description**: List of upgrade names that conflict with this upgrade module. If any of the conflicting upgrades are active, this upgrade module will not activate, even if [TriggeredBy](#triggeredby) conditions are met. This allows modders to create mutually exclusive upgrade paths. The upgrade names must reference valid `Upgrade` entries. Multiple conflicting upgrades can be specified.
- **Default**: Empty (no conflicts)
- **Example**: `ConflictsWith = Upgrade_BasicTraining`

#### `RequiresAllTriggers`

Available in: *(v1.04)*

- **Type**: `Bool`
- **Description**: Controls whether all [TriggeredBy](#triggeredby) upgrades must be active, or if any one is sufficient. When `Yes`, all listed `TriggeredBy` upgrades must be active for this upgrade module to activate. When `No`, any one of the `TriggeredBy` upgrades being active is sufficient. This provides fine-grained control over upgrade activation conditions.
- **Default**: `No`
- **Example**: `RequiresAllTriggers = Yes`

#### `FXListUpgrade`

Available in: *(v1.04)*

- **Type**: `FXList` (see [FXList documentation](../FXList.md))
- **Description**: FXList to play when this upgrade module activates. The FXList is played once when the upgrade is applied, providing visual and audio feedback for the upgrade activation. The FXList must reference a valid `FXList` entry defined in the game. If not specified, no effects are played when the upgrade activates.
- **Default**: `NULL` (no effects)
- **Example**: `FXListUpgrade = FX_HealthUpgradeActivation`

## Enum Value Lists

<a id="maxhealthchangetype-values"></a>
#### `MaxHealthChangeType` Values

Available in: *(v1.04)*

- **Source**: Header file - `TheMaxHealthChangeTypeNames[]` array definition

- **`SAME_CURRENTHEALTH`** *(v1.04)* — Keeps current health unchanged when maximum health is increased. If the object has 80 health out of 100 maximum, and maximum health is increased by 50, the object will have 80 health out of 150 maximum.
- **`PRESERVE_RATIO`** *(v1.04)* — Maintains the same health percentage when maximum health is increased. If the object has 80 health out of 100 maximum (80% health), and maximum health is increased by 50, the object will have 120 health out of 150 maximum (maintaining 80%).
- **`ADD_CURRENT_HEALTH_TOO`** *(v1.04)* — Adds the same amount to current health as is added to maximum health. If maximum health is increased by 50, current health is also increased by 50 (unless it would exceed the new maximum, in which case it is clamped to maximum).
- **`FULLY_HEAL`** *(v1.04)* — Fully heals the object when maximum health is increased. Regardless of current health, the object's health is set to the new maximum health value.

## Examples

### Basic Health Increase

```ini
Upgrade = MaxHealthUpgrade ModuleTag_HealthBoost
  AddMaxHealth = 100.0
  ChangeType = SAME_CURRENTHEALTH
  TriggeredBy = Upgrade_HealthTraining
End
```

### Health Increase with Ratio Preservation

```ini
Upgrade = MaxHealthUpgrade ModuleTag_HealthUpgrade
  AddMaxHealth = 50.0
  ChangeType = PRESERVE_RATIO
  TriggeredBy = Upgrade_AdvancedTraining
End
```

### Health Increase with Current Health Boost

```ini
Upgrade = MaxHealthUpgrade ModuleTag_HealthBoost
  AddMaxHealth = 75.0
  ChangeType = ADD_CURRENT_HEALTH_TOO
  TriggeredBy = Upgrade_MedicalTraining
End
```

### Full Heal with Health Increase

```ini
Upgrade = MaxHealthUpgrade ModuleTag_HealthRestore
  AddMaxHealth = 200.0
  ChangeType = FULLY_HEAL
  TriggeredBy = Upgrade_EmergencyHealing
End
```

### Conditional Upgrade with Conflicts

```ini
Upgrade = MaxHealthUpgrade ModuleTag_ConditionalHealth
  AddMaxHealth = 150.0
  ChangeType = ADD_CURRENT_HEALTH_TOO
  TriggeredBy = Upgrade_EliteTraining
  ConflictsWith = Upgrade_BasicTraining
  RequiresAllTriggers = Yes
End
```

## Usage

Place under `Upgrade = MaxHealthUpgrade ModuleTag_XX` inside [Object](../Object.md) entries. See [Template](#template) for correct syntax.

**Placement**:
- `MaxHealthUpgrade` can only be added to [Object](../Object.md) entries.

**Limitations**:
- Objects must have a body module (such as [ActiveBody](../ObjectBody/ActiveBody.md)) to function properly. If no body module exists, the upgrade will not affect health.
- Health increases are additive, not multiplicative. Multiple `MaxHealthUpgrade` instances will each add their [AddMaxHealth](#addmaxhealth) value to the maximum health.
- The health increase is permanent until the upgrade is removed. Removing the triggering upgrade will not automatically revert the health increase unless the upgrade system is configured to handle removal.

- **Conditions**:
- `MaxHealthUpgrade` increases the object's maximum health when the specified upgrade in [TriggeredBy](#triggeredby) is applied.
- The upgrade integrates with the upgrade system for conditional activation based on [TriggeredBy](#triggeredby), [ConflictsWith](#conflictswith), and other upgrade conditions.
- Different health change types control how current health is affected when maximum health is increased. The [ChangeType](#changetype) property determines whether current health remains unchanged, maintains ratio, is boosted, or is fully healed.
- The upgrade works with objects that have body modules (such as [ActiveBody](../ObjectBody/ActiveBody.md)).
- **Multiple instances behavior**: Multiple instances can coexist; each operates independently with its own health increases and conditions. Each instance will add its [AddMaxHealth](#addmaxhealth) value when its [TriggeredBy](#triggeredby) conditions are met.
- **ObjectReskin**: [ObjectReskin](../ObjectReskin.md) uses the same module system as [Object](../Object.md). Adding `MaxHealthUpgrade` to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- Requires an object with a body module (such as [ActiveBody](../ObjectBody/ActiveBody.md)) to function properly. The upgrade calls `BodyModuleInterface::setMaxHealth()` to modify health, so without a body module, the upgrade has no effect.
- Requires the upgrade system to be properly configured. The [TriggeredBy](#triggeredby) property must reference valid [Upgrade](../Upgrade.md) entries defined in the game. If the referenced upgrades do not exist, the upgrade module will not activate.

## Template

```ini
Upgrade = MaxHealthUpgrade ModuleTag_XX
  ; Health Modification Settings
  AddMaxHealth = 0.0               ; // amount of maximum health to add *(v1.04)*
  ChangeType = SAME_CURRENTHEALTH  ; // how current health is affected *(v1.04)*
  
  ; Upgrade Integration Settings
  TriggeredBy =                    ; // required upgrade flags *(v1.04)*
  ConflictsWith =                  ; // conflicting upgrade flags *(v1.04)*
  RequiresAllTriggers = No         ; // require all TriggeredBy upgrades *(v1.04)*
  FXListUpgrade =                  ; // effects to play when activated *(v1.04)*
End
```

## Notes

- `MaxHealthUpgrade` increases the object's maximum health when the specified upgrade is applied, providing enhanced durability and survivability.
- The upgrade integrates with the upgrade system for conditional activation, allowing complex upgrade trees and dependencies.
- Different health change types (`ChangeType`) control how current health is affected, providing flexibility in health management strategies.
- Health increases are additive: if multiple `MaxHealthUpgrade` instances are active, each adds its `AddMaxHealth` value to the maximum health.
- The upgrade works with objects that have body modules (such as ActiveBody). Objects without body modules will not be affected.
- The health increase is permanent until the upgrade is removed. Removing the triggering upgrade does not automatically revert the health increase unless the upgrade system handles removal explicitly.

## Modder Recommended Use Scenarios

(pending modder review)

## Source Files

**Base Class:** UpgradeModule

- Header: MaxHealthUpgrade.h
- Source: MaxHealthUpgrade.cpp

## Changes History

- 03/01/2025 — AI — Reconstructs documentation following DocAuthoringGuide with complete property documentation, versioning, enum lists, and examples

## Status

- **Documentation Status**: AI-generated
- **Last Updated**: 03/01/2025 by @AI
- **Certification**: 0/2 reviews

### Reviewers

- No reviews yet

