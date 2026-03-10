# Weapon

Status: AI-generated, 0/2 reviews

## Overview

The `Weapon` class defines the properties and behavior of weapons in the game. Weapons are used by units, buildings, and special powers to deal damage, apply status effects, or trigger other game mechanics. Each weapon template specifies damage values, range, firing mechanics, visual effects, and targeting rules.

Available in: *(v1.04)*

**File Location**: Weapon definitions are stored in the file `Data\INI\Weapon\weapon.ini`. This file can exist in multiple locations: inside archives (`.big` , `.gib` files) and as a loose file in the game directory. Only `weapon.ini` is loaded from these locations.

When multiple `weapon.ini` files are found, they are loaded in this order:
1. Files inside archives are loaded first, in descending alphabetical order (Z to A)
2. Loose files in the game directory are loaded last

Since loose files are loaded last, they take precedence over archive files. If the same weapon name appears in multiple `weapon.ini` files, the later-loaded file completely overwrites the earlier one. Each weapon must have a unique name within each file, or the engine will crash when parsing duplicate names.

Unlike some other subsystems, weapons do not use a default folder structure (e.g., `Data\INI\Default\Weapon\`).

Weapons are defined as a list of entries (many entries allowed) in these `.ini` files. They are referenced by units, buildings, and special powers to define their combat capabilities and special abilities.


## Table of Contents

- [Overview](#overview)
- [Properties](#properties)
  - [Basic Damage Properties](#basic-damage-properties)
  - [Range and Targeting](#range-and-targeting)
  - [Firing Mechanics](#firing-mechanics)
  - [Projectile Properties](#projectile-properties)
  - [Visual and Audio Effects](#visual-and-audio-effects)
  - [Targeting and Collision](#targeting-and-collision)
  - [Continuous Fire Properties](#continuous-fire-properties)
  - [Laser and Stream Properties](#laser-and-stream-properties)
  - [Weapon Bonus Properties](#weapon-bonus-properties)
  - [Historic Bonus Properties](#historic-bonus-properties)
  - [Scatter and Accuracy Properties](#scatter-and-accuracy-properties)
  - [Advanced Properties](#advanced-properties)
- [Enum Value Lists](#enum-value-lists)
- [Examples](#examples)
- [Usage](#usage)
- [Template](#template)
- [Notes](#notes)
- [Modder Recommended Use Scenarios](#modder-recommended-use-scenarios)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Properties

### Basic Damage Properties

#### `PrimaryDamage` *(v1.04)*
- **Type**: `Real`
- **Description**: The damage amount applied to objects within the primary damage radius. When a weapon deals damage, objects at or closer than [`PrimaryDamageRadius`](#primarydamageradius) from the impact point receive this damage amount. The primary target (if any) always receives primary damage regardless of distance. This damage is modified by weapon bonuses, armor types, and damage multipliers. If [`PrimaryDamageRadius`](#primarydamageradius) is 0, only the direct target receives this damage (no area effect).
- **Default**: `0.0`
- **Example**: `PrimaryDamage = 350.0`

#### `PrimaryDamageRadius` *(v1.04)*
- **Type**: `Real`
- **Description**: The radius (in world units) from the impact point within which objects receive [`PrimaryDamage`](#primarydamage). Objects at a distance squared less than or equal to this radius squared take primary damage. If this is 0, the weapon only affects the direct target (no area effect). [`SecondaryDamageRadius`](#secondarydamageradius) must be greater than or equal to [`PrimaryDamageRadius`](#primarydamageradius) (or zero). This radius is modified by weapon bonuses that affect radius.
- **Default**: `0.0`
- **Example**: `PrimaryDamageRadius = 40.0`

#### `SecondaryDamage` *(v1.04)*
- **Type**: `Real`
- **Description**: The damage amount applied to objects beyond the primary damage radius but within the secondary damage radius. Objects farther than [`PrimaryDamageRadius`](#primarydamageradius) but at or within [`SecondaryDamageRadius`](#secondarydamageradius) from the impact point receive this damage amount instead of primary damage. This allows creating weapons with a high-damage core area and a lower-damage outer area. If [`SecondaryDamageRadius`](#secondarydamageradius) is 0 or equal to [`PrimaryDamageRadius`](#primarydamageradius), secondary damage is never applied. This damage is modified by weapon bonuses, armor types, and damage multipliers.
- **Default**: `0.0`
- **Example**: `SecondaryDamage = 200.0`

#### `SecondaryDamageRadius` *(v1.04)*
- **Type**: `Real`
- **Description**: The radius (in world units) from the impact point within which objects can receive [`SecondaryDamage`](#secondarydamage). Objects beyond [`PrimaryDamageRadius`](#primarydamageradius) but at or within this radius take secondary damage. This value must be greater than or equal to [`PrimaryDamageRadius`](#primarydamageradius) (or zero). If this is 0 or equal to [`PrimaryDamageRadius`](#primarydamageradius), only primary damage is applied. If this is greater than [`PrimaryDamageRadius`](#primarydamageradius), objects in the ring between the two radii receive secondary damage. This radius is modified by weapon bonuses that affect radius.
- **Default**: `0.0`
- **Example**: `SecondaryDamageRadius = 60.0`

#### `DamageType`
Available in: *(v1.04)*

- **Type**: `DamageType` (see [DamageType documentation](Enum/DamageType.md))
- **Description**: The type of damage this weapon deals, which determines how the damage interacts with armor types, affects gameplay mechanics, and triggers special behaviors. Different damage types are modified differently by armor values - for example, some armor types may be more resistant to certain damage types, while being vulnerable to others. This creates tactical depth where certain weapons are more effective against specific targets. The damage type also affects how the damage is calculated and applied, whether it reduces health points, and what special effects or behaviors it may trigger. Some damage types have unique properties: explosive damage may cause area effects, flame damage may set targets on fire, laser damage may have high accuracy, and status damage may apply special conditions without reducing health. The damage type works in combination with [`PrimaryDamage`](#primarydamage) and [`SecondaryDamage`](#secondarydamage) to determine the final damage dealt, and should typically match the weapon's visual and audio effects for consistency. Commonly paired with appropriate [`DeathType`](#deathtype) values to create cohesive weapon theming (e.g., explosive damage with explosion death type). See the [DamageType documentation](Enum/DamageType.md) for a complete list of available damage types and their specific properties.
- **Default**: `EXPLOSION`
- **Example**: `DamageType = EXPLOSION`

#### `DeathType` *(v1.04)*
- **Type**: `DeathType`
- **Description**: The type of death animation and visual/audio effects that are triggered when this weapon kills a target unit. This property controls how units die visually and audibly, providing appropriate death feedback that matches the weapon type and creates immersive gameplay. Different death types trigger different death animations, visual effects (such as explosions, fire, or laser effects), and audio effects, creating distinct death experiences for different weapon types. The death type is purely cosmetic and does not affect damage calculations, armor interactions, or gameplay mechanics - it only affects how the death is displayed. However, choosing an appropriate death type enhances the player experience by providing visual and audio feedback that matches the weapon's nature. For example, explosive weapons should use explosion death types to show dramatic explosions, while flame weapons should use fire death types to show units burning. The death type should typically match the weapon's [`DamageType`](#damagetype) for thematic consistency (e.g., explosive damage with explosion death, flame damage with fire death). Some death types have special gameplay interactions: for instance, the `BURNED` death type allows flame weapons to target and destroy vegetation regardless of other factors. See the [`DeathType` Values](#deathtype-values) section below for a complete list of available death types and their specific properties.
- **Default**: `NORMAL`
- **Example**: `DeathType = EXPLODED`
- **Available Values**: See [`DeathType` Values](#deathtype-values) section below

<a id="damagestatustype"></a>
#### `DamageStatusType`
Available in: *(v1.04, Zero Hour only)*

- **Type**: `ObjectStatusTypes` (see [ObjectStatusTypes documentation](../Object/Enums/ObjectStatusTypes.md))
- **Description**: If the damage type is [`DAMAGE_STATUS`](Enum/DamageType.md#damage_status) (see [DamageType documentation](Enum/DamageType.md)), this specifies which status effect to apply to the target. The status effect is applied when the weapon deals damage.
- **Default**: [`OBJECT_STATUS_NONE`](../Object/Enums/ObjectStatusTypes.md) (see [ObjectStatusTypes documentation](../Object/Enums/ObjectStatusTypes.md))
- **Example**: `DamageStatusType = CAN_ATTACK`

### Range and Targeting

#### `AttackRange` *(v1.04)*
- **Type**: `Real`
- **Description**: Maximum distance (in world units) at which this weapon will consider targets valid for acquisition and firing. Targets beyond this range are ignored for both targeting and firing. A small margin is applied to prevent targets from teetering on the edge of range. For projectile weapons, this works in combination with [`WeaponSpeed`](#weaponspeed)/[`MinWeaponSpeed`](#minweaponspeed) and accuracy properties. The effective range may be modified by weapon bonuses that affect range. If this is 0, the weapon cannot acquire targets (contact weapons may still function differently).
- **Default**: `0.0`
- **Example**: `AttackRange = 450.0`

#### `MinimumAttackRange` *(v1.04)*
- **Type**: `Real`
- **Description**: Minimum distance (in world units) required before this weapon may fire. If a target is closer than this value (with a small margin applied), the weapon will not fire even if it is otherwise valid and acquired. This prevents self-damage from area effect weapons, enforces standoff behavior for artillery, and prevents units from firing point-blank when that would be undesirable. A small margin is applied to this value to ensure consistent behavior. If this is 0, there is no minimum range restriction.
- **Default**: `0.0`
- **Example**: `MinimumAttackRange = 5.0`

#### `AcceptableAimDelta` *(v1.04)*
- **Type**: `Real` (angle in degrees)
- **Description**: Maximum allowed angular error (in degrees) between the weapon's current aim direction and the ideal direction to the target before a shot can be released. The weapon will wait until the aim error is within this tolerance before firing. Smaller values (e.g., 1-2 degrees) enforce very precise aiming, requiring the weapon to be nearly perfectly aligned before firing - useful for sniper weapons or high-accuracy weapons. Larger values (e.g., 10-15 degrees) allow the weapon to fire while still tracking or moving, creating a more "spray and pray" behavior. A value of 0 means the weapon must be perfectly aimed (may cause issues with moving targets). Very large values (e.g., 999) effectively disable aim checking, allowing immediate firing.
- **Default**: `0.0`
- **Example**: `AcceptableAimDelta = 1`

#### `RequestAssistRange` *(v1.04)*
- **Type**: `Real`
- **Description**: Range (in world units) within which other units with the same weapon will be called to assist in attacking the same target. When a unit with this weapon acquires a target, it broadcasts a request for assistance to other units within this range. Units that receive the request and can target the same enemy will join the attack, creating coordinated fire. This allows multiple units to focus fire on a single target, increasing damage output. The range is measured from the requesting unit's position. If set to 0, the weapon does not request assistance from other units. This property is useful for coordinating attacks from groups of units.
- **Default**: `0.0`
- **Example**: `RequestAssistRange = 100.0`

### Firing Mechanics

#### `DelayBetweenShots` *(v1.04)*
- **Type**: `Int` (milliseconds) or `Min:Max` range
- **Description**: Time delay between consecutive shots fired by this weapon. This property accepts two formats: (1) A single integer value (e.g., `1000`) which creates a constant delay between all shots, or (2) A labeled range format `Min:1000 Max:2000` which creates a variable delay where a random value between min and max (inclusive) is selected for each shot. The values are specified in milliseconds but are converted to game frames. If both min and max are 0, there is no enforced delay between shots (weapon fires as fast as possible). Variable delays add unpredictability to weapon firing patterns.
- **Default**: `0` (no enforced delay)
- **Example**: `DelayBetweenShots = 1000` or `DelayBetweenShots = Min:800 Max:1200`

#### `ClipSize` *(v1.04)*
- **Type**: `Int`
- **Description**: Number of shots that can be fired before the weapon must reload. When the weapon fires, it consumes one shot from the clip. Once the clip is empty (all shots fired), the weapon must reload using [`ClipReloadTime`](#clipreloadtime) before it can fire again. If set to 0, the weapon has infinite ammo and never needs to reload. Higher values allow more shots before reloading, but also increase the time required to reload. This property works in combination with [`AutoReloadsClip`](#autoreloadsclip) and [`AutoReloadWhenIdle`](#autoreloadwhenidle) to control reload behavior.
- **Default**: `0`
- **Example**: `ClipSize = 30`

#### `ClipReloadTime` *(v1.04)*
- **Type**: `Int` (milliseconds)
- **Description**: Time (in milliseconds) required to reload the weapon when the clip is empty. This delay is applied after the weapon has fired all shots in the [`ClipSize`](#clipsize) and needs to reload before it can fire again. The reload time prevents the weapon from firing during this period. If [`AutoReloadsClip`](#autoreloadsclip) is enabled, the reload happens automatically when the clip is empty. Higher values create longer reload times, balancing weapons with large clip sizes. If [`ClipSize`](#clipsize) is 0 (infinite ammo), this property has no effect.
- **Default**: `0`
- **Example**: `ClipReloadTime = 3000`

#### `AutoReloadsClip` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon automatically begins reloading when the [`ClipSize`](#clipsize) is exhausted. When enabled, the weapon automatically starts the reload process (using [`ClipReloadTime`](#clipreloadtime)) as soon as the last shot in the clip is fired. When disabled, the weapon will not reload automatically and must be manually reloaded or will reload when [`AutoReloadWhenIdle`](#autoreloadwhenidle) triggers. This property only has an effect if [`ClipSize`](#clipsize) is greater than 0.
- **Default**: `Yes`
- **Example**: `AutoReloadsClip = Yes`

#### `AutoReloadWhenIdle` *(v1.04)*
- **Type**: `Int` (milliseconds)
- **Description**: Time (in milliseconds) of inactivity (not firing) before the weapon automatically reloads if the clip is not full. If the weapon has not fired for this duration and the current clip has fewer shots than [`ClipSize`](#clipsize), the weapon will automatically reload to fill the clip. This allows weapons to reload during lulls in combat without manual intervention. If set to 0, the weapon will not automatically reload when idle. This property only has an effect if [`ClipSize`](#clipsize) is greater than 0.
- **Default**: `0`
- **Example**: `AutoReloadWhenIdle = 5000`

#### `PreAttackDelay` *(v1.04)*
- **Type**: `Int` (milliseconds)
- **Description**: Delay (in milliseconds) before the weapon fires after acquiring a target. This delay is applied based on the [`PreAttackType`](#preattacktype) setting, which determines when the delay occurs (before each shot, before each new target, or before each new clip). The delay allows time for aiming, charging, or other preparation before firing. Higher values create longer preparation times, useful for weapons that need to lock on, charge up, or aim precisely before firing. If set to 0, there is no pre-attack delay and the weapon fires immediately when ready.
- **Default**: `0`
- **Example**: `PreAttackDelay = 2000`

#### `PreAttackType` *(v1.04)*
- **Type**: `WeaponPrefireType`
- **Description**: Controls when the [`PreAttackDelay`](#preattackdelay) is applied. This determines the timing of the pre-attack delay: before each shot (`PER_SHOT`), before each new target (`PER_ATTACK`), or before each new clip (`PER_CLIP`). This works in combination with [`PreAttackDelay`](#preattackdelay) to create different firing patterns. For example, `PER_ATTACK` allows a weapon to have an initial delay when acquiring a new target but then fire rapidly at that same target, while `PER_SHOT` applies the delay before every single shot.
- **Default**: `PER_SHOT`
- **Example**: `PreAttackType = PER_SHOT`
- **Available Values**: See [WeaponPrefireType Values](#weaponprefiretype-values) section below

### Projectile Properties

#### `ProjectileObject` *(v1.04)*
- **Type**: `object` (see [Object documentation](Object.md))
- **Description**: The projectile object template that is created and launched when the weapon fires. This object represents the physical projectile that travels from the firing point to the target. The projectile object is created at the weapon's firing point with an initial velocity determined by [`WeaponSpeed`](#weaponspeed) and [`MinWeaponSpeed`](#minweaponspeed), and travels toward the target following the weapon's trajectory. The projectile object must be defined in the game's object definitions and can have its own behaviors, visual effects, and collision properties. If not specified, the weapon is an instant-hit weapon that deals damage immediately without a visible projectile. The projectile object is destroyed when it impacts the target, hits terrain, or reaches its maximum range.
- **Example**: `ProjectileObject = TankShell`

#### `WeaponSpeed` *(v1.04)*
- **Type**: `Real` (distance per second)
- **Description**: Speed (in world units per second) at which the projectile travels from the firing point to the target. For projectile weapons, this determines how fast the projectile moves through the air. For instant-hit weapons (no projectile), this property may not apply. Higher values make projectiles travel faster, reducing the time to impact. This works in combination with [`AttackRange`](#attackrange) to determine the effective engagement time. For variable-speed weapons, use [`MinWeaponSpeed`](#minweaponspeed) to set a minimum speed, and if [`ScaleWeaponSpeed`](#scaleweaponspeed) is enabled, the speed may scale with range for lobbed projectiles.
- **Default**: `0.0`
- **Example**: `WeaponSpeed = 1000.0`

#### `MinWeaponSpeed` *(v1.04)*
- **Type**: `Real` (distance per second)
- **Description**: Minimum speed (in world units per second) for variable-speed weapons. This sets the lower bound for projectile speed when the weapon uses variable speed mechanics. The actual speed may vary between [`MinWeaponSpeed`](#minweaponspeed) and [`WeaponSpeed`](#weaponspeed) based on range, angle, or other factors. If [`ScaleWeaponSpeed`](#scaleweaponspeed) is enabled, the speed scales with range, and this minimum ensures the projectile never travels slower than this value. If this is 0 or equal to [`WeaponSpeed`](#weaponspeed), the weapon uses a constant speed.
- **Default**: `0.0`
- **Example**: `MinWeaponSpeed = 500.0`

#### `ScaleWeaponSpeed` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon speed scales with range for lobbed projectiles. When enabled, the projectile speed varies based on the distance to the target, creating a more realistic arc trajectory for artillery or mortar-style weapons. The speed scales between [`MinWeaponSpeed`](#minweaponspeed) and [`WeaponSpeed`](#weaponspeed) based on range, with longer ranges potentially using different speeds to create the desired arc. This is useful for weapons that need to arc over obstacles or create a parabolic trajectory. When disabled, the weapon uses a constant speed as specified by [`WeaponSpeed`](#weaponspeed).
- **Default**: `No`
- **Example**: `ScaleWeaponSpeed = Yes`

#### `WeaponRecoil` *(v1.04)*
- **Type**: `Real` (radians)
- **Description**: Amount of recoil (angular displacement in radians) imparted to the firing unit when the weapon fires. Recoil causes the firing unit to rotate or move backward slightly when firing, simulating the physical force of weapon discharge. Higher values create more pronounced recoil effects, causing the unit to rotate more or move backward further. Recoil can affect the unit's aim and position, making subsequent shots less accurate if the unit hasn't recovered. This is useful for balancing powerful weapons by adding a physical cost to firing. If set to 0, there is no recoil effect. The recoil is applied in the direction opposite to the firing direction.
- **Default**: `0.0`
- **Example**: `WeaponRecoil = 0.1`

### Visual and Audio Effects

#### `FireFX` *(v1.04)*
- **Type**: `FXList` (per veterancy level)
- **Description**: Visual effects (FXList) that are played at the weapon's firing point when the weapon fires. These effects are displayed at the muzzle or firing location and provide visual feedback for weapon discharge. The effects are played every time the weapon fires, synchronized with the firing animation. If you need different effects for different veterancy levels, use [`VeterancyFireFX`](#veterancyfirefx) instead. The FXList can contain multiple effects that play simultaneously or in sequence.
- **Example**: `FireFX = FX_TankCannonFire`

#### `ProjectileDetonationFX` *(v1.04)*
- **Type**: `FXList` (per veterancy level)
- **Description**: Visual effects (FXList) that are played when the projectile from this weapon impacts its target or detonates. These effects are displayed at the impact point and provide visual feedback for the weapon's damage delivery. The effects are played when the projectile collides with a target, hits terrain, or reaches its detonation point. If you need different effects for different veterancy levels, use [`VeterancyProjectileDetonationFX`](#veterancyprojectiledetonationfx) instead. The FXList can contain multiple effects that play simultaneously or in sequence.
- **Example**: `ProjectileDetonationFX = FX_ExplosionLarge`

#### `FireSound` *(v1.04)*
- **Type**: `AudioEventRTS`
- **Description**: Audio event (AudioEventRTS) that is played when the weapon fires. This sound is triggered at the weapon's firing point and provides audio feedback for weapon discharge. The sound plays every time the weapon fires, synchronized with the firing animation. For continuous-fire weapons, you can use [`FireSoundLoopTime`](#firesoundlooptime) to loop the sound for a specified duration. The audio event must be defined in the game's audio system and can include 3D positional audio that varies based on the listener's position relative to the firing unit.
- **Example**: `FireSound = TankCannonFire`

#### `FireSoundLoopTime` *(v1.04)*
- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Duration (in milliseconds) for looping the [`FireSound`](#firesound) audio event. When this is set to a non-zero value, the fire sound loops for the specified duration, creating a continuous firing sound effect. This is useful for weapons that fire continuously or have a sustained firing pattern, such as machine guns or flamethrowers. The sound loops from the start of the audio event for the specified duration. If this is 0, the fire sound plays once per shot without looping.
- **Default**: `0`
- **Example**: `FireSoundLoopTime = 1000`

#### `FireOCL` *(v1.04)*
- **Type**: `ObjectCreationList` (per veterancy level)
- **Description**: ObjectCreationList that specifies objects to be created at the weapon's firing point when the weapon fires. These objects are spawned at the muzzle or firing location and can include muzzle flashes, shell casings, smoke, or other visual elements. The objects are created every time the weapon fires, synchronized with the firing animation. If you need different objects for different veterancy levels, use [`VeterancyFireOCL`](#veterancyfireocl) instead. The ObjectCreationList can contain multiple objects that are created simultaneously or in sequence.
- **Example**: `FireOCL = OCL_MuzzleFlash`

#### `ProjectileDetonationOCL` *(v1.04)*
- **Type**: `ObjectCreationList` (per veterancy level)
- **Description**: ObjectCreationList that specifies objects to be created when the projectile from this weapon impacts its target or detonates. These objects are spawned at the impact point and can include debris, fragments, secondary projectiles, or other visual elements. The objects are created when the projectile collides with a target, hits terrain, or reaches its detonation point. If you need different objects for different veterancy levels, use [`VeterancyProjectileDetonationOCL`](#veterancyprojectiledetonationocl) instead. The ObjectCreationList can contain multiple objects that are created simultaneously or in sequence.
- **Example**: `ProjectileDetonationOCL = OCL_ExplosionDebris`

#### `ProjectileExhaust` *(v1.04)*
- **Type**: `ParticleSystemTemplate` (per veterancy level)
- **Description**: Particle system template that defines the exhaust trail or smoke trail emitted by the projectile as it travels. This creates a visual trail that follows the projectile from the firing point to the impact point, providing visual feedback for projectile movement. The particle system is attached to the projectile and renders continuously while the projectile is in flight. If you need different particle effects for different veterancy levels, use [`VeterancyProjectileExhaust`](#veterancyprojectileexhaust) instead. The particle system template must be defined in the game's particle system definitions.
- **Example**: `ProjectileExhaust = PS_TankShellTrail`

#### `VeterancyFireFX` *(v1.04)*
- **Type**: `FXList` (per veterancy level)
- **Description**: Visual effects (FXList) that are played at the weapon's firing point when the weapon fires, specified per veterancy level. This allows you to define different visual effects for each veterancy level (Veteran, Elite, Heroic). Use this instead of [`FireFX`](#firefx) when you need different effects for each veterancy level. The effects are played every time the weapon fires, synchronized with the firing animation. The format allows specifying effects for each veterancy level separately.
- **Example**: `VeterancyFireFX = VETERAN FX_TankCannonFireVeteran`

#### `VeterancyProjectileDetonationFX` *(v1.04)*
- **Type**: `FXList` (per veterancy level)
- **Description**: Visual effects (FXList) that are played when the projectile impacts its target or detonates, specified per veterancy level. This allows you to define different visual effects for each veterancy level (Veteran, Elite, Heroic). Use this instead of [`ProjectileDetonationFX`](#projectiledetonationfx) when you need different effects for each veterancy level. The effects are displayed at the impact point and provide visual feedback for the weapon's damage delivery. The format allows specifying effects for each veterancy level separately.
- **Example**: `VeterancyProjectileDetonationFX = VETERAN FX_ExplosionLargeVeteran`

#### `VeterancyFireOCL` *(v1.04)*
- **Type**: `ObjectCreationList` (per veterancy level)
- **Description**: ObjectCreationList that specifies objects to be created at the weapon's firing point when the weapon fires, specified per veterancy level. This allows you to define different objects for each veterancy level (Veteran, Elite, Heroic). Use this instead of [`FireOCL`](#fireocl) when you need different objects for each veterancy level. The objects are spawned at the muzzle or firing location and can include muzzle flashes, shell casings, smoke, or other visual elements. The format allows specifying objects for each veterancy level separately.
- **Example**: `VeterancyFireOCL = VETERAN OCL_MuzzleFlashVeteran`

#### `VeterancyProjectileDetonationOCL` *(v1.04)*
- **Type**: `ObjectCreationList` (per veterancy level)
- **Description**: ObjectCreationList that specifies objects to be created when the projectile impacts its target or detonates, specified per veterancy level. This allows you to define different objects for each veterancy level (Veteran, Elite, Heroic). Use this instead of [`ProjectileDetonationOCL`](#projectiledetonationocl) when you need different objects for each veterancy level. The objects are spawned at the impact point and can include debris, fragments, secondary projectiles, or other visual elements. The format allows specifying objects for each veterancy level separately.
- **Example**: `VeterancyProjectileDetonationOCL = VETERAN OCL_ExplosionDebrisVeteran`

#### `VeterancyProjectileExhaust` *(v1.04)*
- **Type**: `ParticleSystemTemplate` (per veterancy level)
- **Description**: Particle system template that defines the exhaust trail or smoke trail emitted by the projectile as it travels, specified per veterancy level. This allows you to define different particle effects for each veterancy level (Veteran, Elite, Heroic). Use this instead of [`ProjectileExhaust`](#projectileexhaust) when you need different effects for each veterancy level. The particle system is attached to the projectile and renders continuously while the projectile is in flight. The format allows specifying particle systems for each veterancy level separately.
- **Example**: `VeterancyProjectileExhaust = VETERAN PS_TankShellTrailVeteran`

### Targeting and Collision

#### `RadiusDamageAffects` *(v1.04)*
- **Type**: `WeaponAffectsMaskType` (bit flags)
- **Description**: Bit flags that determine which types of objects are affected by the weapon's radius damage (both [`PrimaryDamageRadius`](#primarydamageradius) and [`SecondaryDamageRadius`](#secondarydamageradius)). This property controls whether the weapon's area effect damage affects allies, enemies, neutral units, or specific object categories. Multiple flags can be combined to affect multiple types. This allows weapons to selectively damage only certain types of targets, such as weapons that only damage enemies or weapons that damage everything including allies. If no flags are set, radius damage does not affect any objects (only direct hits apply damage). This property works in combination with the weapon's targeting properties to determine what can be damaged.
- **Example**: `RadiusDamageAffects = ALLIES ENEMIES NEUTRALS`
- **Available Values**: See [WeaponAffectsMaskType Values](#weaponaffectsmasktype-values) section below


#### `ProjectileCollidesWith` *(v1.04)*
- **Type**: `WeaponCollideMaskType` (bit flags)
- **Description**: Bit flags that determine which types of objects the [`ProjectileObject`](#projectileobject) can collide with during flight. This property controls collision detection for projectiles, determining what objects will cause the projectile to impact and detonate. Multiple flags can be combined to allow collisions with multiple object types. This allows projectiles to pass through certain objects (by not including their flags) or to impact specific types of objects. For example, a projectile that can only collide with structures and walls will pass through units and terrain, while a projectile that collides with everything will impact the first object it encounters. This property only applies to weapons with a [`ProjectileObject`](#projectileobject) specified.
- **Example**: `ProjectileCollidesWith = STRUCTURES WALLS`
- **Available Values**: See [WeaponCollideMaskType Values](#weaponcollidemasktype-values) section below

#### `AntiGround` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can target and fire at ground-based units and objects. When enabled, the weapon can acquire and attack targets that are on the ground, including ground vehicles, infantry, and ground-based structures. When disabled, the weapon cannot target ground units, restricting it to other target types (airborne units, projectiles, etc.). This property works in combination with other Anti- properties ([`AntiAirborneVehicle`](#antiairbornevehicle), [`AntiAirborneInfantry`](#antiairborneinfantry), [`AntiProjectile`](#antiprojectile), [`AntiSmallMissile`](#antismallmissile), [`AntiBallisticMissile`](#antibalisticmissile), [`AntiMine`](#antimine), [`AntiParachute`](#antiparachute)) to define the weapon's targeting capabilities. All Anti- properties can be enabled simultaneously to allow the weapon to target multiple categories.
- **Default**: `Yes`
- **Example**: `AntiGround = Yes`

#### `AntiAirborneVehicle` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can target and fire at airborne vehicles (aircraft). When enabled, the weapon can acquire and attack targets that are flying vehicles, such as helicopters, jets, and other aircraft. When disabled, the weapon cannot target airborne vehicles, restricting it to other target types (ground units, projectiles, etc.). This property works in combination with other Anti- properties ([`AntiGround`](#antiground), [`AntiAirborneInfantry`](#antiairborneinfantry), [`AntiProjectile`](#antiprojectile), [`AntiSmallMissile`](#antismallmissile), [`AntiBallisticMissile`](#antibalisticmissile), [`AntiMine`](#antimine), [`AntiParachute`](#antiparachute)) to define the weapon's targeting capabilities. This is essential for anti-aircraft weapons that need to engage flying targets.
- **Default**: `No`
- **Example**: `AntiAirborneVehicle = Yes`

#### `AntiAirborneInfantry` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can target and fire at airborne infantry units (paratroopers, flying infantry). When enabled, the weapon can acquire and attack targets that are infantry units that are airborne or flying. When disabled, the weapon cannot target airborne infantry, restricting it to other target types (ground units, vehicles, projectiles, etc.). This property works in combination with other Anti- properties ([`AntiGround`](#antiground), [`AntiAirborneVehicle`](#antiairbornevehicle), [`AntiProjectile`](#antiprojectile), [`AntiSmallMissile`](#antismallmissile), [`AntiBallisticMissile`](#antibalisticmissile), [`AntiMine`](#antimine), [`AntiParachute`](#antiparachute)) to define the weapon's targeting capabilities. This is useful for weapons that need to engage paratroopers or other flying infantry units.
- **Default**: `No`
- **Example**: `AntiAirborneInfantry = No`

#### `AntiProjectile` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can target and fire at projectile objects in flight. When enabled, the weapon can acquire and attack targets that are projectiles (missiles, shells, etc.) that are currently in flight. When disabled, the weapon cannot target projectiles, restricting it to other target types (units, structures, etc.). This property works in combination with other Anti- properties ([`AntiGround`](#antiground), [`AntiAirborneVehicle`](#antiairbornevehicle), [`AntiAirborneInfantry`](#antiairborneinfantry), [`AntiSmallMissile`](#antismallmissile), [`AntiBallisticMissile`](#antibalisticmissile), [`AntiMine`](#antimine), [`AntiParachute`](#antiparachute)) to define the weapon's targeting capabilities. This is essential for point-defense weapons or anti-missile systems that intercept incoming projectiles.
- **Default**: `No`
- **Example**: `AntiProjectile = Yes`

#### `AntiSmallMissile` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can target and fire at small missile objects specifically. When enabled, the weapon can acquire and attack targets that are small missiles (typically defined by the `KINDOF_SMALL_MISSILE` flag). When disabled, the weapon cannot target small missiles, restricting it to other target types. This property works in combination with other Anti- properties ([`AntiGround`](#antiground), [`AntiAirborneVehicle`](#antiairbornevehicle), [`AntiAirborneInfantry`](#antiairborneinfantry), [`AntiProjectile`](#antiprojectile), [`AntiBallisticMissile`](#antibalisticmissile), [`AntiMine`](#antimine), [`AntiParachute`](#antiparachute)) to define the weapon's targeting capabilities. This allows weapons to specifically target small missiles (like anti-tank missiles) while potentially ignoring larger projectiles or other target types. This is useful for point-defense systems that need to intercept specific types of missiles.
- **Default**: `No`
- **Example**: `AntiSmallMissile = Yes`

#### `AntiBallisticMissile` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can target and fire at ballistic missile objects specifically. When enabled, the weapon can acquire and attack targets that are ballistic missiles (typically defined by the `KINDOF_BALLISTIC_MISSILE` flag). When disabled, the weapon cannot target ballistic missiles, restricting it to other target types. This property works in combination with other Anti- properties ([`AntiGround`](#antiground), [`AntiAirborneVehicle`](#antiairbornevehicle), [`AntiAirborneInfantry`](#antiairborneinfantry), [`AntiProjectile`](#antiprojectile), [`AntiSmallMissile`](#antismallmissile), [`AntiMine`](#antimine), [`AntiParachute`](#antiparachute)) to define the weapon's targeting capabilities. This is essential for anti-ballistic missile defense systems that need to intercept long-range ballistic missiles.
- **Default**: `No`
- **Example**: `AntiBallisticMissile = Yes`

#### `AntiMine` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can target and fire at mine objects. When enabled, the weapon can acquire and attack targets that are mines (land mines, naval mines, etc.). When disabled, the weapon cannot target mines, restricting it to other target types. This property works in combination with other Anti- properties ([`AntiGround`](#antiground), [`AntiAirborneVehicle`](#antiairbornevehicle), [`AntiAirborneInfantry`](#antiairborneinfantry), [`AntiProjectile`](#antiprojectile), [`AntiSmallMissile`](#antismallmissile), [`AntiBallisticMissile`](#antibalisticmissile), [`AntiParachute`](#antiparachute)) to define the weapon's targeting capabilities. This is useful for mine-clearing weapons or units that need to destroy mines.
- **Default**: `No`
- **Example**: `AntiMine = Yes`

#### `AntiParachute` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can target and fire at parachute objects. When enabled, the weapon can acquire and attack targets that are parachutes (typically used by paratroopers or supply drops). When disabled, the weapon cannot target parachutes, restricting it to other target types. This property works in combination with other Anti- properties ([`AntiGround`](#antiground), [`AntiAirborneVehicle`](#antiairbornevehicle), [`AntiAirborneInfantry`](#antiairborneinfantry), [`AntiProjectile`](#antiprojectile), [`AntiSmallMissile`](#antismallmissile), [`AntiBallisticMissile`](#antibalisticmissile), [`AntiMine`](#antimine)) to define the weapon's targeting capabilities. This is useful for weapons that need to engage paratroopers while they are descending or to destroy supply drops.
- **Default**: `No`
- **Example**: `AntiParachute = Yes`


### Advanced Properties

#### `ShockWaveAmount` *(v1.04, Generals Zero Hour only)*
- **Type**: `Real`
- **Description**: Intensity or magnitude of the shockwave effect generated when the weapon impacts. The shockwave effect creates a visual and potentially physical disturbance in the environment, simulating the force of an explosion. Higher values create more intense shockwave effects, with stronger visual distortion and potentially greater environmental impact. The shockwave effect is applied within the [`ShockWaveRadius`](#shockwaveradius) and tapers off based on [`ShockWaveTaperOff`](#shockwavetaperoff). This property works in combination with the other shockwave properties to create realistic explosion effects. If set to 0, no shockwave effect is generated.
- **Default**: `0.0`
- **Example**: `ShockWaveAmount = 25.0`

#### `ShockWaveRadius` *(v1.04, Generals Zero Hour only)*
- **Type**: `Real`
- **Description**: Radius (in world units) within which the shockwave effect is applied when the weapon impacts. The shockwave effect extends from the impact point to this distance, creating a circular area of effect. Objects and the environment within this radius experience the shockwave effect with intensity determined by [`ShockWaveAmount`](#shockwaveamount) and distance from the impact point. The effect tapers off toward the edge of the radius based on [`ShockWaveTaperOff`](#shockwavetaperoff). Higher values create larger shockwave areas, affecting a wider area around the impact point. This property works in combination with the other shockwave properties to create realistic explosion effects.
- **Default**: `0.0`
- **Example**: `ShockWaveRadius = 70.0`

#### `ShockWaveTaperOff` *(v1.04, Generals Zero Hour only)*
- **Type**: `Real`
- **Description**: Multiplier (0.0 to 1.0) that determines how much of the [`ShockWaveAmount`](#shockwaveamount) remains at the edge of the [`ShockWaveRadius`](#shockwaveradius). This creates a falloff effect where the shockwave intensity decreases from the impact point to the edge of the radius. A value of 1.0 means the full shockwave amount is applied even at the edge (no falloff), while a value of 0.0 means no shockwave effect at the edge (maximum falloff). Values between 0.0 and 1.0 create a gradual falloff, with the shockwave intensity linearly interpolating from full strength at the impact point to this multiplier value at the edge. This property works in combination with [`ShockWaveAmount`](#shockwaveamount) and [`ShockWaveRadius`](#shockwaveradius) to create realistic explosion effects with proper distance-based intensity falloff.
- **Default**: `0.0`
- **Example**: `ShockWaveTaperOff = 0.50`

#### `ScatterRadius` *(v1.04)*
- **Type**: `Real`
- **Description**: Maximum random offset radius (in world units) applied to the intended impact point to simulate weapon inaccuracy. When the weapon fires, the actual impact point is randomly offset from the target position by up to this radius in any direction. This creates a circular area of potential impact. This scatter is applied to all targets. If this is 0, there is no random scatter (weapon is perfectly accurate). Scatter is useful for simulating artillery, mortars, or low-accuracy weapons. The scatter is applied in addition to any [`ScatterRadiusVsInfantry`](#scatterradiusvsinfantry) when targeting infantry.
- **Default**: `0.0`
- **Example**: `ScatterRadius = 5.0`

#### `ScatterTargetScalar` *(v1.04)*
- **Type**: `Real`
- **Description**: Multiplier applied to the coordinates specified in [`ScatterTarget`](#scattertarget) entries. This allows you to scale preset scatter target offsets without redefining all the coordinates. A value of 1.0 uses the scatter targets at their defined scale. Values greater than 1.0 increase the scatter spread, values less than 1.0 decrease it. This only affects weapons that use [`ScatterTarget`](#scattertarget) entries; it has no effect if [`ScatterTarget`](#scattertarget) is not used.
- **Default**: `0.0`
- **Example**: `ScatterTargetScalar = 1.0`

#### `ScatterRadiusVsInfantry` *(v1.04)*
- **Type**: `Real`
- **Description**: Additional random scatter radius (in world units) applied specifically when the target is infantry. This extra inaccuracy is added to the base [`ScatterRadius`](#scatterradius) when targeting infantry units, simulating the difficulty of hitting small, mobile targets. If the target is not infantry, only [`ScatterRadius`](#scatterradius) applies. This allows weapons to be more accurate against vehicles/buildings but less accurate against infantry. If this is 0, there is no extra scatter against infantry (only base [`ScatterRadius`](#scatterradius) applies).
- **Default**: `0.0`
- **Example**: `ScatterRadiusVsInfantry = 10.0`

#### `DamageDealtAtSelfPosition` *(v1.04)*
- **Type**: `Bool`
- **Description**: When enabled, the damage epicenter is the firing unit's current position rather than the target's position. This causes all radius damage calculations to originate from the shooter, not the impact point. This is essential for suicide weapons (like car bombs) where the explosion should occur at the attacker's location. When disabled (default), damage originates from the target position (for direct hits) or the projectile impact point (for projectile weapons). This property affects both [`PrimaryDamageRadius`](#primarydamageradius) and [`SecondaryDamageRadius`](#secondarydamageradius) calculations.
- **Default**: `No`
- **Example**: `DamageDealtAtSelfPosition = Yes`

#### `ContinueAttackRange` *(v1.04)*
- **Type**: `Real`
- **Description**: Radius (in world units) within which the weapon will automatically search for and acquire new similar targets after destroying the current target, without requiring manual reacquisition. When a target is destroyed, becomes effectively dead, or is a mine that becomes masked, the weapon automatically looks for another valid target of the same type within this range. This allows weapons to chain attacks across multiple targets without player intervention. The weapon temporarily sets the `IGNORING_STEALTH` status flag to find stealthed targets during this search. If this is 0, the weapon does not automatically continue attacking after destroying a target (normal behavior).
- **Default**: `0.0`
- **Example**: `ContinueAttackRange = 50.0`

#### `MinTargetPitch` *(v1.04)*
- **Type**: `Real` (angle in radians)
- **Description**: Lowest vertical aim angle (pitch) allowed when acquiring and firing at targets. This restricts the weapon's ability to aim downward. Values are in radians, where negative values allow aiming below horizontal (downward) and positive values restrict downward aiming. The default of -PI allows aiming in any downward direction. This is useful for weapons that should not be able to fire straight down (e.g., certain artillery) or for balancing purposes.
- **Default**: `-PI` (can aim straight down)
- **Example**: `MinTargetPitch = -30.0`

#### `MaxTargetPitch` *(v1.04)*
- **Type**: `Real` (angle in radians)
- **Description**: Highest vertical aim angle (pitch) allowed when acquiring and firing at targets. This restricts the weapon's ability to aim upward. Values are in radians, where positive values allow aiming above horizontal (upward) and negative values restrict upward aiming. The default of PI allows aiming in any upward direction. This is useful for ground-based weapons that should not be able to fire straight up (e.g., certain cannons) or for balancing purposes.
- **Default**: `PI` (can aim straight up)
- **Example**: `MaxTargetPitch = 30.0`

#### `RadiusDamageAngle` *(v1.04, Generals Zero Hour only)*
- **Type**: `Real` (angle in radians)
- **Description**: Limits radius damage to a directional cone centered on the firing unit's forward orientation. When this value is less than PI (180 degrees), radius damage only affects objects within a cone extending from the firing unit in its facing direction. The cone angle is measured from the center line (half-angle). Objects outside this cone are not affected by radius damage even if they are within the damage radius. When this is PI or greater, radius damage applies in all directions (full 360 degrees). This allows creating directional blasts, flamethrowers, or forward-facing area weapons. The angle between the firing unit's forward direction and the direction to each potential victim is checked; if the victim is outside the allowed cone angle, the victim is skipped.
- **Default**: `PI` (full 360° coverage)
- **Example**: `RadiusDamageAngle = 45.0`

### Continuous Fire Properties

#### `ContinuousFireOne` *(v1.04)*
- **Type**: `Int`
- **Description**: Number of consecutive shots that must be fired before the weapon enters the first continuous fire mode. Once this many shots have been fired in succession, the weapon gains enhanced firing properties (typically faster firing rate or reduced [`DelayBetweenShots`](#delaybetweenshots)). The continuous fire mode persists as long as the weapon continues firing, and resets when firing stops for longer than [`ContinuousFireCoast`](#continuousfirecoast). This allows weapons to "warm up" and fire faster after sustained use. If set to 0, the weapon never enters continuous fire mode. This property works in combination with [`ContinuousFireTwo`](#continuousfiretwo) to create multiple stages of continuous fire.
- **Example**: `ContinuousFireOne = 5`

#### `ContinuousFireTwo` *(v1.04)*
- **Type**: `Int`
- **Description**: Number of consecutive shots that must be fired before the weapon enters the second continuous fire mode (enhanced continuous fire). This is a higher tier of continuous fire that activates after [`ContinuousFireOne`](#continuousfireone) has been reached. Once this many shots have been fired in succession, the weapon gains even more enhanced firing properties. The continuous fire mode persists as long as the weapon continues firing, and resets when firing stops for longer than [`ContinuousFireCoast`](#continuousfirecoast). This allows weapons to have multiple "warm up" stages with progressively better performance. If set to 0, the weapon never enters the second continuous fire mode. This value must be greater than [`ContinuousFireOne`](#continuousfireone) to have any effect.
- **Example**: `ContinuousFireTwo = 10`

#### `ContinuousFireCoast` *(v1.04)*
- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Time (in milliseconds) after the weapon stops firing before the continuous fire mode (activated by [`ContinuousFireOne`](#continuousfireone) and [`ContinuousFireTwo`](#continuousfiretwo)) ends and resets. If the weapon stops firing for longer than this duration, the continuous fire counter resets to 0 and the weapon must fire the required number of shots again to re-enter continuous fire mode. This creates a "cooldown" period where the weapon loses its enhanced firing properties if not used continuously. Lower values make the continuous fire mode reset quickly, while higher values allow brief pauses in firing without losing the continuous fire benefits. If set to 0, continuous fire mode resets immediately when firing stops.
- **Example**: `ContinuousFireCoast = 1000`

### Laser and Stream Properties

#### `LaserName` *(v1.04)*
- **Type**: `AsciiString`
- **Description**: Name of the laser object template that is created and persists while the weapon is firing. The laser object is a visual effect that appears as a continuous beam or line from the weapon's firing point to the target, providing visual feedback for laser weapons or beam weapons. The laser object is created when firing begins and is destroyed when firing stops. This creates a persistent visual connection between the weapon and its target during the firing duration. The laser object must be defined in the game's object definitions. If [`LaserBoneName`](#laserbonename) is specified, the laser is attached to that bone on the firing unit's model; otherwise, it attaches to the default firing point.
- **Example**: `LaserName = LaserBeam`

#### `LaserBoneName` *(v1.04, Generals Zero Hour only)*
- **Type**: `AsciiString`
- **Description**: Name of the bone on the firing unit's 3D model where the [`LaserName`](#lasername) object is attached. This allows precise control over where the laser beam originates on the unit's model. The bone name must match a bone defined in the unit's W3D model file. When specified, the laser object attaches to this bone instead of the default firing point, allowing the laser to originate from specific locations like a weapon barrel, turret muzzle, or other model attachment points. This is useful for units with multiple weapon mounts or when the laser should originate from a specific visual location on the model. If not specified, the laser attaches to the default weapon firing point.
- **Example**: `LaserBoneName = Muzzle`

#### `ProjectileStreamName` *(v1.04)*
- **Type**: `AsciiString`
- **Description**: Name of the object template that is created to visually track and follow the [`ProjectileObject`](#projectileobject) as it travels from the firing point to the target. This object creates a visual stream or trail effect that follows the projectile's path, providing visual feedback for projectile movement. The stream object is created when the projectile is fired and is automatically positioned to track the projectile's current location. This is useful for creating missile trails, smoke trails, or other visual effects that follow projectiles. The stream object must be defined in the game's object definitions and will be automatically destroyed when the projectile impacts or is destroyed.
- **Example**: `ProjectileStreamName = MissileTrail`

### Weapon Bonus Properties

#### `WeaponBonus` *(v1.04)*
- **Type**: `WeaponBonusSet`
- **Description**: Additional bonuses that modify this weapon's properties when specific conditions are met. Weapon bonuses can increase damage, range, rate of fire, or other weapon attributes based on player upgrades, veterancy levels, or other game conditions. Multiple bonus entries can be specified, and they stack multiplicatively or additively depending on the bonus type. Bonuses are typically tied to player upgrades or research, allowing weapons to become more powerful as the game progresses. The bonus format specifies the condition (e.g., `PLAYER_UPGRADE`), the property to modify (e.g., `DAMAGE`, `RANGE`), and the modifier value (e.g., `125%` for 25% increase, or `+50` for absolute increase). This allows creating upgradeable weapons that improve with technology or experience.
- **Example**: `WeaponBonus = PLAYER_UPGRADE DAMAGE 125%`

### Historic Bonus Properties

#### `HistoricBonusTime` *(v1.04)*
- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Time window (in milliseconds) within which multiple weapon instances must fire to trigger the historic bonus. When multiple units with this weapon fire within this time window and within the [`HistoricBonusRadius`](#historicbonusradius), and the total number of firing instances reaches [`HistoricBonusCount`](#historicbonuscount), the historic bonus is triggered and the [`HistoricBonusWeapon`](#historicbonusweapon) is fired. This creates a coordinated attack bonus where multiple units firing together can trigger a special weapon. The time window starts when the first weapon fires, and all qualifying weapon fires must occur within this duration. If the time window expires before the required count is reached, the counter resets. This property works in combination with [`HistoricBonusRadius`](#historicbonusradius), [`HistoricBonusCount`](#historicbonuscount), and [`HistoricBonusWeapon`](#historicbonusweapon) to create coordinated attack bonuses.
- **Example**: `HistoricBonusTime = 5000`

#### `HistoricBonusRadius` *(v1.04)*
- **Type**: `Real`
- **Description**: Radius (in world units) within which weapon instances must be located to count toward the historic bonus trigger. When multiple units with this weapon fire, only those within this radius of each other are considered for the historic bonus count. The radius is measured from the firing units' positions, and all qualifying units must be within this distance of each other. This ensures that only coordinated attacks from nearby units can trigger the historic bonus, creating a tactical requirement for units to be positioned together. This property works in combination with [`HistoricBonusTime`](#historicbonustime), [`HistoricBonusCount`](#historicbonuscount), and [`HistoricBonusWeapon`](#historicbonusweapon) to create coordinated attack bonuses.
- **Example**: `HistoricBonusRadius = 100.0`

#### `HistoricBonusCount` *(v1.04)*
- **Type**: `Int`
- **Description**: Minimum number of weapon instances that must fire within the [`HistoricBonusTime`](#historicbonustime) window and within the [`HistoricBonusRadius`](#historicbonusradius) to trigger the historic bonus. When this many units with this weapon fire within the time window and radius, the historic bonus is activated and the [`HistoricBonusWeapon`](#historicbonusweapon) is fired. This creates a requirement for coordinated attacks, where multiple units must fire together to unlock the special bonus weapon. Higher values require more coordination and units, making the bonus more difficult to trigger but potentially more powerful. This property works in combination with [`HistoricBonusTime`](#historicbonustime), [`HistoricBonusRadius`](#historicbonusradius), and [`HistoricBonusWeapon`](#historicbonusweapon) to create coordinated attack bonuses.
- **Example**: `HistoricBonusCount = 3`

#### `HistoricBonusWeapon` *(v1.04)*
- **Type**: `weapon` (see [Weapon documentation](Weapon.md))
- **Description**: The weapon template that is fired when the historic bonus conditions are met. When [`HistoricBonusCount`](#historicbonuscount) weapon instances fire within the [`HistoricBonusTime`](#historicbonustime) window and within the [`HistoricBonusRadius`](#historicbonusradius), this weapon is automatically fired as a bonus effect. This allows creating special coordinated attack bonuses, such as a nuclear strike when multiple units fire together, or other powerful effects that reward tactical coordination. The weapon is fired at a location determined by the game logic (typically at or near the target of the coordinated attack). This property works in combination with [`HistoricBonusTime`](#historicbonustime), [`HistoricBonusRadius`](#historicbonusradius), and [`HistoricBonusCount`](#historicbonuscount) to create coordinated attack bonuses. The weapon must be defined in the game's weapon definitions.
- **Example**: `HistoricBonusWeapon = NuclearStrike`

### Scatter and Accuracy Properties

#### `ScatterTarget` *(v1.04)*
- **Type**: `Coord2D` (multiple entries)
- **Description**: Specific preset coordinates (relative to the target position) that define where projectiles will impact instead of using random scatter. When this property is used, the weapon cycles through these preset coordinates for each shot, creating a predictable scatter pattern. The coordinates are specified as X, Y offsets from the target position in world units. Multiple entries can be specified, and the weapon will cycle through them sequentially. This allows creating specific impact patterns like a cross, circle, or grid formation. The coordinates can be scaled using [`ScatterTargetScalar`](#scattertargetscalar) to adjust the spread without redefining all coordinates. If this property is not used, the weapon uses random scatter based on [`ScatterRadius`](#scatterradius) and [`ScatterRadiusVsInfantry`](#scatterradiusvsinfantry) instead.
- **Example**: 
```
ScatterTarget = 10.0, 5.0
ScatterTarget = -5.0, 15.0
```

#### `ShotsPerBarrel` *(v1.04)*
- **Type**: `Int`
- **Description**: Number of shots that must be fired from the current barrel or launch point before the weapon cycles to the next available barrel or launch point. This property is used for multi-barrel weapons (like rocket launchers with multiple tubes or multi-cannon weapons) to control the firing sequence. When a weapon has multiple barrels or launch points defined on the unit's model, this property determines how many shots are fired from each barrel before moving to the next one. This creates a realistic firing pattern where barrels fire in sequence rather than all at once. For example, if set to 3, the weapon will fire 3 shots from the first barrel, then 3 shots from the second barrel, and so on, cycling through all available barrels. If set to 0 or 1, the weapon cycles to the next barrel after each shot. This property only has an effect if the unit has multiple barrels or launch points defined in its model.
- **Default**: `0`
- **Example**: `ShotsPerBarrel = 3`

### Additional Advanced Properties

#### `LeechRangeWeapon` *(v1.04)*
- **Type**: `Bool`
- **Description**: When enabled, once the weapon fires at a target within its normal [`AttackRange`](#attackrange), it gains unlimited range for the remainder of that attack cycle. This allows the weapon to continue tracking and hitting the target even if it moves beyond the normal attack range. The "leech range" is activated when the weapon fires and remains active until the attack cycle completes (e.g., projectile hits or misses, or the attack is cancelled). This is useful for weapons like homing missiles that should maintain lock-on regardless of range after launch. If disabled, the weapon respects normal range limits throughout the entire attack.
- **Default**: `No`
- **Example**: `LeechRangeWeapon = Yes`

#### `CapableOfFollowingWaypoints` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the [`ProjectileObject`](#projectileobject) from this weapon can follow waypoint paths defined in the projectile's object definition. When enabled, projectiles can navigate through a series of waypoints instead of traveling directly to the target. This allows creating complex flight paths, curved trajectories, or projectiles that navigate around obstacles. The waypoints must be defined in the projectile object's definition. When disabled, projectiles travel directly toward their target in a straight line (or following their normal trajectory). This property only applies to weapons with a [`ProjectileObject`](#projectileobject) specified. Useful for cruise missiles, guided projectiles, or weapons that need to follow specific flight paths.
- **Default**: `No`
- **Example**: `CapableOfFollowingWaypoints = Yes`

#### `ShowsAmmoPips` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon displays ammunition pips (visual indicators) in the user interface to show the current ammunition status. When enabled, the UI displays small pips or indicators that represent the current ammunition count, typically shown as filled or empty pips corresponding to the number of shots remaining in the clip. This provides visual feedback to the player about the weapon's ammunition status. The pips are typically displayed near the unit's health bar or in the unit's status display. When disabled, no ammunition indicators are shown in the UI. This property is useful for weapons with limited ammunition (when [`ClipSize`](#clipsize) > 0) to provide visual feedback about remaining shots.
- **Default**: `No`
- **Example**: `ShowsAmmoPips = Yes`

#### `AllowAttackGarrisonedBldgs` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether the weapon can attack garrisoned buildings even when the estimated damage would be zero (typically due to the building's armor or the weapon's damage type being ineffective). When enabled, the weapon can still acquire and fire at garrisoned buildings even if the damage calculation indicates no damage would be dealt. This allows weapons to attack buildings for tactical purposes (like suppressing fire, visual effects, or triggering building behaviors) even when they cannot damage them. When disabled, the weapon will not attack garrisoned buildings if the estimated damage is zero, preventing wasted attacks on invulnerable targets. This property is useful for weapons that should be able to target buildings for non-damage purposes, or weapons that should skip ineffective targets.
- **Default**: `No`
- **Example**: `AllowAttackGarrisonedBldgs = No`

#### `PlayFXWhenStealthed` *(v1.04)*
- **Type**: `Bool`
- **Description**: Whether visual effects (specified by [`FireFX`](#firefx), [`ProjectileDetonationFX`](#projectiledetonationfx), and related properties) are played even when the firing unit is in a stealthed state. When enabled, the weapon's visual effects are displayed normally even if the firing unit is stealthed, potentially revealing the unit's position. When disabled, visual effects are suppressed when the firing unit is stealthed, helping maintain stealth. This property allows balancing stealth mechanics - weapons that should reveal the unit when firing (like loud explosions) can have this enabled, while weapons that should maintain stealth (like silenced weapons) can have this disabled. The audio effects are typically not affected by this property and may still play based on other game logic.
- **Default**: `No`
- **Example**: `PlayFXWhenStealthed = No`

#### `SuspendFXDelay` *(v1.04)*
- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Time delay (in milliseconds) before visual effects are suspended or stopped. This property controls how long visual effects (specified by [`FireFX`](#firefx), [`ProjectileDetonationFX`](#projectiledetonationfx), and related properties) continue to play before being automatically suspended. After this duration, the effects are stopped or suspended, even if they would normally continue playing. This is useful for managing effect duration, preventing effects from playing indefinitely, or creating timed visual effects. If set to 0, effects are not automatically suspended and play for their natural duration. This property works in combination with the effect system to control effect lifetime.
- **Default**: `0`
- **Example**: `SuspendFXDelay = 2000`

#### `MissileCallsOnDie` *(v1.04, Generals Zero Hour only)*
- **Type**: `Bool`
- **Description**: Whether the [`ProjectileObject`](#projectileobject) (when it is a missile) calls its `OnDie` script or behavior when it detonates normally. When enabled, the missile's `OnDie` script or death behavior is triggered when the missile detonates (hits its target, reaches its destination, or detonates normally). This allows missiles to execute custom scripts, spawn objects, trigger effects, or perform other actions when they detonate. When disabled, the missile's `OnDie` is not called during normal detonation, only when the missile is destroyed by other means (like being shot down). This property only applies to weapons with a [`ProjectileObject`](#projectileobject) that is a missile-type object. Useful for missiles that need to trigger special behaviors or scripts upon detonation.
- **Default**: `No`
- **Example**: `MissileCallsOnDie = Yes`

## Enum Value Lists

**Note**: Some enums used by Weapon have their own dedicated documentation files and their details are not listed here:
- **`DamageType`** - See [DamageType documentation](Enum/DamageType.md)
- **`ObjectStatusTypes`** - See [ObjectStatusTypes documentation](../Object/Enums/ObjectStatusTypes.md)
- **`KindOfMaskType`** - See [KindOf documentation](../Object/Enums/KindOf.md)

<a id="deathtype-values"></a>
#### `DeathType` Values *(v1.04)*
**Source:** [Damage.h](../../Code/GameEngine/Include/GameLogic/Damage.h) - `DeathType` enum definition

- **`NORMAL`** *(v1.04)* - Standard death animation. The default death type used for most weapons when no specific death type is required. Units killed with this death type use their standard death animation and visual effects as defined in the unit's object definition. This is the most common death type and provides a generic death animation that works for most situations. The death animation typically shows the unit collapsing or falling over in a standard manner. Used by default for most weapons (especially bullet-based weapons), scripted damage, and when no special death effect is needed. This death type does not trigger any special visual or audio effects beyond the unit's standard death animation.
- **`EXPLODED`** *(v1.04)* - Explosion death. Used when units are killed by explosive weapons such as grenades, rockets, artillery shells, or other explosive ordnance. This death type triggers explosion visual effects (typically fire and smoke), explosion animations, and explosion audio effects. The death animation shows the unit being destroyed in an explosion, with debris and fire effects. Commonly used with explosive damage types like [`DAMAGE_EXPLOSION`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) to create appropriate death visuals that match the weapon type. This death type is ideal for weapons that cause explosive damage, as it provides visual and audio feedback that matches the destructive nature of the weapon. The explosion effects are typically more dramatic than standard death animations, providing clear visual feedback that the unit was destroyed by an explosion.
- **`BURNED`** *(v1.04)* - Fire death. Used when units are killed by fire-based weapons. This death type triggers fire-related death animations and visual effects. Special handling: when a weapon with [`DEATH_BURNED`](#deathtype-values) targets shrubbery ([`SHRUBBERY`](../Object/Enums/KindOf.md) (see [KindOf documentation](../Object/Enums/KindOf.md))), it always returns a valid damage estimate (1.0) regardless of other factors, allowing flame weapons to target and destroy vegetation. Commonly used with [`DAMAGE_FLAME`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) damage type.
- **`LASERED`** *(v1.04)* - Laser death. Used when units are killed by laser weapons. This death type triggers laser-specific death animations and visual effects. Used with precision energy weapons to create appropriate death visuals. Commonly paired with [`DAMAGE_LASER`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) damage type.
- **`SUICIDED`** *(v1.04)* - Suicide death. Used when units kill themselves, typically through self-destruct mechanisms, suicide attacks, or scripted suicide behaviors. This death type is used for scripted suicides and self-destruct behaviors where the unit intentionally destroys itself. The death animation and effects reflect a self-inflicted death rather than being killed by an external source, which may affect how the death is displayed or counted in game statistics. This death type is commonly used for terrorist units, suicide bombers, self-destructing vehicles, or units that explode when destroyed. The visual effects may differ from standard death animations to indicate that the unit destroyed itself intentionally, and the death may not count as a kill for the attacking player in some game modes.
- **`CRUSHED`** *(v1.04)* - Crushed death. Used when units are crushed by heavier units or objects. This death type is automatically applied when units are run over by vehicles or crushed by falling objects. Applied during collision detection when a unit is crushed. Death animations show the unit being flattened or crushed. Used with [`DAMAGE_CRUSH`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) damage type.
- **`DETONATED`** *(v1.04)* - Detonation death. Used when projectiles, missiles, or explosive objects detonate normally (as opposed to being shot down, destroyed, or intercepted). This death type represents the normal, intended detonation of an explosive object when it reaches its target or destination, rather than its destruction by other means. The death animation and effects show the explosive object detonating as intended, with appropriate explosion visual and audio effects. Used to distinguish between normal detonations (where the explosive object successfully detonates) and other forms of destruction (where the object is destroyed before it can detonate). This death type is commonly used for missiles, bombs, or other explosive projectiles that are designed to detonate on impact or at a specific location. The visual effects typically show a controlled explosion appropriate for the explosive object's type and size.
- **`TOPPLED`** *(v1.04)* - Toppled death. Used when structures fall over or are toppled. This death type is used for structures that collapse rather than explode. The death animation shows the structure falling over. Used with [`DAMAGE_TOPPLING`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) damage type.
- **`FLOODED`** *(v1.04)* - Flooded death. Used when units are killed by water or flooding. This death type is used by wave guide systems and water-based damage. Units killed by flooding use this death type to show appropriate death visuals. Used with [`DAMAGE_WATER`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) damage type.
- **`SPLATTED`** *(v1.04)* - Splatted death. Used when units fall from heights and are killed by fall damage. This death type is automatically applied by the physics system when units take fatal fall damage. The death animation shows the unit being flattened from impact. Used with [`DAMAGE_FALLING`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) damage type.
- **`POISONED`** *(v1.04)* - Poisoned death. Used when units are killed by poison. This death type is used by poison-based weapons and behaviors. Units killed by poison use this death type to show appropriate death visuals. Used with [`DAMAGE_POISON`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) damage type and `PoisonedBehavior` module.
- **`POISONED_BETA`** *(v1.04)* - Alternative poisoned death. A variant of poisoned death used for different poison effects or visual styles. Used as an alternative to `POISONED` when a different poison death animation is desired.
- **`EXTRA_1`** through **`EXTRA_10`** *(v1.04)* - Custom death types. Reserved death types for modders to define custom death animations and behaviors. These death types can be used to create unique death effects that don't fit into the standard categories. Each extra death type can be assigned custom visual effects and animations through the object's death modules.

<a id="weaponprefiretype-values"></a>
#### `WeaponPrefireType` Values *(v1.04)*
**Source:** [Weapon.h](../../Code/GameEngine/Include/GameLogic/Weapon.h#77) - `WeaponPrefireType` enum definition

- **`PER_SHOT`** *(v1.04)* - Prefire delay for every shot. The `PreAttackDelay` is applied before every single shot fired by this weapon, regardless of target or clip status. This creates a consistent delay between each shot, useful for weapons that need time to aim, charge, or prepare before each individual shot. This is the default behavior and provides the most predictable firing pattern. Useful for weapons that require precise aiming before each shot, such as sniper rifles or charged weapons.
- **`PER_ATTACK`** *(v1.04)* - Prefire delay for each new target. The `PreAttackDelay` is only applied when starting to attack a new target. Once the weapon has fired at least one shot at a target, subsequent shots at the same target do not apply the prefire delay. The game tracks how many shots have been fired at the current target, and if any shots have already been fired, the prefire delay is skipped. This allows weapons to have an initial aiming/charging delay when acquiring a new target, but then fire rapidly at the same target. Useful for weapons that need time to lock onto a new target but can then fire quickly at that target, such as missile launchers or weapons with target acquisition systems.
- **`PER_CLIP`** *(v1.04)* - Prefire delay for each new clip. The `PreAttackDelay` is only applied when firing the first shot from a new clip. If the weapon has a `ClipSize` greater than 0 and the current ammo in clip is less than the clip size (meaning it's not the first shot in the clip), the prefire delay is skipped. This allows weapons to have a delay when starting a new clip but then fire rapidly for the rest of the clip. Useful for weapons that need time to prepare or charge when loading a new clip, such as weapons with clip-based charging mechanisms or weapons that need to stabilize after reloading.

<a id="weaponaffectsmasktype-values"></a>
#### `WeaponAffectsMaskType` Values *(v1.04)*
**Source:** [Weapon.h](../../Code/GameEngine/Include/GameLogic/Weapon.h#110) - `WeaponAffectsMaskType` enum definition

- **`SELF`** *(v1.04)* - Affects the firing unit. When set, the weapon's radius damage can affect the unit that fired it. By default, the firing unit is excluded from radius damage to prevent self-damage. When this flag is set, the firing unit can be damaged by its own weapon's area effect. If the potential victim is the same unit that fired the weapon (or the unit that created the projectile), the victim is included only if this flag is set. Useful for suicide weapons or weapons where self-damage is intentional, such as car bombs or area-effect weapons that should damage the user.
- **`ALLIES`** *(v1.04)* - Affects allied units. When set, the weapon's radius damage affects units that are allied to the firing unit. Allied units are those with an `ALLIES` relationship to the source. This flag controls whether friendly fire is enabled for area damage. If the potential victim is allied to the firing unit, the victim is included only if this flag is set. By default, most weapons include this flag to allow area damage to affect allies. Useful for weapons where friendly fire is acceptable or desired, such as artillery or area-effect weapons.
- **`ENEMIES`** *(v1.04)* - Affects enemy units. When set, the weapon's radius damage affects units that are enemies of the firing unit. Enemy units are those with an `ENEMIES` relationship to the source. This flag controls whether the weapon can damage enemy units within the radius (excluding the primary target, which is always affected). If the potential victim is an enemy of the firing unit, the victim is included only if this flag is set. This is the most common flag and is typically included for offensive weapons. Note: The primary target is always affected regardless of this flag - this flag only affects secondary targets within the radius.
- **`NEUTRALS`** *(v1.04)* - Affects neutral units. When set, the weapon's radius damage affects units that are neutral to the firing unit. Neutral units are those with a `NEUTRAL` relationship to the source. This flag controls whether the weapon can damage neutral units (such as civilians or neutral structures) within the radius. If the potential victim is neutral to the firing unit, the victim is included only if this flag is set. Useful for weapons that should affect neutral units, such as area-effect weapons that don't distinguish between friend and foe, or weapons designed to clear neutral obstacles.
- **`SUICIDE`** *(v1.04)* - Ensures firing unit cannot survive self-damage. When set, this flag guarantees that the firing unit will be killed by its own weapon, even if the calculated damage would not normally be fatal. If this flag is set and the potential victim is the source unit itself, the unit is forced to die. This ensures that suicide weapons always kill the user, regardless of health, armor, or damage calculations. Enough damage is dealt to guarantee death when this flag is active. Used for suicide weapons like car bombs, terrorist attacks, or self-destruct mechanisms where the user must die when the weapon is used.
- **`NOT_SIMILAR`** *(v1.04)* - Doesn't affect similar units (prevents chain reactions). When set, the weapon's radius damage will not affect units that are the same type as the firing unit and are allied to it. If the potential victim is the same unit type as the firing unit and is allied to it, the victim is excluded from damage. This prevents chain reactions where one unit's explosion kills nearby units of the same type, which then explode and kill more, creating a domino effect. Useful for weapons used by groups of similar units (like terrorist bombs) where you want to prevent all units in a group from exploding simultaneously. The primary target is still affected - this only applies to secondary targets within the radius.
- **`NOT_AIRBORNE`** *(v1.04)* - Doesn't affect airborne units (unless primary target). When set, the weapon's radius damage will not affect units that are significantly above the terrain (airborne units), unless the airborne unit is the primary target. If a potential victim is airborne and is not the primary target, the victim is excluded from damage. This allows weapons to affect ground units while ignoring aircraft or paratroopers in the area. Useful for ground-based area weapons like poison fields, radiation zones, or ground explosions that should not affect aircraft. The primary target is always affected regardless of this flag - this only applies to secondary targets within the radius.

<a id="weaponcollidemasktype-values"></a>
#### `WeaponCollideMaskType` Values *(v1.04)*
**Source:** [Weapon.h](../../Code/GameEngine/Include/GameLogic/Weapon.h#136) - `WeaponCollideMaskType` enum definition

- **`ALLIES`** *(v1.04)* - Projectiles collide with allied units. When set, projectiles from this weapon will collide with and detonate when hitting units that are allied to the firing unit. If the potential collision target is allied to the firing unit, a collision occurs only if this flag is set. This allows projectiles to pass through friendly units (when not set) or collide with them (when set). Useful for weapons that should not be blocked by friendly units, or weapons where friendly collisions are desired for gameplay reasons. Note: The primary target is always collided with regardless of this flag - this only applies to non-targeted units that get in the way.
- **`ENEMIES`** *(v1.04)* - Projectiles collide with enemy units. When set, projectiles from this weapon will collide with and detonate when hitting enemy units that are not the primary target. If the potential collision target is an enemy of the firing unit, a collision occurs only if this flag is set. This allows projectiles to pass through enemy units (when not set) or collide with them (when set). Useful for weapons that should only hit their intended target, or weapons where hitting any enemy is desired. Note: The primary target is always collided with regardless of this flag - this only applies to non-targeted units that get in the way.
- **`STRUCTURES`** *(v1.04)* - Projectiles collide with structures. When set, projectiles from this weapon will collide with and detonate when hitting structures that are not controlled by the projectile's owner. If the potential collision target is a structure (has the [`STRUCTURE`](../Object/Enums/KindOf.md) (see [KindOf documentation](../Object/Enums/KindOf.md)) flag) and is not controlled by the same player as the projectile, a collision occurs only if this flag is set. This flag specifically excludes structures controlled by the projectile's owner - use `CONTROLLED_STRUCTURES` to collide with owner's structures. Useful for weapons that should be blocked by enemy or neutral structures, such as most projectile weapons. This is the default collision flag for most weapons.
- **`SHRUBBERY`** *(v1.04)* - Projectiles collide with shrubbery. When set, projectiles from this weapon will collide with and detonate when hitting vegetation, trees, or other shrubbery objects. If the potential collision target is shrubbery (has the [`SHRUBBERY`](../Object/Enums/KindOf.md) (see [KindOf documentation](../Object/Enums/KindOf.md)) flag), a collision occurs only if this flag is set. This allows projectiles to pass through vegetation (when not set) or be blocked by it (when set). Useful for weapons that should interact with terrain features, or weapons that should ignore vegetation for gameplay reasons.
- **`PROJECTILES`** *(v1.04)* - Projectiles collide with other projectiles. When set, projectiles from this weapon will collide with and detonate when hitting other projectiles in flight. If the potential collision target is a projectile (has the [`PROJECTILE`](../Object/Enums/KindOf.md) (see [KindOf documentation](../Object/Enums/KindOf.md)) flag), a collision occurs only if this flag is set. This enables projectile interception mechanics, allowing weapons to shoot down incoming projectiles. Useful for point defense weapons, anti-missile systems, or weapons designed to intercept other projectiles. Note: All missiles are also projectiles, so this flag affects missiles as well.
- **`WALLS`** *(v1.04)* - Projectiles collide with walls. When set, projectiles from this weapon will collide with and detonate when hitting walls or fences. If the potential collision target is a wall or fence, a collision occurs only if this flag is set. This allows projectiles to pass through walls (when not set) or be blocked by them (when set). Useful for weapons that should interact with map geometry, or weapons that should ignore walls for gameplay reasons.
- **`SMALL_MISSILES`** *(v1.04)* - Projectiles collide with small missiles. When set, projectiles from this weapon will collide with and detonate when hitting small missiles. If the potential collision target is a small missile (has the [`SMALL_MISSILE`](../Object/Enums/KindOf.md) (see [KindOf documentation](../Object/Enums/KindOf.md)) flag), a collision occurs only if this flag is set. Note that all missiles are also projectiles, so the `PROJECTILES` flag also affects missiles. This flag provides specific targeting for small missiles, allowing weapons to specifically target this missile type. Useful for point defense weapons that need to distinguish between different missile types, or weapons designed specifically to intercept small missiles.
- **`BALLISTIC_MISSILES`** *(v1.04)* - Projectiles collide with ballistic missiles. When set, projectiles from this weapon will collide with and detonate when hitting ballistic missiles (large missiles typically used by superweapons). If the potential collision target is a ballistic missile (has the [`BALLISTIC_MISSILE`](../Object/Enums/KindOf.md) (see [KindOf documentation](../Object/Enums/KindOf.md)) flag), a collision occurs only if this flag is set. Note that all missiles are also projectiles, so the `PROJECTILES` flag also affects missiles. This flag provides specific targeting for ballistic missiles, allowing weapons to specifically target this missile type. Useful for base defense weapons, anti-missile systems, or weapons designed specifically to intercept large ballistic missiles from superweapons.
- **`CONTROLLED_STRUCTURES`** *(v1.04)* - Projectiles collide with structures controlled by the same player. When set, projectiles from this weapon will collide with and detonate when hitting structures that are controlled by the same player as the projectile's owner. If the potential collision target is a structure and is controlled by the same player, a collision occurs only if this flag is set. This is separate from the `STRUCTURES` flag, which only affects structures not controlled by the owner. This allows fine-grained control over whether projectiles can pass through the owner's own structures. Useful for weapons that should be blocked by friendly structures, or weapons that should ignore friendly structures to prevent accidental friendly fire on your own buildings.


## Examples

### Basic Infantry Weapon
```
Weapon USAInfantryRifle
  PrimaryDamage               = 25.0
  PrimaryDamageRadius         = 0.0
  AttackRange                 = 200.0
  MinimumAttackRange          = 0.0
  AcceptableAimDelta          = 5.0
  DamageType                  = BULLET
  DeathType                   = NORMAL
  WeaponSpeed                 = 1000.0
  DelayBetweenShots           = 1000
  ClipSize                    = 30
  ClipReloadTime              = 3000
  AutoReloadsClip             = Yes
  RadiusDamageAffects         = ENEMIES
  AntiGround                  = Yes
  AntiAirborneInfantry        = Yes
  FireSound                   = RifleFire
  FireFX                      = FX_RifleMuzzleFlash
End
```

### Tank Cannon Weapon
```
Weapon USATankCannon
  PrimaryDamage               = 150.0
  PrimaryDamageRadius         = 5.0
  SecondaryDamage             = 75.0
  SecondaryDamageRadius       = 15.0
  AttackRange                 = 300.0
  MinimumAttackRange          = 10.0
  AcceptableAimDelta          = 2.0
  DamageType                  = EXPLOSION
  DeathType                   = EXPLODED
  WeaponSpeed                 = 800.0
  ProjectileObject            = TankShell
  DelayBetweenShots           = 3000
  ClipSize                    = 1
  ClipReloadTime              = 3000
  AutoReloadsClip             = Yes
  RadiusDamageAffects         = ALLIES ENEMIES NEUTRALS
  AntiGround                  = Yes
  AntiAirborneVehicle         = Yes
  FireSound                   = TankCannonFire
  FireFX                      = FX_TankCannonMuzzleFlash
  ProjectileDetonationFX      = FX_ExplosionLarge
  ProjectileCollidesWith      = STRUCTURES WALLS
  ShockWaveAmount             = 10.0
  ShockWaveRadius             = 20.0
  ShockWaveTaperOff           = 0.5
End
```

### Missile Weapon
```
Weapon USAMissileLauncher
  PrimaryDamage               = 200.0
  PrimaryDamageRadius         = 10.0
  SecondaryDamage             = 100.0
  SecondaryDamageRadius       = 25.0
  AttackRange                 = 400.0
  MinimumAttackRange          = 50.0
  AcceptableAimDelta          = 1.0
  DamageType                  = EXPLOSION
  DeathType                   = EXPLODED
  WeaponSpeed                 = 600.0
  ProjectileObject            = AntiTankMissile
  DelayBetweenShots           = 5000
  ClipSize                    = 1
  ClipReloadTime              = 5000
  AutoReloadsClip             = Yes
  RadiusDamageAffects         = ALLIES ENEMIES NEUTRALS
  AntiGround                  = Yes
  AntiAirborneVehicle         = Yes
  FireSound                   = MissileLaunch
  FireFX                      = FX_MissileLaunch
  ProjectileDetonationFX      = FX_MissileExplosion
  ProjectileCollidesWith      = STRUCTURES WALLS PROJECTILES
  ShockWaveAmount             = 15.0
  ShockWaveRadius             = 30.0
  ShockWaveTaperOff           = 0.4
End
```

### Status Effect Weapon
```
Weapon StealthHawkTargetDesignator
  PrimaryDamage               = 10.0
  PrimaryDamageRadius         = 10.0
  AcceptableAimDelta          = 1.0
  AttackRange                 = 450.0
  MinimumAttackRange          = 5.0
  PreAttackDelay              = 5000
  PreAttackType               = PER_SHOT
  DamageType                  = STATUS
  DamageStatusType            = CAN_ATTACK
  ProjectileObject            = S300Fakemissile
  ClipSize                    = 1
  ClipReloadTime              = 60000
  AutoReloadsClip             = Yes
  AutoReloadWhenIdle          = 60000
  FireFX                      = FX_StealthHawkAttackAirstrike
End
```

### Area Effect Weapon
```
Weapon SCUDMissileDetonation
  PrimaryDamage               = 350.0
  PrimaryDamageRadius         = 40.0
  SecondaryDamage             = 200.0
  SecondaryDamageRadius       = 60.0
  AttackRange                 = 100.0
  DamageType                  = FLAME
  DeathType                   = EXPLODED
  RadiusDamageAffects         = ALLIES ENEMIES NEUTRALS NOT_SIMILAR
  FireSound                   = ScudLauncherWeapon
  DelayBetweenShots           = 30000
  ClipSize                    = 1
  ClipReloadTime              = 30000
  ProjectileCollidesWith      = STRUCTURES
  ShockWaveAmount             = 25.0
  ShockWaveRadius             = 70.0
  ShockWaveTaperOff           = 0.50
End
```

### Anti-Air Weapon
```
Weapon AntiAirMissile
  PrimaryDamage               = 100.0
  PrimaryDamageRadius         = 5.0
  AttackRange                 = 350.0
  MinimumAttackRange          = 20.0
  AcceptableAimDelta          = 3.0
  DamageType                  = EXPLOSION
  DeathType                   = EXPLODED
  WeaponSpeed                 = 1200.0
  ProjectileObject            = AntiAirMissile
  DelayBetweenShots           = 2000
  ClipSize                    = 4
  ClipReloadTime              = 8000
  AutoReloadsClip             = Yes
  RadiusDamageAffects         = ENEMIES
  AntiAirborneVehicle         = Yes
  AntiAirborneInfantry        = Yes
  AntiProjectile              = Yes
  AntiSmallMissile            = Yes
  FireSound                   = AntiAirMissileLaunch
  FireFX                      = FX_AntiAirLaunch
  ProjectileDetonationFX      = FX_AntiAirExplosion
End
```

### Point Defense Weapon
```
Weapon PointDefenseLaser
  PrimaryDamage               = 350.0
  PrimaryDamageRadius         = 22.5
  SecondaryDamage             = 85.0
  SecondaryDamageRadius       = 35.0
  AttackRange                 = 15.0
  DamageType                  = PENALTY
  DeathType                   = LASERED
  RadiusDamageAffects         = SELF ALLIES ENEMIES NEUTRALS
  WeaponSpeed                 = 999999.0
  DelayBetweenShots           = 1
  ClipSize                    = 0
  ClipReloadTime              = 0
  AcceptableAimDelta          = 999
  AntiSmallMissile            = No
  AntiProjectile              = No
  AntiAirborneVehicle         = No
  AntiAirborneInfantry        = No
  AntiBallisticMissile        = Yes
End
```

### Suicide Weapon
```
Weapon CarbombExplosion
  PrimaryDamage               = 500.0
  PrimaryDamageRadius         = 20.0
  AttackRange                 = 1.0
  DamageType                  = EXPLOSION
  DeathType                   = EXPLODED
  RadiusDamageAffects         = ALLIES ENEMIES NEUTRALS NOT_SIMILAR
  DamageDealtAtSelfPosition   = Yes
  WeaponSpeed                 = 999999.0
  ClipSize                    = 1
  ClipReloadTime              = 999999
  AutoReloadsClip             = No
  FireFX                      = FX_CarbombExplosion
  ShockWaveAmount             = 20.0
  ShockWaveRadius             = 25.0
  ShockWaveTaperOff           = 0.3
End
```

### Continuous Fire Weapon
```
Weapon MinigunWeapon
  PrimaryDamage               = 15.0
  PrimaryDamageRadius         = 0.0
  AttackRange                 = 250.0
  MinimumAttackRange          = 0.0
  AcceptableAimDelta          = 10.0
  DamageType                  = BULLET
  DeathType                   = NORMAL
  WeaponSpeed                 = 999999.0
  DelayBetweenShots           = 100
  ClipSize                    = 200
  ClipReloadTime              = 5000
  AutoReloadsClip             = Yes
  ContinuousFireOne           = 10
  ContinuousFireTwo           = 20
  ContinuousFireCoast         = 500
  FireSoundLoopTime           = 1000
  RadiusDamageAffects         = ENEMIES
  AntiGround                  = Yes
  AntiAirborneInfantry        = Yes
  FireSound                   = MinigunFire
  FireFX                      = FX_MinigunMuzzleFlash
  ShowsAmmoPips               = Yes
End
```

### Laser Weapon with Targeting Restrictions
```
Weapon LaserCannon
  PrimaryDamage               = 100.0
  PrimaryDamageRadius         = 2.0
  AttackRange                 = 400.0
  MinimumAttackRange          = 20.0
  AcceptableAimDelta          = 1.0
  DamageType                  = LASER
  DeathType                   = LASERED
  WeaponSpeed                 = 999999.0
  DelayBetweenShots           = 2000
  ClipSize                    = 10
  ClipReloadTime              = 8000
  AutoReloadsClip             = Yes
  LaserName                   = LaserBeam
  LaserBoneName               = Muzzle
  RadiusDamageAffects         = ENEMIES
  AntiGround                  = Yes
  AntiAirborneVehicle         = Yes
  FireSound                   = LaserCannonFire
  FireFX                      = FX_LaserCannonFire
  ProjectileDetonationFX      = FX_LaserImpact
End
```

### Scatter Weapon with Historic Bonus
```
Weapon ArtilleryCannon
  PrimaryDamage               = 200.0
  PrimaryDamageRadius         = 25.0
  SecondaryDamage             = 100.0
  SecondaryDamageRadius       = 40.0
  AttackRange                 = 500.0
  MinimumAttackRange          = 100.0
  AcceptableAimDelta          = 5.0
  DamageType                  = EXPLOSION
  DeathType                   = EXPLODED
  WeaponSpeed                 = 400.0
  ScaleWeaponSpeed            = Yes
  ProjectileObject            = ArtilleryShell
  DelayBetweenShots           = 8000
  ClipSize                    = 1
  ClipReloadTime              = 8000
  AutoReloadsClip             = Yes
  ScatterRadius               = 10.0
  ScatterTargetScalar         = 1.0
  ScatterTarget               = 5.0, 0.0
  ScatterTarget               = -5.0, 0.0
  ScatterTarget               = 0.0, 5.0
  ScatterTarget               = 0.0, -5.0
  HistoricBonusTime           = 10000
  HistoricBonusRadius         = 150.0
  HistoricBonusCount          = 3
  HistoricBonusWeapon         = NuclearStrike
  RadiusDamageAffects         = ALLIES ENEMIES NEUTRALS
  AntiGround                  = Yes
  FireSound                   = ArtilleryFire
  FireFX                      = FX_ArtilleryMuzzleFlash
  ProjectileDetonationFX      = FX_ArtilleryExplosion
  ProjectileCollidesWith      = STRUCTURES WALLS
  ShockWaveAmount             = 15.0
  ShockWaveRadius             = 35.0
  ShockWaveTaperOff           = 0.4
End
```

### Limited Area Effect Weapon
```
Weapon PrecisionStrike
  PrimaryDamage               = 500.0
  PrimaryDamageRadius         = 30.0
  AttackRange                 = 300.0
  DamageType                  = EXPLOSION
  DeathType                   = EXPLODED
  WeaponSpeed                 = 999999.0
  DelayBetweenShots           = 15000
  ClipSize                    = 1
  ClipReloadTime              = 15000
  AutoReloadsClip             = Yes
  RadiusDamageAffects         = ENEMIES
  AntiGround                  = Yes
  FireSound                   = PrecisionStrikeLaunch
  FireFX                      = FX_PrecisionStrikeLaunch
  ProjectileDetonationFX      = FX_PrecisionStrikeExplosion
  ShockWaveAmount             = 20.0
  ShockWaveRadius             = 25.0
  ShockWaveTaperOff           = 0.3
End
```

## Usage

Weapons are defined in `.ini` files within the game's data directories in a root folder called "Weapon". Multiple weapon entries can exist in the same file or across multiple files. Each weapon must have a unique name. See Template for correct syntax.

**Limitations**:
- Each weapon must have a unique name - no two weapons can share the same name even across multiple files.
- Weapons are templates that are referenced by objects, not directly instantiated.
- Weapon slot assignment is handled by [WeaponSet](../WeaponSet.md), not by the Weapon template itself.

**Conditions**:
- Weapons are referenced by units, buildings, and special powers through [WeaponSet](../WeaponSet.md) to define their combat capabilities.
- When a weapon is assigned to a weapon slot, it becomes available for that object to use in combat.
- The game automatically selects the appropriate weapon slot based on target type, range, and weapon preferences.

**Dependencies**:
- Weapons are assigned to objects through [WeaponSet](../WeaponSet.md) which manages multiple weapons on an object.
- Weapons that use projectiles require a valid [Object](../Object.md) template for the `ProjectileObject` property.
- Weapons that use visual effects require valid [FXList](../FXList.md) templates for `FireFX` and `ProjectileDetonationFX` properties.
- Weapons that use status effects require [`DamageType = STATUS`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) and a valid `DamageStatusType` value.

## Template

```ini
Weapon WeaponName
  PrimaryDamage = 0.0                    ; // Main damage amount *(v1.04)*
  PrimaryDamageRadius = 0.0              ; // Primary damage radius *(v1.04)*
  SecondaryDamage = 0.0                  ; // Secondary damage amount *(v1.04)*
  SecondaryDamageRadius = 0.0            ; // Secondary damage radius *(v1.04)*
  DamageType = EXPLOSION                 ; // Damage type (see DamageType documentation) *(v1.04)*
  DeathType = NORMAL                     ; // Death animation type *(v1.04)*
  AttackRange = 0.0                      ; // Maximum attack range *(v1.04)*
  MinimumAttackRange = 0.0               ; // Minimum attack range *(v1.04)*
  AcceptableAimDelta = 0.0               ; // Aim accuracy requirement (degrees) *(v1.04)*
  WeaponSpeed = 999999.0                 ; // Projectile speed (distance per second) *(v1.04)*
  DelayBetweenShots = 0                  ; // Time between shots (milliseconds) *(v1.04)*
  ClipSize = 0                           ; // Shots per clip (0 = infinite) *(v1.04)*
  ClipReloadTime = 0                     ; // Reload time (milliseconds) *(v1.04)*
  AutoReloadsClip = YES                  ; // Auto-reload when clip empty *(v1.04)*
  RadiusDamageAffects = ALLIES ENEMIES NEUTRALS ; // What is affected by radius damage *(v1.04)*
  AntiGround = YES                       ; // Can target ground units *(v1.04)*
End
```

## Notes

- Weapons are defined in `.ini` files within the game's data directories in a root folder called "Weapon".
- Each weapon must have a unique name - no two weapons can share the same name even across multiple files.
- Weapons can be referenced by units, buildings, and special powers.
- Veterancy levels affect visual effects and object creation lists.
- Projectile weapons create separate objects that handle their own movement and collision.
- Status effect weapons use [`DamageType = STATUS`](Enum/DamageType.md) (see [DamageType documentation](Enum/DamageType.md)) and specify the effect in [DamageStatusType](#damagestatustype).
- Area effect weapons use radius damage properties to affect multiple targets.
- Anti-air weapons should target airborne units and projectiles appropriately.
- Point defense weapons typically have very short ranges but high damage and fast firing rates.

## Modder Recommended Use Scenarios

(pending modder review)

## Source Files

**Header (Zero Hour):** [Weapon.h](../../Code/GameEngine/Include/GameLogic/Weapon.h)
**Source (Zero Hour):** [Weapon.cpp](../../Code/GameEngine/Source/GameLogic/Object/Weapon.cpp)
**Header (Generals):** [Weapon.h](../../../../Generals/Code/GameEngine/Include/GameLogic/Weapon.h) 
**Source (Generals):** [Weapon.cpp](../../../../Generals/Code/GameEngine/Source/GameLogic/Object/Weapon.cpp) 

## Changes History

### Retail Generals 1.04 → Retail Zero Hour 1.04

**7 new properties added** to enhance weapon functionality:

- **`DamageStatusType`** *(v1.04, Zero Hour only)*: Added support for status effect weapons. See [`DamageStatusType`](#damagestatustype) property documentation.

- **`ShockWaveAmount`**, **`ShockWaveRadius`**, **`ShockWaveTaperOff`** *(v1.04, Generals Zero Hour only)*: Added three properties for shockwave visual effects. See [`ShockWaveAmount`](#shockwaveamount), [`ShockWaveRadius`](#shockwaveradius), and [`ShockWaveTaperOff`](#shockwavetaperoff) property documentation.

- **`RadiusDamageAngle`** *(v1.04, Generals Zero Hour only)*: Added support for directional radius damage. See [`RadiusDamageAngle`](#radiusdamageangle) property documentation.

- **`LaserBoneName`** *(v1.04, Generals Zero Hour only)*: Added ability to specify laser attachment bone. See [`LaserBoneName`](#laserbonename) property documentation.

- **`MissileCallsOnDie`** *(v1.04, Generals Zero Hour only)*: Added control over missile OnDie script execution. See [`MissileCallsOnDie`](#missilecallsondie) property documentation.

## Document Log

- 16/12/2025 — AI — Initial document created by AI

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by @AI
- Certification: 0/2 reviews

## Reviewers

- [ ] Reviewer 1
- [ ] Reviewer 2
