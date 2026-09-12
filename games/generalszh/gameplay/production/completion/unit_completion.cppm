module;

#include <cstdint>

export module games.generalszh.gameplay.production.completion.unit_completion;

export import games.generalszh.gameplay.production.entry.exit_reservation;

export namespace generalszh::production
{
// Ordered effect-boundary policy, not an ECS system or per-entity dispatch API.
// Boundary owns no duplicated entry state: quantity/reservation operations may
// bind directly to columns. Exceptions escape with their applied prefix intact.
template<class Boundary>
void CompleteUnits(Boundary &boundary)
{
	if (!boundary.HasExit())
	{
		boundary.ReportMissingExit();
		boundary.DiscardEntry();
		return;
	}

	// Completion callbacks can change the live quantity. Do not turn this into
	// a live loop condition or collapse unsuccessful attempts.
	const std::int32_t attempts = boundary.Remaining();
	for (std::int32_t i = 0; i < attempts; ++i)
	{
		ExitReservation reservation = boundary.Reservation();
		if (reservation == ExitReservation::NoneAvailable)
		{
			reservation = boundary.ReserveExit();
			boundary.SetReservation(reservation);
		}

		if (reservation != ExitReservation::NoneAvailable)
		{
			if (boundary.AnimationCount() > 0)
				boundary.RequestDoor(reservation);

			boundary.RequestMarker();

			if (boundary.AnimationCount() == 0 || boundary.DoorReady(reservation))
			{
				auto token = boundary.CreateUnit();
				boundary.SetProducer(token);
				boundary.ExitUnit(token, reservation);
				boundary.SetReservation(ExitReservation::NoneAvailable);

				boundary.VoiceCreated(token);
				boundary.OwnerCreated(token);
				boundary.BuildComplete(token);

				// Make observation order explicit rather than relying on the
				// unspecified evaluation order of equality operands.
				const std::int32_t liveTotal = boundary.Total();
				const std::int32_t liveRemaining = boundary.Remaining();
				if (liveTotal == liveRemaining)
					boundary.FirstUnitVoice(token);

				boundary.RecordProduction(token);
				boundary.CompleteOne();
			}
		}
	}

	if (boundary.Remaining() == 0)
		boundary.DiscardEntry();
}
}
