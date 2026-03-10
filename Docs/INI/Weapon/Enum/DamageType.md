# DamageType

Status: AI-generated, 0/2 reviews

## Overview

`DamageType` is an enum that defines the different types of damage that can be dealt to objects in the game. Damage types are used to determine how damage interacts with armor, what visual and audio effects are played, and what special behaviors are triggered. Different damage types can have different effectiveness against different armor types, and some damage types have special mechanics (e.g., surrender damage, status damage).

Damage types are used throughout the game for:
- **Weapon damage**: Weapons specify their damage type to determine how they interact with targets
- **Armor resistance**: Armor types can have different resistances to different damage types
- **Visual effects**: Different damage types trigger different visual and audio effects
- **Special mechanics**: Some damage types have special behaviors (e.g., surrender damage causes units to surrender, status damage applies status conditions)

**For modders**: In INI files, you specify the damage type for weapons using the `DamageType` property. Damage types interact with armor types defined in [Armor](../Armor.md) templates to determine final damage amounts. Some damage types have special mechanics that affect how they interact with objects.

Available in: *(v1.04)*

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
- [Enum Value Lists](#enum-value-lists)
  - [Standard Damage Types](#standard-damage-types)
- [Examples](#examples)
- [Notes](#notes)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

**Limitations**:
- Damage types are assigned to weapons in the [Weapon](../Weapon.md) template using the `DamageType` property.
- Damage types interact with armor types to determine final damage amounts through damage multipliers.
- Some damage types have special mechanics that bypass normal health damage (e.g., status damage).
- Damage types are used by visual effect systems to determine which effects to play when damage is dealt.

**Conditions**:
- Damage types are specified in weapon templates to define how weapons deal damage.
- Armor templates define resistance multipliers for each damage type.
- Some damage types have special behaviors that trigger when they are dealt (e.g., `DAMAGE_SURRENDER` causes units to surrender instead of die).
- Damage types can be combined in damage type flags for systems that need to check for multiple damage types.

**Dependencies**:
- Damage types are used by [Weapon](../Weapon.md) templates to define weapon damage characteristics.
- Damage types interact with [Armor](../Armor.md) templates to determine damage multipliers.
- Some damage types require specific body modules or behaviors to function (e.g., surrender damage requires units with the `CAN_SURRENDER` KindOf flag).

## Enum Value Lists

### DamageType Values

Available in: *(v1.04)*

### Standard Damage Types

- **`DAMAGE_EXPLOSION`** *(v1.04)* - Explosive damage. Standard explosive damage type used by most explosive weapons like grenades, rockets, and bombs. Used for weapons that deal damage through explosive force. Typically effective against structures and vehicles. **Reduces health points**.

- **`DAMAGE_CRUSH`** *(v1.04)* - Crush damage. Damage type used when objects are crushed by heavier units or objects. Used for vehicles running over lighter units or objects. Typically very effective against infantry and light vehicles. **Reduces health points**.

- **`DAMAGE_ARMOR_PIERCING`** *(v1.04)* - Armor-piercing damage. Damage type designed to penetrate armor. Used by anti-tank weapons and armor-piercing rounds. Typically very effective against heavily armored targets. **Reduces health points**.

- **`DAMAGE_SMALL_ARMS`** *(v1.04)* - Small arms damage. Damage type used by small firearms like pistols and rifles. Used for infantry weapons and light firearms. Typically effective against infantry and light targets. **Reduces health points**.

- **`DAMAGE_GATTLING`** *(v1.04)* - Gatling gun damage. Damage type used by rapid-fire weapons like gatling guns and miniguns. Used for high-rate-of-fire weapons. Typically effective against infantry and light vehicles. **Reduces health points**.

- **`DAMAGE_RADIATION`** *(v1.04)* - Radiation damage. Damage type used by radiation-based weapons. Used for weapons that deal damage through radiation exposure. May have special effects on certain unit types. **Reduces health points**.

- **`DAMAGE_FLAME`** *(v1.04)* - Flame damage. Damage type used by flamethrowers and fire-based weapons. Used for weapons that deal damage through fire. Can set objects on fire and cause burning effects. **Reduces health points**.

- **`DAMAGE_LASER`** *(v1.04)* - Laser damage. Damage type used by laser weapons. Used for precision energy weapons. Typically very accurate and effective against various targets. **Reduces health points**.

- **`DAMAGE_SNIPER`** *(v1.04)* - Sniper damage. Damage type used by sniper rifles. Used for long-range precision weapons. Typically very effective against infantry and can kill pilots in vehicles. **Reduces health points**.

- **`DAMAGE_POISON`** *(v1.04)* - Poison damage. Damage type used by poison-based weapons. Used for weapons that deal damage through toxic effects. May have special effects on biological targets. **Reduces health points**.

- **`DAMAGE_HEALING`** *(v1.04)* - Healing damage (negative damage). Special damage type that heals instead of damages. Used by healing systems to restore health. Negative damage values heal the target. **Reduces health points**.

- **`DAMAGE_UNRESISTABLE`** *(v1.04)* - Unresistable damage. Damage type that bypasses all armor resistances. Used for scripting to cause 'armorproof' damage. This damage type ignores all armor multipliers and deals full damage. **Reduces health points**.

- **`DAMAGE_WATER`** *(v1.04)* - Water damage. Damage type used by water-based attacks. Used for weapons that deal damage through water or flooding. May have special effects on certain unit types. **Reduces health points**.

- **`DAMAGE_DEPLOY`** *(v1.04)* - Deploy damage. Special damage type for transports to deploy units and order them to all attack. Used by transport systems to deploy passengers. Does not deal actual damage but triggers deployment behavior. **Reduces health points**.

- **`DAMAGE_SURRENDER`** *(v1.04)* - Surrender damage. If something "dies" to surrender damage, they surrender instead of being destroyed. Used by surrender systems to force units to surrender. Units with the `CAN_SURRENDER` KindOf flag will surrender instead of die when killed by this damage type. **Reduces health points**.

- **`DAMAGE_HACK`** *(v1.04)* - Hack damage. Damage type used by hacking attacks. Used for weapons that hack or disable electronic systems. May have special effects on electronic targets. **Reduces health points**.

- **`DAMAGE_KILLPILOT`** *(v1.04)* - Kill pilot damage. Special snipe attack that kills the pilot and renders a vehicle unmanned. Used by sniper weapons to kill vehicle pilots. Vehicles affected by this damage type become unmanned and can be hijacked. **Does not reduce health points**.

- **`DAMAGE_PENALTY`** *(v1.04)* - Penalty damage. Damage from game penalty (you won't receive radar warnings BTW). Used by penalty systems to deal damage as a penalty. Does not trigger radar warnings. **Reduces health points**.

- **`DAMAGE_FALLING`** *(v1.04)* - Falling damage. Damage type used when objects fall from heights. Used by physics systems when objects take fall damage. Typically results in `DEATH_SPLATTED` death type. **Reduces health points**.

- **`DAMAGE_MELEE`** *(v1.04)* - Melee damage. Blades, clubs, and other melee weapons. Used for close-combat weapons. Typically effective against infantry. **Reduces health points**.

- **`DAMAGE_DISARM`** *(v1.04)* - Disarm damage. "Special" damage type used for disarming mines, bombs, etc. (NOT for "disarming" an opponent!). Used by mine clearing systems to disarm explosive devices. Does not deal health damage but disarms explosive objects. **Reduces health points**.

- **`DAMAGE_HAZARD_CLEANUP`** *(v1.04)* - Hazard cleanup damage. Special damage type for cleaning up hazards like radiation or bio-poison. Used by cleanup systems to remove environmental hazards. Does not deal health damage but removes hazard effects. **Reduces health points**.

- **`DAMAGE_PARTICLE_BEAM`** *(v1.04)* - Particle beam damage. Incinerates virtually everything (insanely powerful orbital beam). Used by superweapon systems. Extremely powerful damage type that can destroy almost anything. **Reduces health points**.

- **`DAMAGE_TOPPLING`** *(v1.04)* - Toppling damage. Damage from getting toppled. Used by topple systems when structures fall over. Results in toppled state rather than destruction. **Reduces health points**.

- **`DAMAGE_INFANTRY_MISSILE`** *(v1.04)* - Infantry missile damage. Damage type used by infantry-carried missile weapons. Used for portable missile launchers. Typically effective against vehicles and structures. **Reduces health points**.

- **`DAMAGE_AURORA_BOMB`** *(v1.04)* - Aurora bomb damage. Damage type used by Aurora bomber weapons. Used for specialized bomber weapons. Typically very effective against structures. **Reduces health points**.

- **`DAMAGE_LAND_MINE`** *(v1.04)* - Land mine damage. Damage type used by land mines. Used for explosive traps. Typically very effective against vehicles and infantry. **Reduces health points**.

- **`DAMAGE_JET_MISSILES`** *(v1.04)* - Jet missile damage. Damage type used by jet aircraft missiles. Used for air-to-ground and air-to-air missiles from jet aircraft. Typically effective against various targets. **Reduces health points**.

- **`DAMAGE_STEALTHJET_MISSILES`** *(v1.04)* - Stealth jet missile damage. Damage type used by stealth jet aircraft missiles. Used for specialized stealth aircraft weapons. Typically effective against various targets. **Reduces health points**.

- **`DAMAGE_MOLOTOV_COCKTAIL`** *(v1.04)* - Molotov cocktail damage. Damage type used by molotov cocktail weapons. Used for incendiary thrown weapons. Sets targets on fire. **Reduces health points**.

- **`DAMAGE_COMANCHE_VULCAN`** *(v1.04)* - Comanche Vulcan damage. Damage type used by Comanche helicopter Vulcan cannon. Used for specialized helicopter weapons. Typically very effective against infantry and light vehicles. **Reduces health points**.

- **`DAMAGE_FLESHY_SNIPER`** *(v1.04, Zero Hour only)* - Fleshy sniper damage. Like `DAMAGE_SNIPER`, but generally does no damage to vehicles. Used by sniper weapons that are specifically designed to target infantry. Only effective against infantry, vehicles are immune. **Reduces health points**.

## Examples

### Basic Damage Type Usage

```ini
Weapon = TankCannon WeaponSlot_PRIMARY
  DamageType = ARMOR_PIERCING
  Damage = 150
  Range = 200
End
```

In this example, the weapon uses `ARMOR_PIERCING` damage type, which is effective against armored targets.

## Notes

- **Damage Type and Armor Interaction**: Damage types interact with armor types through damage multipliers defined in [Armor](../Armor.md) templates. The final damage amount is calculated by multiplying the base damage by the armor's resistance multiplier for that specific damage type.

- **Health Point Damage**: In Retail Zero Hour, all damage types **reduce health points** (there are no non-HP damage types in retail versions).

- **Damage Type Combinations**: Multiple damage types can be checked simultaneously by systems that need to verify if a weapon deals any of several damage types.

## Source Files

**Header (Generals):** [Damage.h](../../../../Generals/Code/GameEngine/Include/GameLogic/Damage.h) *(if retail Generals codebase exists)*
- `DamageType` enum definition (lines 43-97): Defines all available damage types for Retail Generals

**Header (Zero Hour):** [Damage.h](../../Code/GameEngine/Include/GameLogic/Damage.h)
- `DamageType` enum definition (lines 43-97): Defines all available damage types for Retail Zero Hour

**Source (Generals):** [Damage.cpp](../../../../Generals/Code/GameEngine/Source/GameLogic/Damage.cpp) *(if retail Generals codebase exists)*
- Damage type name mapping: Maps damage type names to their values for INI file parsing

**Source (Zero Hour):** [Damage.cpp](../../Code/GameEngine/Source/GameLogic/Damage.cpp)
- Damage type name mapping: Maps damage type names to their values for INI file parsing

## Changes History

- In Retail Zero Hour, 1 new damage type was added: `DAMAGE_FLESHY_SNIPER` (like `DAMAGE_SNIPER`, but generally does no damage to vehicles).

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

AI-generated, 0/2 reviews

## Reviewers

- [ ] Reviewer 1
- [ ] Reviewer 2

