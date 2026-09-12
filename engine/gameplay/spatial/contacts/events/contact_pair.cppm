module;

#include <compare>
#include <cstdint>
#include <string_view>
#include <utility>

export module engine.gameplay.spatial.contacts.events.contact_pair;
export import engine.ecs.core.entity;
export import engine.events.schema.message_registry;

export namespace engine::gameplay::spatial::contacts
{
	inline constexpr bool EntityLess(const ecs::Entity left, const ecs::Entity right) noexcept
	{
		return left.index < right.index ||
			(left.index == right.index && left.generation < right.generation);
	}

	struct ContactPair final
	{
		ecs::Entity first{};
		ecs::Entity second{};

		friend constexpr bool operator==(const ContactPair &, const ContactPair &) noexcept = default;
	};

	inline constexpr ContactPair CanonicalPair(ecs::Entity left, ecs::Entity right) noexcept
	{
		return EntityLess(right, left) ? ContactPair{right, left} : ContactPair{left, right};
	}

	inline constexpr bool IsCanonical(const ContactPair value) noexcept
	{
		return value.first.IsValid() && value.second.IsValid() &&
			EntityLess(value.first, value.second);
	}

	struct ContactPairLess final
	{
		constexpr bool operator()(const ContactPair left, const ContactPair right) const noexcept
		{
			if (left.first != right.first)
				return EntityLess(left.first, right.first);
			return EntityLess(left.second, right.second);
		}
	};
}

export namespace engine::events
{
	template<> struct MessageTraits<engine::gameplay::spatial::contacts::ContactPair>
	{
		static constexpr std::string_view StableName = "engine.gameplay.spatial.contacts.pair";
		static constexpr std::uint32_t Version = 1;
		static constexpr MessageKind Kind = MessageKind::Fact;
		static constexpr RecordPolicy Recording = RecordPolicy::Transient;
	};
}
