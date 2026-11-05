/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include "Common/GameCommon.h"


class FrameRateLimit
{
public:
	FrameRateLimit();

	Real wait(UnsignedInt maxFps);
	void reset(); ///< Move the timing anchor to now, discarding any time elapsed since the last call to wait.

private:
	Int64 m_freq;
	Int64 m_start;
};


enum FpsValueChange
{
	FpsValueChange_Increase,
	FpsValueChange_Decrease,
};


class RenderFpsPreset
{
public:
	enum CPP_11(: UnsignedInt)
	{
		UncappedFpsValue = 1000000,
	};

	static UnsignedInt changeFpsValue(UnsignedInt value, FpsValueChange change);

private:
	static UnsignedInt getNextFpsValue(UnsignedInt value);
	static UnsignedInt getPrevFpsValue(UnsignedInt value);

	static const UnsignedInt s_fpsValues[];
};


class LogicTimeScaleFpsPreset
{
public:

	static UnsignedInt changeFpsValue(UnsignedInt value, FpsValueChange change, UnsignedInt snapValue = 0);

private:
	static UnsignedInt getNextFpsValue(UnsignedInt value, UnsignedInt snapValue = 0);
	static UnsignedInt getPrevFpsValue(UnsignedInt value, UnsignedInt snapValue = 0);

	static const UnsignedInt s_fpsValues[];
};

