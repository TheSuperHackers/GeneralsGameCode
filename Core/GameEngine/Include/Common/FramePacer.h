/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Common/FrameRateLimit.h"


// TheSuperHackers @todo Use unsigned integers for fps values
// TheSuperHackers @todo Consolidate the GlobalData::m_useFpsLimit and FramePacer::m_enableFpsLimit
// TheSuperHackers @todo Implement new fast forward in here
class FramePacer
{
public:

	typedef UnsignedInt LogicTimeQueryFlags;
	enum LogicTimeQueryFlags_ CPP_11(: LogicTimeQueryFlags)
	{
		IgnoreFrozenTime = 1<<0, ///< Ignore frozen time for the query
		IgnoreHaltedGame = 1<<1, ///< Ignore halted game for the query
	};

	FramePacer();
	~FramePacer();

	void update(); ///< Signal that the app/render update is done and wait for the fps limit if applicable.

	void setFramesPerSecondLimit( Int fps ); ///< Set the update fps limit.
	Int  getFramesPerSecondLimit() const; ///< Get the update fps limit.
	void enableFramesPerSecondLimit( Bool enable ); ///< Enable or disable the update fps limit.
	Bool isFramesPerSecondLimitEnabled() const; ///< Returns whether the fps limit is enabled here.
	Bool isActualFramesPerSecondLimitEnabled() const; ///< Returns whether the fps limit is actually enabled when considering all game settings and setups.
	Int  getActualFramesPerSecondLimit() const; // Get the actual update fps limit.

	Real getUpdateTime() const; ///< Get the last update delta time in seconds.
	Real getUpdateFps() const; ///< Get the last update fps.

	void setTimeFrozen(Bool frozen); ///< Set time frozen. Allows scripted camera movement.
	void setGameHalted(Bool halted); ///< Set game halted. Does not allow scripted camera movement.
	Bool isTimeFrozen() const;
	Bool isGameHalted() const;

	void setLogicTimeScaleFps( Int fps ); ///< Set the logic time scale fps and therefore scale the simulation time. Is capped by the max render fps and does not apply to network matches.
	Int  getLogicTimeScaleFps() const; ///< Get the raw logic time scale fps value.
	void enableLogicTimeScale( Bool enable ); ///< Enable or disable the logic time scale setup. If disabled, the simulation time scale is bound to the render frame time or network update time.
	Bool isLogicTimeScaleEnabled() const; ///< Check whether the logic time scale setup is enabled.
	Int  getActualLogicTimeScaleFps(LogicTimeQueryFlags flags = 0) const; ///< Get the real logic time scale fps, depending on the max render fps, network state and enabled state.
	Real getActualLogicTimeScaleRatio(LogicTimeQueryFlags flags = 0) const; ///< Get the real logic time scale ratio, depending on the max render fps, network state and enabled state.
	Real getActualLogicTimeScaleOverFpsRatio(LogicTimeQueryFlags flags = 0) const; ///< Get the real logic time scale over render fps ratio, used to scale down steps in render updates to match logic updates.
	Real getLogicTimeStepSeconds(LogicTimeQueryFlags flags = 0) const; ///< Get the logic time step in seconds
	Real getLogicTimeStepMilliseconds(LogicTimeQueryFlags flags = 0) const; ///< Get the logic time step in milliseconds

protected:

	FrameRateLimit m_frameRateLimit;

	Int m_maxFPS; ///< Maximum frames per second for rendering
	Int m_logicTimeScaleFPS; ///< Maximum frames per second for logic time scale

	Real m_updateTime; ///< Last update delta time in seconds

	Bool m_enableFpsLimit;
	Bool m_enableLogicTimeScale;
	Bool m_isTimeFrozen;
	Bool m_isGameHalted;
};

extern FramePacer* TheFramePacer;
