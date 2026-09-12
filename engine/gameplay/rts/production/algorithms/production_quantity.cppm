module;

#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>

export module engine.gameplay.rts.production.algorithms.production_quantity;
export import engine.gameplay.rts.production.components.production_quantity;

export namespace engine::gameplay::rts::production
{
inline std::int32_t Remaining(const ProductionQuantity &quantity) noexcept
{
	// The signed subtraction is the debug-only precondition check. The value
	// operation below remains defined for arbitrary signed snapshots in Release.
	assert(static_cast<std::int64_t>(quantity.total) -
			static_cast<std::int64_t>(quantity.completed) >=
			(std::numeric_limits<std::int32_t>::min)() &&
		static_cast<std::int64_t>(quantity.total) -
			static_cast<std::int64_t>(quantity.completed) <=
			(std::numeric_limits<std::int32_t>::max)());
	return std::bit_cast<std::int32_t>(
		static_cast<std::uint32_t>(quantity.total) -
		static_cast<std::uint32_t>(quantity.completed));
}

inline void CompleteOne(ProductionQuantity &quantity) noexcept
{
	assert(static_cast<std::int64_t>(quantity.completed) + 1 >=
			static_cast<std::int64_t>((std::numeric_limits<std::int32_t>::min)()) &&
		static_cast<std::int64_t>(quantity.completed) + 1 <=
			static_cast<std::int64_t>((std::numeric_limits<std::int32_t>::max)()));
	quantity.completed = std::bit_cast<std::int32_t>(
		static_cast<std::uint32_t>(quantity.completed) + std::uint32_t{1});
}

inline void SetQuantity(ProductionQuantity &quantity,
	const std::int32_t total,
	const std::int32_t completed = 0) noexcept
{
	quantity.total = total;
	quantity.completed = completed;
}
}
