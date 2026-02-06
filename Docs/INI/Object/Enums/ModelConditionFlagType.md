# ModelConditionFlagType

Status: AI-generated, 0/2 reviews

## Overview

The `ModelConditionFlagType` (commonly referred to as `ModelConditionFlags` or `ModelCondition`) is a bit flag system used to represent visual states and animation conditions for objects. Model conditions affect which model animations, visual states, and particle effects are displayed. Objects can have multiple model condition flags set simultaneously, allowing for complex visual state combinations. Model conditions are used throughout the game for visual feedback, animations, damage states, weapon firing states, and various other visual representations. This is a system-level type, not a module added inside `Object` entries.

Available in: *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)*

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
- [Enum Value Lists](#enum-value-lists)
  - [Damage States](#damage-states)
  - [Environmental Conditions](#environmental-conditions)
  - [Fire and Burn States](#fire-and-burn-states)
  - [Construction States](#construction-states)
  - [Movement and Action States](#movement-and-action-states)
  - [Aircraft States](#aircraft-states)
  - [Deployment and Packing](#deployment-and-packing)
  - [Garrison and Containment](#garrison-and-containment)
  - [Docking States](#docking-states)
  - [Door States (4 doors, 4 states each)](#door-states-4-doors-4-states-each)
  - [Combat and Weapon States - Weapon Slot A](#combat-and-weapon-states---weapon-slot-a)
  - [Combat and Weapon States - Weapon Slot B](#combat-and-weapon-states---weapon-slot-b)
  - [Combat and Weapon States - Weapon Slot C](#combat-and-weapon-states---weapon-slot-c)
  - [Combat and Weapon States - Weapon Slots D-H](#combat-and-weapon-states---weapon-slots-d-h)
  - [Turret and Rotation](#turret-and-rotation)
  - [Special Combat States](#special-combat-states)
  - [Radar and Power](#radar-and-power)
  - [Capture and Special States](#capture-and-special-states)
  - [Special Animations](#special-animations)
  - [Preorder and Movement](#preorder-and-movement)
  - [Rider States (8 riders)](#rider-states-8-riders)
  - [Weapon Set and Upgrade States](#weapon-set-and-upgrade-states)
  - [Armor Set States](#armor-set-states)
  - [User-Definable Flags (5 user flags)](#user-definable-flags-5-user-flags)
  - [Armor Upgrade Flags (5 armor upgrades)](#armor-upgrade-flags-5-armor-upgrades)
  - [Weapon Upgrade Flags (5 weapon upgrades)](#weapon-upgrade-flags-5-weapon-upgrades)
  - [Component Damage States](#component-damage-states)
  - [Surrender State](#surrender-state)
- [Examples](#examples)
- [Notes](#notes)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

**Limitations**:
- Model condition flags are bit flags that can be combined. Multiple flags can be set on a single object simultaneously.
- Model condition flags are set automatically by game systems based on object state, damage, actions, and other conditions.
- Some model condition flags are mutually exclusive or have specific activation conditions (e.g., `FIRING_A` and `RELOADING_A` cannot be active simultaneously for the same weapon slot).
- Model condition flags are used internally by the game engine; they are not directly set in INI files but are referenced in model definitions and animation configurations.

**Conditions**:
- Model condition flags are set automatically by game systems based on object state (e.g., damage states, weapon firing, movement, construction).
- Multiple model condition flags can be active simultaneously (e.g., `DAMAGED` and `NIGHT` can both be active).
- Model condition flags affect which model animations and visual states are displayed.
- Some model condition flags are temporary and automatically cleared after a duration (e.g., `SPECIAL_CHEERING`).

**Dependencies**:
- Model condition flags are used by model rendering systems to select appropriate animations and visual states.
- Various game systems automatically set model condition flags (e.g., damage systems set `DAMAGED`, weapon systems set `FIRING_A`, construction systems set `PARTIALLY_CONSTRUCTED`).
- Model definitions and animation configurations reference model condition flags to determine which animations to play.

## Enum Value Lists

### `ModelConditionFlagType` Values
Available in: *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)*

### Damage States
- **`TOPPLED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object has been toppled over. Set automatically by `ToppleUpdate` when a structure falls over due to structural damage. Used for visual feedback when structures collapse sideways rather than vertically. The flag is set when the toppling animation begins, allowing models to display toppled state animations. Used by structure collapse systems to indicate a toppled building state.

- **`FRONTCRUSHED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Front of object has been crushed. Set automatically by `CrushDie` when a vehicle is crushed from the front or totally crushed. Used when crush damage type is `FRONT_END_CRUSH` or `TOTAL_CRUSH`. Provides visual feedback for vehicles that have been crushed by heavier units or crushed objects. Used to display front-end damage animations on vehicles.

- **`BACKCRUSHED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Back of object has been crushed. Set automatically by `CrushDie` when a vehicle is crushed from the back or totally crushed. Used when crush damage type is `BACK_END_CRUSH` or `TOTAL_CRUSH`. Provides visual feedback for vehicles that have been crushed by heavier units or crushed objects. Used to display back-end damage animations on vehicles.

- **`DAMAGED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is damaged. Set automatically by `Drawable::reactToBodyDamageStateChange()` when the object's body damage state changes to `BODYDAMAGETYPE_DAMAGED`. This corresponds to the first damage threshold (typically when health drops below 50-70% of maximum). Used extensively by garrison systems to find garrison fire points when structures are in damaged state. Used by damage state systems to trigger damage animations and visual effects. Automatically cleared and set as damage states change.

- **`REALLYDAMAGED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is really damaged. Set automatically by `Drawable::reactToBodyDamageStateChange()` when the object's body damage state changes to `BODYDAMAGETYPE_REALLY_DAMAGED`. This corresponds to the second damage threshold (typically when health drops below 10-35% of maximum). Used by garrison systems to find garrison fire points when structures are in really damaged state. Used by damage state systems to trigger severe damage animations and visual effects. Structures in this state typically cannot accept new garrisoned units unless they have the `GARRISONABLE_UNTIL_DESTROYED` KindOf flag.

- **`RUBBLE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is in rubble state. Set automatically by `Drawable::reactToBodyDamageStateChange()` when the object's body damage state changes to `BODYDAMAGETYPE_RUBBLE`. This is the final damage state before destruction. Used by minefield systems to indicate destroyed mine objects. Used by structure collapse and topple systems to clear the rubble state when structures are being toppled or collapsed. Used by rendering systems to determine if structures should be displayed as rubble (structures with this flag may have reduced height for collision detection). Automatically cleared by `StructureToppleUpdate` and `StructureCollapseUpdate` when structures begin toppling or collapsing.

- **`SPECIAL_DAMAGED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object has special damage state. Set automatically by damage systems for objects that require special damage handling beyond the standard damage states. Used by garrison systems to clear this flag when finding garrison fire points. Used for objects that have unique damage states that don't fit the standard `DAMAGED`, `REALLY_DAMAGED`, or `RUBBLE` progression.

- **`SPLATTED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object has been splatted. Used for visual feedback when objects are flattened or crushed flat by heavy units or objects. Used by death and damage systems to indicate objects that have been completely flattened. Allows models to display splatted animations for units that have been crushed flat.

- **`EXPLODED_FLAILING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is exploding and flailing. Used for visual feedback during explosive death animations where the object flails around during destruction. Used by death animation systems to trigger flailing explosion animations. Allows models to display chaotic flailing animations during explosive destruction sequences.

- **`EXPLODED_BOUNCING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is exploding and bouncing. Used for visual feedback during explosive death animations where the object bounces around during destruction. Used by death animation systems to trigger bouncing explosion animations. Allows models to display bouncing animations during explosive destruction sequences, particularly for vehicles or objects that bounce after explosions.

### Environmental Conditions
- **`NIGHT`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Night time conditions. Set automatically by `Object::boundDrawableToObject()` when `TheGlobalData->m_forceModelsToFollowTimeOfDay` is enabled and the current time of day is `TIME_OF_DAY_NIGHT`. Updated dynamically as time of day changes. Used by rendering systems to determine if objects should display night-time visual effects (e.g., headlights on vehicles are hidden when this flag is set). Used by model rendering to select night-time animations and visual states. Allows models to display different appearances during night versus day.

- **`SNOW`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Snow conditions. Set automatically by `Object::boundDrawableToObject()` when `TheGlobalData->m_forceModelsToFollowWeather` is enabled and the current weather is `WEATHER_SNOWY`. Updated dynamically as weather changes. Used by rendering systems to determine if objects should display snow-related visual effects. Used by model rendering to select snow-condition animations and visual states. Allows models to display different appearances in snowy weather conditions.

- **`FLOODED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is flooded. Set automatically by `WaveGuideUpdate` when objects are submerged in water or flooded areas. Used for visual feedback when objects are in flooded conditions. Used by wave guide systems to indicate objects that are affected by water flooding. Allows models to display flooded state animations and visual effects.

- **`OVER_WATER`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Units that can go over water want cool effects for doing so. Set automatically by `Locomotor` when units are moving over water surfaces. Cleared when units leave water. Used by locomotors to track when units are traversing water. Used by rendering systems to display water-traversal visual effects (e.g., water splashes, wake effects). Allows models to display special animations and effects when units move over water. Used by `RailroadGuideAIUpdate` for railroad guides that extend over water.

### Fire and Burn States
- **`AFLAME`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is on fire. Set automatically by `FlammableUpdate::tryToIgnite()` when an object catches fire. Cleared when the fire duration expires (`AflameDuration`). While this flag is set, the object takes periodic fire damage (`AflameDamageAmount` every `AflameDamageDelay`). Used by `FlammableUpdate` to track active fire state. Triggers burning sound effects while active. Used by fire spread systems to propagate fire to nearby flammable objects. Automatically cleared when fire duration expires or when the object transitions to burned state. Allows models to display active fire animations and particle effects.

- **`SMOLDERING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is smoldering. Set automatically by `FlammableUpdate` when the object transitions from `AFLAME` to burned state but the fire has not fully extinguished. Set when the burned delay expires while the object is still aflame. Used to indicate objects that have been burned but may still have residual smoke or smoldering effects. Allows models to display smoldering animations and smoke effects after the main fire has extinguished.

- **`BURNED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object has been burned. Set automatically by `NeutronMissileSlowDeathUpdate` when objects are affected by neutron missile effects. Also set by `FlammableUpdate` when objects are burned. Used by `ToppleUpdate` to check if objects should create burned stump objects when toppled. Used to indicate objects that have been permanently burned or charred. Objects with this flag may have different visual appearances and cannot catch fire again if they have the burned status set. Allows models to display burned/charred animations and visual states.

### Construction States
- **`AWAITING_CONSTRUCTION`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is awaiting construction. Set automatically by construction systems when a structure is placed but not yet being actively built. Used by `DozerAIUpdate` to identify structures that are ready for construction to begin. Used by worker AI systems to find construction targets. Cleared when construction begins. Used to indicate structures that are queued for construction but not currently being built.

- **`PARTIALLY_CONSTRUCTED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is partially constructed. Set automatically by `BuildAssistant` when a structure's construction percent is greater than 0% but less than 100%. Used by `DozerAIUpdate` and `WorkerAIUpdate` to identify structures that are currently under construction. Used by construction systems to indicate structures that are partially built. Allows models to display partially constructed building animations. Typically set in combination with `ACTIVELY_BEING_CONSTRUCTED` when construction is in progress.

- **`ACTIVELY_BEING_CONSTRUCTED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is actively being constructed. Set automatically by `BuildAssistant` when a structure is currently being built by workers. Used by `DozerAIUpdate` and `WorkerAIUpdate` to identify structures that are actively under construction. Used by construction systems to indicate structures that are being worked on right now. Allows models to display active construction animations (e.g., workers visible, construction effects). Typically set in combination with `PARTIALLY_CONSTRUCTED` when construction is in progress.

- **`ACTIVELY_CONSTRUCTING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is actively constructing something. Set on construction units (like dozers) when they are actively building another structure. Used to indicate that the object itself is performing construction work. Allows models to display construction animations on the builder unit. Used by construction systems to track which units are currently building.

- **`CONSTRUCTION_COMPLETE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object construction is complete. Set automatically when a structure reaches 100% construction completion. Used to indicate that construction has finished. Cleared when construction-related flags are reset. Allows models to display completion animations or transition to fully constructed state.

- **`POST_COLLAPSE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object has collapsed. Set automatically by structure collapse systems when a structure has finished collapsing. Used to indicate the final state after a structure has collapsed. Used by collapse animation systems to mark the end of the collapse sequence. Allows models to display post-collapse animations or remain in collapsed state.

### Movement and Action States
- **`MOVING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is moving. Set automatically by `AIStates` when units are actively moving across terrain. Cleared when units stop moving. Used by AI systems to track unit movement state. Used by pathfinding systems to determine if units are in motion. Used by `TensileFormationUpdate` to synchronize formation movement animations. Used by `RailedTransportDockUpdate` to indicate when units are being moved during docking operations. Used by `AIUpdate` to clear the flag when units stop. Mutually exclusive with `CLIMBING` - units set `CLIMBING` when traversing cliffs instead of `MOVING`. Allows models to display movement animations and effects.

- **`PRONE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is in prone position. Set automatically by `ProneUpdate` when infantry units go prone. Cleared when units stand up. Used by prone update systems to track when units are in prone position. Allows models to display prone animations for infantry units. Used for units that can take cover or reduce their profile by going prone.

- **`FREEFALL`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is in freefall. Set automatically by `ParachuteContain` when objects are falling without a parachute. Used in conjunction with `PARACHUTING` - objects transition from `FREEFALL` to `PARACHUTING` when the parachute deploys. Cleared when objects land or when the parachute is deployed. Used by parachute systems to track falling objects. Used by `SlowDeathBehavior` to display freefall animations for units that are falling (e.g., units snagged in trees). Allows models to display freefall animations.

- **`CLIMBING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - For units climbing up or down cliffs. Set automatically by `AIStates` when units are traversing cliff terrain (`PathfindCell::CELL_CLIFF`). Mutually exclusive with `MOVING` - units set `CLIMBING` instead of `MOVING` when climbing cliffs. Used by pathfinding systems to indicate units that are climbing terrain features. Cleared when units finish climbing and return to normal movement. Allows models to display climbing animations for units traversing cliffs.

- **`PARACHUTING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is parachuting. Set automatically by `ParachuteContain` when objects deploy their parachutes. Used in conjunction with `FREEFALL` - objects transition from `FREEFALL` to `PARACHUTING` when the parachute opens. Cleared when objects land. Used by parachute systems to track objects that are descending with parachutes. Used by `SlowDeathBehavior` to display parachuting animations for units that are falling (e.g., units snagged in trees look like they are parachuting). Allows models to display parachute deployment and descent animations.

- **`RAPPELLING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is rappelling. Used for units that are rappelling down surfaces (typically infantry units using rappelling equipment). Allows models to display rappelling animations and visual effects. Used by units with the `CAN_RAPPEL` KindOf flag to indicate they are in rappelling state.

### Aircraft States *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)*
- **`JETAFTERBURNER`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Shows "flames" for extra motive force (eg, when taking off). Used by aircraft to display afterburner visual effects when extra thrust is needed. Allows models to display afterburner flame effects and increased exhaust visuals. Used during takeoff sequences or high-speed maneuvers.

- **`JETEXHAUST`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Shows "exhaust" for motive force. Used by aircraft to display standard exhaust visual effects during normal flight. Allows models to display jet exhaust effects and trail visuals. Used to indicate aircraft engines are active and producing thrust.

- **`LANDED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Aircraft landed state. Set automatically by `ChinookAIUpdate` when aircraft complete landing sequence. Used by aircraft AI systems to indicate aircraft are on the ground. Cleared when aircraft begin takeoff sequence. Used by `ChinookAIUpdate` to track landing state. Allows models to display landed aircraft animations and visual states. Used to indicate aircraft that have successfully landed and are stationary on the ground.

- **`TAKEOFF`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Aircraft takeoff state. Set automatically by `ChinookAIUpdate` when aircraft begin takeoff sequence. Used by aircraft AI systems to indicate aircraft are preparing to take off. Transitions to `TAKING_OFF` during the takeoff sequence. Used by `ChinookAIUpdate` to track takeoff initiation. Cleared when aircraft complete takeoff or when landing sequences begin. Allows models to display takeoff preparation animations.

- **`TAKING_OFF`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Aircraft taking off state. Set automatically by `ChinookAIUpdate` during active takeoff sequence. Used by aircraft AI systems to indicate aircraft are actively taking off. Cleared when aircraft complete takeoff or when landing sequences begin. Used by `ChinookAIUpdate` to track active takeoff. Allows models to display active takeoff animations and visual effects. Used to indicate aircraft that are in the process of lifting off from the ground.

- **`LANDING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Aircraft landing state. Set automatically by `ChinookAIUpdate` during active landing sequence. Used by aircraft AI systems to indicate aircraft are actively landing. Cleared when aircraft complete landing (transitioning to `LANDED`) or when takeoff sequences begin. Used by `ChinookAIUpdate` to track active landing. Allows models to display landing animations and visual effects. Used to indicate aircraft that are in the process of descending and landing.

### Deployment and Packing
- **`PACKING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Packs an object. Set automatically by `SlavedUpdate` and `DeployStyleAIUpdate` when objects begin packing (transitioning from deployed to packed state). Used by deployment systems to indicate objects are packing up. Used by `HackInternetAIUpdate` to clear packing state during hack sequences. Used by `ParticleUplinkCannonUpdate` to manage packing/unpacking states. Used by `SpecialAbilityUpdate` to clear packing state during special abilities. Mutually exclusive with `UNPACKING` and `DEPLOYED`. Allows models to display packing animations. Cleared when packing completes (transitioning to packed state).

- **`UNPACKING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Unpacks an object. Set automatically by `DeployStyleAIUpdate` and `HackInternetAIUpdate` when objects begin unpacking (transitioning from packed to deployed state). Used by deployment systems to indicate objects are unpacking. Used by `ParticleUplinkCannonUpdate` to manage unpacking states. Used by `SpecialAbilityUpdate` to manage unpacking during special abilities. Mutually exclusive with `PACKING` and `DEPLOYED`. Allows models to display unpacking animations. Cleared when unpacking completes (transitioning to `DEPLOYED` state).

- **`DEPLOYED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - A deployed object state. Set automatically by `ParticleUplinkCannonUpdate` and `DeployStyleAIUpdate` when objects complete unpacking. Used by deployment systems to indicate objects are fully deployed and operational. Used by `ParticleUplinkCannonUpdate` to manage deployed state. Cleared when objects begin packing. Mutually exclusive with `PACKING` and `UNPACKING`. Allows models to display deployed state animations. Indicates objects that are fully unpacked and ready for use.

### Garrison and Containment
- **`GARRISONED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is garrisoned. Set automatically by `GarrisonContain` when units enter garrison positions in structures. Cleared when units exit garrison positions. Used by garrison systems to track which units are garrisoned. Used by `GarrisonContain` to find garrison fire points based on structure damage state (pristine, damaged, really damaged). Used by `CaveContain` to clear garrison state when units exit caves. Used by garrison initialization to set up fire point positions for different damage states. Allows models to display garrisoned unit animations and visual states.

- **`ENEMYNEAR`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Enemy is near. Set automatically by `EnemyNearUpdate` when enemies are detected within range. Cleared when no enemies are nearby. Used by structures and units to indicate enemy presence. Used by `EnemyNearUpdate` to track enemy proximity and update visual states accordingly. Allows models to display different animations when enemies are nearby (e.g., defensive postures, alert states).

- **`LOADED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Loaded woot! ... like a transport is loaded. Set automatically by `TransportContain` and `MobNexusContain` when units are loaded into transports or containers. Cleared when units are unloaded. Used by transport systems to indicate units are inside transports. Used by `TransportContain` to track loaded units. Used by `MobNexusContain` to track units in mob nexus containers. Allows models to display loaded state animations (e.g., units visible inside transports).

- **`CARRYING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is carrying something. Set automatically by `W3DModelDraw` when objects are carrying other objects. Used by rendering systems to indicate objects that are visually carrying other objects. Cleared when objects stop carrying. Allows models to display carrying animations and visual effects. Used for objects that physically carry other objects (distinct from `LOADED` which is for transport containers).

### Docking States
- **`DOCKING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - This encloses the whole time you are Entering, Actioning, and Exiting a dock. Set automatically by `DockUpdate` when units enter docking sequences. Used as a master flag that encompasses the entire docking process. Cleared when docking sequences complete. Used by `DockUpdate` to track overall docking state. Used in combination with `DOCKING_BEGINNING`, `DOCKING_ACTIVE`, and `DOCKING_ENDING` to indicate specific phases. Allows models to display general docking animations. Indicates objects are in some phase of the docking process.

- **`DOCKING_BEGINNING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - From Enter to Action. Set automatically by `DockUpdate` when units begin docking (entering dock positions). Used by `DockUpdate` to track the initial docking phase. Transitions to `DOCKING_ACTIVE` when docking action begins. Used by `DockUpdate::isClearToApproach()` to check if docking is in beginning phase. Allows models to display docking entry animations. Cleared when transitioning to active phase or when docking completes.

- **`DOCKING_ACTIVE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - From Action to Exit. Set automatically by `DockUpdate` when units are in the active docking phase (performing dock actions). Used by `DockUpdate` to track the active docking phase. Transitions from `DOCKING_BEGINNING` and transitions to `DOCKING_ENDING`. Allows models to display active docking animations (e.g., loading/unloading, repair actions). Cleared when transitioning to ending phase or when docking completes.

- **`DOCKING_ENDING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Exit all the way to next enter (use only animations that end with this). Set automatically by `DockUpdate` when units are exiting dock positions. Used by `DockUpdate` to track the final docking phase. Transitions from `DOCKING_ACTIVE` when exit begins. Used for animations that properly end the docking sequence. Cleared when docking completes and units leave dock positions. Allows models to display docking exit animations. Only use animations that properly end with this state for smooth transitions.

### Door States (4 doors, 4 states each)
- **`DOOR_1_OPENING`** through **`DOOR_4_OPENING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Door opening state for doors 1-4. Set automatically by `ProductionUpdate`, `BattlePlanUpdate`, `MissileLauncherBuildingUpdate`, `CheckpointUpdate`, `OpenContain`, `SpectreGunshipUpdate`, `DeliverPayloadAIUpdate`, and `BaikonurLaunchPower` when doors begin opening. Used by door control systems to indicate doors are in the process of opening. Transitions to `DOOR_*_WAITING_OPEN` when opening completes. Cleared when doors begin closing or are interrupted. Allows models to display door opening animations. Used for structures with doors that need to open for unit access or weapon deployment (e.g., production buildings, missile launchers, aircraft hangars).

- **`DOOR_1_CLOSING`** through **`DOOR_4_CLOSING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Door closing state for doors 1-4. Set automatically by door control systems when doors begin closing. Used by `ProductionUpdate`, `BattlePlanUpdate`, `MissileLauncherBuildingUpdate`, `CheckpointUpdate`, `OpenContain`, `SpectreGunshipUpdate`, and `DeliverPayloadAIUpdate` to manage door closing. Transitions from `DOOR_*_WAITING_OPEN` or `DOOR_*_WAITING_TO_CLOSE` when closing begins. Cleared when doors finish closing or are interrupted. Allows models to display door closing animations. Used for structures with doors that need to close after unit access or weapon deployment.

- **`DOOR_1_WAITING_OPEN`** through **`DOOR_4_WAITING_OPEN`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Door waiting open state for doors 1-4. Set automatically by door control systems when doors have finished opening and are waiting in the open position. Used by `ProductionUpdate`, `BattlePlanUpdate`, `MissileLauncherBuildingUpdate`, and `CheckpointUpdate` to indicate doors are open and waiting. Transitions from `DOOR_*_OPENING` when opening completes. Transitions to `DOOR_*_WAITING_TO_CLOSE` or `DOOR_*_CLOSING` when closing begins. Allows models to display open door state animations. Used for structures where doors remain open for a period before automatically closing.

- **`DOOR_1_WAITING_TO_CLOSE`** through **`DOOR_4_WAITING_TO_CLOSE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Door waiting to close state for doors 1-4. Set automatically by door control systems when doors are open but scheduled to close soon. Used by `ProductionUpdate`, `BattlePlanUpdate`, `MissileLauncherBuildingUpdate`, and `CheckpointUpdate` to indicate doors are waiting to close. Transitions from `DOOR_*_WAITING_OPEN` when the close timer is initiated. Transitions to `DOOR_*_CLOSING` when closing begins. Allows models to display door state animations while waiting to close. Used for structures where doors have a delay before closing after opening.

### Combat and Weapon States - Weapon Slot A
- **`ATTACKING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Simply set when a unit is fighting -- terrorist moving with a target will flail arms like a psycho. Set automatically by `AIStates` when units are engaged in combat. Cleared when units stop attacking. Used by AI systems to indicate units that are actively fighting. Used by special ability systems (like terrorist attacks) to trigger attack animations. Allows models to display combat animations (e.g., terrorist units flailing arms when moving with a target). Used to indicate general combat state regardless of specific weapon slot activity.

- **`PREATTACK_A`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Use for pre-attack animations (like aiming, pulling out a knife, or detonating explosives). Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot A is in pre-attack state (`WSF_PREATTACK`). Used by weapon systems to indicate the weapon is preparing to fire. Allows models to display pre-attack animations such as aiming, drawing weapons, or preparing explosives. Used by special abilities like hack attacks and internet center attacks to trigger pre-attack sequences. Automatically sets `USING_WEAPON_A` when active.

- **`FIRING_A`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot A is firing. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot A is firing (`WSF_FIRING`). Used by weapon systems to indicate the weapon is actively firing. Used by garrison systems to set firing state on garrisoned units when they fire. Used by `HackInternetAIUpdate` to clear firing state during hack sequences. Used by `MobMemberSlavedUpdate` to clear firing state for mob members. Used by `SlavedUpdate` to manage firing states for slaved objects. Automatically sets `USING_WEAPON_A` when active. Allows models to display firing animations and muzzle flash effects.

- **`BETWEEN_FIRING_SHOTS_A`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot A is between firing shots. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot A is between firing shots (`WSF_BETWEEN_FIRING_SHOTS`). Used by weapon systems to indicate the brief pause between individual shots in a burst. Used by `SlavedUpdate` to clear this state for slaved objects. Used by `MobMemberSlavedUpdate` to clear this state for mob members. Automatically sets `USING_WEAPON_A` when active. Allows models to display brief pause animations between shots.

- **`RELOADING_A`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot A is reloading. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot A is reloading (`WSF_RELOADING`). Used by weapon systems to indicate the weapon is reloading ammunition. Used by `SlavedUpdate` to clear reloading state for slaved objects. Used by `MobMemberSlavedUpdate` to clear reloading state for mob members. Automatically sets `USING_WEAPON_A` when active. Allows models to display reloading animations. Mutually exclusive with `FIRING_A` - weapons cannot fire and reload simultaneously.

- **`USING_WEAPON_A`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - These bits are set if firing, reloading, between shots, or preattack. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` whenever weapon slot A is in any active state (`WSF_FIRING`, `WSF_RELOADING`, `WSF_BETWEEN_FIRING_SHOTS`, or `WSF_PREATTACK`). Used as a convenience flag to check if weapon slot A is in any active state without checking individual states. Used by `AIStates` to exclude units with active weapons from dying state transitions. Used by `SlavedUpdate` and `MobMemberSlavedUpdate` to clear weapon usage states. Allows models to display general weapon usage animations when any weapon activity is occurring.

### Combat and Weapon States - Weapon Slot B
- **`PREATTACK_B`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Use for pre-attack animations (like aiming, pulling out a knife, or detonating explosives). Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot B is in pre-attack state (`WSF_PREATTACK`). Used by weapon systems to indicate weapon slot B is preparing to fire. Allows models to display pre-attack animations for weapon slot B. Automatically sets `USING_WEAPON_B` when active.

- **`FIRING_B`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot B is firing. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot B is firing (`WSF_FIRING`). Used by weapon systems to indicate weapon slot B is actively firing. Used by `SlavedUpdate` to clear firing state for slaved objects. Automatically sets `USING_WEAPON_B` when active. Allows models to display firing animations for weapon slot B.

- **`BETWEEN_FIRING_SHOTS_B`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot B is between firing shots. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot B is between firing shots (`WSF_BETWEEN_FIRING_SHOTS`). Used by weapon systems to indicate the brief pause between individual shots in a burst for weapon slot B. Used by `SlavedUpdate` to clear this state for slaved objects. Automatically sets `USING_WEAPON_B` when active. Allows models to display brief pause animations between shots for weapon slot B.

- **`RELOADING_B`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot B is reloading. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot B is reloading (`WSF_RELOADING`). Used by weapon systems to indicate weapon slot B is reloading ammunition. Used by `SlavedUpdate` to clear reloading state for slaved objects. Automatically sets `USING_WEAPON_B` when active. Allows models to display reloading animations for weapon slot B. Mutually exclusive with `FIRING_B`.

- **`USING_WEAPON_B`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - These bits are set if firing, reloading, between shots, or preattack. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` whenever weapon slot B is in any active state. Used as a convenience flag to check if weapon slot B is in any active state. Used by `AIStates` to exclude units with active weapons from dying state transitions. Allows models to display general weapon usage animations when weapon slot B is active.

### Combat and Weapon States - Weapon Slot C
- **`PREATTACK_C`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Use for pre-attack animations (like aiming, pulling out a knife, or detonating explosives). Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot C is in pre-attack state (`WSF_PREATTACK`). Used by weapon systems to indicate weapon slot C is preparing to fire. Allows models to display pre-attack animations for weapon slot C. Automatically sets `USING_WEAPON_C` when active.

- **`FIRING_C`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot C is firing. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot C is firing (`WSF_FIRING`). Used by weapon systems to indicate weapon slot C is actively firing. Used by `SlavedUpdate` to clear firing state for slaved objects. Automatically sets `USING_WEAPON_C` when active. Allows models to display firing animations for weapon slot C.

- **`BETWEEN_FIRING_SHOTS_C`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot C is between firing shots. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot C is between firing shots (`WSF_BETWEEN_FIRING_SHOTS`). Used by weapon systems to indicate the brief pause between individual shots in a burst for weapon slot C. Used by `SlavedUpdate` to clear this state for slaved objects. Automatically sets `USING_WEAPON_C` when active. Allows models to display brief pause animations between shots for weapon slot C.

- **`RELOADING_C`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon slot C is reloading. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when weapon slot C is reloading (`WSF_RELOADING`). Used by weapon systems to indicate weapon slot C is reloading ammunition. Used by `SlavedUpdate` to clear reloading state for slaved objects. Automatically sets `USING_WEAPON_C` when active. Allows models to display reloading animations for weapon slot C. Mutually exclusive with `FIRING_C`.

- **`USING_WEAPON_C`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - These bits are set if firing, reloading, between shots, or preattack. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` whenever weapon slot C is in any active state. Used as a convenience flag to check if weapon slot C is in any active state. Used by `AIStates` to exclude units with active weapons from dying state transitions. Allows models to display general weapon usage animations when weapon slot C is active.

### Combat and Weapon States - Weapon Slots D-H *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)*
- **`PREATTACK_D`** through **`PREATTACK_H`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Pre-attack state for weapon slots D through H. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when the respective weapon slot is in pre-attack state (`WSF_PREATTACK`). Used by weapon systems to indicate the weapon slot is preparing to fire. Allows models to display pre-attack animations for the respective weapon slot. Automatically sets the corresponding `USING_WEAPON_*` flag when active.

- **`FIRING_D`** through **`FIRING_H`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Firing state for weapon slots D through H. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when the respective weapon slot is firing (`WSF_FIRING`). Used by weapon systems to indicate the weapon slot is actively firing. Used by `AIStates` to exclude units with active weapons from dying state transitions. Automatically sets the corresponding `USING_WEAPON_*` flag when active. Allows models to display firing animations for the respective weapon slot.

- **`BETWEEN_FIRING_SHOTS_D`** through **`BETWEEN_FIRING_SHOTS_H`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Between firing shots state for weapon slots D through H. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when the respective weapon slot is between firing shots (`WSF_BETWEEN_FIRING_SHOTS`). Used by weapon systems to indicate the brief pause between individual shots in a burst. Used by `AIStates` to exclude units with active weapons from dying state transitions. Automatically sets the corresponding `USING_WEAPON_*` flag when active. Allows models to display brief pause animations between shots for the respective weapon slot.

- **`RELOADING_D`** through **`RELOADING_H`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Reloading state for weapon slots D through H. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` when the respective weapon slot is reloading (`WSF_RELOADING`). Used by weapon systems to indicate the weapon slot is reloading ammunition. Used by `AIStates` to exclude units with active weapons from dying state transitions. Automatically sets the corresponding `USING_WEAPON_*` flag when active. Allows models to display reloading animations for the respective weapon slot. Mutually exclusive with the corresponding `FIRING_*` flag.

- **`USING_WEAPON_D`** through **`USING_WEAPON_H`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon usage state for weapon slots D through H. Set automatically by `WeaponSet::getModelConditionForWeaponSlot()` whenever the respective weapon slot is in any active state (firing, reloading, between shots, or preattack). Used as a convenience flag to check if the weapon slot is in any active state without checking individual states. Used by `AIStates` to exclude units with active weapons from dying state transitions. Allows models to display general weapon usage animations when the respective weapon slot is active.

### Turret and Rotation
- **`TURRET_ROTATE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Turret is rotating. Set automatically by `TurretAI` when turrets are actively rotating to face targets. Cleared when turrets stop rotating (target acquired or rotation complete). Used by turret AI systems to indicate turrets are in motion. Used by `TurretAI` to track turret rotation state. Allows models to display turret rotation animations. Used to indicate when turrets are actively turning to face their targets.

### Special Combat States
- **`DYING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is dying. Set automatically by `AIStates` when units enter death sequence. Cleared when units finish dying or are removed. Used by AI systems to transition units to death animations. Used in combination with other flags to exclude units from certain state transitions (units with `DYING` flag are excluded from other combat states). Mutually exclusive with weapon usage and movement states during death sequence. Allows models to display death animations and visual effects.

- **`STUNNED_FLAILING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is stunned and flailing. Set automatically by `Object` when units receive stun damage that causes flailing behavior. Used by `PhysicsUpdate` to transition from flailing to regular stunned state when stun effects wear off. Used to indicate units that are in a more severe stun state with chaotic movement. Allows models to display flailing animations for stunned units. Transitioned to `STUNNED` when the flailing effect ends.

- **`STUNNED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is stunned. Set automatically by `PhysicsUpdate` when stun effects are applied. Used by `PhysicsUpdate` to track stunned state. Cleared when stun effects wear off. Used to indicate units that are temporarily disabled by stun effects. Allows models to display stunned animations. Units with this flag are typically unable to move or attack until the flag is cleared.

- **`PANICKING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Yes, it's spelled with a "k". look it up. Used for units that are in a panicking state. Allows models to display panic animations and visual effects. Used by morale systems to indicate units that are panicking due to combat stress or morale loss.

- **`CONTINUOUS_FIRE_SLOW`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Continuous fire slow. Set automatically by `FiringTracker::coolDown()` when weapons transition from faster firing states to slower firing. Used by `FiringTracker` to track weapon firing intensity - indicates weapons that are firing continuously but at a slower rate. Mutually exclusive with `CONTINUOUS_FIRE_MEAN` and `CONTINUOUS_FIRE_FAST` - only one continuous fire state can be active at a time. Cleared when firing stops completely. Allows models to display slower continuous fire animations. Used to indicate weapons that have been firing for a while and are cooling down.

- **`CONTINUOUS_FIRE_MEAN`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Continuous fire mean. Set automatically by `FiringTracker::heatUp()` when weapons have been firing for a moderate duration. Used by `FiringTracker` to track weapon firing intensity - indicates weapons that are firing continuously at a medium rate. Mutually exclusive with `CONTINUOUS_FIRE_SLOW` and `CONTINUOUS_FIRE_FAST`. Transitions to `CONTINUOUS_FIRE_FAST` if firing continues. Used to indicate weapons that are heating up from continuous fire. Triggers "VoiceRapidFire" audio event when set. Allows models to display medium-rate continuous fire animations.

- **`CONTINUOUS_FIRE_FAST`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Continuous fire fast. Set automatically by `FiringTracker::heatUp()` when weapons have been firing continuously at maximum intensity. Used by `FiringTracker` to track weapon firing intensity - indicates weapons that are firing continuously at a fast rate due to heat buildup. Mutually exclusive with `CONTINUOUS_FIRE_SLOW` and `CONTINUOUS_FIRE_MEAN`. Represents the highest intensity continuous fire state. Used to indicate weapons that are at maximum firing rate due to continuous fire. Allows models to display fast-rate continuous fire animations and increased visual effects.

### Radar and Power
- **`RADAR_EXTENDING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Radar is extending. Set automatically by `RadarUpdate` when radar structures begin extending their radar dishes. Used by `RadarUpdate` to track radar extension animation. Transitions to `RADAR_UPGRADED` when extension completes. Allows models to display radar extension animations. Used to indicate radar structures that are in the process of extending their radar equipment.

- **`RADAR_UPGRADED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Radar is upgraded. Set automatically by `RadarUpdate` when radar extension completes. Used by `RadarUpdate` to indicate radar structures have fully extended and upgraded their radar equipment. Transitions from `RADAR_EXTENDING` when extension completes. Allows models to display upgraded radar visual states. Used to indicate radar structures that have completed their upgrade sequence.

- **`POWER_PLANT_UPGRADED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - To show special control rods on the cold fusion plant. Set automatically by `PowerPlantUpdate` when power plant upgrades complete. Used by `PowerPlantUpdate` to indicate power plants have upgraded control rods. Transitions from `POWER_PLANT_UPGRADING` when upgrade completes. Allows models to display special control rod visual effects on cold fusion power plants. Used to indicate power plants that have upgraded to show special visual effects.

- **`POWER_PLANT_UPGRADING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - While special control rods on the cold fusion plant are extending. Set automatically by `PowerPlantUpdate` when power plant upgrades begin. Used by `PowerPlantUpdate` to track power plant upgrade animation. Transitions to `POWER_PLANT_UPGRADED` when upgrade completes. Cleared when upgrade is cancelled or interrupted. Allows models to display control rod extension animations. Used to indicate power plants that are in the process of upgrading their control rods.

### Capture and Special States
- **`RAISING_FLAG`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is raising flag. Set automatically by `SpecialAbilityUpdate` when units perform flag-raising actions (e.g., capturing buildings). Used by special ability systems to indicate flag-raising activities. Used by `SpecialAbilityUpdate` to clear this state during other special abilities. Allows models to display flag-raising animations. Used for units that are capturing structures or raising flags.

- **`CAPTURED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object has been captured. Set automatically by `TechBuildingBehavior` when structures are captured by enemy forces. Used by capture systems to indicate structures that have been captured. Cleared when structures are recaptured or destroyed. Used by `TechBuildingBehavior` to track capture state. Allows models to display captured structure visual states (e.g., different flag colors, ownership changes).

- **`SOLD`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is being sold. Set automatically by `BuildAssistant` when structures are being sold (construction percent reaches 0% during sale). Used by construction systems to indicate structures that are being sold back. Used during the sell-back process to track sale state. Allows models to display selling animations. Cleared when sale completes or is cancelled.

- **`ARMED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Armed like a mine or bomb is armed (not like a human is armed). Used for objects like mines, bombs, or explosives that are in an armed/active state. Used to distinguish between armed and unarmed explosive objects. Allows models to display armed state animations and visual effects. Used by explosive systems to indicate objects that are ready to detonate.

- **`DISGUISED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is disguised. Set automatically by `StealthUpdate` when units activate disguise abilities. Cleared when disguise is removed or units are detected. Used by stealth systems to track disguised units. Used by `StealthUpdate` to manage disguise state. Allows models to display disguised visual states (e.g., different model appearances, faction disguises). Used for units that can disguise themselves as enemy units.

- **`JAMMED`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Jammed as in missile jammed by ECM. Set automatically by `MissileAIUpdate` when missiles are jammed by electronic countermeasures (ECM). Used by missile systems to indicate missiles that have been jammed. Used by ECM systems to track jammed projectiles. Allows models to display jammed state animations (e.g., missiles spiraling out of control). Used for missiles and projectiles that are affected by jamming systems.

- **`SECOND_LIFE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is in second life state. Set automatically by `BattleBusSlowDeathBehavior` when objects with second life mechanics activate their second life. Used by `UndeadBody` to indicate objects that have been revived after death. Used by second life systems to track objects that have been given a second chance. Allows models to display second life visual states. Used for objects like the Battle Bus that can survive death and continue operating.

### Special Animations
- **`SPECIAL_CHEERING`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - When units do a victory cheer (or player initiated cheer). Special model conditions work as following: Something turns it on... but a timer in the object will turn them off after a given amount of time. Set automatically by `AIGroup` when units perform victory cheers or player-initiated cheer commands. Used by AI and command systems to trigger cheering animations. Automatically cleared by `Object::clearModelConditionFlags()` after a duration (typically 3 seconds as set by `AIGroup`). Used by `Object` to clear the flag after the timer expires. Allows models to display cheering animations and visual effects. Used for units that want to celebrate victories or respond to player cheer commands.

### Preorder and Movement
- **`PREORDER`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is in preorder state. Set automatically by `PreorderCreate` when objects are created for preorder players. Used by `PreorderCreate` to track preorder player objects. Cleared when preorder objects are finalized or converted to normal objects. Allows models to display preorder-specific visual states or animations. Used to indicate objects that were created for players who preordered the game.

- **`CENTER_TO_LEFT`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is moving center to left. Set automatically by `AnimationSteeringUpdate` when units initiate a turn from center position to left. Used by steering animation systems to indicate units are turning left. Transitions to `LEFT_TO_CENTER` when the turn completes or unit straightens. Cleared when units complete the turn or change direction. Allows models to display turning animations for left turns. Used for smooth transition animations during unit steering.

- **`LEFT_TO_CENTER`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is moving left to center. Set automatically by `AnimationSteeringUpdate` when units straighten from a left turn back to center. Used by steering animation systems to indicate units are straightening from a left turn. Transitions from `CENTER_TO_LEFT` when the turn completes. Cleared when units complete the straightening or change direction. Allows models to display straightening animations after left turns. Used for smooth transition animations during unit steering.

- **`CENTER_TO_RIGHT`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is moving center to right. Set automatically by `AnimationSteeringUpdate` when units initiate a turn from center position to right. Used by steering animation systems to indicate units are turning right. Transitions to `RIGHT_TO_CENTER` when the turn completes or unit straightens. Cleared when units complete the turn or change direction. Allows models to display turning animations for right turns. Used for smooth transition animations during unit steering.

- **`RIGHT_TO_CENTER`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Object is moving right to center. Set automatically by `AnimationSteeringUpdate` when units straighten from a right turn back to center. Used by steering animation systems to indicate units are straightening from a right turn. Transitions from `CENTER_TO_RIGHT` when the turn completes. Cleared when units complete the straightening or change direction. Allows models to display straightening animations after right turns. Used for smooth transition animations during unit steering.

### Rider States (8 riders)
- **`RIDER1`** through **`RIDER8`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Rider states for different riders on multi-rider vehicles. Used by `RiderChangeContain` and `WeaponSet` to indicate which rider position is active. Used by `RiderChangeContain` to track rider model condition flags for vehicles like combat bikes that can carry multiple riders. Used by `FlightDeckBehavior` for flight deck door management. Allows models to display different rider positions and animations for multi-rider vehicles. Used for vehicles that can carry multiple riders in different positions (e.g., combat bikes with sidecar riders).

### Weapon Set and Upgrade States
- **`WEAPONSET_VETERAN`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon set veteran. Set automatically by weapon set systems when units achieve veteran status. Used by `WeaponSet` to indicate veteran-level weapon sets. Allows models to display veteran weapon visual states. Used for units that have gained veterancy and upgraded their weapons.

- **`WEAPONSET_ELITE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon set elite. Set automatically by weapon set systems when units achieve elite status. Used by `WeaponSet` to indicate elite-level weapon sets. Allows models to display elite weapon visual states. Used for units that have gained elite status and upgraded their weapons.

- **`WEAPONSET_HERO`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon set hero. Set automatically by weapon set systems when units achieve hero status. Used by `WeaponSet` to indicate hero-level weapon sets. Allows models to display hero weapon visual states. Used for units that have gained hero status and upgraded their weapons.

- **`WEAPONSET_CRATEUPGRADE_ONE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon set crate upgrade one. Set automatically by crate systems when units collect weapon upgrade crates. Used by `WeaponSet` to indicate first-level crate weapon upgrades. Used by `SalvageCrateCollide` for weapon crate upgrades. Allows models to display first-level crate upgrade weapon visual states. Used for units that have collected weapon upgrade crates.

- **`WEAPONSET_CRATEUPGRADE_TWO`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon set crate upgrade two. Set automatically by crate systems when units collect second-level weapon upgrade crates. Used by `WeaponSet` to indicate second-level crate weapon upgrades. Used by `SalvageCrateCollide` for weapon crate upgrades (transitions from `WEAPONSET_CRATEUPGRADE_ONE`). Allows models to display second-level crate upgrade weapon visual states. Used for units that have collected multiple weapon upgrade crates.

- **`WEAPONSET_PLAYER_UPGRADE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Weapon set player upgrade. Set automatically by upgrade systems when player upgrades are applied to units. Used by `WeaponSet` to indicate player-purchased weapon upgrades. Used by `MobMemberSlavedUpdate` to check for weapon upgrade states. Allows models to display player upgrade weapon visual states. Used for units that have received player-purchased weapon upgrades.

- **`WEAPONSET_PLAYER_UPGRADE2`** through **`WEAPONSET_PLAYER_UPGRADE4`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Additional weapon set player upgrade levels 2-4. Set automatically by upgrade systems when multiple player upgrades are applied to units. Used by `WeaponSet` to indicate additional player-purchased weapon upgrade levels. Allows models to display multiple player upgrade weapon visual states. Used for units that have received multiple player-purchased weapon upgrades (GMX Zero Hour only for levels 2-4).

### Armor Set States
- **`ARMORSET_CRATEUPGRADE_ONE`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Armor set crate upgrade one. Set automatically by crate systems when units collect armor upgrade crates. Used by `SalvageCrateCollide` to set armor set flags when armor crates are collected. Used by armor systems to indicate first-level crate armor upgrades. Transitions to `ARMORSET_CRATEUPGRADE_TWO` when second crate is collected. Allows models to display first-level crate upgrade armor visual states. Used for units that have collected armor upgrade crates.

- **`ARMORSET_CRATEUPGRADE_TWO`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Armor set crate upgrade two. Set automatically by crate systems when units collect second-level armor upgrade crates. Used by `SalvageCrateCollide` to transition from `ARMORSET_CRATEUPGRADE_ONE` to `ARMORSET_CRATEUPGRADE_TWO` when second armor crate is collected. Used by armor systems to indicate second-level crate armor upgrades. Allows models to display second-level crate upgrade armor visual states. Used for units that have collected multiple armor upgrade crates.

### User-Definable Flags (5 user flags)
- **`USER_1`** through **`USER_5`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Wildcard flags available for custom use by modders. These flags are not used by the default game systems and are reserved for modder-defined purposes. Can be used with upgrade modules, custom behaviors, or any other custom functionality. Allows modders to create custom visual states and animations that can be triggered by their own systems. These flags can be set and cleared by custom code to indicate any modder-defined states.

### Armor Upgrade Flags (5 armor upgrades)
- **`ARMOR_UPGRADE_1`** through **`ARMOR_UPGRADE_5`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Wildcard flags for armor upgrade visual states. These flags are available for use with upgrade modules or custom armor upgrade systems. Can be used to indicate different levels or types of armor upgrades. Allows modders to create custom armor upgrade visual states and animations. These flags can be set by upgrade systems to display different armor appearances based on upgrade levels or types.

### Weapon Upgrade Flags (5 weapon upgrades)
- **`WEAPON_UPGRADE_1`** through **`WEAPON_UPGRADE_5`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04)* - Wildcard flags for weapon upgrade visual states. These flags are available for use with upgrade modules or custom weapon upgrade systems. Can be used to indicate different levels or types of weapon upgrades. Allows modders to create custom weapon upgrade visual states and animations. These flags can be set by upgrade systems to display different weapon appearances based on upgrade levels or types.

### Component Damage States *(GMX Zero Hour only)*
- **`COMPONENT_ENGINE_DESTROYED`** *(GMX Zero Hour only)* - Component engine has been destroyed. Set automatically by `Drawable::reactToBodyDamageStateChange()` when engine components are destroyed. Used by component damage systems to indicate engine destruction. Used by `Drawable` to clear all component damage flags when resetting damage states. Allows models to display destroyed engine visual states. Used for vehicles with component-based damage systems where engines can be individually destroyed.

- **`COMPONENT_ENGINE_DAMAGED`** *(GMX Zero Hour only)* - Component engine has been damaged. Set automatically by `Drawable::reactToBodyDamageStateChange()` when engine components are damaged. Used by component damage systems to indicate engine damage. Used by `Drawable` to clear all component damage flags when resetting damage states. Allows models to display damaged engine visual states. Used for vehicles with component-based damage systems where engines can be individually damaged.

- **`COMPONENT_TURRET_DESTROYED`** *(GMX Zero Hour only)* - Component turret has been destroyed. Set automatically by `Drawable::reactToBodyDamageStateChange()` when turret components are destroyed. Used by component damage systems to indicate turret destruction. Used by `Drawable` to clear all component damage flags when resetting damage states. Allows models to display destroyed turret visual states. Used for vehicles with component-based damage systems where turrets can be individually destroyed.

- **`COMPONENT_TURRET_DAMAGED`** *(GMX Zero Hour only)* - Component turret has been damaged. Set automatically by `Drawable::reactToBodyDamageStateChange()` when turret components are damaged. Used by component damage systems to indicate turret damage. Used by `Drawable` to clear all component damage flags when resetting damage states. Allows models to display damaged turret visual states. Used for vehicles with component-based damage systems where turrets can be individually damaged.

- **`COMPONENT_WEAPON_A_DESTROYED`** through **`COMPONENT_WEAPON_H_DESTROYED`** *(GMX Zero Hour only)* - Component weapon destroyed state for weapon slots A-H. Set automatically by `Drawable::reactToBodyDamageStateChange()` when weapon components are destroyed. Used by component damage systems to indicate specific weapon slot destruction. Used by `Drawable` to clear all component damage flags when resetting damage states. Allows models to display destroyed weapon visual states for specific weapon slots. Used for vehicles with component-based damage systems where individual weapons can be destroyed.

- **`COMPONENT_WEAPON_A_DAMAGED`** through **`COMPONENT_WEAPON_H_DAMAGED`** *(GMX Zero Hour only)* - Component weapon damaged state for weapon slots A-H. Set automatically by `Drawable::reactToBodyDamageStateChange()` when weapon components are damaged. Used by component damage systems to indicate specific weapon slot damage. Used by `Drawable` to clear all component damage flags when resetting damage states. Allows models to display damaged weapon visual states for specific weapon slots. Used for vehicles with component-based damage systems where individual weapons can be damaged.

- **`COMPONENT_A_DESTROYED`** through **`COMPONENT_H_DESTROYED`** *(GMX Zero Hour only)* - Component destroyed state for generic components A-H. Set automatically by `Drawable::reactToBodyDamageStateChange()` when generic components are destroyed. Used by component damage systems to indicate generic component destruction. Used by `Drawable` to clear all component damage flags when resetting damage states. Allows models to display destroyed component visual states. Used for vehicles with component-based damage systems where generic components can be destroyed.

- **`COMPONENT_A_DAMAGED`** through **`COMPONENT_H_DAMAGED`** *(GMX Zero Hour only)* - Component damaged state for generic components A-H. Set automatically by `Drawable::reactToBodyDamageStateChange()` when generic components are damaged. Used by component damage systems to indicate generic component damage. Used by `Drawable` to clear all component damage flags when resetting damage states. Allows models to display damaged component visual states. Used for vehicles with component-based damage systems where generic components can be damaged.

### Surrender State *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04 - conditional compilation)*
- **`SURRENDER`** *(GMX Generals, GMX Zero Hour, Retail Generals 1.04, Retail Zero Hour 1.04 - conditional compilation)* - When units surrender. Set automatically by surrender systems when units surrender to enemy forces. Used by surrender mechanics to indicate units that have surrendered. This flag is conditionally compiled and may not be available in all builds. Allows models to display surrender animations and visual states. Used for units that can surrender during combat, typically removing them from player control.

## Examples

Model condition flags are set automatically by game systems based on object state, damage, actions, and other conditions. However, they are configured in INI files through `W3DModelDraw` modules to define which models, animations, and visual states should be displayed for each combination of flags.

### INI Configuration in W3DModelDraw Modules

Model condition flags are used in `W3DModelDraw` modules (like `W3DDefaultDraw`) through the `ConditionState` property. Each `ConditionState` block defines a visual state that matches specific model condition flags. The system automatically selects the best matching state based on the object's current model condition flags.

#### Basic Example: Damage States

```ini
Draw = W3DModelDraw ModuleTag_Draw
  ConditionState = NONE
    Model = MyUnit.w3d
    Animation = Idle
    AnimationMode = LOOP
  End

  ConditionState = DAMAGED
    Model = MyUnitDamaged.w3d
    Animation = IdleDamaged
    AnimationMode = LOOP
  End

  ConditionState = REALLY_DAMAGED
    Model = MyUnitReallyDamaged.w3d
    Animation = IdleReallyDamaged
    AnimationMode = LOOP
  End
End
```

In this example:
- When the object has no damage flags set, it uses the `NONE` state with `MyUnit.w3d`
- When the `DAMAGED` flag is set (health drops below first threshold), it switches to `MyUnitDamaged.w3d`
- When the `REALLY_DAMAGED` flag is set (health drops below second threshold), it switches to `MyUnitReallyDamaged.w3d`

The system automatically sets these flags based on health percentage, and `W3DModelDraw::replaceModelConditionState()` calls `findBestInfo()` to select the matching visual state.

#### Advanced Example: Multiple Flags Combined

```ini
Draw = W3DModelDraw ModuleTag_Draw
  ConditionState = NONE
    Model = Tank.w3d
    Animation = Idle
    AnimationMode = LOOP
  End

  ConditionState = DAMAGED
    Model = TankDamaged.w3d
    Animation = IdleDamaged
    AnimationMode = LOOP
  End

  ConditionState = NIGHT
    Model = TankNight.w3d
    Animation = Idle
    AnimationMode = LOOP
  End

  ConditionState = DAMAGED NIGHT
    Model = TankDamagedNight.w3d
    Animation = IdleDamaged
    AnimationMode = LOOP
  End

  ConditionState = FIRING_A
    Model = Tank.w3d
    Animation = Fire
    AnimationMode = ONCE
  End

  ConditionState = FIRING_A DAMAGED
    Model = TankDamaged.w3d
    Animation = FireDamaged
    AnimationMode = ONCE
  End
End
```

This example shows how multiple flags can be combined:
- `NIGHT` alone uses `TankNight.w3d`
- `DAMAGED` alone uses `TankDamaged.w3d`
- `DAMAGED NIGHT` combined uses `TankDamagedNight.w3d` (when both flags are active)
- `FIRING_A` uses the firing animation
- `FIRING_A DAMAGED` uses the firing animation on the damaged model

The matching system (`findBestInfo()` in `W3DModelDrawModuleData`) finds the best matching state by comparing the object's current model condition flags with the flags specified in each `ConditionState`.

#### Example: Using Transition States

```ini
Draw = W3DModelDraw ModuleTag_Draw
  DefaultConditionState = NONE
    Model = Unit.w3d
    Animation = Idle
    AnimationMode = LOOP
  End

  ConditionState = DAMAGED
    Model = UnitDamaged.w3d
    Animation = IdleDamaged
    AnimationMode = LOOP
  End

  TransitionState = NONE DAMAGED
    Model = Unit.w3d
    Animation = TakeDamage
    AnimationMode = ONCE
  End
End
```

Transition states are used for animations that play when transitioning between states:
- When transitioning from `NONE` to `DAMAGED`, the `TakeDamage` animation plays once
- After the transition completes, the system switches to the `DAMAGED` state

#### Example: Hide/Show Subobjects Based on Flags

```ini
Draw = W3DModelDraw ModuleTag_Draw
  ConditionState = NONE
    Model = Tank.w3d
    Animation = Idle
    AnimationMode = LOOP
    HideSubObject = DamagedParts
    ShowSubObject = IntactParts
  End

  ConditionState = DAMAGED
    Model = Tank.w3d
    Animation = IdleDamaged
    AnimationMode = LOOP
    HideSubObject = IntactParts
    ShowSubObject = DamagedParts
  End
End
```

This example shows how the same model can have different subobjects visible/hidden based on flags:
- In `NONE` state, `DamagedParts` are hidden and `IntactParts` are shown
- In `DAMAGED` state, `IntactParts` are hidden and `DamagedParts` are shown

### Runtime Usage

At runtime, the system works as follows:

1. **Flag Setting**: Game systems automatically set model condition flags based on object state (e.g., `Drawable::reactToBodyDamageStateChange()` sets `DAMAGED`, `REALLY_DAMAGED`, etc.)

2. **State Selection**: `W3DModelDraw::replaceModelConditionState()` is called with the current flags, which calls `findBestInfo()` to find the best matching `ConditionState`

3. **State Matching**: The `findBestInfo()` method uses `SparseMatchFinder` to match the object's current flags against all defined states, finding the one that best matches (can match multiple flags)

4. **Visual Update**: Once a state is selected, the system:
   - Loads/switches to the appropriate model (if different)
   - Sets the appropriate animations
   - Hides/shows subobjects as specified
   - Updates other visual properties defined in that state

### Code Reference

The key code locations where this happens:

- **INI Parsing**: `W3DModelDrawModuleData::parseConditionState()` (line 1417 in `W3DModelDraw.cpp`) parses `ConditionState` blocks and stores flags in `ModelConditionInfo::m_conditionsYesVec`

- **State Matching**: `W3DModelDrawModuleData::findBestInfo()` (line 1741) matches current flags to states

- **State Application**: `W3DModelDraw::replaceModelConditionState()` (line 3279) applies the selected state to the visual representation

- **Flag Checking**: `ModelConditionFlags::test()` is used to check if specific flags are set (e.g., `c.test(MODELCONDITION_NIGHT)` checks if the NIGHT flag is active)

## Notes

- Model condition flags are bit flags that can be combined. Objects typically have multiple model condition flags set simultaneously.
- Model condition flags are set automatically by game systems based on object state, damage, actions, and other conditions. They are not directly set in INI files.
- Some model condition flags are mutually exclusive or have specific activation conditions (e.g., `FIRING_A` and `RELOADING_A` cannot be active simultaneously for the same weapon slot).
- Some model condition flags are temporary and automatically cleared after a duration (e.g., `SPECIAL_CHEERING`).
- Model condition flags affect which model animations and visual states are displayed.
- The `USER_1` through `USER_5`, `ARMOR_UPGRADE_1` through `ARMOR_UPGRADE_5`, and `WEAPON_UPGRADE_1` through `WEAPON_UPGRADE_5` flags are available for custom use by modders.
- Component damage states (`COMPONENT_*`) are only available in GMX Zero Hour.
- Surrender state (`SURRENDER`) is conditionally compiled and may not be available in all builds.
- Note: These values are saved in save files, so you MUST NOT REMOVE OR CHANGE existing values!

## Source Files

**Header (Generals):** [ModelState.h](../../Generals/Code/GameEngine/Include/Common/ModelState.h)
- `ModelConditionFlagType` enum definition (line 90)
- `ModelConditionFlags` typedef (line 353)

**Header (Zero Hour):** [ModelState.h](../../GeneralsMD/Code/GameEngine/Include/Common/ModelState.h)
- `ModelConditionFlagType` enum definition (line 90)
- `ModelConditionFlags` typedef (line 353)

**Source (Generals):** [BitFlags.cpp](../../Generals/Code/GameEngine/Source/Common/BitFlags.cpp)
- `ModelConditionFlags::s_bitNameList[]` array definition (line 41)

**Source (Zero Hour):** [BitFlags.cpp](../../GeneralsMD/Code/GameEngine/Source/Common/BitFlags.cpp)
- `ModelConditionFlags::s_bitNameList[]` array definition (line 41)

## Changes History

- In GMX Zero Hour, 93 new ModelConditionFlagType flags were added including 4 aircraft landing states, 4 movement direction states, 8 rider states, 2 special combat states, 3 special states, 2 armor set states, 15 upgrade flags, 3 weapon set upgrade states, 25 weapon slot states, and 36 component damage states.

## Document Log

- 16/12/2025 — AI — Initial document created.

## Status

- Documentation Status: AI-generated, 0/2 reviews

## Reviewers

- *No reviewers yet*

