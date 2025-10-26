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

// FILE: W3DSmudge.h /////////////////////////////////////////////////////////

#pragma once

#include "GameClient/Smudge.h"
#include "sharebuf.h"

class SmudgeGroupClass;	//forward reference.
class Vector3;
class Vector4;
class TextureClass;
class RenderInfoClass;
class DX8IndexBufferClass;

//#define USE_COPY_RECTS	1	//this was the old method that didn't render to texture. Just copied backbuffer into texture. Slow on Nvidia.

class W3DSmudgeManager : public SmudgeManager
{
public:
	W3DSmudgeManager( void );
	virtual ~W3DSmudgeManager();

	virtual void init(void);
	virtual void reset (void);

	void render (RenderInfoClass &rinfo);
	void ReleaseResources(void);
	void ReAcquireResources(void);

private:
	Bool testHardwareSupport(void);		///<test if video card supports the effect.

	enum { MAX_POINTS_PER_GROUP = 512 };

	SmudgeGroupClass *m_smudgeGroup;							///< the point group that contains all of the particles
	ShareBufferClass<Vector3> *m_posBuffer;			///< array of particle positions
	ShareBufferClass<unsigned int> *m_RGBABuffer;		///< array of particle color and alpha
	ShareBufferClass<float> *m_sizeBuffer;			///< array of particle sizes

#ifdef USE_COPY_RECTS
	TextureClass *m_backgroundTexture;
#endif
	DX8IndexBufferClass	*m_indexBuffer;
	Int m_backBufferWidth;
	Int m_backBufferHeight;
};
