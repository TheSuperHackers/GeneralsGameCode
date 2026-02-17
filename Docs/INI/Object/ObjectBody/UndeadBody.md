# UndeadBody

Status: AI-generated, 0/2 reviews

## Overview

The `UndeadBody` module extends ActiveBody with a "second life" mechanic that intercepts the first death and grants the object a second life instead of dying. When the object would normally die from health-damaging damage, it instead survives with 1 health point, then immediately transitions to its second life with new maximum health, fully healed, and a different armor set. This creates a "zombie" or "undead" effect where objects appear to die but continue fighting. The second death is permanent and cannot be prevented. UndeadBody manages health, damage states, healing, subdual damage, and visual damage effects like ActiveBody, but adds the second life transition mechanic. This is a module added inside `Object` entries.

Available in: *(v1.04)* (Zero Hour)

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
  - [Limitations](#limitations)
  - [Conditions](#conditions)
  - [Dependencies](#dependencies)
- [Properties](#properties)
  - [Second Life Settings](#second-life-settings)
- [Enum Value Lists](#enum-value-lists)
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

Place under `Body = UndeadBody ModuleTag_XX` inside [Object](../Object.md) entries. UndeadBody can only be added to [Object](../Object.md) entries in Retail. See [Template](#template) for correct syntax.

**Placement**:
- **Retail**: UndeadBody can only be added to `Object` entries.

Only one body module (ActiveBody, InactiveBody, StructureBody, UndeadBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- UndeadBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- The second life mechanic only works once per object lifetime. After the first death is intercepted, the object enters its second life and cannot use the mechanic again.
- The second death is permanent and cannot be prevented. Once in second life, the object will die normally when health reaches 0.
- Unresistable damage bypasses the second life mechanic entirely and causes immediate death, even on the first life.
- Requires SlowDeathBehavior module to function properly. The game will crash with an assertion if UndeadBody is used without SlowDeathBehavior: `"Hmm, this is wrong"`.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the health will be clamped to [MaxHealth](#maxhealth) during health operations. Health cannot go below `0.0` or above [MaxHealth](#maxhealth) during first life.
- [SubdualDamageCap](#subdualdamagecap) can disable objects without destroying them when subdual damage equals or exceeds [MaxHealth](#maxhealth). Subdual damage properties are available only in Zero Hour.

**Conditions**:
- Objects with UndeadBody can be targeted by [Weapon](../Weapon.md) attacks and affected by [DamageType](../DamageType.md). Health is reduced when weapons deal damage.
- **Second life activation**: When the object would die from health-damaging damage (excluding unresistable damage), the death is intercepted. The damage amount is clamped to leave exactly 1 health point, then the object transitions to second life. The transition occurs after damage is applied but before death modules are processed, allowing damage effects to play normally.
- **Second life transition**: When second life activates, the object transitions to its second life with new maximum health set to [SecondLifeMaxHealth](#secondlifemaxhealth) and is fully healed. The second life uses a different armor set for different damage resistance. SlowDeathBehavior module plays death animations and effects without actually killing the object. Visual effects are updated to show the second life state. The object is marked as in second life, preventing further second life activations.
- **Second life behavior**: Once in second life, the object behaves like a normal ActiveBody object. It can take damage, be healed, and die normally. The second death is permanent and cannot be prevented.
- **Unresistable damage bypass**: Unresistable damage bypasses the second life mechanic entirely. If unresistable damage would kill the object, it dies immediately regardless of whether it's on first or second life.
- Veterancy levels can modify maximum health and healing rates through the veterancy bonus system (see [GameData](../GameData.md) for veterancy health bonuses). Veterancy bonuses apply to both first and second life.
- **ObjectReskin (Retail)**: ObjectReskin uses the same module system as [Object](../Object.md). Adding UndeadBody to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- Requires proper [Armor](../Armor.md) and [DamageType](../DamageType.md) definitions to function correctly. UndeadBody relies on armor systems to modify incoming damage before it is applied to health. Second life uses a different armor set for different damage resistance.
- Requires SlowDeathBehavior module to function properly. When second life activates, UndeadBody triggers SlowDeathBehavior to play death animations and effects without actually killing the object.
- Has all the same properties as [ActiveBody](ActiveBody.md).
- Objects with UndeadBody can be healed by [AutoHealBehavior](../ObjectBehaviorsModules/AutoHealBehavior.md). AutoHealBehavior heals main health for both first life and second life of objects with UndeadBody. When an object transitions to second life, AutoHealBehavior continues to work normally using the new [SecondLifeMaxHealth](#secondlifemaxhealth) value.

## Properties

UndeadBody has all the same properties as [ActiveBody](ActiveBody.md) and adds one additional property for second life configuration.

### Health Settings

Available in: *(v1.04)* (Zero Hour)

#### `MaxHealth`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Maximum health points the object can have during its first life. Higher values make objects more durable and resistant to damage. This determines the total damage capacity before second life activation. If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) during health operations. Health is automatically clamped to [MaxHealth](#maxhealth) as the upper limit and `0.0` as the lower limit during damage and healing operations in first life. When second life activates, maximum health changes to [SecondLifeMaxHealth](#secondlifemaxhealth).
- **Default**: `0.0`
- **Example**: `MaxHealth = 500.0`

#### `InitialHealth`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Starting health points when the object is created. Lower values spawn objects at reduced health. If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the health will be clamped to [MaxHealth](#maxhealth) during health operations. Health cannot go below `0.0` during first life. When second life activates, the object is fully healed to [SecondLifeMaxHealth](#secondlifemaxhealth).
- **Default**: `0.0`
- **Example**: `InitialHealth = 500.0`

### Subdual Damage Settings

Available in: *(v1.04)* (Zero Hour)

#### `SubdualDamageCap`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Maximum subdual damage that can accumulate before the object is subdued (disabled). Higher values allow objects to absorb more subdual damage before becoming incapacitated. At 0 (default), objects cannot be subdued (disabled). Subdual damage accumulates separately from normal health damage and can disable objects without destroying them. When subdual damage equals or exceeds [MaxHealth](#maxhealth), the object becomes subdued (disabled). Only subdual damage types affect this cap: `SUBDUAL_MISSILE`, `SUBDUAL_VEHICLE`, `SUBDUAL_BUILDING`, and `SUBDUAL_UNRESISTABLE` (see [DamageType documentation](../DamageType.md)). Subdual damage does not trigger second life activation.
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

### Second Life Settings

Available in: *(v1.04)* (Zero Hour)

#### `SecondLifeMaxHealth`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Maximum health points for the object's second life. When the object "dies" the first time, it transitions to this new maximum health and is fully healed. Higher values make the second life more durable and resistant to damage. Lower values make the second life fragile. When set to 1 (default), the second life is very fragile and can be killed with minimal damage. The second life uses a different armor set for different damage resistance than the first life.
- **Default**: `1.0`
- **Example**: `SecondLifeMaxHealth = 50.0`

## Enum Value Lists

#### `BodyDamageType` Values
Available in: *(v1.04)* (Zero Hour)

**Source:** BodyModule.h - `BodyDamageType` enum definition; string names from `TheBodyDamageTypeNames[]`

**Retail 1.04 Values** *(available in v1.04)*:

Damage states are calculated based on health percentage thresholds defined in [GameData](../GameData.md) (typically `UnitDamagedThreshold` = 50-70% and `UnitReallyDamagedThreshold` = 10-35%):

- **`PRISTINE`** *(v1.04)* - Unit appears in pristine condition (health > `UnitDamagedThreshold`, typically > 50-70%)
- **`DAMAGED`** *(v1.04)* - Unit has been damaged (health between `UnitReallyDamagedThreshold` and `UnitDamagedThreshold`, typically 10-35% to 50-70%)
- **`REALLYDAMAGED`** *(v1.04)* - Unit is extremely damaged / nearly destroyed (health between 0% and `UnitReallyDamagedThreshold`, typically 0% to 10-35%)
- **`RUBBLE`** *(v1.04)* - Unit has been reduced to rubble/corpse/exploded-hulk (health = 0%)

## Examples

### Zombie Infantry Unit
```ini
Body = UndeadBody ModuleTag_01
  MaxHealth = 100.0
  InitialHealth = 100.0
  SecondLifeMaxHealth = 50.0
  SubdualDamageCap = 0.0
  SubdualDamageHealRate = 0
  SubdualDamageHealAmount = 0.0
End

Behavior = SlowDeathBehavior ModuleTag_02
  DeathTypes = NORMAL
  MinDamage = 0.0
  MaxDamage = 1000.0
  Probability = 100
End
```

### Undead Vehicle
```ini
Body = UndeadBody ModuleTag_03
  MaxHealth = 500.0
  InitialHealth = 500.0
  SecondLifeMaxHealth = 200.0
  SubdualDamageCap = 0.0
  SubdualDamageHealRate = 0
  SubdualDamageHealAmount = 0.0
End

Behavior = SlowDeathBehavior ModuleTag_04
  DeathTypes = NORMAL
  MinDamage = 0.0
  MaxDamage = 1000.0
  Probability = 100
End
```

## Template

```ini
Body = UndeadBody ModuleTag_XX
  MaxHealth = 0.0                 ; // maximum health points (first life) *(v1.04)*
  InitialHealth = 0.0             ; // starting health points (first life) *(v1.04)*

  SubdualDamageCap = 0.0          ; // maximum subdual damage before subdual *(v1.04)*
  SubdualDamageHealRate = 0       ; // milliseconds between subdual damage healing *(v1.04)*
  SubdualDamageHealAmount = 0.0   ; // amount of subdual damage healed per interval *(v1.04)*

  ; Second Life Settings
  SecondLifeMaxHealth = 1.0       ; // maximum health for second life (fully healed on transition) *(v1.04)*
End
```

## Notes

- Damage states ([BodyDamageType Values](#bodydamagetype-values)) are updated automatically when [MaxHealth](#maxhealth) changes.
- UndeadBody provides a "second life" mechanic where objects survive their first death. When the object would die, it instead transitions to second life with new maximum health, fully healed, and a different armor set for different damage resistance.
- Requires SlowDeathBehavior module to properly trigger the second life transition. The game will crash with an assertion if UndeadBody is used without SlowDeathBehavior.
- Unresistable damage bypasses the second life mechanic entirely and causes immediate death, even on the first life.
- Second death is permanent and cannot be prevented. Once in second life, the object behaves like a normal ActiveBody object and will die normally when health reaches 0.
- Subdual damage types that affect this system: `SUBDUAL_MISSILE`, `SUBDUAL_VEHICLE`, `SUBDUAL_BUILDING`, and `SUBDUAL_UNRESISTABLE` (see [DamageType documentation](../DamageType.md)).

## Modder Recommended Use Scenarios

- UndeadBody is used by objects such as Battle Bus.

(pending modder review)

## Source Files

**Base Class:** [ActiveBody](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/ActiveBody.h) (Retail Zero Hour)

- Header (Retail Zero Hour): [UndeadBody.h](../../GeneralsMD/Code/GameEngine/Include/GameLogic/Module/UndeadBody.h)
- Source (Retail Zero Hour): [UndeadBody.cpp](../../GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Body/UndeadBody.cpp)

## Changes History

- Retail Zero Hour 1.04 — Adds UndeadBody (second life mechanic that intercepts first death).

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)

