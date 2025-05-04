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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/ringobj.h                            $*
 *                                                                                             *
 *                       Author:: Jason Andersen                                               *
 *                                                                                             *
 *                     $Modtime:: 3/23/01 7:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef RINGOBJ_H
#define RINGOBJ_H

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "shader.h"
#include "proto.h"
#include "obbox.h"
#include "quat.h"
#include "vector3.h"
#include "vector2.h"
#include "prim_anim.h"

class VertexMaterialClass;

typedef LERPAnimationChannelClass<Vector3>	RingColorChannelClass;
typedef LERPAnimationChannelClass<float>		RingAlphaChannelClass;
typedef LERPAnimationChannelClass<Vector2>	RingScaleChannelClass;


/**
** W3dRingStruct
** This structure is used to define a ring object in the W3D file format.
*/
struct W3dRingStruct
{
	uint32				Version;						// file format version
	uint32				Attributes;					// box attributes (above #define's)
	char					Name[2*W3D_NAME_LEN];	// name is in the form <containername>.<boxname>

	W3dVectorStruct	Center;						// center of the box
	W3dVectorStruct	Extent;						// extent of the box

	float					AnimDuration;				// Animation time (in seconds)

	W3dVectorStruct	DefaultColor;
	float					DefaultAlpha;
	Vector2				DefaultInnerScale;
	Vector2				DefaultOuterScale;

	Vector2				InnerExtent;
	Vector2				OuterExtent;

	char					TextureName[2*W3D_NAME_LEN];
	W3dShaderStruct	Shader;
	int					TextureTileCount;

	//
	// Note this structure is followed by a series of
	// W3dRingKeyArrayStruct structures which define the
	// variable set of keyframes for each channel
};

// Note: RING_NUM_LOD does not include the NULL LOD.
#define RING_NUM_LOD	(20)
#define RING_LOWEST_LOD (10)
#define RING_HIGHEST_LOD (50)

/*
** RingRenderObjClass: Procedurally generated render rings
**
*/
class RingRenderObjClass : public RenderObjClass
{	

public:

	// These are bit masks, so they should enum 1,2,4,8,10,20,40 etc...
	enum	RingFlags {
		USE_CAMERA_ALIGN	= 0x00000001,
		USE_ANIMATION_LOOP= 0x00000002,
	};

	RingRenderObjClass(void);
	RingRenderObjClass(const W3dRingStruct & def);
	RingRenderObjClass(const RingRenderObjClass & src);
	RingRenderObjClass & operator = (const RingRenderObjClass &);
	~RingRenderObjClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface 
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);
	virtual void 					Set_Transform(const Matrix3D &m); 
	virtual void 					Set_Position(const Vector3 &v);
   virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass	& sphere) const;
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

	virtual void					Prepare_LOD(CameraClass &camera);
	virtual void					Increment_LOD(void);
	virtual void					Decrement_LOD(void);
	virtual float					Get_Cost(void) const;
	virtual float					Get_Value(void) const;
	virtual float					Get_Post_Increment_Value(void) const;
	virtual void					Set_LOD_Level(int lod);
	virtual int						Get_LOD_Level(void) const;
	virtual int						Get_LOD_Count(void) const;
	virtual void					Set_LOD_Bias(float bias)	{ LODBias = MAX(bias, 0.0f); }
	virtual int						Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const;

	virtual void					Scale(float scale);
	virtual void					Scale(float scalex, float scaley, float scalez);

	virtual void					Set_Hidden(int onoff)				{ RenderObjClass::Set_Hidden (onoff); Update_On_Visibilty (); }
	virtual void					Set_Visible(int onoff)				{ RenderObjClass::Set_Visible (onoff); Update_On_Visibilty (); }
	virtual void					Set_Animation_Hidden(int onoff)	{ RenderObjClass::Set_Animation_Hidden (onoff); Update_On_Visibilty (); }
	virtual void					Set_Force_Visible(int onoff)		{ RenderObjClass::Set_Force_Visible (onoff); Update_On_Visibilty (); }

	const	AABoxClass	&			Get_Box(void);

	virtual int				  		Get_Num_Polys(void) const;
	virtual const char		  *Get_Name(void) const;
	virtual void					Set_Name(const char * name);

	unsigned int					Get_Flags(void) {return Flags;}
	void								Set_Flags(unsigned int flags) { Flags = flags; }
	void								Set_Flag(unsigned int flag, bool onoff) { Flags &= (~flag); if (onoff) Flags |= flag; }

	// Animation access
	bool								Is_Animating (void)		{ return IsAnimating; }
	void								Start_Animating (void)	{ IsAnimating = true; anim_time = 0; }
	void								Stop_Animating (void)	{ IsAnimating = false; anim_time = 0; }

	// Texture tiling access
	int								Get_Texture_Tiling (void) const		{ return TextureTileCount; }
	void								Set_Texture_Tiling (int count)		{ TextureTileCount = count; }

	// Current state access
	void								Set_Color(const Vector3 & color)			{ Color = color; }
	void								Set_Alpha(float alpha)						{ Alpha = alpha; }
	void								Set_Inner_Scale(const Vector2 & scale) { InnerScale = scale; }
	void								Set_Outer_Scale(const Vector2 & scale) { OuterScale = scale; }

	const Vector3 &				Get_Color(void) const			{ return Color; }
	float								Get_Alpha(void) const			{ return Alpha; }
	const Vector2 &				Get_Inner_Scale(void) const	{ return InnerScale; }
	const Vector2 &				Get_Outer_Scale(void) const	{ return OuterScale; }

	Vector3							Get_Default_Color(void) const;
	float								Get_Default_Alpha(void) const;
	Vector2							Get_Default_Inner_Scale(void) const;
	Vector2							Get_Default_Outer_Scale(void) const;

	// Size/position methods
	const Vector2 &				Get_Inner_Extent (void) const						{ return InnerExtent; }
	const Vector2 &				Get_Outer_Extent (void) const						{ return OuterExtent; }
	void								Set_Inner_Extent (const Vector2 &extent);
	void								Set_Outer_Extent (const Vector2 &extent);
	void								Set_Local_Center_Extent(const Vector3 & center,const Vector3 & extent);
	void								Set_Local_Min_Max(const Vector3 & min,const Vector3 & max);

	// Texture access
	void							  	Set_Texture(TextureClass *tf);
	TextureClass	*				Peek_Texture(void)					{ return RingTexture; }
	ShaderClass &					Get_Shader(void)						{ return RingShader; }
	void								Set_Shader(ShaderClass &shader)	{ RingShader = shader; }

	// Animation Control
	float								Get_Animation_Duration(void) const		{ return AnimDuration; }
	void								Set_Animation_Duration(float time)		{ AnimDuration = time; Restart_Animation(); }
	void								Restart_Animation (void)					{ anim_time = 0; }

	// Animatable channel access
	RingColorChannelClass &			Get_Color_Channel (void)				{ return ColorChannel; }
	const RingColorChannelClass &	Peek_Color_Channel (void)				{ return ColorChannel; }	
	
	RingAlphaChannelClass &			Get_Alpha_Channel (void)				{ return AlphaChannel; }
	const RingAlphaChannelClass &	Peek_Alpha_Channel (void)				{ return AlphaChannel; }

	RingScaleChannelClass &			Get_Inner_Scale_Channel (void)		{ return InnerScaleChannel; }
	const RingScaleChannelClass &	Peek_Inner_Scale_Channel (void)		{ return InnerScaleChannel; }

	RingScaleChannelClass &			Get_Outer_Scale_Channel (void)		{ return OuterScaleChannel; }
	const RingScaleChannelClass &	Peek_Outer_Scale_Channel (void)		{ return OuterScaleChannel; }

	void								Set_Color_Channel (const RingColorChannelClass &data)				{ ColorChannel = data; }
	void								Set_Alpha_Channel (const RingAlphaChannelClass &data)				{ AlphaChannel = data; }
	void								Set_Inner_Scale_Channel (const RingScaleChannelClass &data)		{ InnerScaleChannel = data; }
	void								Set_Outer_Scale_Channel (const RingScaleChannelClass &data)		{ OuterScaleChannel = data; }

protected:
	
	virtual void					update_cached_box(void);
	void								Update_On_Visibilty(void);

	// Initialization stuff
	void								Init_Material (void);
	static void						Generate_Shared_Mesh_Arrays (void);


	// Animation Stuff
	void								animate(void);		// animation update function
	float								anim_time;			// what time in seconds are we in the animation
	float								AnimDuration;
	bool								IsAnimating;

	// LOD Stuff
	void								calculate_value_array(float screen_area, float *values) const;
	float								LODBias;
	int						 		CurrentLOD;
	float								Value[RING_NUM_LOD + 2];// Value array needs # of LODs + 1 (RING_NUM_LOD doesn't include null LOD)

	RingColorChannelClass		ColorChannel;
	RingAlphaChannelClass		AlphaChannel;
	RingScaleChannelClass		InnerScaleChannel;
	RingScaleChannelClass		OuterScaleChannel;

	void						 		update_mesh_data(const Vector3 & center,const Vector3 & extent);
	void						 		render_ring(RenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent);
	void						 		vis_render_ring(SpecialRenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent);

	char						 		Name[2*W3D_NAME_LEN];
	Vector3					 		ObjSpaceCenter;
	Vector3					 		ObjSpaceExtent;

	Vector2							InnerExtent;
	Vector2							OuterExtent;

	int								TextureTileCount;

	// Current State
	Vector3					 		Color;
	float								Alpha;
	Vector2							InnerScale;
	Vector2							OuterScale;

	// Flags
	unsigned int					Flags;

	VertexMaterialClass	  	  *RingMaterial;
	ShaderClass					  	RingShader;
	TextureClass				  *RingTexture;

	AABoxClass						CachedBox;

	// Friend classes
	friend class RingPrototypeClass;
};

inline void RingRenderObjClass::Set_Inner_Extent (const Vector2 &extent)
{
	InnerExtent = extent;
}

inline void RingRenderObjClass::Set_Outer_Extent (const Vector2 &extent)
{
	OuterExtent		= extent;
	ObjSpaceExtent.X = extent.X;
	ObjSpaceExtent.Y = extent.Y;
	ObjSpaceExtent.Z = 0;
	update_cached_box();
}

inline void RingRenderObjClass::Set_Local_Center_Extent(const Vector3 & center,const Vector3 & extent)
{
	ObjSpaceCenter = center;
	ObjSpaceExtent = extent;
	update_cached_box();
}

inline void RingRenderObjClass::Set_Local_Min_Max(const Vector3 & min,const Vector3 & max)
{
	ObjSpaceCenter = (max + min) / 2.0f;
	ObjSpaceExtent = (max - min) / 2.0f;
	update_cached_box();
}

inline const AABoxClass & RingRenderObjClass::Get_Box(void) 
{ 
	Validate_Transform();
	update_cached_box();
	return CachedBox; 
}

/*
** Loader for rings
*/
class RingLoaderClass : public PrototypeLoaderClass
{
public:
	virtual int						Chunk_Type (void)  { return W3D_CHUNK_RING; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};

/*
** Prototype for Ring objects
*/
class RingPrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(RingPrototypeClass)

public:
	RingPrototypeClass (void);
	RingPrototypeClass (RingRenderObjClass *ring);

	virtual const char *			Get_Name(void) const;
	virtual int						Get_Class_ID(void) const;
	virtual RenderObjClass *	Create(void);
	virtual void							DeleteSelf()										{ delete this; }

	bool								Load (ChunkLoadClass &cload);
	bool								Save (ChunkSaveClass &csave);

protected:
	~RingPrototypeClass (void);

private:
	W3dRingStruct					Definition;

	RingColorChannelClass		ColorChannel;
	RingAlphaChannelClass		AlphaChannel;
	RingScaleChannelClass		InnerScaleChannel;
	RingScaleChannelClass		OuterScaleChannel;
};

/*
** Instance of the loader which the asset manager installs
*/
extern RingLoaderClass			_RingLoader;


#endif // RINGOBJ_H

// EOF - ringobj,h


