module;

#include <cstdint>
#include <optional>

export module games.generalszh.gameplay.visibility.definitions.visibility_policy;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.rts.visibility.components.visibility_observer;

export namespace generalszh::visibility
{
struct VisibilityPolicyInput final
{
	bool alive{};
	engine::gameplay::navigation::Cell actorCell{engine::gameplay::navigation::InvalidCell};
	std::optional<engine::gameplay::containment::PassengerMembership> membership{};
	engine::gameplay::navigation::Cell carrierCell{engine::gameplay::navigation::InvalidCell};
	bool carrierCellValid{};
	bool underConstruction{};
	std::optional<std::uint32_t> constructionRadiusCells{};
};

// The legacy Object::look path allows a garrisonable contained observer to
// look from its enclosing container, but explicitly excludes non-garrisonable
// transports. The carrier cell is a resolved generation-safe projection from
// the containment producer; the passenger's stale transform is never read.
inline engine::gameplay::rts::visibility::VisibilityEligibility ResolveVisibilityEligibility(
	const VisibilityPolicyInput &input)
{
	using namespace engine::gameplay::containment;
	using engine::gameplay::navigation::InvalidCell;
	using engine::gameplay::rts::visibility::VisibilityEligibility;

	VisibilityEligibility result{};
	if (!input.alive) return result;

	engine::gameplay::navigation::Cell resolvedCell = input.actorCell;
	if (input.membership && IsContained(*input.membership))
	{
		if (IsTransportOwned(*input.membership) || !IsGarrisonOwned(*input.membership))
			return result;
		if (!input.carrierCellValid || input.carrierCell == InvalidCell)
			return result;
		resolvedCell = input.carrierCell;
	}
	if (resolvedCell == InvalidCell) return result;

	result.active = true;
	result.resolvedCellValid = true;
	result.resolvedCell = resolvedCell;
	if (input.underConstruction && input.constructionRadiusCells)
	{
		result.radiusOverride = true;
		result.radiusCells = *input.constructionRadiusCells;
	}
	return result;
}
} // namespace generalszh::visibility
