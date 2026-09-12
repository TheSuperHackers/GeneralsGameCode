module;
#include "games/generalszh/simulation/gameplay_state.h"
#include <algorithm>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

export module games.generalszh.simulation.reset.persistent_world_reset;
export import engine.ecs.core.world;

export namespace generalszh
{
extern "C++"
{
// Cold composition ABI only. Each owner retains its typed capture storage.
// No per-entity erased callbacks, schema registration, or simulation dispatch.
struct PersistentResetParticipant
{
	std::string_view stableName;
	GameplayState *state;
	void *context;
	std::span<const ecs::Entity> (*capture)(void *);
	void (*release)(void *);
	void (*restore)(void *);
	void (*discard)(void *) noexcept;
	void (*fail)(void *) noexcept;
	// Cached-World lifetime bindings released by this participant. Cold preflight
	// metadata only; actual release is checked again before World replacement.
	std::size_t worldBindings{0};
};

class PersistentWorldReset
{
public:
	static void Execute(GameplayState &state, std::span<const PersistentResetParticipant> participants)
	{
		state.BeginReset();
		try { ExecuteLocked(state, participants); }
		catch (...) { state.EndReset(); throw; }
		state.EndReset();
	}
private:
	static void ExecuteLocked(GameplayState &state, std::span<const PersistentResetParticipant> participants)
	{
		std::vector<PersistentResetParticipant> ordered(participants.begin(), participants.end());
		for (const auto &participant : ordered)
		{
			if (participant.stableName.empty() || participant.state != &state || !participant.context
				|| !participant.capture || !participant.release || !participant.restore
				|| !participant.discard || !participant.fail)
				throw std::invalid_argument("Invalid persistent reset participant");
			for (const unsigned char character : participant.stableName)
				if (!((character >= 'a' && character <= 'z') || (character >= '0' && character <= '9')
					|| character == '.' || character == '_' || character == '-' || character == '/'))
					throw std::invalid_argument("Persistent participant names must be canonical lowercase ASCII");
		}
		std::sort(ordered.begin(), ordered.end(), [](const auto &a, const auto &b) { return a.stableName < b.stableName; });
		for (std::size_t index = 0; index < ordered.size(); ++index)
		{
			if (index && ordered[index - 1].stableName == ordered[index].stableName)
				throw std::logic_error("Duplicate persistent reset participant name");
			for (std::size_t prior = 0; prior < index; ++prior)
				if (ordered[index].context == ordered[prior].context)
					throw std::logic_error("Duplicate persistent reset participant instance");
		}
		struct Captures
		{
			std::span<const PersistentResetParticipant> participants;
			~Captures() { for (const auto &participant : participants) participant.discard(participant.context); }
		} cleanup{ordered};
		std::vector<ecs::Entity> owned;
		for (const auto &participant : ordered)
		{
			const auto entities = participant.capture(participant.context);
			owned.insert(owned.end(), entities.begin(), entities.end());
		}
		std::sort(owned.begin(), owned.end(), [](ecs::Entity a, ecs::Entity b) {
			return a.index != b.index ? a.index < b.index : a.generation < b.generation;
		});
		for (std::size_t index = 0; index < owned.size(); ++index)
		{
			if (!state.World().IsAlive(owned[index])) throw std::logic_error("Persistent reset claims a stale entity");
			if (index && owned[index - 1] == owned[index]) throw std::logic_error("Persistent reset entity has multiple owners");
		}
		if (owned.size() != state.Count()) throw std::logic_error("Release non-persistent gameplay entities before reset");
		std::size_t declaredBindings = 0;
		for (const auto &participant : ordered)
		{
			if (participant.worldBindings > state.m_worldBindings - declaredBindings)
				throw std::logic_error("Persistent reset overclaims cached-World bindings");
			declaredBindings += participant.worldBindings;
		}
		if (declaredBindings != state.m_worldBindings)
			throw std::logic_error("Persistent reset omits cached-World owners");
		try
		{
			for (const auto &participant : ordered) participant.release(participant.context);
			state.ReplaceEmptyWorld();
			for (const auto &participant : ordered) participant.restore(participant.context);
		}
		catch (...)
		{
			state.m_resetFailed = true;
			for (const auto &participant : ordered) participant.fail(participant.context);
			throw;
		}
	}
};
}
}
