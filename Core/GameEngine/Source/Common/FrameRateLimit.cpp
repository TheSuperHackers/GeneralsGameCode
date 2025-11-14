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

#include "PreRTS.h"
#include "Common/FrameRateLimit.h"


FrameRateLimit::FrameRateLimit()
{
	LARGE_INTEGER freq;
	LARGE_INTEGER start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	m_freq = freq.QuadPart;
	m_start = start.QuadPart;
}

Real FrameRateLimit::wait(UnsignedInt maxFps)
{
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);
	double elapsedSeconds = static_cast<double>(tick.QuadPart - m_start) / m_freq;
	const double targetSeconds = 1.0 / maxFps;
	const double sleepSeconds = targetSeconds - elapsedSeconds - 0.002; // leave ~2ms for spin wait

	if (sleepSeconds > 0.0)
	{
		// Non busy wait with Munkee sleep
		DWORD dwMilliseconds = static_cast<DWORD>(sleepSeconds * 1000);
		Sleep(dwMilliseconds);
	}

	// Busy wait for remaining time
	do
	{
		QueryPerformanceCounter(&tick);
		elapsedSeconds = static_cast<double>(tick.QuadPart - m_start) / m_freq;
	}
	while (elapsedSeconds < targetSeconds);

	m_start = tick.QuadPart;
	return (Real)elapsedSeconds;
}


const UnsignedInt RenderFpsPreset::s_fpsValues[] = {
	30, 50, 56, 60, 65, 70, 72, 75, 80, 85, 90, 100, 110, 120, 144, 240, 480, UncappedFpsValue };

static_assert(LOGICFRAMES_PER_SECOND <= 30, "Min FPS values need to be revisited!");

UnsignedInt RenderFpsPreset::getNextFpsValue(UnsignedInt value)
{
	const Int first = 0;
	const Int last = ARRAY_SIZE(s_fpsValues) - 1;
	for (Int i = first; i < last; ++i)
	{
		if (value >= s_fpsValues[i] && value < s_fpsValues[i + 1])
		{
			return s_fpsValues[i + 1];
		}
	}
	return s_fpsValues[last];
}

UnsignedInt RenderFpsPreset::getPrevFpsValue(UnsignedInt value)
{
	const Int first = 0;
	const Int last = ARRAY_SIZE(s_fpsValues) - 1;
	for (Int i = last; i > first; --i)
	{
		if (value <= s_fpsValues[i] && value > s_fpsValues[i - 1])
		{
			return s_fpsValues[i - 1];
		}
	}
	return s_fpsValues[first];
}

UnsignedInt RenderFpsPreset::changeFpsValue(UnsignedInt value, FpsValueChange change)
{
	switch (change)
	{
	default:
	case FpsValueChange_Increase: return getNextFpsValue(value);
	case FpsValueChange_Decrease: return getPrevFpsValue(value);
	}
}


UnsignedInt LogicTimeScaleFpsPreset::getNextFpsValue(UnsignedInt value)
{
	return value + StepFpsValue;
}

UnsignedInt LogicTimeScaleFpsPreset::getPrevFpsValue(UnsignedInt value)
{
	if (value - StepFpsValue < MinFpsValue)
	{
		return MinFpsValue;
	}
	else
	{
		return value - StepFpsValue;
	}
}

UnsignedInt LogicTimeScaleFpsPreset::changeFpsValue(UnsignedInt value, FpsValueChange change)
{
	switch (change)
	{
	default:
	case FpsValueChange_Increase: return getNextFpsValue(value);
	case FpsValueChange_Decrease: return getPrevFpsValue(value);
	}
}
