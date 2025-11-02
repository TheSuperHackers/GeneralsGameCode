/*
**	Command & Conquer Generals(tm)
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
//  (c) 2025																																//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DBrightnessFilter.h /////////////////////////////////////////////////
//
// Brightness adjustment post-processing filter
// Implements GenTool-style brightness control (-128 to +256 range)
// - Range 1-128: lifts black level linearly
// - Range 129-256: lifts brightness while preserving black level
//
// Author: @bobtista, November 2025
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "W3DShaderManager.h"

/*=========  ScreenBrightnessFilter	=============================================================*/
/// Applies brightness/gamma adjustment to the viewport using pixel shaders
class ScreenBrightnessFilter : public W3DFilterInterface
{
public:
	virtual Int set(FilterModes mode);		///<setup shader for the specified rendering pass.
	virtual Int init(void);			///<perform any one time initialization and validation
	virtual void reset(void);		///<do any custom resetting necessary to bring W3D in sync.
	virtual Int shutdown(void);		///<release resources used by shader
	virtual Bool preRender(Bool &skipRender, CustomScenePassModes &scenePassMode); ///< Set up at start of render.
	virtual Bool postRender(FilterModes mode, Coord2D &scrollDelta, Bool &doExtraRender); ///< Called after render.
	virtual Bool setup(FilterModes mode); ///< Called when the filter is started.

	static void setBrightnessValue(Int value);	///< Set brightness value (-128 to +256, GenTool range)
	static Int getBrightnessValue();	///< Get current brightness value

protected:
	static DWORD m_pixelShaderHandle;	///< Handle to pixel shader
	static Int m_brightnessValue;		///< Current brightness value (-128 to +256)
	static Bool m_isEnabled;			///< Whether brightness adjustment is enabled (disabled when value is 0)
};

/*=========  ScreenBrightnessFilterFixedFunction	=============================================================*/
/// Fallback brightness filter using fixed-function pipeline for cards without pixel shader support
class ScreenBrightnessFilterFixedFunction : public W3DFilterInterface
{
public:
	virtual Int set(FilterModes mode);
	virtual Int init(void);
	virtual void reset(void);
	virtual Int shutdown(void);
	virtual Bool preRender(Bool &skipRender, CustomScenePassModes &scenePassMode);
	virtual Bool postRender(FilterModes mode, Coord2D &scrollDelta, Bool &doExtraRender);
	virtual Bool setup(FilterModes mode);

	static void setBrightnessValue(Int value);
	static Int getBrightnessValue();

protected:
	static Int m_brightnessValue;
	static Bool m_isEnabled;
};

