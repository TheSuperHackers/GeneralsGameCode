module;

#include <cstdint>
#include <string_view>

export module engine.gameplay.spatial.contacts.components.contact_geometry;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::spatial::contacts
{
	using MicroCells = std::uint32_t;

	enum class ContactShape : std::uint8_t
	{
		Circle,
		AxisAlignedBox
	};

	// For Circle, extentX is the radius and extentY is zero.  For an
	// AxisAlignedBox, extents are the x/y half-extents.  Zero Hour height,
	// isSmall and heading remain content-boundary omissions; they are not hot
	// authoritative fields in this 2D capability.
	struct ContactGeometry final
	{
		ContactShape shape{ContactShape::Circle};
		MicroCells extentX{};
		MicroCells extentY{};

		friend constexpr bool operator==(const ContactGeometry &, const ContactGeometry &) noexcept = default;
	};
}

export namespace ecs
{
	template<> struct ComponentTraits<engine::gameplay::spatial::contacts::ContactGeometry>
	{
		static constexpr std::string_view StableName = "engine.gameplay.spatial.contacts.geometry";
		static constexpr std::uint32_t Version = 1;
		static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
	};
}
