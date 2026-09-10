#include "W3DDevice/GameClient/W3DRenderServices.h"
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

// FILE: W3DTracerDraw.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Tracer drawing
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <array>
#include <cmath>


#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/Module/W3DTracerDraw.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"


import Graphics.Scene.Lines.Tracer;
import Graphics.Scene.OrderedDraws;


class W3DTracerRenderObject final : public W3DRenderObject
{
public:
	W3DTracerRenderObject(float length, float width, const RGBColor &color, float opacity)
	{
		m_description = {length, width, {color.red, color.green, color.blue, 1.0f}};
		m_opacity = opacity;
	}

	W3DTracerRenderObject(const W3DTracerRenderObject &source)
		: W3DRenderObject(source), m_description(source.m_description), m_opacity(source.m_opacity)
	{
	}

	~W3DTracerRenderObject() override
	{
		m_graphics.Release(Graphics::Get_Prop_Renderer());
	}

	W3DRenderObject *Clone() const override
	{
		return NEW W3DTracerRenderObject(*this);
	}

	int Class_ID() const override
	{
		return CLASSID_UNKNOWN;
	}

	int Get_Num_Polys() const override
	{
		return static_cast<int>(Graphics::TracerIndexCount / 3);
	}

	void Render(W3DRenderContext &rinfo) override
	{
		if (!Is_Not_Hidden_At_All())
			return;

		// Alpha tracers retain the authored layer used by the old static-sort
		// path. The generic extractor preserves RenderObj hooks and performs the
		// immediate graphics submission while the scene queue is draining.
		if (m_opacity < 1.0f && Graphics::Get_Scene_Draw_Queue().Is_Enabled()) {
			if (Graphics::Get_Scene_Draw_Queue().Enqueue<Extract_Ordered_Draw>(
				Graphics::TracerAuthoredLayer, *this))
				return;
		}

		Submit(rinfo);
	}

	bool Submit(W3DRenderContext &rinfo)
	{
		if (!m_graphics.Set_Description(Graphics::Get_Prop_Renderer(), m_description))
			return false;
		Matrix3D view;
		Matrix4x4 projection;
		rinfo.Camera.Get_View_Matrix(&view);
		rinfo.Camera.Get_Backend_Projection_Matrix(&projection);
		const Matrix4x4 view_matrix(view);
		const Matrix4x4 view_projection = projection * view_matrix;
		const Matrix4x4 world_matrix(Get_Transform());

		Graphics::TracerDrawData data;
		Copy_Matrix(data.view_projection, view_projection);
		Copy_Matrix(data.view, view_matrix);
		Copy_Matrix(data.world, world_matrix);
		const Vector3 camera = rinfo.Camera.Get_Position();
		data.camera_position = {camera.X, camera.Y, camera.Z, 1.0f};
		data.camera_depth = {view_matrix[2][0], view_matrix[2][1], view_matrix[2][2], view_matrix[2][3]};
		data.opacity = m_opacity;
		data.front_counter_clockwise = !Get_W3D_Render_Services().Is_Reflection_Render_Pass();
		return m_graphics.Submit(Graphics::Get_Prop_Renderer(), Graphics::Get_Prop_Submission(), data);
	}

	void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override
	{
		const float half_length = m_description.length * 0.5f;
		const float half_width = m_description.width * 0.5f;
		sphere.Center.Set(half_length, 0.0f, 0.0f);
		sphere.Radius = std::sqrt(half_length * half_length + 2.0f * half_width * half_width);
	}

	void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override
	{
		box.Center.Set(m_description.length * 0.5f, 0.0f, 0.0f);
		box.Extent.Set(m_description.length * 0.5f, m_description.width * 0.5f,
			m_description.width * 0.5f);
	}

	bool Set_Description(float length, float width, const RGBColor &color)
	{
		const Graphics::TracerDescription description{
			length, width, {color.red, color.green, color.blue, 1.0f}};
		if (!m_graphics.Set_Description(Graphics::Get_Prop_Renderer(), description))
			return false;
		m_description = description;
		Invalidate_Cached_Bounding_Volumes();
		return true;
	}

	void Set_Opacity(float opacity)
	{
		m_opacity = opacity;
	}

private:
	static void Copy_Matrix(std::array<float, 16> &destination, const Matrix4x4 &source)
	{
		for (unsigned row = 0; row < 4; ++row)
			for (unsigned column = 0; column < 4; ++column)
				destination[row * 4 + column] = source[row][column];
	}

	Graphics::TracerRenderer m_graphics;
	Graphics::TracerDescription m_description{};
	float m_opacity = 1.0f;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTracerDraw::W3DTracerDraw( Thing *thing, const ModuleData* moduleData ) : DrawModule( thing, moduleData )
{

	// set opacity
	m_opacity = 1.0f;
	m_length = 20.0f;
	m_width = 0.5f;
	m_color.red = 0.9f;
	m_color.green = 0.8f;
	m_color.blue = 0.7f;
	m_speedInDistPerFrame = 1.0f;
	m_theTracer = nullptr;

}

void W3DTracerDraw::createTracer(const Matrix3D& transform)
{
	if (m_theTracer != nullptr)
		return;

	m_theTracer = NEW W3DTracerRenderObject(m_length, m_width, m_color, m_opacity);
	W3DDisplay::m_3DScene->Add_Render_Object(m_theTracer);
	m_theTracer->Set_Transform(transform);
}



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTracerDraw::setTracerParms(Real speed, Real length, Real width, const RGBColor& color, Real initialOpacity)
{
	m_speedInDistPerFrame = speed;
	m_length = length;
	m_width = width;
	m_color = color;
	m_opacity = initialOpacity;
	if (m_theTracer)
	{
		m_theTracer->Set_Description(m_length, m_width, m_color);
		m_theTracer->Set_Opacity( m_opacity );
		m_theTracer->Set_Transform( *getDrawable()->getTransformMatrix() );
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTracerDraw::~W3DTracerDraw()
{

	// remove tracer from the scene and delete
	if( m_theTracer )
	{
		W3DDisplay::m_3DScene->Remove_Render_Object( m_theTracer );
		REF_PTR_RELEASE( m_theTracer );
	}
}

//-------------------------------------------------------------------------------------------------
void W3DTracerDraw::reactToTransformChange( const Matrix3D *oldMtx,
																							 const Coord3D *oldPos,
																							 Real oldAngle )
{
	if( m_theTracer )
		m_theTracer->Set_Transform( *getDrawable()->getTransformMatrix() );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTracerDraw::doDrawModule(const Matrix3D* transformMtx)
{
    if (!m_theTracer) createTracer(*transformMtx);
    const UnsignedInt expiration = getDrawable()->getExpirationDate();
    if (expiration != 0) {
        const UnsignedInt frame = TheGameLogic->getFrame();
        m_opacity = expiration > frame ? m_opacity-m_opacity/(expiration-frame) : 0.0f;
        m_theTracer->Set_Opacity(m_opacity);
    }
    if (m_speedInDistPerFrame != 0.0f) {
        Matrix3D position = m_theTracer->Get_Transform();
        position.Translate(Vector3(m_speedInDistPerFrame,0,0));
        m_theTracer->Set_Transform(position);
    }
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DTracerDraw::crc( Xfer *xfer )
{

	// extend base class
	DrawModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DTracerDraw::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DrawModule::xfer( xfer );

	// no data to save here, nobody will ever notice

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DTracerDraw::loadPostProcess()
{

	// extend base class
	DrawModule::loadPostProcess();

}
