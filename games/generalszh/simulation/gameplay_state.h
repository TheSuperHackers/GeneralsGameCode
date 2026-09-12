#pragma once
#include <cstddef>
#include <atomic>
namespace ecs { class World; }
namespace generalszh
{
// Narrow textual boundary for legacy composition. The module owns and
// explicitly finalizes the engine World; no global service lookup is performed.
class GameplayState
{
public:
	GameplayState();
	~GameplayState() noexcept;
	GameplayState(const GameplayState &) = delete;
	GameplayState &operator=(const GameplayState &) = delete;
	ecs::World &World() noexcept;
	std::size_t Count() const noexcept;
	void Reset(); // No live entities, cached-world bindings or jobs.
private:
	friend class PersistentWorldReset;
	friend class GameplayWorldBinding;
	void BeginReset();
	void EndReset() noexcept;
	void ReplaceEmptyWorld();
	std::atomic_flag m_resetting = ATOMIC_FLAG_INIT;
	bool m_resetFailed{false};
	std::size_t m_worldBindings{0};
	struct Impl;
	Impl *m_impl;
};
}
