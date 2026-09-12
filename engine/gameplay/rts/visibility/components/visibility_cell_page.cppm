module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

export module engine.gameplay.rts.visibility.components.visibility_cell_page;
export import engine.ecs.core.component_registry;
export import engine.gameplay.rts.visibility.components.visibility_observer;

export namespace engine::gameplay::rts::visibility
{
inline constexpr std::size_t VisibilityPageLanes = 256;

// A page represents contiguous map cells. The lane index derives from
// firstCell + lane; this component is topology metadata and is read-only in
// the visibility system.
struct VisibilityCellIndex final
{
	std::uint32_t firstCell{};
	std::uint16_t count{};
	std::uint16_t reserved{};
};

struct VisibilityExploredMaskPage final
{
	std::array<ObserverMask, VisibilityPageLanes> values{};
};

struct VisibilityVisibleMaskPage final
{
	std::array<ObserverMask, VisibilityPageLanes> values{};
};

constexpr std::size_t CellPagePayloadBytes() noexcept
{
	return sizeof(VisibilityCellIndex) + sizeof(VisibilityExploredMaskPage) + sizeof(VisibilityVisibleMaskPage);
}
} // namespace engine::gameplay::rts::visibility

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::visibility::VisibilityCellIndex>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.visibility.cell_index";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::visibility::VisibilityExploredMaskPage>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.visibility.explored_page";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};

template<> struct ComponentTraits<engine::gameplay::rts::visibility::VisibilityVisibleMaskPage>
{
	static constexpr std::string_view StableName = "engine.gameplay.rts.visibility.visible_page";
	static constexpr std::uint32_t Version = 1;
	static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
} // namespace ecs
