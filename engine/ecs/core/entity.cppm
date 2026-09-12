module;

#include <cstdint>
#include <limits>

export module engine.ecs.core.entity;

export namespace ecs
{

using EntityIndex = std::uint32_t;
using EntityGeneration = std::uint32_t;

struct Entity
{
	static constexpr EntityIndex InvalidIndex = std::numeric_limits<EntityIndex>::max();
	static constexpr EntityGeneration InvalidGeneration = 0;

	EntityIndex index{InvalidIndex};
	EntityGeneration generation{InvalidGeneration};

	constexpr Entity() noexcept = default;
	constexpr Entity(EntityIndex entityIndex, EntityGeneration entityGeneration) noexcept :
		index(entityIndex),
		generation(entityGeneration)
	{
	}

	static constexpr Entity Null() noexcept
	{
		return Entity{};
	}

	constexpr bool IsValid() const noexcept
	{
		return index != InvalidIndex && generation != InvalidGeneration;
	}

	friend constexpr bool operator==(Entity left, Entity right) noexcept
	{
		return left.index == right.index && left.generation == right.generation;
	}

	friend constexpr bool operator!=(Entity left, Entity right) noexcept
	{
		return !(left == right);
	}
};

using EntityId = Entity;

} // namespace ecs
