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

// FILE: W3DPropBuffer.h //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  W3DPropBuffer.h
//
// Created:    John Ahlquist, May 2001
//
// Desc:       Draw buffer to handle all the props in a scene.
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef __W3DPROP_BUFFER_H_
#define __W3DPROP_BUFFER_H_

//-----------------------------------------------------------------------------
//           Includes
//-----------------------------------------------------------------------------
#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "Common/AsciiString.h"
#include "Common/GlobalData.h"

//-----------------------------------------------------------------------------
//           Forward References
//-----------------------------------------------------------------------------
class RenderInfoClass;
class LightClass;
class W3DPropDrawModuleData;
class W3DShroudMaterialPassClass;
class GeometryInfo;

//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------

/// The individual data for a prop.
typedef struct {
	RenderObjClass *m_robj;			///< Render object for this kind of prop.
	Int					id;
	Coord3D			location;			///< Drawing location
	Int					propType;					///< Type of prop. Index into m_propTypes
	ObjectShroudStatus ss;
	Bool				visible;					///< Visible flag, updated each frame.
	SphereClass bounds;				///< Bounding sphere for culling to set the visible flag.
} TProp;

/// The individual data for a prop type.
typedef struct {
	RenderObjClass *m_robj;			///< Render object for this kind of prop.
	AsciiString			m_robjName;	///< Name of the render obj.
	SphereClass			m_bounds;		///< Bounding boxes for the base prop models.
} TPropType;

//
// W3DPropBuffer: Draw buffer for the props.
//
//
class W3DPropBuffer : Snapshot
{
friend class BaseHeightMapRenderObjClass;


public:

	W3DPropBuffer(void);
	~W3DPropBuffer(void);
	/// Add a prop at location.  Name is the w3d model name.
	void addProp(Int id, Coord3D location, Real angle, Real scale, const AsciiString &modelName);
	/// Remove a prop.
	void removeProp(Int id);
	/// Remove a prop.
	Bool updatePropPosition(Int id, const Coord3D &location, Real angle, Real scale);
	/// Let us know that the shroud has changed.
	void notifyShroudChanged(void);

	void removePropsForConstruction(
		const Coord3D* pos,
		const GeometryInfo& geom,
		Real angle
	);

	/// Add a type of prop.  Name is the w3d model name.
	Int addPropType(const AsciiString &modelName);
	/// Empties the prop buffer.
	void clearAllProps(void);
	/// Draws the props.  Uses camera for culling.
	void drawProps(RenderInfoClass &rinfo);
	/// Called when the view changes, and sort key needs to be recalculated.
	void doFullUpdate(void) {m_doCull = true;};

protected:
	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

protected:
	enum { MAX_PROPS=4000};
	enum {MAX_TYPES = 64};

	TProp	m_props[MAX_PROPS];			///< The prop buffer.  All props are stored here.
	Int			m_numProps;						///< Number of props in m_props.
	Bool		m_anythingChanged;	///< Set to true if visibility or sorting changed.
	Bool		m_initialized;		///< True if the subsystem initialized.
	Bool		m_doCull;
	TPropType m_propTypes[MAX_TYPES];	///< Info about a kind of prop.
	Int			m_numPropTypes;						///< Number of entries in m_propTypes.
	W3DShroudMaterialPassClass	*m_propShroudMaterialPass;	///< Custom render pass which applies shrouds to objects

	LightClass *m_light;

	void cull(CameraClass * camera);						 ///< Culls the props.
};

#endif  // end __W3DPROP_BUFFER_H_
