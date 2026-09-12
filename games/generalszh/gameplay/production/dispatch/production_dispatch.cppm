module;

export module games.generalszh.gameplay.production.dispatch.production_dispatch;

export namespace generalszh::production
{
// The boundary owns captured queue/config/time/object references and the
// authoritative progress predicate. Gameplay callbacks may change the live
// boundary state; the repeated type reads below are therefore intentional.
template<class Boundary>
void DispatchProduction(Boundary &boundary)
{
	if (boundary.HasDoorAnimations())
		boundary.UpdateDoors();

	boundary.UpdateMarker();
	boundary.FlushFlags();

	if (!boundary.HasEntry())
		return;
	if (boundary.IsSold())
		return;

	auto owner = boundary.Owner();
	if (!boundary.HasOwner(owner))
	{
		boundary.DiscardEntry();
		return;
	}

	if (boundary.IsUnit() && !boundary.Allowed(owner))
	{
		if (!boundary.IsDozer())
		{
			boundary.CancelUnit();
			return;
		}
	}

	boundary.Advance();
	auto required = boundary.IsUnit()
		? boundary.UnitRequirement(owner)
		: boundary.UpgradeRequirement(owner);
	boundary.Refresh(required);

	if (boundary.IsComplete())
	{
		if (boundary.IsUnit())
			boundary.CompleteUnit();
		else if (boundary.IsUpgrade())
			boundary.CompleteUpgrade(owner);
	}
}
}
