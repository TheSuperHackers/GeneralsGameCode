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


namespace
{
	typedef HANDLE (WINAPI *CreateWaitableTimerExWFunction)(LPSECURITY_ATTRIBUTES, LPCWSTR, DWORD, DWORD);

	HANDLE CreateFrameRateTimer()
	{
		const DWORD createWaitableTimerHighResolution = 0x00000002;
		HANDLE timer = nullptr;
		HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
		if (kernel32 != nullptr)
		{
			CreateWaitableTimerExWFunction createWaitableTimerExW =
				reinterpret_cast<CreateWaitableTimerExWFunction>(GetProcAddress(kernel32, "CreateWaitableTimerExW"));
			if (createWaitableTimerExW != nullptr)
			{
				timer = createWaitableTimerExW(
					nullptr, nullptr, createWaitableTimerHighResolution, TIMER_MODIFY_STATE | SYNCHRONIZE);
			}
		}

		if (timer == nullptr)
			timer = CreateWaitableTimer(nullptr, FALSE, nullptr);

		return timer;
	}
}


FrameRateLimit::FrameRateLimit()
{
	LARGE_INTEGER freq;
	LARGE_INTEGER start;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	m_freq = freq.QuadPart;
	m_start = start.QuadPart;
	m_waitableTimer = CreateFrameRateTimer();
}

FrameRateLimit::~FrameRateLimit()
{
	if (m_waitableTimer != nullptr)
		CloseHandle(static_cast<HANDLE>(m_waitableTimer));
}

Int64 FrameRateLimit::calculateCoarseWaitTicks(Int64 remainingTicks, Int64 spinTicks)
{
	if (remainingTicks <= spinTicks)
		return 0;

	return remainingTicks - spinTicks;
}

Real FrameRateLimit::wait(UnsignedInt maxFps)
{
	PROFILER_SECTION;
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);
	Int64 elapsedTicks = tick.QuadPart - m_start;
	if (maxFps == 0 || m_freq <= 0)
	{
		m_start = tick.QuadPart;
		return m_freq > 0 ? static_cast<Real>(static_cast<double>(elapsedTicks) / m_freq) : 0.0f;
	}

	const Int64 targetTicks = (m_freq + maxFps - 1) / maxFps;
	const Int64 spinTicks = m_freq / 5000; // Keep only the final ~0.2 ms as a busy wait.
	const Int64 coarseWaitTicks = calculateCoarseWaitTicks(targetTicks - elapsedTicks, spinTicks);

	if (coarseWaitTicks > 0)
	{
		Bool waited = false;
		if (m_waitableTimer != nullptr)
		{
			LARGE_INTEGER dueTime;
			dueTime.QuadPart = -((coarseWaitTicks * 10000000 + m_freq - 1) / m_freq);
			if (SetWaitableTimer(static_cast<HANDLE>(m_waitableTimer), &dueTime, 0, nullptr, nullptr, FALSE))
				waited = WaitForSingleObject(static_cast<HANDLE>(m_waitableTimer), INFINITE) == WAIT_OBJECT_0;
		}

		if (!waited)
		{
			const DWORD milliseconds = static_cast<DWORD>((coarseWaitTicks * 1000) / m_freq);
			if (milliseconds > 0)
				Sleep(milliseconds);
		}
	}

	// Busy wait only for the final scheduling jitter.
	do
	{
		QueryPerformanceCounter(&tick);
		elapsedTicks = tick.QuadPart - m_start;
	}
	while (elapsedTicks < targetTicks);

	m_start = tick.QuadPart;
	return static_cast<Real>(static_cast<double>(elapsedTicks) / m_freq);
}

void FrameRateLimit::reset()
{
	LARGE_INTEGER tick;
	QueryPerformanceCounter(&tick);
	m_start = tick.QuadPart;
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
