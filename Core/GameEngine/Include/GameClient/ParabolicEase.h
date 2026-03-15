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
// ParabolicEase.h
// Ease in and out based on a parabolic function.
// Author: Robert Minsk May 12, 2003
// ============================================================================

#pragma once

// ============================================================================
#include "Lib/BaseType.h"
// ============================================================================
/// Ease in and out based on a linear velocity.
/**
 * This ends up being a function that is parabolic at both ends and a linear
 * middle section with respect to position.
 *
 * velocity(0.0) = 0.0
 * velocity(in) = v0
 * velocity(out) = v0
 * velocity(1.0) = 0.0
 *
 * From 0.0->in velocity is linearly increasing.
 * From out->1.0 velocity is linearly decreasing.
 *
 * velocity(t) = v0*t/in					t = [0, in]			Linear increasing segment
 *						 = v0								t = (in, out]		Constant segment
 *						 = (1-t)*v0/(1-out)	t = (out, 1.0]	Linear decreasing segment
 *
 * We need to calculate v0.  We want the total distance covered to be 1.0.
 *
 * 1 = integral(velocity(t), 0, 1)
 * 1 = integral(velocity(t), 0, in) +
 * 		 integral(velocity(t), in, out) +
 * 		 integral(velocity(t), out, 1.0)
 * 1 = v0*in/2 + v0*(out - in) + v0*(1 - out)/2
 * 	 = v0*(out-in+1)/2
 * v0 = 2/(out-in+1)
 *
 * Now we can calculate the distance function.
 *
 * d(0->in)		= integral(velocity(t), 0, s)
 *						= v0*s*s/(2*in)
 * d(in->out) = d(0->in) + integral(velocity(t), in, s)
 *						= (v0*in/2) + (v0*(s - in))
 * d(out->1)	= d(0->in) + d(in->out) + integral(velocity(t), out, s)
 * 						= (v0*in/2) + (v0*(out - in)) + (s-s*s/2-out+out*out/2)*v0/(1-out)
 */
class ParabolicEase
{
  public:
		explicit ParabolicEase(Real easeInTime = 0.0f, Real easeOutTime = 0.0f)
			{ setEaseTimes(easeInTime, easeOutTime); }

		/// Initialize the ease-in/ease-out function.
		/**
		 * \param easeInTime/\param easeOutTime is the amount of time to
		 * accomplish the transition.  The time is normalized from 0 to 1.
		 */
		void setEaseTimes(Real easeInTime, Real easeOutTime);

		/// Evaluate the ease-in/ease-out function at time \param param.
		/**
		 * \param param is normalized from 0 to 1.
		 */
		Real operator ()(Real param) const;

	private:
		Real m_in, m_out;
};

// ============================================================================
