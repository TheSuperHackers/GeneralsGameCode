module;
#include <cstdint>
#include <optional>
#include <string_view>
export module games.generalszh.gameplay.production.runtime.production_runtime;
export import engine.gameplay.rts.production.systems.build_system;
export import engine.gameplay.rts.production.quantity.production_quantity;
export import engine.gameplay.rts.production.queue.production_queue;
export import games.generalszh.gameplay.production.entry.production_entry_metadata;
export import games.generalszh.gameplay.upgrades.state.upgrade_status;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.combat.systems.weapon_system;
export import engine.gameplay.navigation.systems.movement_system;
export import games.generalszh.gameplay.combat.acquisition.acquisition_policy;
export import engine.gameplay.rts.orders.systems.engagement_system;
export import engine.gameplay.combat.systems.healing_system;
export import engine.gameplay.combat.systems.periodic_damage_system;
export import engine.gameplay.rts.harvesting.components.harvest_state;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export import engine.gameplay.combat.regeneration.components.regeneration;
export import engine.gameplay.progression.components.progression_state;
export import engine.gameplay.combat.lifetime.timed_life_system;
export import engine.gameplay.containment.definitions.transport_definition;
export import games.generalszh.gameplay.capture.components.capture_state;
export import games.generalszh.gameplay.capture.definitions.capture_definition;
// Compatibility imports for existing consumers; declarations live by responsibility.
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.production.definitions.build_definition;
