module;
#include "world_binding.h"
#include "games/generalszh/simulation/gameplay_state.h"
#include <exception>
#include <limits>
#include <stdexcept>
export module games.generalszh.simulation.lifecycle.world_binding;
import engine.ecs.core.world;

extern "C++"
{
namespace generalszh
{
GameplayWorldBinding::GameplayWorldBinding(GameplayState &state) : m_state(state)
{
	if (state.m_resetFailed || state.World().IsScheduledExecutionActive())
		throw std::logic_error("Cannot bind failed gameplay state or bind during scheduled execution");
	if (state.m_worldBindings == (std::numeric_limits<std::size_t>::max)())
		throw std::overflow_error("Gameplay World binding capacity exhausted");
	++state.m_worldBindings;
}

GameplayWorldBinding::~GameplayWorldBinding() noexcept
{
	if (m_state.m_worldBindings == 0 || m_state.World().IsScheduledExecutionActive())
		std::terminate();
	--m_state.m_worldBindings;
}
}
}
