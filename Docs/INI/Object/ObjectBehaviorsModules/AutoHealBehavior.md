# AutoHealBehavior

Status: AI-generated, 0/2 reviews

## Overview

The `AutoHealBehavior` module manages automatic healing systems that can heal the object itself or nearby friendly units within a specified radius. It supports upgrade-based activation, area healing, particle effects, and selective healing based on object types. The behavior is commonly used for medical buildings, repair facilities, and units with regenerative abilities. This is a module added inside `Object` entries.

Available in: *(v1.04)* (Generals, Zero Hour)

## Table of Contents

- [Overview](#overview)
- [Properties](#properties)
  - [Basic Healing Settings](#basic-healing-settings)
  - [Area Healing Settings](#area-healing-settings)
  - [Upgrade Integration](#upgrade-integration)
  - [Particle Effects](#particle-effects)
  - [Target Selection](#target-selection)
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

### Basic Healing Settings

Available in: *(v1.04)*

#### `HealingAmount`
Available in: *(v1.04)*

- **Type**: `Int`
- **Description**: Amount of health points healed per healing pulse. Higher values heal more health per tick, providing faster recovery. Lower values heal less per tick, requiring more time for full healing. At 0 (default), no healing occurs and the behavior is effectively disabled.
- **Default**: `0`
- **Example**: `HealingAmount = 3`

#### `HealingDelay`
Available in: *(v1.04)*

- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Time interval between healing pulses. Lower values provide faster healing rates, while higher values slow down the healing process. This controls how frequently healing occurs. The value is parsed as milliseconds and converted to game frames. If not specified (defaults to UINT_MAX), the behavior will not heal regularly and must be activated manually or through upgrades.
- **Default**: `4294967295` (UINT_MAX, effectively disabled - must be set for healing to occur)
- **Example**: `HealingDelay = 1000`

#### `StartsActive`
Available in: *(v1.04)*

- **Type**: `Bool`
- **Description**: Whether the healing behavior is active from the start. When `Yes`, healing begins immediately upon object creation. When `No` (default), healing must be activated by upgrades or other conditions.
- **Default**: `No`
- **Example**: `StartsActive = No`

#### `SingleBurst`
Available in: *(v1.04)*

- **Type**: `Bool`
- **Description**: Whether to perform only a single healing burst instead of continuous healing. When `Yes`, healing occurs once then stops. When `No` (default), healing continues at regular intervals until disabled.
- **Default**: `No`
- **Example**: `SingleBurst = Yes`

#### `StartHealingDelay`
Available in: *(v1.04)*

- **Type**: `UnsignedInt` (milliseconds)
- **Description**: Delay after taking damage before auto-healing starts. Higher values require longer waiting periods after damage before healing begins. At 0 (default), healing starts immediately when conditions are met. This only applies when healing the object itself (radius = 0).
- **Default**: `0`
- **Example**: `StartHealingDelay = 5000`

### Area Healing Settings

Available in: *(v1.04)*

#### `Radius`
Available in: *(v1.04)*

- **Type**: `Real` (distance)
- **Description**: Radius for area healing effect. Higher values extend the healing range to affect more nearby units. At 0.0 (default), only heals the object itself.
- **Default**: `0.0`
- **Example**: `Radius = 50.0`

#### `AffectsWholePlayer`
Available in: *(v1.04)*

- **Type**: `Bool`
- **Description**: Whether to affect all objects owned by the player instead of just those in radius. When `Yes`, heals all player units regardless of distance. When `No` (default), only affects units within the specified [Radius](#radius).
- **Default**: `No`
- **Example**: `AffectsWholePlayer = Yes`

#### `SkipSelfForHealing`
Available in: *(v1.04, Zero Hour only)*

- **Type**: `Bool`
- **Description**: Whether to skip healing the object itself when doing area healing. When `Yes`, the healing object does not heal itself, only nearby units. When `No` (default), the object heals both itself and nearby units.
- **Default**: `No`
- **Example**: `SkipSelfForHealing = Yes`

### Upgrade Integration

Available in: *(v1.04)*

These properties are inherited from `UpgradeMux` and allow AutoHealBehavior to be activated by upgrades.

#### `TriggeredBy`
Available in: *(v1.04)*

- **Type**: Upgrade name or list of upgrade names (see [Upgrade documentation](../Upgrade.md))
- **Description**: Required upgrade names to activate the healing behavior. When set, healing only occurs when at least one of the specified upgrades is active (unless [RequiresAllTriggers](#requiresalltriggers) is `Yes`). When empty (default), healing can occur without upgrade requirements. Multiple upgrade names can be specified as a space-separated list. This property accepts a list of upgrade names that are resolved to upgrade flags internally.
- **Default**: `""` (empty, no requirements)
- **Example**: `TriggeredBy = Upgrade_GLAJunkRepair`

#### `RequiredAnyUpgradeOf`
Available in: *(v1.04)*

- **Type**: Upgrade name or list of upgrade names (see [Upgrade documentation](../Upgrade.md))
- **Description**: Alternative upgrade names that can activate the healing behavior. When set, healing occurs when at least one of the specified upgrades is active (in addition to [TriggeredBy](#triggeredby) requirements). When empty (default), only [TriggeredBy](#triggeredby) upgrades are checked. Multiple upgrade names can be specified as a space-separated list. This property accepts a list of upgrade names that are resolved to upgrade flags internally.
- **Default**: `""` (empty)
- **Example**: `RequiredAnyUpgradeOf = Upgrade_MedicalUpgrade Upgrade_RepairUpgrade`

#### `RequiredAllUpgradesOf`
Available in: *(v1.04)*

- **Type**: Upgrade name or list of upgrade names (see [Upgrade documentation](../Upgrade.md))
- **Description**: Upgrade names that must all be active for the healing behavior to activate. When set, all specified upgrades must be active (in addition to [TriggeredBy](#triggeredby) requirements). When empty (default), only [TriggeredBy](#triggeredby) upgrades are checked. Multiple upgrade names can be specified as a space-separated list. This property accepts a list of upgrade names that are resolved to upgrade flags internally.
- **Default**: `""` (empty)
- **Example**: `RequiredAllUpgradesOf = Upgrade_MedicalUpgrade Upgrade_TechUpgrade`

#### `ConflictsWith`
Available in: *(v1.04)*

- **Type**: Upgrade name or list of upgrade names (see [Upgrade documentation](../Upgrade.md))
- **Description**: Upgrade names that conflict with this healing behavior. When any of the specified upgrades are active, healing is disabled. When empty (default), no upgrade conflicts prevent healing. Multiple upgrade names can be specified as a space-separated list. This property accepts a list of upgrade names that are resolved to upgrade flags internally.
- **Default**: `""` (empty, no conflicts)
- **Example**: `ConflictsWith = Upgrade_GLACamoNetting`

#### `RemovesUpgrades`
Available in: *(v1.04)*

- **Type**: Upgrade name or list of upgrade names (see [Upgrade documentation](../Upgrade.md))
- **Description**: Upgrade names that are removed when this healing behavior activates. When set, the specified upgrades are removed from the object when the healing behavior is activated. When empty (default), no upgrades are removed. Multiple upgrade names can be specified as a space-separated list. This property accepts a list of upgrade names that are resolved to upgrade flags internally.
- **Default**: `""` (empty)
- **Example**: `RemovesUpgrades = Upgrade_OldHealing`

#### `RequiresAllTriggers`
Available in: *(v1.04)*

- **Type**: `Bool`
- **Description**: Whether all [TriggeredBy](#triggeredby) upgrades must be active (`Yes`) or just one (`No`). When `Yes`, all specified upgrades must be active for healing. When `No` (default), only one upgrade needs to be active.
- **Default**: `No`
- **Example**: `RequiresAllTriggers = Yes`

#### `FXListUpgrade`
Available in: *(v1.04)*

- **Type**: `FXList` (see [FXList documentation](../FXList.md))
- **Description**: FXList played when the upgrade activates. When set, displays visual and audio effects when the healing behavior is activated by upgrades. When empty (default), no upgrade activation effects are shown.
- **Default**: `""` (empty)
- **Example**: `FXListUpgrade = FX_HealUpgrade`

### Particle Effects

Available in: *(v1.04)*

#### `RadiusParticleSystemName`
Available in: *(v1.04)*

- **Type**: `ParticleSystemTemplate` (see [ParticleSystem documentation](../ParticleSystem.md))
- **Description**: Particle system played for the entire area healing effect duration. When set, displays visual effects during area healing. When empty (default), no particle effects are shown.
- **Default**: `""` (empty)
- **Example**: `RadiusParticleSystemName = FX_HealArea`

#### `UnitHealPulseParticleSystemName`
Available in: *(v1.04)*

- **Type**: `ParticleSystemTemplate` (see [ParticleSystem documentation](../ParticleSystem.md))
- **Description**: Particle system played on each unit when it receives healing. When set, displays visual effects on individual units during healing. When empty (default), no pulse effects are shown.
- **Default**: `""` (empty)
- **Example**: `UnitHealPulseParticleSystemName = FX_HealPulse`

### Target Selection

Available in: *(v1.04)*

#### `KindOf`
Available in: *(v1.04)*

- **Type**: `KindOfMaskType` (bit flags) (see [KindOfMaskType Values](#kindofmasktype-values) section)
- **Description**: Object types that can be healed by this behavior. When set, only objects with matching types can be healed. When 0 (default, all bits set), all object types can be healed.
- **Default**: `0` (all types)
- **Example**: `KindOf = INFANTRY VEHICLE`

#### `ForbiddenKindOf`
Available in: *(v1.04, Zero Hour only)*

- **Type**: `KindOfMaskType` (bit flags) (see [KindOfMaskType Values](#kindofmasktype-values) section)
- **Description**: Object types that cannot be healed by this behavior. When set, objects with matching types are excluded from healing. When 0 (default), no object types are forbidden from healing.
- **Default**: `0` (no restrictions)
- **Example**: `ForbiddenKindOf = STRUCTURE`

## Enum Value Lists

#### `KindOfMaskType` Values
Available in: *(v1.04)*

**Source:** `GeneralsMD/Code/GameEngine/Source/Common/System/KindOf.cpp` - `KindOfMaskType::s_bitNameList[]` array definition

- **`OBSTACLE`** *(v1.04)* - Obstacle objects
- **`SELECTABLE`** *(v1.04)* - Selectable objects
- **`IMMOBILE`** *(v1.04)* - Immobile objects
- **`CAN_ATTACK`** *(v1.04)* - Objects that can attack
- **`STICK_TO_TERRAIN_SLOPE`** *(v1.04)* - Objects that stick to terrain slopes
- **`CAN_CAST_REFLECTIONS`** *(v1.04)* - Objects that can cast reflections
- **`SHRUBBERY`** *(v1.04)* - Shrubbery/vegetation
- **`STRUCTURE`** *(v1.04)* - Building structures
- **`INFANTRY`** *(v1.04)* - Infantry units
- **`VEHICLE`** *(v1.04)* - Vehicle units
- **`AIRCRAFT`** *(v1.04)* - Aircraft units
- **`HUGE_VEHICLE`** *(v1.04)* - Large vehicle units
- **`DOZER`** *(v1.04)* - Dozer vehicles
- **`HARVESTER`** *(v1.04)* - Harvester vehicles
- **`COMMANDCENTER`** *(v1.04)* - Command center buildings
- **`LINEBUILD`** *(v1.04)* - Line building structures
- **`SALVAGER`** *(v1.04)* - Salvager units
- **`WEAPON_SALVAGER`** *(v1.04)* - Weapon salvager units
- **`TRANSPORT`** *(v1.04)* - Transport units
- **`BRIDGE`** *(v1.04)* - Bridge structures
- **`LANDMARK_BRIDGE`** *(v1.04)* - Landmark bridge structures
- **`BRIDGE_TOWER`** *(v1.04)* - Bridge tower structures
- **`PROJECTILE`** *(v1.04)* - Projectile objects
- **`PRELOAD`** *(v1.04)* - Preloaded objects
- **`NO_GARRISON`** *(v1.04)* - Objects that cannot be garrisoned
- **`WAVEGUIDE`** *(v1.04)* - Wave guide objects
- **`WAVE_EFFECT`** *(v1.04)* - Wave effect objects
- **`NO_COLLIDE`** *(v1.04)* - Objects that don't collide
- **`REPAIR_PAD`** *(v1.04)* - Repair pad structures
- **`HEAL_PAD`** *(v1.04)* - Heal pad structures
- **`STEALTH_GARRISON`** *(v1.04)* - Stealth garrison structures
- **`CASH_GENERATOR`** *(v1.04)* - Cash generator structures
- **`DRAWABLE_ONLY`** *(v1.04)* - Drawable only objects
- **`MP_COUNT_FOR_VICTORY`** *(v1.04)* - Multiplayer victory count objects
- **`REBUILD_HOLE`** *(v1.04)* - Rebuild hole structures
- **`SCORE`** *(v1.04)* - Score objects
- **`SCORE_CREATE`** *(v1.04)* - Score creation objects
- **`SCORE_DESTROY`** *(v1.04)* - Score destruction objects
- **`NO_HEAL_ICON`** *(v1.04)* - Objects without heal icon
- **`CAN_RAPPEL`** *(v1.04)* - Objects that can rappel
- **`PARACHUTABLE`** *(v1.04)* - Parachutable objects
- **`CAN_BE_REPULSED`** *(v1.04)* - Objects that can be repulsed
- **`MOB_NEXUS`** *(v1.04)* - Mob nexus objects
- **`IGNORED_IN_GUI`** *(v1.04)* - Objects ignored in GUI
- **`CRATE`** *(v1.04)* - Crate objects
- **`CAPTURABLE`** *(v1.04)* - Capturable objects
- **`CLEARED_BY_BUILD`** *(v1.04)* - Objects cleared by building
- **`SMALL_MISSILE`** *(v1.04)* - Small missile projectiles
- **`ALWAYS_VISIBLE`** *(v1.04)* - Always visible objects
- **`UNATTACKABLE`** *(v1.04)* - Unattackable objects
- **`MINE`** *(v1.04)* - Mine objects
- **`CLEANUP_HAZARD`** *(v1.04)* - Cleanup hazard objects
- **`PORTABLE_STRUCTURE`** *(v1.04)* - Portable structure objects
- **`ALWAYS_SELECTABLE`** *(v1.04)* - Always selectable objects
- **`ATTACK_NEEDS_LINE_OF_SIGHT`** *(v1.04)* - Objects requiring line of sight to attack
- **`WALK_ON_TOP_OF_WALL`** *(v1.04)* - Objects that walk on top of walls
- **`DEFENSIVE_WALL`** *(v1.04)* - Defensive wall structures
- **`FS_POWER`** *(v1.04)* - Firestorm power structures
- **`FS_FACTORY`** *(v1.04)* - Firestorm factory structures
- **`FS_BASE_DEFENSE`** *(v1.04)* - Firestorm base defense structures
- **`FS_TECHNOLOGY`** *(v1.04)* - Firestorm technology structures
- **`AIRCRAFT_PATH_AROUND`** *(v1.04)* - Objects aircraft path around
- **`LOW_OVERLAPPABLE`** *(v1.04)* - Low overlappable objects
- **`FORCEATTACKABLE`** *(v1.04)* - Force attackable objects
- **`AUTO_RALLYPOINT`** *(v1.04)* - Auto rally point objects
- **`TECH_BUILDING`** *(v1.04)* - Technology buildings
- **`POWERED`** *(v1.04)* - Powered structures
- **`PRODUCED_AT_HELIPAD`** *(v1.04)* - Objects produced at helipad
- **`DRONE`** *(v1.04)* - Drone objects
- **`CAN_SEE_THROUGH_STRUCTURE`** *(v1.04)* - Objects that can see through structures
- **`BALLISTIC_MISSILE`** *(v1.04)* - Ballistic missile projectiles
- **`CLICK_THROUGH`** *(v1.04)* - Click through objects
- **`SUPPLY_SOURCE_ON_PREVIEW`** *(v1.04)* - Supply source on preview objects
- **`PARACHUTE`** *(v1.04)* - Parachute objects
- **`GARRISONABLE_UNTIL_DESTROYED`** *(v1.04)* - Garrisonable until destroyed objects
- **`BOAT`** *(v1.04)* - Boat objects
- **`IMMUNE_TO_CAPTURE`** *(v1.04)* - Immune to capture objects
- **`HULK`** *(v1.04)* - Hulk objects
- **`SHOW_PORTRAIT_WHEN_CONTROLLED`** *(v1.04)* - Show portrait when controlled objects
- **`SPAWNS_ARE_THE_WEAPONS`** *(v1.04)* - Spawns are the weapons objects
- **`CANNOT_BUILD_NEAR_SUPPLIES`** *(v1.04)* - Cannot build near supplies objects
- **`SUPPLY_SOURCE`** *(v1.04)* - Supply source objects
- **`REVEAL_TO_ALL`** *(v1.04)* - Reveal to all objects
- **`DISGUISER`** *(v1.04)* - Disguiser objects
- **`INERT`** *(v1.04)* - Inert objects
- **`HERO`** *(v1.04)* - Hero objects
- **`IGNORES_SELECT_ALL`** *(v1.04)* - Ignores select all objects
- **`DONT_AUTO_CRUSH_INFANTRY`** *(v1.04)* - Don't auto crush infantry objects
- **`CLIFF_JUMPER`** *(v1.04)* - Cliff jumper objects
- **`FS_SUPPLY_DROPZONE`** *(v1.04)* - Firestorm supply dropzone structures
- **`FS_SUPERWEAPON`** *(v1.04)* - Firestorm superweapon structures
- **`FS_BLACK_MARKET`** *(v1.04)* - Firestorm black market structures
- **`FS_SUPPLY_CENTER`** *(v1.04)* - Firestorm supply center structures
- **`FS_STRATEGY_CENTER`** *(v1.04)* - Firestorm strategy center structures
- **`MONEY_HACKER`** *(v1.04)* - Money hacker objects
- **`ARMOR_SALVAGER`** *(v1.04)* - Armor salvager objects
- **`REVEALS_ENEMY_PATHS`** *(v1.04)* - Reveals enemy paths objects
- **`BOOBY_TRAP`** *(v1.04)* - Booby trap objects
- **`FS_FAKE`** *(v1.04)* - Firestorm fake structures
- **`FS_INTERNET_CENTER`** *(v1.04)* - Firestorm internet center structures
- **`BLAST_CRATER`** *(v1.04)* - Blast crater structures
- **`PROP`** *(v1.04)* - Prop objects
- **`OPTIMIZED_TREE`** *(v1.04)* - Optimized tree objects
- **`FS_ADVANCED_TECH`** *(v1.04)* - Firestorm advanced tech structures
- **`FS_BARRACKS`** *(v1.04)* - Firestorm barracks structures
- **`FS_WARFACTORY`** *(v1.04)* - Firestorm war factory structures
- **`FS_AIRFIELD`** *(v1.04)* - Firestorm airfield structures
- **`AIRCRAFT_CARRIER`** *(v1.04)* - Aircraft carrier objects
- **`NO_SELECT`** *(v1.04)* - No select objects
- **`REJECT_UNMANNED`** *(v1.04)* - Reject unmanned objects
- **`CANNOT_RETALIATE`** *(v1.04)* - Cannot retaliate objects
- **`TECH_BASE_DEFENSE`** *(v1.04)* - Tech base defense objects
- **`EMP_HARDENED`** *(v1.04)* - EMP hardened objects
- **`DEMOTRAP`** *(v1.04)* - Demo trap objects
- **`CONSERVATIVE_BUILDING`** *(v1.04)* - Conservative building objects
- **`IGNORE_DOCKING_BONES`** *(v1.04)* - Ignore docking bones objects
- **`TANK`** *(v1.04)* - Tank vehicles
- **`APC`** *(v1.04)* - Armored Personnel Carrier vehicles
- **`IFV`** *(v1.04)* - Infantry Fighting Vehicle vehicles
- **`TRUCK`** *(v1.04)* - Truck vehicles
- **`VTOL`** *(v1.04)* - Vertical Take-Off and Landing aircraft
- **`JET`** *(v1.04)* - Jet aircraft
- **`HELICOPTER`** *(v1.04)* - Helicopter aircraft
- **`HOT_AIR_BALLOON`** *(v1.04)* - Hot air balloon aircraft
- **`BLIMP`** *(v1.04)* - Blimp aircraft
- **`LARGE_AIRCRAFT`** *(v1.04)* - Large aircraft
- **`MEDIUM_AIRCRAFT`** *(v1.04)* - Medium aircraft
- **`SMALL_AIRCRAFT`** *(v1.04)* - Small aircraft
- **`ARTILLERY`** *(v1.04)* - Artillery units
- **`HEAVY_ARTILLERY`** *(v1.04)* - Heavy artillery units
- **`ANTI_AIR`** *(v1.04)* - Anti-air units
- **`SAM`** *(v1.04)* - Surface-to-Air Missile units
- **`SCOUT`** *(v1.04)* - Scout units
- **`COMMANDO`** *(v1.04)* - Commando units
- **`HEAVY_INFANTRY`** *(v1.04)* - Heavy infantry units
- **`SUPERHEAVY_VEHICLE`** *(v1.04)* - Super heavy vehicle units
- **`EW_RADAR`** *(v1.04)* - Electronic Warfare radar units
- **`EW_RADAR_JAMMER`** *(v1.04)* - Electronic Warfare radar jammer units
- **`EW_RADIO_JAMMER`** *(v1.04)* - Electronic Warfare radio jammer units
- **`EW_JAMMABLE`** *(v1.04)* - Electronic Warfare jammable units
- **`EW_DIRECT_JAMMABLE`** *(v1.04)* - Electronic Warfare directly jammable units
- **`EW_AREA_JAMMABLE`** *(v1.04)* - Electronic Warfare area jammable units
- **`EW_RADIO_JAMMABLE`** *(v1.04)* - Electronic Warfare radio jammable units
- **`EXTRA1`** through **`EXTRA16`** *(v1.04)* - Custom KindOf types

## Examples

### Scout Van Auto Heal
```ini
Behavior = AutoHealBehavior ModuleTag_01
  HealingAmount = 3
  HealingDelay = 1000
  TriggeredBy = Upgrade_GLAJunkRepair
End
```

### Scrap Yard Auto Heal
```ini
Behavior = AutoHealBehavior ModuleTag_02
  HealingAmount = 2
  HealingDelay = 1000
  StartsActive = No
  TriggeredBy = Upgrade_GLAJunkRepair
End
```

### Medical Building Area Heal
```ini
Behavior = AutoHealBehavior ModuleTag_03
  HealingAmount = 5
  HealingDelay = 2000
  Radius = 100.0
  KindOf = INFANTRY VEHICLE
  ForbiddenKindOf = STRUCTURE
  RadiusParticleSystemName = FX_HealArea
  UnitHealPulseParticleSystemName = FX_HealPulse
  StartsActive = Yes
End
```

### Single Burst Heal
```ini
Behavior = AutoHealBehavior ModuleTag_04
  HealingAmount = 50
  SingleBurst = Yes
  KindOf = INFANTRY
  AffectsWholePlayer = Yes
  SkipSelfForHealing = Yes
End
```

## Usage

Place under `Behavior = AutoHealBehavior ModuleTag_XX` inside [Object](../Object.md) entries. See [Template](#template) for correct syntax.

**Placement**:
- AutoHealBehavior can only be added to `Object` entries.

Multiple instances of AutoHealBehavior can be added to the same object. Each instance operates independently with its own healing rates, conditions, and upgrade requirements.

**Limitations**:
- Healing only works on objects within the same team/player. Objects must be owned by the same player as the healing object.
- Area healing is limited by the specified [Radius](#radius) or player ownership ([AffectsWholePlayer](#affectswholeplayer)). Objects outside the radius (or not owned by the player when `AffectsWholePlayer` is `No`) cannot be healed.
- The behavior can be configured for single burst healing ([SingleBurst](#singleburst)) or continuous healing over time. When [SingleBurst](#singleburst) is `Yes`, healing occurs once then stops.
- Objects must have a health system (typically [ActiveBody](../ObjectBody/ActiveBody.md)) to receive healing. Objects without health systems cannot be healed.
- When [HealingAmount](#healingamount) is 0, no healing occurs and the behavior is effectively disabled.
- When [HealingDelay](#healingdelay) is not specified (defaults to UINT_MAX), the behavior will not heal regularly and must be activated manually or through upgrades.

**Conditions**:
- AutoHealBehavior can heal individual objects (when [Radius](#radius) is 0.0) or provide area healing within a specified radius (when [Radius](#radius) > 0.0).
- The behavior integrates with the upgrade system for conditional activation. When [TriggeredBy](#triggeredby) is set, healing only occurs when the specified upgrades are active (see [Upgrade Integration](#upgrade-integration) properties).
- Area healing can target specific object types using [KindOf](#kindof) and [ForbiddenKindOf](#forbiddenkindof) filters (see [Target Selection](#target-selection) properties).
- Particle effects provide visual feedback for healing operations when [RadiusParticleSystemName](#radiusparticlesystemname) or [UnitHealPulseParticleSystemName](#unithealpulseparticlesystemname) are set.
- When [StartsActive](#startsactive) is `Yes`, healing begins immediately upon object creation. When `No`, healing must be activated by upgrades.
- When [SingleBurst](#singleburst) is `Yes`, healing occurs once then the behavior goes to sleep. When `No`, healing continues at regular intervals.
- When [StartHealingDelay](#starthealingdelay) is set, the object waits for the specified delay after taking damage before auto-healing starts. This only applies when healing the object itself ([Radius](#radius) = 0).
- AutoHealBehavior continues to heal even when objects are held (stunned/immobilized) or jammed (electronically disabled). Healing is not interrupted by these disabled states.
- **Multiple instances behavior**: Multiple instances of AutoHealBehavior can coexist on the same object. Each instance operates independently with its own healing rates, conditions, upgrade requirements, and target filters.
- **ObjectReskin**: ObjectReskin uses the same module system as Object. Adding AutoHealBehavior to an ObjectReskin entry with the same `ModuleTag` name as the base object will cause a duplicate module tag error, as ObjectReskin does not support automatic module replacement.

**Dependencies**:
- Requires an object with a health system (typically [ActiveBody](../ObjectBody/ActiveBody.md)) to function. AutoHealBehavior works with all body types that implement `BodyModuleInterface`: [ActiveBody](../ObjectBody/ActiveBody.md), [StructureBody](../ObjectBody/StructureBody.md), [HighlanderBody](../ObjectBody/HighlanderBody.md), [ImmortalBody](../ObjectBody/ImmortalBody.md), [UndeadBody](../ObjectBody/UndeadBody.md) (both first and second life), and [HiveStructureBody](../ObjectBody/HiveStructureBody.md). AutoHealBehavior does not work with [InactiveBody](../ObjectBody/InactiveBody.md) because those objects are marked as "effectively dead" and are skipped by the healing system.
- Healing is affected by damage types and armor systems (see [Armor](../Armor.md)). Armor and damage type interactions apply to healing calculations.

## Template

```ini
Behavior = AutoHealBehavior ModuleTag_XX
  ; Basic Healing Settings
  HealingAmount = 0                       ; // health points healed per pulse *(v1.04)*
  HealingDelay = 4294967295               ; // milliseconds between healing pulses (must be set for healing) *(v1.04)*
  StartsActive = No                       ; // whether healing starts active *(v1.04)*
  SingleBurst = No                        ; // whether to perform single burst only *(v1.04)*
  StartHealingDelay = 0                   ; // delay after damage before healing starts *(v1.04)*

  ; Area Healing Settings
  Radius = 0.0                            ; // healing radius (0 = self only) *(v1.04)*
  AffectsWholePlayer = No                 ; // affect all player objects *(v1.04)*
  SkipSelfForHealing = No                 ; // skip self when area healing *(v1.04, Zero Hour only)*

  ; Upgrade Integration
  TriggeredBy =                           ; // required upgrade names *(v1.04)*
  RequiredAnyUpgradeOf =                  ; // alternative upgrade names (any required) *(v1.04)*
  RequiredAllUpgradesOf =                 ; // upgrade names (all required) *(v1.04)*
  ConflictsWith =                         ; // conflicting upgrade names *(v1.04)*
  RemovesUpgrades =                       ; // upgrade names to remove on activation *(v1.04)*
  RequiresAllTriggers = No                ; // require all TriggeredBy upgrades *(v1.04)*
  FXListUpgrade =                         ; // upgrade activation FXList *(v1.04)*

  ; Particle Effects
  RadiusParticleSystemName =              ; // area healing particle system *(v1.04)*
  UnitHealPulseParticleSystemName =       ; // unit healing particle system *(v1.04)*

  ; Target Selection
  KindOf =                                ; // allowed object types *(v1.04)*
  ForbiddenKindOf =                       ; // forbidden object types *(v1.04, Zero Hour only)*
End
```

## Notes

- AutoHealBehavior can heal individual objects or provide area healing within a specified radius. The behavior supports three modes: self-healing ([Radius](#radius) = 0), radius-based area healing, and player-wide healing ([AffectsWholePlayer](#affectswholeplayer) = Yes).
- The behavior integrates with the upgrade system for conditional activation. When [TriggeredBy](#triggeredby) is set, healing only occurs when the specified upgrades are active. Upgrade conflicts ([ConflictsWith](#conflictswith)) can disable healing when conflicting upgrades are active. Additional upgrade requirements can be specified using [RequiredAnyUpgradeOf](#requiredanyupgradeof) and [RequiredAllUpgradesOf](#requiredallupgradesof).
- Area healing can target specific object types using [KindOf](#kindof) and [ForbiddenKindOf](#forbiddenkindof) filters. Objects must match [KindOf](#kindof) types and not match [ForbiddenKindOf](#forbiddenkindof) types to be healed.
- Particle effects provide visual feedback for healing operations. [RadiusParticleSystemName](#radiusparticlesystemname) displays effects for the entire area healing duration, while [UnitHealPulseParticleSystemName](#unithealpulseparticlesystemname) displays effects on each unit when it receives healing.
- The behavior can be configured for single burst healing ([SingleBurst](#singleburst)) or continuous healing over time. When [SingleBurst](#singleburst) is `Yes`, healing occurs once then stops.
- Objects with [ActiveBody](../ObjectBody/ActiveBody.md) can be healed by AutoHealBehavior. Healing is affected by damage types and armor systems (see [Armor](../Armor.md)).
- When [StartHealingDelay](#starthealingdelay) is set, the object waits for the specified delay after taking damage before auto-healing starts. This only applies when healing the object itself ([Radius](#radius) = 0).
- AutoHealBehavior continues to heal even when objects are held (stunned/immobilized) or jammed (electronically disabled). Healing is not interrupted by these disabled states.
- Multiple instances of AutoHealBehavior can coexist on the same object. Each instance operates independently with its own healing rates, conditions, upgrade requirements, and target filters.

## Modder Recommended Use Scenarios

(pending modder review)

## Source Files

**Base Class:** `UpdateModule`, `UpgradeMux`, `DamageModuleInterface`

- Header (Retail Zero Hour 1.04): `GeneralsMD/Code/GameEngine/Include/GameLogic/Module/AutoHealBehavior.h`
- Source (Retail Zero Hour 1.04): `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Behavior/AutoHealBehavior.cpp`
- Header (Retail Generals 1.04): `Generals/Code/GameEngine/Include/GameLogic/Module/AutoHealBehavior.h`
- Source (Retail Generals 1.04): `Generals/Code/GameEngine/Source/GameLogic/Object/Behavior/AutoHealBehavior.cpp`

## Changes History

- No changes done since 1.04

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated
- Last Updated: 16/12/2025 by AI
- Certification: 0/2 reviews

### Reviewers

- (pending)

