module;

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.visibility.components.visibility_lease_page;
export import engine.ecs.core.component_registry;
export import engine.gameplay.rts.visibility.components.visibility_cell_page;
export import engine.gameplay.rts.visibility.components.visibility_observer;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;

export namespace engine::gameplay::rts::visibility
{
struct VisibilityLeasePage final
{
	std::uint32_t pageIndex{};
	std::array<ParticipantHandle, VisibilityPageLanes> owners{};
	std::array<VisibilityRegion, VisibilityPageLanes> regions{};
	std::array<std::uint64_t, VisibilityPageLanes> expiresAt{};
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
		owners[slot] = {};
		regions[slot] = {};
		expiresAt[slot] = 0;
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
template<> struct ComponentTraits<engine::gameplay::rts::visibility::VisibilityLeasePage>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.visibility.lease_page";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
} // namespace ecs
