# KindOf

Status: AI-generated, 0/2 reviews

## Overview

KindOf is a system used to categorize and filter objects in the game. Each KindOf flag represents a specific category or property that an object can have. Objects can have multiple KindOf flags at the same time, which allows flexible classification and filtering.

For example, a tank might have `KindOf = VEHICLE CAN_ATTACK SELECTABLE`, meaning it's categorized as a vehicle, can attack, and can be selected by the player.

KindOf flags are used throughout the game for many purposes:
- **Weapon targeting**: Weapons can prefer certain types of targets (e.g., anti-infantry weapons prefer infantry)
- **AI behavior**: AI systems use KindOf to filter and prioritize targets
- **Special abilities**: Special powers and abilities can affect specific object types
- **Gameplay mechanics**: Systems like garrisoning, pathfinding, and victory conditions use KindOf flags

**For modders**: In INI files, you simply list the KindOf flags you want an object to have. The game automatically combines them into a collection of flags. You don't need to worry about the technical details - just list the flags that describe your object.

Available in: *(v1.04)*

## Table of Contents

- [Overview](#overview)
- [Usage](#usage)
- [Enum Value Lists](#enum-value-lists)
  - [Basic Object Types](#basic-object-types)
  - [Prison/Surrender System](#prison-surrender-system)
  - [Building and Structure Types](#building-and-structure-types)
  - [Unit Role Types](#unit-role-types)
  - [Naval Categories](#naval-categories)
  - [Projectile and Special Objects](#projectile-and-special-objects)
  - [Faction Structure Types (FS)](#faction-structure-types-fs)
  - [Building and Structure Behaviors](#building-and-structure-behaviors)
  - [Visibility and Interaction](#visibility-and-interaction)
  - [Movement and Pathfinding](#movement-and-pathfinding)
  - [Garrison and Containment](#garrison-and-containment)
  - [Special Abilities](#special-abilities)
  - [Supply and Resources](#supply-and-resources)
  - [Gameplay and Scoring](#gameplay-and-scoring)
  - [Special Object States](#special-object-states)
  - [Special Behaviors](#special-behaviors)
  - [Water and Terrain](#water-and-terrain)
  - [Custom/Extra Flags](#customextra-flags)
- [Examples](#examples)
- [Notes](#notes)
- [Source Files](#source-files)
- [Changes History](#changes-history)
- [Document Log](#document-log)
- [Status](#status)
- [Reviewers](#reviewers)

## Usage

**Limitations**:
- Multiple KindOf flags can be set on a single object by listing them in the `KindOf` property.
- KindOf flags are defined in the object's `KindOf` property in INI files. The property accepts a space-separated list of KindOf flag names.
- Some KindOf flags may have dependencies or interactions with other flags (e.g., `STICK_TO_TERRAIN_SLOPE` requires `IMMOBILE` to be set).
- KindOf flags are case-sensitive and must match exactly as defined in the value list below.

**Conditions**:
- KindOf flags are set in the object's `KindOf` property in INI files (e.g., `KindOf = INFANTRY VEHICLE`).
- Multiple KindOf flags can be combined by listing them space-separated in the `KindOf` property.
- Systems throughout the game use KindOf flags to filter objects (e.g., weapons can target specific KindOf types, AI systems can filter by KindOf, special abilities can affect specific KindOf types).
- Game systems can check for multiple flags at once, allowing complex filtering rules.

**Dependencies**:
- KindOf flags are defined at the object template level and apply to all instances of that object.
- Various game systems reference KindOf flags for filtering and targeting (e.g., [Weapon](../Weapon.md) targeting, AI behavior, special powers).

## Enum Value Lists

### KindOf Flag Values

The following KindOf flags are available for use in INI files. Each flag represents a specific category or property that objects can have. You can combine multiple flags by listing them in the `KindOf` property.

**Source (Generals):** [KindOf.h](../../../Generals/Code/GameEngine/Include/Common/KindOf.h) - KindOf flag definitions (lines 41-254) *(if retail Generals codebase exists)*

**Source (Zero Hour):** [KindOf.h](../../Code/GameEngine/Include/Common/KindOf.h) - KindOf flag definitions (lines 41-254)

**Parsing (Generals):** [KindOf.cpp](../../../Generals/Code/GameEngine/Source/Common/System/KindOf.cpp) - Flag name mapping (line 35) - Maps flag names to their values for INI file parsing. *(if retail Generals codebase exists)*

**Parsing (Zero Hour):** [KindOf.cpp](../../Code/GameEngine/Source/Common/System/KindOf.cpp) - Flag name mapping (line 35) - Maps flag names to their values for INI file parsing.

The following KindOf flags are available and can be combined:

### Basic Object Types
- **`OBSTACLE`** *(v1.04)* - Objects that block land-based pathfinders. Pathfinding systems use this flag to identify objects that units cannot move through or must navigate around. Objects with this flag are treated as obstacles by the pathfinding system, affecting unit movement and AI path planning.

- **`SELECTABLE`** *(v1.04)* - Actually means MOUSE-INTERACTABLE (doesn't mean you can select it!). This flag determines whether an object can be interacted with via mouse clicks. Objects with this flag can be clicked, hovered over, and interacted with through the UI. The default selectability state is determined by this flag, though it can be overridden per-object instance. Note: This flag does NOT guarantee the object is selectable - it only enables mouse interaction.

- **`IMMOBILE`** *(v1.04)* - Fixed in location. Objects with this flag cannot move and are permanently positioned at their spawn location. Used by structures and static objects. Objects with this flag do not receive AI update interfaces (AIUpdate) unless they also have other specific requirements. Immobile objects with weapons have special range checking behavior - they must be within weapon range to attack targets, as they cannot move to close distance. This flag is also used by weapon systems to determine if an object can adjust its position to attack targets.

- **`CAN_ATTACK`** *(v1.04)* - Indicates the object can attack other objects. Objects with this flag can use weapons to attack targets. This flag is used by weapon targeting systems to identify objects that are capable of offensive actions. Typically combined with other flags like `INFANTRY`, `VEHICLE`, `STRUCTURE`, or `AIRCRAFT` to define what type of object can attack.

- **`STICK_TO_TERRAIN_SLOPE`** *(v1.04)* - Object should be stuck at ground level and aligned to terrain slope. When set, the object automatically aligns itself to the terrain slope at its location. **Requires that `IMMOBILE` flag is also set** - this flag only works on immobile objects. Objects with this flag are positioned at ground level and rotate to match the terrain angle.

- **`CAN_CAST_REFLECTIONS`** *(v1.04)* - Object can cast reflections in water. When set, the object will create reflection effects when positioned over water surfaces. Used for visual rendering effects to create realistic water reflections. Objects with this flag or the `FLAG_DRAWS_IN_MIRROR` flag are rendered with mirror effects.

- **`SHRUBBERY`** *(v1.04)* - Tree, bush, or other vegetation objects. Objects with this flag are treated as vegetation by the game systems. Shrubbery can only be damaged by flame weapons (`DEATH_BURNED` damage type) - all other damage types return 0.0 damage. Shrubbery objects are filtered out of map loading when trees are disabled in graphics settings. Used for decorative terrain objects that can be destroyed by fire.

- **`STRUCTURE`** *(v1.04)* - Building structure of some sort (buildable or not). Objects with this flag are identified as structures by game systems. Used extensively for weapon targeting (weapons can prefer structures as targets), AI behavior (AI can target structures for attacks), script actions (scripts can filter for structures), and victory conditions. Structures are typically combined with `IMMOBILE` to indicate they cannot move. Structures with sniper damage types check if they have garrisoned units before allowing damage. Structures are used in multiplayer victory calculations and scoring systems.

- **`INFANTRY`** *(v1.04)* - Infantry unit (soldier, etc.). Objects with this flag are identified as infantry units. Used extensively for weapon targeting (weapons can prefer infantry as targets), AI behavior (AI can target infantry units), garrison systems (infantry can garrison buildings unless `NO_GARRISON` is set), weapon accuracy (infantry-specific inaccuracy modifiers), and script actions. Infantry units can typically garrison buildings and are affected by anti-infantry weapons. Used for airborne infantry targeting calculations.

- **`VEHICLE`** *(v1.04)* - Vehicle unit (tank, jeep, plane, helicopter, etc.). Objects with this flag are identified as vehicles by game systems. Used extensively for weapon targeting (weapons can prefer vehicles as targets), AI behavior (AI can target vehicles), airborne vehicle targeting, and script actions. Vehicles are typically mobile units that can move around the map.

- **`AIRCRAFT`** *(v1.04)* - Aircraft unit (plane, helicopter, etc.) that is predominantly a flyer. Objects with this flag are identified as aircraft units. **Important: Hovercraft are NOT aircraft** - they are vehicles. Aircraft units are affected by aircraft pathfinding (they path around tall objects with `AIRCRAFT_PATH_AROUND` flag). Used for weapon targeting, AI behavior, and categorization.

- **`HUGE_VEHICLE`** *(v1.04)* - Unit that is technically a vehicle, but WAY larger than normal (e.g., Overlord). Objects with this flag are identified as exceptionally large vehicles. Used for special handling of oversized vehicle units. Typically combined with `VEHICLE` to indicate a large vehicle.

- **`DOZER`** *(v1.04)* - Construction unit (dozer). Objects with this flag are identified as construction units that can build structures. Used extensively by construction systems, AI behavior (AI uses dozers for building), production systems (production checks if builder is a dozer), pathfinding (dozers can move through obstacles that other units cannot), and script actions. Dozers are excluded from certain AI calculations and can enter unmanned vehicles (unless `REJECT_UNMANNED` is set). Dozers are used by the build assistant system and can be commanded to construct buildings. AI systems filter dozers separately from other units for construction tasks.

- **`HARVESTER`** *(v1.04)* - Resource gathering unit (harvester). Objects with this flag are identified as resource gathering units. Used extensively by AI behavior (AI uses harvesters for resource gathering), resource systems (harvesters collect supplies), pathfinding (harvesters can use special pathfinding rules), and script actions. Harvesters are typically supply trucks or specialized resource gathering vehicles. AI systems filter harvesters separately for resource management tasks. Harvesters are excluded from certain combat calculations.

- **`COMMANDCENTER`** *(v1.04)* - Command center building. Objects with this flag are identified as command centers. Used by game systems to identify the primary command structure. Script actions can check for command centers to determine player base status. Command centers are typically important structures that serve as the primary base building.

### Prison/Surrender System *(v1.04 - conditional compilation)*
- **`PRISON`** *(v1.04 - conditional compilation)* - A prison detention center kind of thing
- **`COLLECTS_PRISON_BOUNTY`** *(v1.04 - conditional compilation)* - When prisoners are delivered to these, the player gets money
- **`POW_TRUCK`** *(v1.04 - conditional compilation)* - A pow truck can pick up and return prisoners
- **`CAN_SURRENDER`** *(v1.04 - conditional compilation)* - Object that can surrender

### Building and Structure Types
- **`LINEBUILD`** *(v1.04)* - Wall-type structure that is built in a line. Objects with this flag are identified as structures that are constructed in a continuous line, like walls. Used by construction systems to handle line-based building mechanics. Line-build structures are typically walls or barriers that can be placed in a continuous line.

- **`BRIDGE`** *(v1.04)* - Bridge structure (special structure). Objects with this flag are identified as bridges. Used extensively by game systems for special bridge handling: weapon systems check for bridges to prevent targeting (bridges cannot be force-attacked), wave guide systems check bridge height for water effects, pathfinding systems use bridges for unit movement, and weapon systems check bridges for collision behavior. Bridges are special structures that connect terrain across water or gaps.

- **`LANDMARK_BRIDGE`** *(v1.04)* - Landmark bridge (special bridge that isn't resizable). Objects with this flag are identified as landmark bridges that have fixed sizes and cannot be resized. Used for special bridges that are part of the map terrain rather than buildable structures. Landmark bridges are typically pre-placed on maps and cannot be modified.

- **`BRIDGE_TOWER`** *(v1.04)* - Bridge tower that can be targeted for bridge destruction. Objects with this flag are identified as bridge towers that support bridges. Used by weapon systems (bridge towers cannot be force-attacked, similar to bridges), wave guide systems to check bridge structure, and worker AI systems to identify bridge repair targets. Destroying bridge towers can cause bridges to collapse. Bridge towers are typically used to support bridge structures.

- **`REBUILD_HOLE`** *(v1.04)* - GLA rebuild hole. Objects with this flag are identified as rebuild holes used by the GLA faction. Rebuild holes are special structures that allow destroyed buildings to be rebuilt. Used for GLA-specific rebuilding mechanics.

- **`DEFENSIVE_WALL`** *(v1.04)* - Wall can't be driven through, even if crusher, so pathfinder must path around it. Objects with this flag are identified as defensive walls that cannot be destroyed by crusher units. Used by pathfinding systems to ensure units path around these walls rather than attempting to drive through them. Defensive walls are typically indestructible barriers that block unit movement.

- **`PORTABLE_STRUCTURE`** *(v1.04)* - Flag to identify building-like subobjects an Overlord is allowed to Contain. Objects with this flag are identified as structures that can be contained by units like the Overlord. Used by contain systems to determine which structures can be carried by transport units. Portable structures are typically small buildings or sub-objects that can be moved.

- **`WALK_ON_TOP_OF_WALL`** *(v1.04)* - Units can walk on top of a wall made of these kind of objects. Objects with this flag allow units to walk on top of them. Used by pathfinding systems to enable units to traverse walls. Used by game logic systems to check if units can walk on wall objects. Used for wall structures that units can climb and walk across.

### Unit Role Types
- **`SALVAGER`** *(v1.04)* - Units that can create and use Salvage Crates. Objects with this flag can pick up salvage crates and receive benefits from them. Used by the salvage crate collision system to filter which units can interact with salvage crates. Only units with this flag can pick up salvage crates when they collide with them. Used extensively by AI systems and script actions to identify salvage-capable units. Salvage crates search for nearby units with this flag when determining valid targets.

- **`WEAPON_SALVAGER`** *(v1.04)* - Subset of salvager that can get weapon upgrades from salvage. Objects with this flag are salvagers that specifically receive weapon upgrades when picking up salvage crates. This is a specialized flag that combines with `SALVAGER` to indicate the type of upgrade received from salvage. Used by salvage systems to determine upgrade type distribution.

- **`ARMOR_SALVAGER`** *(v1.04, Zero Hour only)* - Subset of salvager that can get armor upgrades from salvage. Objects with this flag are salvagers that specifically receive armor upgrades when picking up salvage crates. This is a specialized flag that combines with `SALVAGER` to indicate the type of upgrade received from salvage. Used by salvage systems to determine upgrade type distribution.

- **`TRANSPORT`** *(v1.04)* - A true transport unit (has TransportContain module). Objects with this flag are identified as transport units that can carry other units. Used by script actions to identify transport units and by transport systems to determine if a unit can carry passengers. Transport units must have a TransportContain module to function properly. Script actions check for this flag when determining if a unit can transport other units.

- **`HERO`** *(v1.04)* - Hero units (any of the single-instance infantry: JarmenKell, BlackLotus, ColonelBurton). Objects with this flag are identified as hero units. Used by weapon systems for special hero-specific behavior (e.g., `DAMAGE_KILLPILOT` damage type works differently on heroes with primary weapons). Hero units are typically unique, single-instance units with special abilities. Used for filtering and special gameplay mechanics.

### Naval Categories
- **`BOAT`** *(v1.04)* - Naval vessel unit. Objects with this flag are identified as boats or naval units. Used by disguise systems to prevent units from disguising as boats (boats cannot be disguised as). Used by action manager systems to filter boat targets. Used for categorization of naval units that operate on water surfaces.

### Projectile and Special Objects
- **`PROJECTILE`** *(v1.04)* - Projectile object (instead of being a ground or air unit, this object is special). Objects with this flag are identified as projectiles launched by weapons. Used by weapon systems for collision detection (weapons can collide with projectiles), projectile targeting, and weapon targeting. All projectiles should have this flag. Projectiles are typically temporary objects that exist only during weapon firing.

- **`SMALL_MISSILE`** *(v1.04)* - Small missile object: **ONLY USED FOR ANTI-MISSILE TARGETING PURPOSES!** Keep using `PROJECTILE` for general projectiles! Objects with this flag are identified as small missiles that can be targeted by anti-missile systems. Used by anti-missile targeting systems and countermeasure systems. All missiles are also projectiles, so this flag should be combined with `PROJECTILE`. This flag is specifically for anti-missile weapon targeting - do not use it for general projectile identification.

- **`BALLISTIC_MISSILE`** *(v1.04)* - Large ballistic missiles that are specifically large enough to be targeted by base defenses. Objects with this flag are identified as large ballistic missiles. Used by base defense targeting systems. Ballistic missiles are typically large strategic weapons that can be intercepted by base defenses. Used by weapon systems to determine if a weapon can target ballistic missiles.

- **`CRATE`** *(v1.04)* - Bonus crate object. Objects with this flag are identified as bonus crates that can be picked up by units. Used by crate collision systems to identify pickup objects. Bonus crates typically provide temporary bonuses or upgrades when collected by units.

- **`MINE`** *(v1.04)* - Landmine object (possibly also extends to Col. Burton timed charges). Objects with this flag are identified as landmines. Used by weapon systems for special mine targeting (mines can be targeted by disarm weapons and anti-mine weapons) and weapon collision detection. Mines can be disarmed by weapons with `DAMAGE_DISARM` damage type. Mines have special rendering and selection behavior. Used for explosive traps that activate when units step on them.

- **`DEMOTRAP`** *(v1.04, Zero Hour only)* - Demo trap object added strictly only for disarming purposes. Objects with this flag are identified as demo traps. Used by weapon systems for special trap targeting (traps can be targeted by disarm weapons) and weapon collision detection. **Important: They don't act like mines which have rendering and selection implications!** Demo traps are used for disarming gameplay mechanics but don't have the same visual/selection behavior as mines. Used for Col. Burton timed charges and similar objects.

- **`BOOBY_TRAP`** *(v1.04, Zero Hour only)* - Sticky bomb that gets set off by 5 random and unrelated events. Objects with this flag are identified as booby traps. Used by weapon systems for special trap targeting (booby traps can be targeted by disarm weapons) and weapon collision detection. Booby traps are activated by specific game events rather than direct unit contact. Used for explosive traps with complex activation triggers.

- **`HULK`** *(v1.04)* - Hulk object types for special handling via scripts or other systems. Objects with this flag are identified as hulks (destroyed unit remains). Used for special handling of destroyed unit remains. Scripts can use this flag to identify hulk objects for special processing or cleanup. Hulks are typically the remains of destroyed vehicles or structures.

- **`PARACHUTE`** *(v1.04)* - Parachute object. Objects with this flag are identified as parachutes. Used for weapons that can target parachuting units. Parachutes are typically temporary objects used during unit airdrops. Used for airborne unit delivery systems.

- **`PROP`** *(v1.04, Zero Hour only)* - Prop object (visual only, doesn't interact with other objects). Objects with this flag are identified as decorative props. Props are visual-only objects that don't interact with other game objects (e.g., rocks, street signs, inert fire hydrants). Props are filtered out of certain game systems since they don't affect gameplay. Used for decorative map elements that don't need game logic interaction.

- **`OPTIMIZED_TREE`** *(v1.04, Zero Hour only)* - Optimized, client-side only tree. Objects with this flag are identified as optimized trees that exist only on the client side. Used for performance optimization - these trees are rendered but don't have full game object logic. Optimized trees are filtered out of map loading when trees are disabled in graphics settings. Used for decorative vegetation that doesn't need full game object functionality.

### Faction Structure Types (FS)
- **`FS_POWER`** *(v1.04)* - Faction structure power building. Objects with this flag are identified as power-generating structures. Used by `ThingTemplate` to generate unit type strings (e.g., "UNITTYPE:FS_POWER"). Typically combined with `STRUCTURE` to indicate power plant buildings. Used for categorization of structures that generate power for the faction. Power buildings are essential for powering other structures that require electricity.

- **`FS_FACTORY`** *(v1.04)* - Faction structure factory building. Objects with this flag are identified as factory structures for unit production. Used by `ThingTemplate` to generate unit type strings. Typically combined with `STRUCTURE` to indicate factory buildings. Used for categorization of structures that produce units. Factory buildings include war factories, barracks, and other production facilities.

- **`FS_BASE_DEFENSE`** *(v1.04)* - Faction structure base defense. Objects with this flag are identified as defensive structures for base protection. Used by `ThingTemplate` to generate unit type strings (e.g., "UNITTYPE:FS_BASE_DEFENSE"). Typically combined with `STRUCTURE` to indicate defensive buildings. Used for categorization of structures designed for base defense, such as guard towers, bunkers, and defensive emplacements.

- **`FS_TECHNOLOGY`** *(v1.04)* - Faction structure technology building. Objects with this flag are identified as technology research structures. Used by `ThingTemplate` to generate unit type strings. Typically combined with `STRUCTURE` to indicate technology buildings. Used for categorization of structures that provide technology research and upgrades. Technology buildings unlock new units, abilities, and upgrades for the faction.

- **`FS_SUPPLY_DROPZONE`** *(v1.04, Zero Hour only)* - Supply dropzone structure. Objects with this flag are identified as supply dropzone structures where supplies can be airdropped. Used for categorization of structures that serve as supply delivery points. Supply dropzones allow players to receive supply deliveries via airdrop, providing resources without requiring ground transport.

- **`FS_SUPERWEAPON`** *(v1.04, Zero Hour only)* - Superweapon structure. Objects with this flag are identified as superweapon structures like nuclear missile silos, particle uplink cannons, and scud storms. Used for categorization of structures that house powerful superweapons. Superweapon structures provide devastating area-of-effect attacks but typically have long cooldown periods and high resource costs.

- **`FS_BLACK_MARKET`** *(v1.04, Zero Hour only)* - Black market structure. Objects with this flag are identified as black market structures. Used for categorization of structures that provide black market services. Black market structures typically offer unique units, upgrades, or abilities that are not available through standard production facilities.

- **`FS_SUPPLY_CENTER`** *(v1.04, Zero Hour only)* - Supply center structure. Objects with this flag are identified as supply center structures that generate or manage supplies. Used by `ThingTemplate` to generate unit type strings (e.g., "UNITTYPE:SUPPLY_CENTER"). Typically combined with `STRUCTURE` to indicate supply center buildings. Used for categorization of structures that provide supply generation or management capabilities.

- **`FS_STRATEGY_CENTER`** *(v1.04, Zero Hour only)* - Strategy center structure. Objects with this flag are identified as strategy center structures. Used for categorization of structures that provide strategic capabilities. Strategy centers typically offer strategic upgrades, abilities, or command functions that affect overall faction strategy.

- **`FS_FAKE`** *(v1.04, Zero Hour only)* - Fake structure. Objects with this flag are identified as fake structures that appear as real buildings but are actually decoys. Used by `Drawable` to determine terrain decal rendering - fake structures show shadows for allies/neutrals but no terrain decal for enemies. Used by `ThingTemplate` to generate unit type strings (e.g., "UNITTYPE:FS_FAKE"). Typically combined with `STRUCTURE` to indicate fake building structures. Used for categorization of decoy structures that appear as real buildings to deceive enemies.

- **`FS_INTERNET_CENTER`** *(v1.04, Zero Hour only)* - Internet center structure. Objects with this flag are identified as internet center structures that provide hacking and cyber warfare capabilities. Used by `ActiveBody` to check if units inside internet centers should resume hacking when recovered from subdual. Used by script actions to identify internet center structures. Used for categorization of structures that provide hacking abilities and cyber warfare capabilities. Units garrisoned in internet centers can perform hacking attacks.

- **`FS_ADVANCED_TECH`** *(v1.04, Zero Hour only)* - Advanced technology structure. Objects with this flag are identified as advanced technology buildings that represent each faction's advanced tech tree structure (strategy center, propaganda center, and palace). Used for categorization of the highest-tier technology structures for each faction. Advanced tech structures typically provide the most powerful upgrades and abilities available to a faction.

- **`FS_BARRACKS`** *(v1.04, Zero Hour only)* - Barracks structure. Objects with this flag are identified as barracks structures for infantry production. Used for categorization of structures that produce infantry units. Barracks structures are production facilities specifically designed for training and deploying infantry units.

- **`FS_WARFACTORY`** *(v1.04, Zero Hour only)* - War factory or arms dealer structure. Objects with this flag are identified as war factory structures for vehicle production. Used for categorization of structures that produce vehicles and armored units. War factory structures are production facilities specifically designed for manufacturing and deploying vehicles and combat units.

- **`FS_AIRFIELD`** *(v1.04, Zero Hour only)* - Airfield structure. Objects with this flag are identified as airfield structures that serve as landing and takeoff points for aircraft. Used by `JetAIUpdate` and `ChinookAIUpdate` to identify valid airfield structures for aircraft operations. Used by `Weapon` to prevent projectiles from colliding with airfields when targeting aircraft parked on them. Used by `AIPathfind` to check for airfield obstacles during pathfinding. Used for categorization of structures that function as airfields for aircraft operations. Aircraft can land, take off, and be parked at airfield structures.

### Building and Structure Behaviors
- **`TECH_BUILDING`** *(v1.04)* - Neutral tech building (Oil derrick, Hospital, Radio Station, Refinery). Objects with this flag are identified as neutral tech buildings that can be captured. Used by game systems to identify capturable neutral structures. Tech buildings are typically neutral structures that provide benefits when captured by players.

- **`TECH_BASE_DEFENSE`** *(v1.04, Zero Hour only)* - Tech Building that acts as base defense when captured. Objects with this flag are tech buildings that function as base defenses after being captured. Used to identify tech buildings that provide defensive capabilities when captured. Combines with `TECH_BUILDING` to indicate defensive tech structures.

- **`POWERED`** *(v1.04)* - This object gets the Underpowered disabled condition when its owning player has power consumption exceed supply. Objects with this flag are affected by power shortages. When the owning player's power consumption exceeds power supply, objects with this flag receive the `DISABLED_UNDERPOWERED` condition, which disables their functionality. Used by power systems to determine which structures are affected by power shortages. Used for buildings that require power to function.

- **`CASH_GENERATOR`** *(v1.04)* - Used to check if the unit generates cash. Checked by cash hackers and other systems. Objects with this flag are identified as structures that generate income. Used by cash hacking systems to identify valid targets for cash generation abilities. Used by AI systems to identify income-generating structures (excluding them from certain calculations). Used by script actions to identify cash-generating buildings. Used by academy stats to track cash generation sources.

- **`MONEY_HACKER`** *(v1.04, Zero Hour only)* - Unit that generates money from air. Needed for things that directly power them up. Objects with this flag are identified as units that generate money passively. Used by script actions to identify money-generating units. Used by academy stats to track money-generating units. Used by open contain systems to identify money-generating riders. Used for units that generate income without requiring a structure.

- **`REPAIR_PAD`** *(v1.04)* - Is a repair pad object that can repair other machines. Objects with this flag are identified as repair pads that can repair vehicles and structures. Used by action manager systems to identify valid repair destinations. Units can be ordered to move to repair pads for automatic repair. Repair pads automatically repair nearby damaged units.

- **`HEAL_PAD`** *(v1.04)* - Is a heal pad object that can heal flesh and bone units. Objects with this flag are identified as heal pads that can heal infantry units. Used by auto-find healing update systems to identify valid healing destinations. Used by command systems to identify heal pad targets. Units can be ordered to move to heal pads for automatic healing. Heal pads automatically heal nearby damaged infantry units.

- **`CONSERVATIVE_BUILDING`** *(v1.04, Zero Hour only)* - Conservative structures aren't considered part of your base for sneak attack boundary calculations. Objects with this flag are excluded from base boundary calculations for sneak attack detection. Used by AI systems to determine base boundaries for sneak attack mechanics. Conservative buildings don't count as part of the base perimeter, allowing for more flexible base defense calculations.

### Visibility and Interaction
- **`ALWAYS_VISIBLE`** *(v1.04)* - Is never obscured by fog of war or shroud. Mostly for UI feedback objects. Objects with this flag are always visible to all players, regardless of fog of war or shroud state. Used for UI feedback objects that need to remain visible for gameplay purposes. Objects with this flag bypass normal visibility checks and are rendered even when they should be hidden.

- **`ALWAYS_SELECTABLE`** *(v1.04)* - Is never unselectable (even if effectively dead). Mostly for UI feedback objects. Objects with this flag can always be selected, even when they would normally be unselectable (e.g., when effectively dead). Used by selection systems to override normal selectability restrictions. Used for UI feedback objects that need to remain selectable. Selection checks verify this flag to allow selection of effectively dead objects. Used by AI group systems to allow non-AI units with this flag.

- **`NO_SELECT`** *(v1.04, Zero Hour only)* - Can't select it but you can mouse over it to see its health (drones!). Objects with this flag cannot be selected by the player, but can still be hovered over to view their health status. Used by selection systems to prevent selection while allowing mouse interaction. Typically used for drones and other units that should be visible but not directly controllable. Selection systems check this flag and exclude objects with it from selection lists.

- **`CLICK_THROUGH`** *(v1.04)* - Objects with this will never be picked by mouse interactions! Objects with this flag are completely ignored by mouse interaction systems. These objects cannot be clicked, selected, or interacted with through the mouse. Used for objects that should be completely transparent to mouse interactions. Mouse interaction systems check this flag and skip objects with it.

- **`UNATTACKABLE`** *(v1.04)* - You cannot target this thing, it probably doesn't really exist. Objects with this flag cannot be targeted by weapons or attacks. Used by weapon systems to prevent targeting of special objects that don't represent real game entities. Weapon attack checks verify this flag and return `ATTACKRESULT_NOT_POSSIBLE` if the target has this flag. Used for objects that have "life" for lifetime updates and vision but aren't real game objects that can be attacked.

- **`CANNOT_RETALIATE`** *(v1.04, Zero Hour only)* - Unit will not retaliate if asked. Objects with this flag will not automatically retaliate when attacked. Used by AI and combat systems to prevent automatic retaliation behavior. Units with this flag must be manually commanded to attack enemies.

- **`REVEAL_TO_ALL`** *(v1.04)* - This object reveals shroud for all players. Objects with this flag reveal the fog of war/shroud for all players, not just the owner. Used by visibility systems to create shared visibility objects. Used for objects that should provide shared intelligence to all players.

- **`REVEALS_ENEMY_PATHS`** *(v1.04, Zero Hour only)* - Like the listening outpost... when selected, any enemy drawable will show paths when moused over. Objects with this flag reveal enemy unit paths when selected. When the player selects an object with this flag, enemy units will display their movement paths when moused over. Used for intelligence gathering objects that provide tactical information.

- **`CAN_SEE_THROUGH_STRUCTURE`** *(v1.04)* - Structure does not block line of sight. Objects with this flag do not block line of sight for weapons or vision systems. Used by pathfinding and weapon systems to allow units to see through these structures. Used for transparent or open structures that don't obstruct vision.

- **`DISGUISER`** *(v1.04)* - This object has the ability to disguise. Objects with this flag can disguise themselves as other units. Used by weapon systems for special disguise behavior - force attacks can target disguised units, and disguised units that are stealthed and undetected can be attacked if they're disguised as enemies. Used by stealth systems to determine if an object can be disguised. Used for units like bomb trucks that can disguise as enemy vehicles.

- **`IGNORED_IN_GUI`** *(v1.04)* - Object that is a member of a mob (i.e. GLAInfantryAngryMob). Objects with this flag are ignored by GUI systems. Used for objects that are part of a mob or group that should not be individually displayed in the UI. Used for mob member units that are controlled by a mob nexus object.

- **`IGNORES_SELECT_ALL`** *(v1.04)* - Too late to figure out intelligently if something should respond to a Select All command. Objects with this flag do not respond to "Select All" commands. Used by selection systems to exclude specific units from group selection. Used for units that should not be selected as part of a group.

- **`SHOW_PORTRAIT_WHEN_CONTROLLED`** *(v1.04)* - Only shows portraits when controlled. Objects with this flag only display their portrait in the UI when they are being directly controlled by the player. Used by UI systems to conditionally display unit portraits. Used for units that should only show portraits under specific conditions.

### Movement and Pathfinding
- **`NO_COLLIDE`** *(v1.04)* - Object never collides with other objects or is collided with. Objects with this flag are completely ignored by collision detection systems. Used by AI update systems to skip collision avoidance logic - units with this flag do not need to tell other units to get out of the way. Used for objects that should pass through other objects without physical interaction. Used for projectiles, effects, or other objects that should not participate in collision detection.

- **`AIRCRAFT_PATH_AROUND`** *(v1.04)* - Structure is tall enough that aircraft must path around it. Objects with this flag are identified as tall obstacles that aircraft cannot fly over. Used by aircraft pathfinding systems to create paths that navigate around tall buildings and structures. Used by `AIPathfind` to identify obstacles that aircraft must avoid during pathfinding. Aircraft pathfinding algorithms check for this flag to determine if a structure blocks aircraft movement. Used for tall buildings, towers, and other structures that extend high enough to interfere with aircraft flight paths.

- **`LOW_OVERLAPPABLE`** *(v1.04)* - When objects overlap with this object, they overlap at a low height rather than using the object's geometry height. Objects with this flag are treated as flat or low-profile objects for overlap calculations. Used by `Drawable` systems to determine overlap height - when objects overlap with objects having this flag, they use `FLATTENED_OBJECT_HEIGHT` instead of the object's actual geometry height. Used for flattened objects, crushed vehicles, infantry, or other low-profile objects that should allow other objects to overlap at ground level.

- **`WALK_ON_TOP_OF_WALL`** *(v1.04)* - Units can walk on top of walls made of these objects. Objects with this flag allow units to traverse and walk across their top surface. Used by pathfinding systems to enable units to walk on wall structures. Used by game logic systems to check if units can walk on wall objects. Used for wall structures that units can climb and traverse, allowing infantry to move across walls.

- **`CLIFF_JUMPER`** *(v1.04, Zero Hour only)* - Unit cannot climb cliffs but can jump off of them. Objects with this flag are identified as units that can jump from cliffs but cannot climb up them. Used by transport contain systems for special cliff jumping behavior with hero units. Used by AI group systems to exclude cliff jumpers from certain group behaviors. Used by disguise systems to prevent units from disguising as cliff jumpers. Used by neutron blast behavior for special handling. Used for units like Colonel Burton that can jump from elevated positions but cannot climb cliffs.

- **`CAN_BE_REPULSED`** *(v1.04)* - Object runs away from repulsor objects when damaged. Objects with this flag are affected by repulsor mechanics - when damaged, they become repulsors themselves and cause other repulsable objects to flee. Used by `ActiveBody` to set the `OBJECT_STATUS_REPULSOR` status when damaged objects with this flag are hit. Used by AI systems to check for nearby repulsors and move units away from them. Used by `AIStates` to interrupt movement and pathfinding when repulsors are detected nearby. Used for civilian units and other objects that should flee when damaged or when near damaged objects.

### Garrison and Containment
- **`NO_GARRISON`** *(v1.04)* - Unit may not garrison buildings, even if infantry bit is set. Objects with this flag are prevented from garrisoning buildings, regardless of whether they have the `INFANTRY` flag. Used by garrison systems to explicitly exclude units from garrisoning behavior. Garrison checks first verify that the unit does not have this flag before allowing garrisoning. Script actions check for this flag when determining if infantry can garrison. Used to prevent specific infantry units from garrisoning buildings.

- **`STEALTH_GARRISON`** *(v1.04)* - Enemy teams can't tell that unit is in building, and if they garrison that building, the stealth unit will eject. Objects with this flag are identified as stealth garrison units. When garrisoned, enemy teams cannot detect that the unit is inside the building. If an enemy team attempts to garrison the same building, the stealth garrisoned unit will be automatically ejected. Used by garrison systems for special stealth behavior. Used by open contain systems to check for stealth garrison behavior. Used by mob nexus systems to expose stealthed units when needed.

- **`GARRISONABLE_UNTIL_DESTROYED`** *(v1.04)* - Object is capable of garrisoning troops until completely destroyed. Objects with this flag (typically buildings) can continue to accept garrisoned units even when severely damaged. Normally, buildings in the `BODY_REALLYDAMAGED` state cannot accept new garrisoned units, but buildings with this flag can. Used by garrison systems to override the normal damage state restrictions on garrisoning. Allows buildings to continue functioning as garrisons even when heavily damaged.

- **`REJECT_UNMANNED`** *(v1.04, Zero Hour only)* - Unit cannot enter an unmanned vehicle. Objects with this flag are prevented from entering unmanned vehicles. Used by transport and contain systems to prevent specific units from entering vehicles that don't have a driver. Some units (like drones) can enter unmanned vehicles, but units with this flag cannot. Used to prevent specific unit types from using unmanned vehicles.

### Special Abilities
- **`CAN_RAPPEL`** *(v1.04)* - Unit can rappel down from aircraft or elevated positions. Objects with this flag are identified as units that can perform rappelling actions. Used by `ChinookAIUpdate` to identify units that can rappel from transport helicopters during combat drops. Used by `AIStates` to check if units can enter rappelling state. Used by control bar command systems to enable rappelling commands. When units with this flag are in transport helicopters, they can be deployed via rappelling instead of landing. Used for special forces infantry units that can rappel from helicopters.

- **`PARACHUTABLE`** *(v1.04)* - Object can be transported and deployed via parachute. Objects with this flag can be loaded into parachute containers and deployed via airdrop. Used by `ParachuteContain` to determine which units can be transported by parachutes (infantry are always parachutable regardless of this flag). Used for non-infantry units that should be deployable via parachute, such as vehicles or equipment that can be airdropped. Note: Infantry units are automatically parachutable and do not need this flag.

- **`ATTACK_NEEDS_LINE_OF_SIGHT`** *(v1.04)* - Unit requires clear line of sight to attack targets. Objects with this flag must have unobstructed line of sight to their targets before they can attack. Used by `AIStates` to check line of sight before allowing attacks. Used by weapon systems to enforce line of sight requirements for units with this flag. Units with this flag cannot attack targets that are blocked by terrain, structures, or other obstacles. Used for units that require direct visual contact with targets to attack.

- **`FORCEATTACKABLE`** *(v1.04)* - Object can always be force-attacked, even if not selectable. Objects with this flag can be targeted by force-attack commands even when they cannot be selected normally. Used by command translation systems to allow force-attacking of non-selectable objects. Used by selection systems to prevent selection of objects with this flag that are not selectable (objects must be either selectable or force-attackable, not just force-attackable). Used by weapon systems to allow targeting of objects that would normally be untargetable. Used for objects like cargo planes that should be attackable but not selectable.

- **`DONT_AUTO_CRUSH_INFANTRY`** *(v1.04)* - AI-controlled units with this flag do not automatically attempt to crush infantry. Objects with this flag are prevented from automatically crushing infantry units when controlled by AI. Used by `AIStates` to check if units should attempt to crush infantry - AI units without this flag will automatically try to crush infantry when possible. Used for units that should not engage in crushing behavior, such as transport vehicles or support units. Allows fine control over which units participate in infantry crushing mechanics.

- **`SPAWNS_ARE_THE_WEAPONS`** *(v1.04)* - Spawned units (slaves) are treated as this object's weapons rather than separate entities. Objects with this flag evaluate their spawned units as weapons for targeting and attack purposes. Used by `WeaponSet` to handle range checking for immobile objects with spawn slaves (similar to how immobile objects work). Used by `Object` to disable spawn slaves when the object is disabled or when handling special states. Used by `AIStates` to pass the closest spawn member to weapon evaluation functions when attacking. Used by command systems to handle attack commands for objects where spawn members function as weapons. Used by stealth and EMP systems for special handling. Used for objects like Stinger Sites where the spawned soldiers function as the actual weapons of the structure.

### Supply and Resources
- **`SUPPLY_SOURCE`** *(v1.04)* - Object provides supplies that can be harvested. Objects with this flag are identified as sources of supplies that harvesters can collect. Used by `DockUpdate` to identify supply sources that dozers and harvesters can dock with for resource collection. Used by `BuildAssistant` to check proximity restrictions when building near supply sources. Used for supply piles, oil derricks, and other objects that generate harvestable resources.

- **`SUPPLY_SOURCE_ON_PREVIEW`** *(v1.04)* - Supply source that should be displayed on the map preview screen. Objects with this flag are identified as supply sources that should be visible on the map preview/minimap. Used by `MapUtil` to populate supply position lists for map preview display. Used for supply sources that players should be able to see on the map preview before starting the game. Typically combined with `SUPPLY_SOURCE` to indicate both functionality and preview visibility.

- **`CANNOT_BUILD_NEAR_SUPPLIES`** *(v1.04)* - Structure cannot be built too close to supply sources. Objects with this flag are prevented from being constructed within a certain distance of objects with the `SUPPLY_SOURCE` flag. Used by `BuildAssistant` to enforce minimum distance requirements from supply sources (controlled by `TheGlobalData->m_SupplyBuildBorder`). Used for structures like supply centers that should maintain spacing from other supply sources. Prevents players from building supply-related structures too close together.

### Gameplay and Scoring
- **`MP_COUNT_FOR_VICTORY`** *(v1.04)* - Structure counts toward victory conditions in multiplayer games. Objects with this flag are tracked for victory conditions - if a player loses all structures with this flag, they lose the game. Used by `Object` to trigger EVA announcements when structures with this flag are destroyed. Used by `PropagandaTowerBehavior` and scoring systems to identify structures that affect victory conditions. Used for critical structures like command centers and production facilities that determine player survival in multiplayer matches.

- **`SCORE`** *(v1.04)* - Object counts for multiplayer scoring and short-game calculations. Objects with this flag contribute to player scores in multiplayer games for both creation and destruction. Used by `PropagandaTowerBehavior` to identify objects that receive propaganda bonuses. Used by scoring systems to track points earned from building and destroying objects with this flag. Used by `W3DScene` for scoring calculations. Used for buildings and structures that should contribute to overall player score in multiplayer matches.

- **`SCORE_CREATE`** *(v1.04)* - Object only contributes to multiplayer score when created. Objects with this flag award points to the player when they are built, but do not award points when destroyed. Used by `PropagandaTowerBehavior` to identify objects that receive propaganda bonuses. Used by scoring systems to track points earned from creating objects with this flag. Used for structures where construction is rewarded but destruction is not scored separately.

- **`SCORE_DESTROY`** *(v1.04)* - Object only contributes to multiplayer score when destroyed. Objects with this flag award points to the player who destroys them, but do not award points when created. Used by `PropagandaTowerBehavior` to identify objects that receive propaganda bonuses. Used by scoring systems to track points earned from destroying objects with this flag. Used for structures where destruction is rewarded but construction is not scored separately.

### Special Object States
- **`CAPTURABLE`** *(v1.04)* - Is "capturable" even if not an enemy (should generally be used only for structures, eg, Tech buildings). Objects with this flag can be captured by players, even if they are not currently enemies. Used by object systems to determine if an object can be captured. Typically used for neutral tech buildings that can be captured by any player. Allows structures to be capturable regardless of current diplomatic status.

- **`IMMUNE_TO_CAPTURE`** *(v1.04)* - Under no circumstances can this building ever be captured. Objects with this flag cannot be captured by any means. Used by capture systems to prevent capture of specific buildings. Overrides any capture attempts, ensuring the building remains with its original owner.

- **`CLEARED_BY_BUILD`** *(v1.04)* - Is auto-cleared from the map when built over via construction. Objects with this flag are automatically removed from the map when a building is constructed on top of them. Used by game logic systems during construction to clear objects that would interfere with building placement. Used for objects that should be removed when players build structures in their location.

- **`CLEANUP_HAZARD`** *(v1.04)* - Radiation and bio-poison are samples of area conditions that can be cleaned up (or avoided). Objects with this flag are identified as environmental hazards that can be cleaned up or avoided. Used by cleanup systems to identify hazards that units can remove or bypass. Used for area effects like radiation and bio-poison that affect gameplay but can be mitigated.

- **`BLAST_CRATER`** *(v1.04, Zero Hour only)* - Deeply gouges out the terrain under object footprint. Objects with this flag create permanent terrain deformation when placed. Used by script actions to identify blast craters that modify terrain permanently. Used for explosion effects that leave permanent marks on the terrain.

- **`INERT`** *(v1.04)* - This object shouldn't be considered for any sort of interaction with any player. Objects with this flag are ignored by player interaction systems. Used for objects that should not participate in any player-related gameplay mechanics. These objects exist in the game world but don't interact with players.

- **`DRAWABLE_ONLY`** *(v1.04)* - Template is used only to create drawables (not Objects). Objects with this flag are templates that only create visual representations, not actual game objects. Used for visual-only templates that don't need game logic. These templates create drawable entities for rendering but don't have object functionality.

- **`PRELOAD`** *(v1.04)* - All model data will be preloaded even if not on map. Objects with this flag have their model data preloaded at game start, even if they're not present on the current map. Used for performance optimization - preloading prevents loading delays when these objects are created. Used for commonly used objects that need instant availability.

### Special Behaviors
- **`MOB_NEXUS`** *(v1.04)* - Object that coordinates the members of a mob (i.e. GLAInfantryAngryMob). Objects with this flag are identified as mob nexus objects that coordinate multiple mob member units. Used by mob contain systems to manage groups of units that act as a single entity. Mob nexus objects control the behavior of their member units, which typically have the `IGNORED_IN_GUI` flag.

- **`AUTO_RALLYPOINT`** *(v1.04)* - When immobile-structure-object is selected, left clicking on ground will set new rally point without requiring command button. Objects with this flag automatically set rally points when selected and the player left-clicks on the ground. Used for structures that need quick rally point placement. Eliminates the need for a separate command button to set rally points.

- **`PRODUCED_AT_HELIPAD`** *(v1.04)* - Hacky fix for Comanche. Objects with this flag are identified as units that are produced at helipads. Used for special handling of units that spawn at helipad locations rather than standard production facilities.

- **`DRONE`** *(v1.04)* - Object drone type. Used for filtering them out of battle plan bonuses, making un-snipable, and whatever else may come up. Objects with this flag are identified as drone units. Used extensively by game systems: physics update systems check for drones to handle unmanned vehicle behavior, object systems check for drones to allow unmanned vehicle status, body systems check for drones for special death handling, AI dock systems check for drones for docking behavior, and selection systems may use drones for special selection behavior. Drones are typically autonomous units that can operate without a driver.

- **`NO_HEAL_ICON`** *(v1.04)* - Do not ever display healing icons on these objects. Objects with this flag never display healing icons in the UI, even when being healed. Used by UI systems to suppress healing indicator display. Used for objects that shouldn't show healing feedback to the player.

- **`EMP_HARDENED`** *(v1.04, Zero Hour only)* - Like a delivery plane (B52, B3, CargoPlane, etc.) or a SpectreGunship, which sort-of IS the weapon. Objects with this flag are identified as units that are resistant to EMP effects or function as both unit and weapon. Used for special units that have unique weapon/unit relationships. Used for units where the unit itself functions as the weapon delivery system.

- **`IGNORE_DOCKING_BONES`** *(v1.04, Zero Hour only)* - Structure will not look up docking bones. Patch 1.03 hack. Objects with this flag skip docking bone lookup during structure placement. Used for structures that don't use standard docking bone systems. This is a workaround for specific structures that had docking issues in patch 1.03.

- **`SPAWNS_ARE_THE_WEAPONS`** *(v1.04)* - Evaluate the spawn slaves as this object's weapons. Objects with this flag treat their spawned units as weapons rather than separate entities. Used by weapon systems for special weapon behavior - immobile objects with this flag or objects with spawn slaves evaluate spawn members as weapons for targeting and attack purposes. Used for objects like Stinger Sites where the spawn members function as the actual weapons. When ordered to attack, these objects pass the closest spawn member to weapon evaluation functions.

### Water and Terrain
- **`WAVEGUIDE`** *(v1.04)* - Water wave propagation object. Objects with this flag are identified as water wave objects that create and propagate wave effects. Used by `WaveGuideUpdate` to identify and process water wave objects, ignoring them during wave collision detection. Used by `DamDie` to exclude wave guide objects from certain death processing. Used for objects that create water wave effects, such as tsunami generators or wave propagation systems.

- **`WAVE_EFFECT`** *(v1.04)* - Wave effect point marker. Objects with this flag mark locations where wave effects should be applied or displayed. Used for marking specific points on the map where wave effects should occur. Used for visual effects and wave interaction points that trigger wave-related gameplay mechanics.

### Custom/Extra Flags
- **`EXTRA1`** through **`EXTRA16`** *(v1.04)* - Custom KindOf types available for modder-defined object categories

## Examples

### Basic Infantry Unit
```ini
Object = MyInfantry
  KindOf = INFANTRY SELECTABLE CAN_ATTACK
End
```

### Vehicle
```ini
Object = MyTank
  KindOf = VEHICLE CAN_ATTACK SELECTABLE
End
```

### Structure with Special Behaviors
```ini
Object = MyBuilding
  KindOf = STRUCTURE IMMOBILE MP_COUNT_FOR_VICTORY POWERED
End
```

### Aircraft
```ini
Object = MyHelicopter
  KindOf = AIRCRAFT SELECTABLE CAN_ATTACK
End
```

## Notes

### Usage
- Multiple KindOf flags can be combined on a single object. Objects typically have multiple KindOf flags set to describe their various properties.
- Some KindOf flags have dependencies. For example, `STICK_TO_TERRAIN_SLOPE` requires `IMMOBILE` to be set.
- KindOf flags are used throughout the game for filtering, targeting, and AI behavior. Systems like [Weapon](../Weapon.md) targeting and AI systems use KindOf flags to identify object types.
- The `EXTRA1` through `EXTRA16` flags are available for custom categorization by modders.
- Prison/surrender system flags (`PRISON`, `COLLECTS_PRISON_BOUNTY`, `POW_TRUCK`, `CAN_SURRENDER`) are conditionally compiled and may not be available in all builds.
- KindOf flags are case-sensitive and must match exactly as defined in the value list above.

## Source Files

**Header (Generals):** [KindOf.h](../../../Generals/Code/GameEngine/Include/Common/KindOf.h) *(if retail Generals codebase exists)*
- KindOf flag definitions (lines 41-254): Defines all available KindOf flags like `KINDOF_INFANTRY`, `KINDOF_VEHICLE`, etc.
- Type definitions (line 256): Defines how multiple flags are stored together

**Header (Zero Hour):** [KindOf.h](../../Code/GameEngine/Include/Common/KindOf.h)
- KindOf flag definitions (lines 41-254): Defines all available KindOf flags like `KINDOF_INFANTRY`, `KINDOF_VEHICLE`, etc.
- Type definitions (line 256): Defines how multiple flags are stored together

**Source (Generals):** [KindOf.cpp](../../../Generals/Code/GameEngine/Source/Common/System/KindOf.cpp) *(if retail Generals codebase exists)*
- Flag name mapping (line 35): Maps flag names to their values for INI file parsing (e.g., `"INFANTRY"`, `"VEHICLE"`)
- Initialization functions: Sets up predefined flag combinations like faction structure flags

**Source (Zero Hour):** [KindOf.cpp](../../Code/GameEngine/Source/Common/System/KindOf.cpp)
- Flag name mapping (line 35): Maps flag names to their values for INI file parsing (e.g., `"INFANTRY"`, `"VEHICLE"`)
- Initialization functions: Sets up predefined flag combinations like faction structure flags

## Changes History

- In Retail Zero Hour, 25 new KindOf flags were added including 1 movement kindof, 11 faction structure kindofs, and 13 gameplay mechanics kindofs while `AIRFIELD` was replaced by `FS_AIRFIELD`.

## Document Log

- 16/12/2025 — AI — Initial document created.


## Status

- Documentation Status: AI-generated, 0/2 reviews

## Reviewers

- *No reviewers yet*

