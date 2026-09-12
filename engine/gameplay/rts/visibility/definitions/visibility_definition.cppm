module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>

export module engine.gameplay.rts.visibility.definitions.visibility_definition;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.time.simulation_time;
export import engine.gameplay.rts.visibility.components.visibility_observer;

export namespace engine::gameplay::rts::visibility
{
struct VisibilityRegion final
{
	engine::gameplay::navigation::Cell center{engine::gameplay::navigation::InvalidCell};
	std::uint32_t radius{};
	bool valid{};

	friend constexpr bool operator==(const VisibilityRegion &, const VisibilityRegion &) noexcept = default;
};

struct VisibilityDefinition final
{
	VisibilityDefinitionId id{};
	std::uint32_t shroudClearingRadiusCells{};
	std::uint32_t constructionRadiusCells{};
	engine::time::Duration unlookPersistDuration{};
	std::uint64_t unlookPersistTicks{};
};

inline VisibilityDefinition CompileVisibilityDefinition(const VisibilityDefinitionId id,
	const std::uint32_t shroudClearingRadiusCells, const std::uint32_t constructionRadiusCells,
	const engine::time::Duration unlookPersistDuration, const engine::time::FixedStep step)
{
	return VisibilityDefinition{id, shroudClearingRadiusCells, constructionRadiusCells,
		unlookPersistDuration, step.TicksFor(unlookPersistDuration)};
}

class VisibilityDefinitions final
{
public:
	VisibilityDefinitions(std::span<const VisibilityDefinition> definitions, const engine::time::FixedStep step) :
		m_definitions(definitions), m_step(step)
	{
		for (std::size_t index = 0; index != m_definitions.size(); ++index)
		{
			if (m_definitions[index].id != index)
				throw std::invalid_argument("Visibility definition IDs must be dense");
			if (m_definitions[index].unlookPersistTicks !=
				m_step.TicksFor(m_definitions[index].unlookPersistDuration))
				throw std::invalid_argument("Visibility definition duration was not compiled for its fixed step");
		}
	}

	const VisibilityDefinition &Get(const VisibilityDefinitionId id) const
	{
		if (id >= m_definitions.size()) throw std::out_of_range("Visibility definition ID is outside the catalog");
		return m_definitions[id];
	}

	std::uint64_t MaximumGraceTicks() const noexcept
	{
		std::uint64_t result{};
		for (const auto &definition : m_definitions)
			if (definition.unlookPersistTicks > result) result = definition.unlookPersistTicks;
		return result;
	}

	engine::time::FixedStep Step() const noexcept { return m_step; }

private:
	std::span<const VisibilityDefinition> m_definitions;
	engine::time::FixedStep m_step;
};

class VisibilityTopology final
{
public:
	VisibilityTopology(const std::uint32_t width, const std::uint32_t height) : m_width(width), m_height(height)
	{
		if (!width || !height) throw std::invalid_argument("Visibility topology dimensions must be positive");
		const auto count = std::uint64_t{width} * height;
		if (count >= engine::gameplay::navigation::InvalidCell)
			throw std::length_error("Visibility topology exceeds cell index capacity");
		m_count = static_cast<std::uint32_t>(count);
	}

	std::uint32_t Width() const noexcept { return m_width; }
	std::uint32_t Height() const noexcept { return m_height; }
	std::uint32_t Count() const noexcept { return m_count; }
	bool Contains(const engine::gameplay::navigation::Cell cell) const noexcept { return cell < m_count; }

	std::uint32_t X(const engine::gameplay::navigation::Cell cell) const noexcept { return cell % m_width; }
	std::uint32_t Y(const engine::gameplay::navigation::Cell cell) const noexcept { return cell / m_width; }
	engine::gameplay::navigation::Cell CellAt(const std::uint32_t x, const std::uint32_t y) const noexcept
	{
		return static_cast<engine::gameplay::navigation::Cell>(std::uint64_t{y} * m_width + x);
	}

private:
	std::uint32_t m_width{};
	std::uint32_t m_height{};
	std::uint32_t m_count{};
};
} // namespace engine::gameplay::rts::visibility
