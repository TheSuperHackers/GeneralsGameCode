module;

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>

export module engine.gameplay.rts.visibility.components.visibility_history_page;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export import engine.gameplay.rts.visibility.components.visibility_observer;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;
export import engine.gameplay.rts.visibility.components.visibility_cell_page;

export namespace engine::gameplay::rts::visibility
{
struct VisibilityHistoryPage final
{
	std::uint32_t pageIndex{};
	std::array<ecs::Entity, VisibilityPageLanes> sources{};
	std::array<ParticipantHandle, VisibilityPageLanes> owners{};
	std::array<VisibilityRegion, VisibilityPageLanes> regions{};
	std::array<VisibilityDefinitionId, VisibilityPageLanes> definitions{};
	std::array<std::uint64_t, VisibilityPageLanes> graceTicks{};
	std::array<std::uint8_t, VisibilityPageLanes> flags{};
	std::array<std::uint64_t, VisibilityPageLanes / 64> occupied{};

	bool Occupied(const std::size_t slot) const noexcept
	{
		assert(slot < VisibilityPageLanes);
		return (occupied[slot / 64] & (std::uint64_t{1} << (slot % 64))) != 0;
	}

	void SetOccupied(const std::size_t slot) noexcept
	{
		assert(slot < VisibilityPageLanes);
		occupied[slot / 64] |= std::uint64_t{1} << (slot % 64);
	}

	void Clear(const std::size_t slot) noexcept
	{
		assert(slot < VisibilityPageLanes);
		occupied[slot / 64] &= ~(std::uint64_t{1} << (slot % 64));
		sources[slot] = {};
		owners[slot] = {};
		regions[slot] = {};
		definitions[slot] = 0;
		graceTicks[slot] = 0;
		flags[slot] = 0;
	}

	std::size_t FindFree() const noexcept
	{
		for (std::size_t word = 0; word != occupied.size(); ++word)
		{
			const auto freeBits = ~occupied[word];
			if (freeBits != 0) return word * 64 + std::countr_zero(freeBits);
		}
		return VisibilityPageLanes;
	}
};
} // namespace engine::gameplay::rts::visibility

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::visibility::VisibilityHistoryPage>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.visibility.history_page";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
} // namespace ecs
