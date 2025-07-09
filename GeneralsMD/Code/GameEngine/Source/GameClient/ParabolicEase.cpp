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

// ============================================================================
// Copyright (C) 2003, 2004 Electronic Arts
//
// ParabolicEase.cpp
// Ease in and out based on a parabolic function.
// Author: Robert Minsk May 12, 2003
// ============================================================================
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "GameClient/ParabolicEase.h"
// ============================================================================
// ============================================================================
namespace {
// ============================================================================
template <typename TYPE>
inline TYPE
clamp(TYPE val, TYPE min = TYPE(0), TYPE max = TYPE(1))
{
	if (val < min) {
		val = min;
	} else if (val > max) {
		val = max;
	}
	return val;
}

// ============================================================================
} // namespace
// ============================================================================
void
ParabolicEase::setEaseTimes(Real easeInTime, Real easeOutTime)
{
	m_in = easeInTime;
	if (m_in < 0.0f || m_in > 1.0f) {
		DEBUG_CRASH(("Ease-in out of range (in = %g)\n", m_in));
		m_in = clamp(m_in);
	}

	m_out = 1.0f - easeOutTime;
	if (m_out < 0.0f || m_out > 1.0f) {
		DEBUG_CRASH(("Ease-out out of range (out = %g)\n", m_out));
		m_out = clamp(m_out);
	}

	if (m_in > m_out) {
		DEBUG_CRASH(("Ease-in and ease-out overlap (in = %g, out = %g)\n", m_in, m_out));
		m_in = m_out;
	}
}

// ----------------------------------------------------------------------------
Real
ParabolicEase::operator ()(Real param) const
{
	if (param < 0.0f || param > 1.0f) {
		DEBUG_CRASH(("Ease-in/ease-out parameter out of range (param = %g)\n", param));
		param = clamp(param);
	}
#if 0
	const Real v0 = 2.0f/(1.0f + out - in);
	if (param < m_in) {
		// "param < in" avoids a division by zero at param = 0.0f and in = 0.0f
		return v0*param*param/(2.0f*m_in);
	} else if (param <= m_out) {
		// "param <= out" avoid a division by zero when param = 1.0f and out = 1.0f
		return v0*(m_in/2.0f + (param - m_in));
	} else {
		return v0*(m_in/2.0f + (m_out - m_in) +
					 (param - m_out + (m_out*m_out - param*param)/2.0f)/(1.0f - m_out));
	}
#else
	const Real v0 = 1.0f + m_out - m_in;
	if (param < m_in) {
		// "param < in" avoids a division by zero at param = 0.0f and in = 0.0f
		return param*param/(v0*m_in);
	} else if (param <= m_out) {
		// "param <= out" avoid a division by zero when param = 1.0f and out = 1.0f
		return (m_in + 2.0*(param - m_in))/v0;
	} else {
		return (m_in + 2.0*(m_out - m_in) + (2.0*(param - m_out) + m_out*m_out - param*param)/(1.0f - m_out))/v0;
	}
#endif
}
