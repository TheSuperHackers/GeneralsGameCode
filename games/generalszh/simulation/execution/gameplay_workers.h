#pragma once
#include <cstddef>
namespace generalszh
{
// Explicit game-composition owner, independent of replaceable World state.
class GameplayWorkers
{
public:
	explicit GameplayWorkers(std::size_t workers = 1);
	~GameplayWorkers() noexcept;
	GameplayWorkers(const GameplayWorkers &) = delete;
	GameplayWorkers &operator=(const GameplayWorkers &) = delete;
	std::size_t Count() const noexcept;
private:
	friend class GameplayWorkerAccess;
	struct Impl;
	Impl *m_impl;
};
}
