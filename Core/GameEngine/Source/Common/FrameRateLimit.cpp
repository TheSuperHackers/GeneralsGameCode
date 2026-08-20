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
	m_nextDeadline = m_start;
	m_lastFps = 0;
}

Real FrameRateLimit::wait(UnsignedInt maxFps)
{
	PROFILER_SECTION;
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);

	// The uncapped sentinel is deliberately handled without a synthetic 1 MHz wait.
	// This keeps benchmark and fast-forward paths from paying an unnecessary timing cost.
	if (maxFps == 0 || maxFps >= RenderFpsPreset::UncappedFpsValue)
	{
		const Real elapsedSeconds = static_cast<Real>(static_cast<double>(tick.QuadPart - m_start) / m_freq);
		m_start = tick.QuadPart;
		m_nextDeadline = m_start;
		m_lastFps = 0;
		return elapsedSeconds;
	}

	const Int64 targetTicks = static_cast<Int64>(static_cast<double>(m_freq) / maxFps);
	if (m_lastFps != maxFps || m_nextDeadline <= m_start)
	{
		// Re-anchor when the user changes the render limit or after a reset.
		m_lastFps = maxFps;
		m_nextDeadline = tick.QuadPart + targetTicks;
	}

	// Sleep until close to the deadline, then use the high-resolution counter for the
	// final fraction. Unlike the old implementation, the deadline advances from the
	// previous deadline, which prevents small sleep errors from becoming visible drift.
	for (;;)
	{
		QueryPerformanceCounter(&tick);
		const Int64 remainingTicks = m_nextDeadline - tick.QuadPart;
		if (remainingTicks <= 0)
		{
			break;
		}

		const double remainingSeconds = static_cast<double>(remainingTicks) / m_freq;
		if (remainingSeconds > 0.0015)
		{
			DWORD sleepMilliseconds = static_cast<DWORD>(remainingSeconds * 1000.0);
			if (sleepMilliseconds > 1)
			{
				Sleep(sleepMilliseconds - 1);
			}
		}
	}

	const Int64 elapsedTicks = tick.QuadPart - m_start;
	m_start = tick.QuadPart;
	m_nextDeadline += targetTicks;
	if (m_nextDeadline <= tick.QuadPart)
	{
		// Recover cleanly after a long stall without trying to replay missed render frames.
		m_nextDeadline = tick.QuadPart + targetTicks;
	}

	return static_cast<Real>(static_cast<double>(elapsedTicks) / m_freq);
}

void FrameRateLimit::reset()
{
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);
	m_start = tick.QuadPart;
	m_nextDeadline = m_start;
	m_lastFps = 0;
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
