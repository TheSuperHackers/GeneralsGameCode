#pragma once

namespace generalszh
{
class GameplayState;

// Cold composition lifetime, not entity ownership or a component-access API.
// Declare this before cached queries/schedulers so they die before the binding.
// Acquire/release only at owner-thread setup/reset boundaries, never in jobs.
class GameplayWorldBinding
{
public:
	explicit GameplayWorldBinding(GameplayState &state);
	~GameplayWorldBinding() noexcept;
	GameplayWorldBinding(const GameplayWorldBinding &) = delete;
	GameplayWorldBinding &operator=(const GameplayWorldBinding &) = delete;
private:
	GameplayState &m_state;
};
}
