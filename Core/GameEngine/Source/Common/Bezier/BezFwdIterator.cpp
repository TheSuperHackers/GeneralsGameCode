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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/BezFwdIterator.h"

//-------------------------------------------------------------------------------------------------
BezFwdIterator::BezFwdIterator(): mStep(0), mStepsDesired(0)
{
	mCurrPoint.zero();
	mDDDq.zero();
	mDDq.zero();
	mDq.zero();
}

//-------------------------------------------------------------------------------------------------
BezFwdIterator::BezFwdIterator(Int stepsDesired, const BezierSegment *bezSeg)
{
	mCurrPoint.zero();
	mDDDq.zero();
	mDDq.zero();
	mDq.zero();
	mStepsDesired = stepsDesired;
	mBezSeg = (*bezSeg);
}

//-------------------------------------------------------------------------------------------------
void BezFwdIterator::start()
{
	mStep = 0;

	if (mStepsDesired <= 1)
		return;

	float d	 = 1.0f / (mStepsDesired - 1);
	float d2 = d * d;
	float d3 = d * d2;

	// Convert each coordinate into cubic polynomial coefficients. The forward
	// difference stepper then advances the polynomial without any graphics API
	// types or matrix conventions leaking into the engine.
	const float coefficients[3][3] = {
		{
			- mBezSeg.m_controlPoints[0].x + 3.0f * mBezSeg.m_controlPoints[1].x -
				3.0f * mBezSeg.m_controlPoints[2].x + mBezSeg.m_controlPoints[3].x,
			3.0f * mBezSeg.m_controlPoints[0].x - 6.0f * mBezSeg.m_controlPoints[1].x +
				3.0f * mBezSeg.m_controlPoints[2].x,
			-3.0f * mBezSeg.m_controlPoints[0].x + 3.0f * mBezSeg.m_controlPoints[1].x
		},
		{
			- mBezSeg.m_controlPoints[0].y + 3.0f * mBezSeg.m_controlPoints[1].y -
				3.0f * mBezSeg.m_controlPoints[2].y + mBezSeg.m_controlPoints[3].y,
			3.0f * mBezSeg.m_controlPoints[0].y - 6.0f * mBezSeg.m_controlPoints[1].y +
				3.0f * mBezSeg.m_controlPoints[2].y,
			-3.0f * mBezSeg.m_controlPoints[0].y + 3.0f * mBezSeg.m_controlPoints[1].y
		},
		{
			- mBezSeg.m_controlPoints[0].z + 3.0f * mBezSeg.m_controlPoints[1].z -
				3.0f * mBezSeg.m_controlPoints[2].z + mBezSeg.m_controlPoints[3].z,
			3.0f * mBezSeg.m_controlPoints[0].z - 6.0f * mBezSeg.m_controlPoints[1].z +
				3.0f * mBezSeg.m_controlPoints[2].z,
			-3.0f * mBezSeg.m_controlPoints[0].z + 3.0f * mBezSeg.m_controlPoints[1].z
		}
	};

	mCurrPoint = mBezSeg.m_controlPoints[0];

	int i = 3;
	while (i--) {
		const float a = coefficients[i][0];
		const float b = coefficients[i][1];
		const float c = coefficients[i][2];

		float *pD, *pDD, *pDDD;

		if (i == 2) {
			pD = &mDq.z;
			pDD = &mDDq.z;
			pDDD = &mDDDq.z;
		} else if (i == 1) {
			pD = &mDq.y;
			pDD = &mDDq.y;
			pDDD = &mDDDq.y;
		} else if (i == 0) {
			pD = &mDq.x;
			pDD = &mDDq.x;
			pDDD = &mDDDq.x;
		}

		(*pD) = a * d3 + b * d2 + c * d;
		(*pDD) = 6 * a * d3 + 2 * b * d2;
		(*pDDD) = 6 * a * d3;
	}
}

//-------------------------------------------------------------------------------------------------
Bool BezFwdIterator::done()
{
	return (mStep >= mStepsDesired);
}

//-------------------------------------------------------------------------------------------------
const Coord3D& BezFwdIterator::getCurrent() const
{
	return mCurrPoint;
}

//-------------------------------------------------------------------------------------------------
void BezFwdIterator::next()
{
	mCurrPoint.add(mDq);
	mDq.add(mDDq);
	mDDq.add(mDDDq);

	++mStep;
}

