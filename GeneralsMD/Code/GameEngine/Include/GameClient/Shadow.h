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

// FILE: Shadow.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Modified: Mark Wilczynski, February 2002
// Desc:   Shadow descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SHADOW_H_
#define __SHADOW_H_

//
// skeleton definition of shadow types
//

// shadow bit flags, keep this in sync with TheShadowNames
enum ShadowType CPP_11(: Int)
{
	SHADOW_NONE											=	0x00000000,
	SHADOW_DECAL										=	0x00000001,		//shadow decal applied via modulate blend
	SHADOW_VOLUME										=	0x00000002,
	SHADOW_PROJECTION								=	0x00000004,
	SHADOW_DYNAMIC_PROJECTION				= 0x00000008,		//extra setting for shadows which need dynamic updates
	SHADOW_DIRECTIONAL_PROJECTION		= 0x00000010,		//extra setting for shadow decals that rotate with sun direction
	SHADOW_ALPHA_DECAL							= 0x00000020,		//not really for shadows but for other decal uses. Alpha blended.
	SHADOW_ADDITIVE_DECAL						= 0x00000040		//not really for shadows but for other decal uses. Additive blended.
};
#ifdef DEFINE_SHADOW_NAMES
static const char* const TheShadowNames[] =
{
	"SHADOW_DECAL",
	"SHADOW_VOLUME",
	"SHADOW_PROJECTION",
	"SHADOW_DYNAMIC_PROJECTION",
	"SHADOW_DIRECTIONAL_PROJECTION",
	"SHADOW_ALPHA_DECAL",
	"SHADOW_ADDITIVE_DECAL",
	NULL
};
#endif  // end DEFINE_SHADOW_NAMES

#define MAX_SHADOW_LIGHTS 1	//maximum number of shadow casting light sources in scene - support for more than 1 has been dropped from most code.

class RenderObjClass; //forward reference
class RenderCost;	//forward reference

//Interface to all shadow objects.
class Shadow
{

public:

		struct	ShadowTypeInfo
		{
				char	m_ShadowName[64];	//when set, overrides the default model shadow (used mostly for Decals).
				ShadowType m_type;			//type of shadow
				Bool	allowUpdates;			//whether to update the shadow image when object/light moves.
				Bool	allowWorldAlign;	//whether to align shadow to world geometry or draw as horizontal decal.
				Real	m_sizeX;			//world size of decal projection
				Real	m_sizeY;			//world size of decal projection
				Real	m_offsetX;			//world shift along x axis
				Real	m_offsetY;			//world shift along y axis
		};

		Shadow(void) : m_diffuse(0xffffffff), m_color(0xffffffff), m_opacity (0x000000ff), m_localAngle(0.0f) {}

		///<if this is set, then no render will occur, even if enableShadowRender() is enabled. Used by Shroud.
		void enableShadowInvisible(Bool isEnabled);
		void enableShadowRender(Bool isEnabled);
		Bool isRenderEnabled(void) {return m_isEnabled;}
		Bool isInvisibleEnabled(void) {return m_isInvisibleEnabled;}
		virtual void release(void)=0;	///<release this shadow from suitable manager.
		void setOpacity(Int value); ///<adjust opacity of decal/shadow
		void setColor(Color value);///<adjust ARGB color of decal/shadow
		void setAngle(Real angle);		///<adjust orientation around z-axis
		void setPosition(Real x, Real y, Real z);

		void setSize(Real sizeX, Real sizeY)
		{
			m_decalSizeX = sizeX;
			m_decalSizeY = sizeY;

			if (sizeX == 0)
				m_oowDecalSizeX = 0;
			else
				m_oowDecalSizeX = 1.0f/sizeX ;

			if (sizeY == 0)
				m_oowDecalSizeY = 0;
			else
				m_oowDecalSizeY = 1.0f/sizeY ;

		};

		#if defined(RTS_DEBUG)
		virtual void getRenderCost(RenderCost & rc) const = 0;
		#endif

protected:

		Bool m_isEnabled;	/// toggle to turn rendering of this shadow on/off.
		Bool m_isInvisibleEnabled;	/// if set, overrides and causes no rendering.
		UnsignedInt m_opacity;		///< value between 0 (transparent) and 255 (opaque)
		UnsignedInt m_color;		///< color in ARGB format. (Alpha is ignored).
		ShadowType m_type;		/// type of projection
		Int		m_diffuse;		/// diffuse color used to tint/fade shadow.
		Real	m_x,m_y,m_z;	/// world position of shadow center when not bound to robj/drawable.
		Real	m_oowDecalSizeX;		/// 1/(world space extent of texture in x direction)
		Real	m_oowDecalSizeY;		/// 1/(world space extent of texture in y direction)
		Real	m_decalSizeX;		/// 1/(world space extent of texture in x direction)
		Real	m_decalSizeY;		/// 1/(world space extent of texture in y direction)
		Real	m_localAngle;		/// yaw or rotation around z-axis of shadow image when not bound to robj/drawable.
};





inline void Shadow::enableShadowRender(Bool isEnabled)
{
	m_isEnabled=isEnabled;
}

inline void Shadow::enableShadowInvisible(Bool isEnabled)
{
	m_isInvisibleEnabled=isEnabled;
}

///@todo: Pull these out so casting, etc. is only done for visible decals.
inline void Shadow::setOpacity(Int value)
{
	m_opacity=value;

	if (m_type & SHADOW_ALPHA_DECAL)
	{
		m_diffuse = (m_color & 0x00ffffff) + (value << 24);
//		m_diffuse = m_color | (value << 24);ML changed
	}
	else
	{
		if (m_type & SHADOW_ADDITIVE_DECAL)
		{
			Real fvalue=(Real)m_opacity/255.0f;
			m_diffuse=REAL_TO_INT(((Real)(m_color & 0xff) * fvalue))
					|REAL_TO_INT(((Real)((m_color >> 8) & 0xff) * fvalue))
					|REAL_TO_INT(((Real)((m_color >> 16) & 0xff) * fvalue));
		}
	}
}

inline void Shadow::setColor(Color value)
{
	m_color = value & 0x00ffffff;	//filter out alpha

	if (m_type & SHADOW_ALPHA_DECAL)
	{
		m_diffuse=m_color | (m_opacity << 24);
	}
	else
	{
		if (m_type & SHADOW_ADDITIVE_DECAL)
		{
			Real fvalue=(Real)m_opacity/255.0f;
			m_diffuse=REAL_TO_INT(((Real)(m_color & 0xff) * fvalue))
					|REAL_TO_INT(((Real)((m_color >> 8) & 0xff) * fvalue))
					|REAL_TO_INT(((Real)((m_color >> 16) & 0xff) * fvalue));
		}
	}
}

inline void Shadow::setPosition(Real x, Real y, Real z)
{
	m_x=x; m_y=y; m_z=z;
}

inline void Shadow::setAngle(Real angle)
{
	m_localAngle=angle;
}

class Drawable;	//forward ref.

class ProjectedShadowManager
{
public:
	virtual ~ProjectedShadowManager() { };
	virtual Shadow	*addDecal(RenderObjClass *, Shadow::ShadowTypeInfo *shadowInfo)=0;	///<add a non-shadow decal
	virtual Shadow	*addDecal(Shadow::ShadowTypeInfo *shadowInfo)=0;	///<add a non-shadow decal which does not follow an object.
};

extern ProjectedShadowManager *TheProjectedShadowManager;

#endif // __SHADOW_H_

