#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
namespace generalszh { class GameplayState; class GameplayWorkers; struct PersistentResetParticipant; }

// Textual ABI boundary for the existing GeneralsMD headers. Implementation is
// in lifetime_store.cppm; no ECS layout, legacy Object, or INI type crosses it.
namespace generalszh { namespace legacy {

class LifetimeLease;
struct LifetimeExecutionConfig
{
	std::size_t capacity{65536};
	std::uint32_t ticksPerSecond{30};
};
struct LifetimeInput { std::uint32_t object{}; bool enabled{false}; };
struct LifetimeEffect
{
	std::uint32_t object{}, index{}, generation{};
	std::uint64_t revision{}, boundary{};
	bool destroy{false};
};
class LifetimeStore
{
public:
	LifetimeStore();
	explicit LifetimeStore(GameplayState &state);
	LifetimeStore(GameplayState &state, GameplayWorkers &workers, LifetimeExecutionConfig config = {});
	~LifetimeStore() noexcept;
	LifetimeStore(const LifetimeStore &) = delete;
	LifetimeStore &operator=(const LifetimeStore &) = delete;
	std::size_t Count() const noexcept;
	void Reset(); // Requires every lease from the preceding game to be released.
	PersistentResetParticipant ResetParticipant();
	// One executing timer domain per World. Fill eligibility, execute joined
	// chunk work, consume/apply in returned order, then explicitly finish.
	// Spans/tokens are local to this store/boundary, not portable entity handles.
	// On effect failure call Fail and tear down; never retry a partially applied
	// boundary. No automatic commit or destructor playback.
	std::span<LifetimeInput> PrepareInputs();
	std::span<const LifetimeEffect> Execute(std::uint64_t tick);
	bool Consume(const LifetimeEffect &effect);
	void FinishEffects();
	void Fail() noexcept;
private:
	struct Impl;
	Impl *m_impl;
	friend class LifetimeLease;
};

// Scoped ownership of one timer entity. Moving/copying legacy modules is not
// supported; releasing the module invalidates its ECS handle automatically.
class LifetimeLease
{
public:
	explicit LifetimeLease(LifetimeStore &store, std::uint32_t object = 0, bool destroy = false);
	~LifetimeLease() noexcept;
	LifetimeLease(const LifetimeLease &) = delete;
	LifetimeLease &operator=(const LifetimeLease &) = delete;
	std::uint32_t Deadline() const;
	void SetDeadline(std::uint32_t frame);
	void SetObject(std::uint32_t object); // Restore/remap identity at the legacy load boundary.
private:
	LifetimeStore &m_store;
	std::uint32_t m_index;
	std::uint32_t m_generation;
};

} }
