#pragma once
#include <cstddef>
#include <cstdint>

namespace generalszh { class GameplayState; class GameplayWorkers; struct PersistentResetParticipant; }
namespace generalszh::legacy
{
// Plain initialization/transfer value, never retained alongside a live binding.
struct PowerTotals
{
	std::int32_t production{0};
	std::int32_t consumption{0};
};

class PowerLease;
class PowerStore
{
public:
	explicit PowerStore(GameplayState &state);
	PowerStore(GameplayState &state, GameplayWorkers &workers, std::uint32_t ticksPerSecond = 30);
	~PowerStore() noexcept;
	PowerStore(const PowerStore &) = delete;
	PowerStore &operator=(const PowerStore &) = delete;
	std::size_t Count() const noexcept;
	PersistentResetParticipant ResetParticipant();
	// Explicit joined boundary. Finish before another execution/reset. Unexpected
	// execution/effect failure is fatal; no prefix replay or implicit commit.
	void ExecuteRecovery(std::uint64_t tick);
	bool NextRecovery(std::int32_t &player);
	void FinishRecovery();
	void Fail() noexcept;
private:
	struct Impl;
	Impl *m_impl;
	friend class PowerLease;
};

// Stable adapter binding, not an ECS entity handle or a second state owner.
class PowerLease
{
public:
	explicit PowerLease(PowerStore &store, PowerTotals initial = {});
	~PowerLease() noexcept;
	PowerLease(const PowerLease &) = delete;
	PowerLease &operator=(const PowerLease &) = delete;
	PowerTotals State() const;
	void Restore(PowerTotals value);
	void Reset();
	void SetPlayer(std::int32_t player);
	void AddProduction(std::int32_t delta);
	void AddConsumption(std::int32_t delta);
	// Legacy frame conversion stays at this adapter boundary. Modern gameplay
	// uses the wide PowerSuppression component and explicit simulation time.
	std::uint32_t SabotagedUntilFrame() const;
	void SetSabotagedUntilFrame(std::uint32_t deadline);
	bool IsSuppressedAt(std::uint32_t frame) const;
	bool RecoverSuppressionAt(std::uint32_t frame);
private:
	PowerStore &m_store;
	std::size_t m_slot;
};
}
