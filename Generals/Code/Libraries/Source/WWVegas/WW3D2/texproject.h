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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/texproject.h                           $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/23/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef TEXPROJECT_H
#define TEXPROJECT_H

#include "always.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "obbox.h"
#include "matpass.h"
#include "matrixmapper.h"
#include "cullsys.h"
#include "multilist.h"
#include "projector.h"


class SpecialRenderInfoClass;
class RenderObjClass;
class MaterialPassClass;
class SurfaceClass;



/**
** TexProjectClass
** This class is used to project textures onto object in the world.  It contains
** a pointer to the texture to be projected, a bounding volume for the projection,
** and a material pass which can perform the projection.
**
** Design Goals:
** - Texture Projectors should be easy to place in the world and move around.
**   solution: Store the bounding volume and any other parameters in a coordinate
**   system local to the projector.  Update the world-space cache whenever needed.
**   The automatically generated shadow texture projectors are a special case subset
**   of the general texture projection code.  For this reason, a local bounding
**   volume will be stored and its world-space equivalent will be updated whenever
**   needed.
**
** - Texture projectors need to be compatible with the culling systems.
**   solution: inherit the Cullable interface.
**
** - Texture projectors need to be quickly pulled in and out of many lists during
**   the rendering process.
**   solution: list system similar to PhysListClass?  I really need to templatize this.
**   - done:  MultiListClass!
**
** - Dynamic Texture projectors need to be updated in stages.  Update/recompute only the
**   bounding volume first, then defer computing the actual texture until it is
**   determined that the volume falls into the frustum and is applied to at least one
**   object.
**   solution: Code the bounding volume/projection paramter generation separate from
**   the texture generation code.  A derived texture projection object. Let texture
**   projectors know about the object they are projecting so that they can have that
**   object rendered from the desired viewpoint.  Need a 'Dirty' flag and a pointer to
**   a 'parent' object.  Perhaps separate this code into a ShadowProjectClass since shadows
**   are the only case I can think of that need the ability to re-render their 'parent'
**   object.  Everything else should be a static texture-map either generated 'artistically'
**   or generated off-line.
**
*/
class TexProjectClass : public ProjectorClass, public CullableClass, public MultiListObjectClass
{
public:

	TexProjectClass(void);
	virtual ~TexProjectClass(void);

	/*
	** Material settings
	*/
	void						Set_Texture_Size(int size);
	int						Get_Texture_Size(void);
	void						Init_Multiplicative(void);
	void						Init_Additive(void);
	void						Set_Intensity(float intensity,bool immediate = false);	// 1.0 = on 100%, 0.0 = 'off'
	float						Get_Intensity(void);
	bool						Is_Intensity_Zero(void);
	void						Set_Attenuation(float attenuation);								// 1.0 = on, 0.0 = off
	float						Get_Attenuation(void);
	void						Enable_Attenuation(bool onoff);
	bool						Is_Attenuation_Enabled(void);
	MaterialPassClass *	Peek_Material_Pass(void);

	/*
	** Options
	*/
	void						Enable_Affect_Dynamic_Objects(bool onoff)				{ Set_Flag(AFFECT_DYNAMIC_OBJS,onoff); }
	bool						Is_Affect_Dynamic_Objects_Enabled(void)				{ return Get_Flag(AFFECT_DYNAMIC_OBJS); }
	void						Enable_Affect_Static_Objects(bool onoff)				{ Set_Flag(AFFECT_STATIC_OBJS,onoff); }
	bool						Is_Affect_Static_Objects_Enabled(void)					{ return Get_Flag(AFFECT_STATIC_OBJS); }
	void						Enable_Depth_Gradient(bool onoff);
	bool						Is_Depth_Gradient_Enabled(bool onoff);

	/*
	** Manual initialization of a TexProjectClass
	** 1 - call ProjectorClass::Set_Transform
	** 2 - call Set_Projection_Perspective -or- Set_Projection_Ortho
	** 3 - call Set_Texture.
	*/
	virtual void			Set_Perspective_Projection(float hfov,float vfov,float znear,float zfar);
	virtual void			Set_Ortho_Projection(float xmin,float xmax,float ymin,float ymax,float znear,float zfar);

	void						Set_Texture(TextureClass * texture);
	TextureClass *			Get_Texture(void) const;
	TextureClass *			Peek_Texture(void) const;

	/*
	** Automatic initialization of a TexProjectClass.
	** First set up your projection parameters, give the projector a render target, then call Compute_Texture
	*/
	bool						Compute_Perspective_Projection(RenderObjClass * obj,const Vector3 & lightpos,float znear=-1.0f,float zfar=-1.0f);
	bool						Compute_Perspective_Projection(const AABoxClass & obj_box,const Matrix3D & tm,const Vector3 & lightpos,float znear=-1.0f,float zfar=-1.0f);

	bool						Compute_Ortho_Projection(RenderObjClass * obj,const Vector3 & lightdir,float znear=-1.0f,float zfar=-1.0f);
	bool						Compute_Ortho_Projection(const AABoxClass & obj_box,const Matrix3D & tm,const Vector3 & lightdir,float znear=-1.0f,float zfar=-1.0f);

	bool						Needs_Render_Target(void);
	void						Set_Render_Target(TextureClass * render_target);
	TextureClass *			Peek_Render_Target(void);

	bool						Compute_Texture(RenderObjClass * model,SpecialRenderInfoClass * context);

	/*
	** Prep for rendering, called by the scene prior to usage.
	*/
	virtual void			Pre_Render_Update(const Matrix3D & camera);

	/*
	** virtual interface for getting the pointer of the object that generated this shadow.
	** defaults to returning NULL.  This is implemented by some derived classes and used by
	** the system to prevent a projection from being applied to the object that generated
	** the projection...
	** (gth) feels kludgy, this got a little messy when I moved this code into WW3D from WWPhys
	*/
	virtual void *			Get_Projection_Object_ID(void) const { return NULL; }

protected:

	void						Set_Flag(uint32 flag,bool onoff);
	bool						Get_Flag(uint32 flag) const;
	virtual void			Update_WS_Bounding_Volume(void);
	void						Configure_Camera(CameraClass & camera);

	enum FlagsType
	{
		PERSPECTIVE				= 0x00000001,		// PERSPECTIVE or ORTHO
		ADDITIVE					= 0x00000002,		// ADDITIVE or MULTIPLICATIVE
		TEXTURE_DIRTY			= 0x00000004,		// used by derived DynTexProjectClass
		VOLATILE					= 0x00000008,		// this is a volatile texture.
		ATTENUATE				= 0x00000010,		// this projector should be attenuated based on distance to viewer
		AFFECT_DYNAMIC_OBJS	= 0x00000020,		// this projector affects dynamic objects
		AFFECT_STATIC_OBJS	= 0x00000040,		// this projector affects static objects
		USE_DEPTH_GRADIENT	= 0x00000080,		// fade the projection as a function of depth
		HAS_RENDER_TARGET		= 0x00000100,		// the texture we have can be a render target
		SIZE_MASK				= 0xFFF00000,		// desired texture size stored in upper 3 nibbles
		SIZE_SHIFT				= 20,

		DEFAULT_FLAGS			= ATTENUATE | AFFECT_DYNAMIC_OBJS | AFFECT_STATIC_OBJS | (64<<SIZE_SHIFT)
	};

	uint32						Flags;

	/*
	** Intensity Controls
	*/
	float							DesiredIntensity;					// last input desired intensity.
	float							Intensity;							// basic shadow intensity.  value between 0 and 1
	float							Attenuation;						// attenuation factor based on distance from camera.  value between 0 and 1

	/*
	** Material pass to be added to any object that intersects the volume
	*/
	MaterialPassClass *		MaterialPass;
	MatrixMapperClass *		Mapper1;
	TextureClass *				RenderTarget;

	/*
	** I have to remember all of these values so that I can properly initialize a CameraClass
	** when we do render-to-texture.  Some day we will merge CameraClass and ProjectorClass.
	*/
	float							HFov;				// horizontal fov (for perspective mode)
	float							VFov;				// vertical fov (for perspective mode)
	float							XMin;				// left x clip plane (for ortho)
	float							XMax;				// right x clip plane (for ortho)
	float							YMin;				// bottom y clip plane (for ortho)
	float							YMax;				// top y clip plane (for ortho)
	float							ZNear,ZFar;		// z clip planes (for both modes)
};


/*
** Texture Projector Lists
*/
typedef RefMultiListClass<TexProjectClass>		TexProjListClass;
typedef RefMultiListIterator<TexProjectClass>	TexProjListIterator;


#endif

