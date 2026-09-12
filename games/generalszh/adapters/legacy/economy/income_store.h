#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
namespace generalszh { class GameplayState; class GameplayWorkers; struct PersistentResetParticipant; }
namespace generalszh::legacy
{
struct IncomeStateRecord
{
	std::uint32_t deadline{0};
	bool bonusAvailable{false};
	bool initialized{false};
};
class IncomeLease;
struct IncomeExecutionConfig { std::size_t capacity{65536}; std::uint32_t ticksPerSecond{30}; };
struct IncomeInput
{
	std::uint32_t object{}, moduleTag{};
	std::int32_t player{-1}, boost{};
	bool active{false}, neutral{true}, constructionComplete{false}, visible{false}, dispatchEnabled{true};
};
struct IncomeEffect
{
	std::uint32_t object{}, money{};
	std::int32_t player{-1}, score{}, display{};
	bool pay{false}, show{false}, capture{false};
};
class IncomeStore
{
public:
	explicit IncomeStore(GameplayState &state);
	IncomeStore(GameplayState &state, GameplayWorkers &workers, IncomeExecutionConfig config = {});
	~IncomeStore() noexcept;
	IncomeStore(const IncomeStore &) = delete;
	IncomeStore &operator=(const IncomeStore &) = delete;
	std::size_t Count() const noexcept;
	PersistentResetParticipant ResetParticipant();
	std::span<IncomeInput> PrepareInputs();
	void Execute(std::uint64_t tick);
	// Advances before the external effect. Outputs are committed facts scoped
	// to this boundary, not resolvable entity handles. No retry after failure.
	bool NextEffect(IncomeEffect &effect);
	void FinishEffects();
	void Fail() noexcept;
private:
	struct Impl;
	Impl *m_impl;
	friend class IncomeLease;
};
class IncomeLease
{
public:
	IncomeLease(IncomeStore &store, std::uint32_t frame, std::uint32_t interval,
		std::uint32_t object = 0, std::uint32_t moduleTag = 0, std::int32_t base = 0,
		std::int32_t bonus = 0, bool actualMoney = true);
	~IncomeLease() noexcept;
	IncomeLease(const IncomeLease &) = delete;
	IncomeLease &operator=(const IncomeLease &) = delete;
	bool BeginUpdate(std::uint32_t frame, std::uint32_t interval);
	void Capture(std::uint32_t frame, std::int32_t player);
	void SetObject(std::uint32_t object);
	void Rearm(std::uint32_t frame, std::uint32_t interval);
	void ConsumeCaptureBonus();
	IncomeStateRecord State() const;
	void Restore(IncomeStateRecord state); // Scalar legacy persistence boundary only.
private:
	IncomeStore &m_store;
	std::uint32_t m_index;
	std::uint32_t m_generation;
	std::size_t m_slot;
};
}
