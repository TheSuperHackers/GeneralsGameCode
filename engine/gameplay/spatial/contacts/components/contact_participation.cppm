module;

#include <cstdint>
#include <limits>
#include <string_view>

export module engine.gameplay.spatial.contacts.components.contact_participation;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::spatial::contacts
{
	struct ContactParticipation final
	{
		bool enabled{true};
		std::uint32_t categoryMask{1};
		std::uint32_t collisionMask{(std::numeric_limits<std::uint32_t>::max)()};

		friend constexpr bool operator==(const ContactParticipation &,
			const ContactParticipation &) noexcept = default;
	};
}

export namespace ecs
{
	template<> struct ComponentTraits<engine::gameplay::spatial::contacts::ContactParticipation>
	{
		static constexpr std::string_view StableName = "engine.gameplay.spatial.contacts.participation";
		static constexpr std::uint32_t Version = 1;
		static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
	};
}
