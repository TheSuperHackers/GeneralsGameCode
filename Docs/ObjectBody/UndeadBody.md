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
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

Used by objects that should have a second life after death, becoming "undead" with different armor and health characteristics. This creates dramatic gameplay where enemies must "kill" units twice. This is a **body module** that must be embedded within object definitions. Use the [Template](#template) below by copying it into your object definition. Then, customize it as needed, making sure to review any limitations, conditions, or dependencies related to its usage.

**Placement**:
- **Retail**: UndeadBody can only be added to `Object` entries (ObjectExtend does not exist in Retail).

Only one body module (ActiveBody, InactiveBody, StructureBody, UndeadBody, etc.) can exist per object. If multiple body modules are added to the same object, the game will crash with a "Duplicate bodies" assertion error during object creation. This restriction applies regardless of `ModuleTag` names - the object can only have one body module total.

**Limitations**:
- UndeadBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- The second life mechanic only works once per object lifetime. After the first death is intercepted, the object enters its second life and cannot use the mechanic again.
- The second death is permanent and cannot be prevented. Once in second life, the object will die normally when health reaches 0.
- Unresistable damage (`DAMAGE_UNRESISTABLE`) bypasses the second life mechanic entirely and causes immediate death, even on the first life.
- Requires SlowDeathBehavior module to function properly. The game will crash with an assertion if UndeadBody is used without SlowDeathBehavior: `"Hmm, this is wrong"`.
- If [InitialHealth](#initialhealth) exceeds [MaxHealth](#maxhealth), the current health will be clamped to [MaxHealth](#maxhealth) when the first health change occurs. Health cannot go below `0.0` or above [MaxHealth](#maxhealth) during first life.
- [SubdualDamageCap](#subdualdamagecap) can disable objects without destroying them when subdual damage equals or exceeds [MaxHealth](#maxhealth). Subdual damage properties are available only in Zero Hour.

**Conditions**:
- Objects with UndeadBody can be targeted by weapons (see [Weapon documentation](../Weapon.md)) and affected by damage types. Health is reduced when weapons deal damage, and damage states are updated based on health percentage.
- **Second life activation**: When the object would die from health-damaging damage (excluding unresistable damage), the death is intercepted. The damage amount is clamped to leave exactly 1 health point, then the object transitions to second life. The transition occurs after damage is applied but before death modules are processed, allowing damage effects to play normally.
- **Second life transition**: When second life activates, the object:
  - Sets maximum health to [SecondLifeMaxHealth](#secondlifemaxhealth) and fully heals to the new maximum.
  - Sets armor set flag to `ARMORSET_SECOND_LIFE`, allowing different damage resistance for second life.
  - Triggers SlowDeathBehavior module to play death animations and effects without actually dying.
  - Sets `MODELCONDITION_SECOND_LIFE` model condition for visual effects.
  - Marks the object as in second life, preventing further second life activations.
- **Second life behavior**: Once in second life, the object behaves like a normal ActiveBody object. It can take damage, be healed, and die normally. The second death is permanent and cannot be prevented.
- **Unresistable damage bypass**: Unresistable damage (`DAMAGE_UNRESISTABLE`) bypasses the second life mechanic entirely. If unresistable damage would kill the object, it dies immediately regardless of whether it's on first or second life.
- **Damage calculation**: UndeadBody uses raw damage amount comparison to determine if damage would be lethal. This compares the damage amount directly against current health.
- Veterancy levels can modify maximum health and healing rates through upgrade systems. Veterancy bonuses apply to both first and second life.
- UndeadBody integrates with armor systems (see [Armor documentation](../Armor.md)) and damage effects. Armor modifies incoming damage before it is applied to health. Second life uses `ARMORSET_SECOND_LIFE` armor set for different damage resistance.
- Damage states are calculated based on global thresholds defined in game data and affect visual appearance and particle systems.
- **SlowDeathBehavior requirement**: UndeadBody requires at least one SlowDeathBehavior module to function. When second life activates, UndeadBody selects a SlowDeathBehavior module using probability modifiers and triggers it. The SlowDeathBehavior plays death animations and effects without actually killing the object.

**Dependencies**:
- Requires proper armor and damage type definitions to function correctly. UndeadBody relies on armor systems to modify incoming damage before it is applied to health. Second life uses `ARMORSET_SECOND_LIFE` armor set.
- Requires SlowDeathBehavior module to function properly. UndeadBody triggers SlowDeathBehavior when second life activates to play death animations and effects.
- Inherits all properties and functionality from ActiveBody.

## Properties

UndeadBody inherits all properties from ActiveBody and adds one additional property for second life configuration.

### Health Settings

Available in: *(v1.04)* (Zero Hour)

#### `MaxHealth`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Maximum health points the object can have during its first life. Higher values make objects more durable and resistant to damage. This determines the total damage capacity before second life activation. If `InitialHealth` exceeds `MaxHealth`, the current health will be clamped to `MaxHealth` when the first health change occurs. Health is automatically clamped to `MaxHealth` as the upper limit and `0.0` as the lower limit during damage and healing operations in first life. When second life activates, maximum health changes to [SecondLifeMaxHealth](#secondlifemaxhealth).
- **Default**: `0.0`
- **Example**: `MaxHealth = 500.0`

#### `InitialHealth`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Starting health points when the object is created. Higher values allow objects to spawn with more health than their maximum, providing temporary damage buffer. Lower values spawn objects at reduced health. The initial health value is set directly during object creation; if it exceeds `MaxHealth`, it will be clamped to `MaxHealth` when the first health change occurs. Health cannot go below `0.0` during first life. When second life activates, the object is fully healed to [SecondLifeMaxHealth](#secondlifemaxhealth).
- **Default**: `0.0`
- **Example**: `InitialHealth = 500.0`

### Subdual Damage Settings

Available in: *(v1.04)* (Zero Hour)

#### `SubdualDamageCap`
Available in: *(v1.04)* (Zero Hour)

- **Type**: `Real`
- **Description**: Maximum subdual damage that can accumulate before the object is subdued (disabled). Higher values allow objects to absorb more subdual damage before becoming incapacitated. At 0 (default), objects cannot be subdued (disabled). Subdual damage accumulates separately from normal health damage and can disable objects without destroying them. When subdual damage equals or exceeds `MaxHealth`, the object becomes subdued (disabled). Subdual damage does not trigger second life activation.
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
- **Description**: Maximum health points for the object's second life. When the object "dies" the first time, it transitions to this new maximum health and is fully healed. Higher values make the second life more durable and resistant to damage. Lower values make the second life fragile. When set to 1 (default), the second life is very fragile and can be killed with minimal damage. The second life uses `ARMORSET_SECOND_LIFE` armor set for different damage resistance than the first life.
- **Default**: `1.0`
- **Example**: `SecondLifeMaxHealth = 50.0`

## Enum Value Lists

#### `BodyDamageType` Values
Available in: *(v1.04)* (Zero Hour)

**Source:** BodyModule.h - `BodyDamageType` enum definition; string names from `TheBodyDamageTypeNames[]`

**Retail 1.04 Values**:

- **`PRISTINE`** *(v1.04)* - Unit should appear in pristine condition
- **`DAMAGED`** *(v1.04)* - Unit has been damaged
- **`REALLYDAMAGED`** *(v1.04)* - Unit is extremely damaged / nearly destroyed
- **`RUBBLE`** *(v1.04)* - Unit has been reduced to rubble/corpse/exploded-hulk, etc

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

- UndeadBody provides a "second life" mechanic where objects survive their first death and transition to a second life with new maximum health and armor.
- When the object would die from health-damaging damage (excluding unresistable damage), the death is intercepted. The damage is clamped to leave 1 health point, then the object transitions to second life.
- The second life uses `ARMORSET_SECOND_LIFE` armor set for different damage resistance than the first life. Define armor sets in your Armor definitions to take advantage of this.
- The second life transition fully heals the object to [SecondLifeMaxHealth](#secondlifemaxhealth).
- Requires SlowDeathBehavior module to function properly. UndeadBody triggers SlowDeathBehavior when second life activates to play death animations and effects without actually killing the object.
- Unresistable damage (`DAMAGE_UNRESISTABLE`) bypasses the second life mechanic entirely and causes immediate death, even on the first life.
- Second death is permanent and cannot be prevented. Once in second life, the object will die normally when health reaches 0.
- Damage calculation uses raw damage amount comparison to determine if second life should activate. This compares the damage amount directly against current health.
- UndeadBody automatically manages damage states ([BodyDamageType Values](#bodydamagetype-values) such as PRISTINE, DAMAGED, REALLYDAMAGED, RUBBLE) based on health percentage thresholds defined in game data. Damage states affect visual appearance and particle systems.
- Damage states are calculated based on global thresholds and updated automatically when [MaxHealth](#maxhealth) or [SecondLifeMaxHealth](#secondlifemaxhealth) changes.
- Subdual damage can disable objects without destroying them. Subdual damage accumulates separately from normal health damage and is limited by [SubdualDamageCap](#subdualdamagecap). Subdual damage does not trigger second life activation.
- Objects automatically heal subdual damage over time if [SubdualDamageHealRate](#subdualdamagehealrate) and [SubdualDamageHealAmount](#subdualdamagehealamount) are set. The healing is handled by helper systems that run at the specified intervals.
- Objects with UndeadBody can be targeted by weapons (see [Weapon documentation](../Weapon.md)) and affected by damage types. Armor modifies incoming damage before it is applied to health. Second life uses `ARMORSET_SECOND_LIFE` armor set.
- Veterancy levels can modify maximum health and healing rates through upgrade systems. Veterancy bonuses apply to both first and second life.
- UndeadBody integrates with armor systems (see [Armor documentation](../Armor.md)) and damage effects.
- Creates interesting tactical scenarios where enemies must "kill" units twice, making undead units more challenging to eliminate.
- Commonly used for zombie-themed units, boss characters, or units with resurrection capabilities.
- Only one body module is allowed per object; multiple bodies cause a startup assertion.

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

