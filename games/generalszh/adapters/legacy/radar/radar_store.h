#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace generalszh { class GameplayState; class GameplayWorkers; struct PersistentResetParticipant; }

namespace generalszh::legacy
{
// Plain initialization/transfer value. Live radar state belongs to the ECS
// RadarAvailability component and is never retained beside a lease.
struct RadarValue
{
	std::int32_t producers{0};
	std::int32_t resistantProducers{0};
	bool suppressed{false};
};

class RadarLease;
struct RadarExecutionConfig
{
	std::size_t workers{1};
	std::size_t inputCapacity{65536};
	std::uint32_t ticksPerSecond{30}; // Legacy composition default, not engine policy.
};
struct RadarSound
{
	std::int32_t playerIndex{};
	bool online{};
};
class RadarStore
{
public:
	explicit RadarStore(GameplayState &state, RadarExecutionConfig config = {});
	RadarStore(GameplayState &state, GameplayWorkers &workers, RadarExecutionConfig config = {});
	~RadarStore() noexcept;
	RadarStore(const RadarStore &) = delete;
	RadarStore &operator=(const RadarStore &) = delete;
	std::size_t Count() const noexcept;
	PersistentResetParticipant ResetParticipant();
	// Explicit simulation/presentation boundary. Results remain valid until the
	// next Execute. Never called by getters, recorders or destructors.
	std::span<const RadarSound> Execute(std::uint64_t tick);

private:
	struct Impl;
	Impl *m_impl;
	friend class RadarLease;
};

// Stable adapter binding, not an ECS entity handle or a second state owner.
class RadarLease
{
public:
	explicit RadarLease(RadarStore &store, RadarValue initial = {});
	~RadarLease() noexcept;
	RadarLease(const RadarLease &) = delete;
	RadarLease &operator=(const RadarLease &) = delete;
	RadarValue State() const;
	void Restore(RadarValue value);
	void Reset();
	bool HasRadar() const;
	// Record only. Reads observe the last completed simulation boundary.
	void AddProvider(bool resistant, bool soundEligible = false, std::int32_t playerIndex = 0);
	void RemoveProvider(bool resistant, bool soundEligible = false, std::int32_t playerIndex = 0);
	void SetSuppressed(bool suppressed, bool soundEligible = false, std::int32_t playerIndex = 0);

private:
	RadarStore &m_store;
	std::size_t m_slot;
};
}
