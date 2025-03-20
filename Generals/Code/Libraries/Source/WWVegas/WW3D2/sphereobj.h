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
 *                     $Archive:: /Commando/Code/ww3d2/sphereobj.h                            $*
 *                                                                                             *
 *                       Author:: Jason Andersen                                               *
 *                                                                                             *
 *                     $Modtime:: 5/05/01 6:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef SPHEREOBJ_H
#define SPHEREOBJ_H

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "shader.h"
#include "proto.h"
#include "obbox.h"
#include	"Vector3i.h"
#include	"quat.h"
#include "prim_anim.h"

class TextureClass;

struct AlphaVectorStruct
{
	AlphaVectorStruct (void)
		:	angle (true),
			intensity (1.0F)		{ }
	AlphaVectorStruct (const AlphaVectorStruct &src) { *this = src; }

	bool operator== (const AlphaVectorStruct &src)	{ return (angle.X == src.angle.X) && (angle.Y == src.angle.Y) && (angle.Z == src.angle.Z) && (intensity == src.intensity); }
	bool operator!= (const AlphaVectorStruct &src)	{ return ! operator== (src); }

	const AlphaVectorStruct &	operator= (const AlphaVectorStruct &src)	{ angle = src.angle; intensity = src.intensity; return *this; }

	Quaternion	angle;
	float			intensity;
};

class AlphaVectorChannel : public PrimitiveAnimationChannelClass<AlphaVectorStruct>
{
public:
	AlphaVectorStruct	Evaluate (float time)
	{
		int key_count				= m_Data.Count ();
		AlphaVectorStruct value	= m_Data[key_count - 1].Get_Value ();

		//
		//	Don't interpolate past the last keyframe
		//
		if (time < m_Data[key_count - 1].Get_Time ()) {

			// Check to see if the last key index is valid
			if (time < m_Data[m_LastIndex].Get_Time ()) {
				m_LastIndex = 0;
			}

			KeyClass *key1 = &m_Data[m_LastIndex];
			KeyClass *key2 = &m_Data[key_count - 1];

			//
			// Search, using last_key as our starting point
			//
			for (int keyidx = m_LastIndex; keyidx < (key_count - 1); keyidx ++) {

				if (time < m_Data[keyidx+1].Get_Time ()) {
					key1 = &m_Data[keyidx];
					key2 = &m_Data[keyidx+1];
					m_LastIndex = keyidx;
					break;
				}
			}

			// Calculate the linear percent between the two keys
			float percent	= (time - key1->Get_Time ()) / (key2->Get_Time () - key1->Get_Time ());

			// Slerp the quaternions and lerp the intensities
			value.intensity = (key1->Get_Value ().intensity + (key2->Get_Value ().intensity - key1->Get_Value ().intensity) * percent);
			Fast_Slerp (value.angle, key1->Get_Value ().angle, key2->Get_Value ().angle, percent);
		}

		return value;
	}
};

struct W3dSphereStruct
{
	uint32				Version;							// file format version
	uint32				Attributes;						// sphere attributes (above #define's)
	char					Name[2*W3D_NAME_LEN];		// name is in the form <containername>.<spherename>
	
	W3dVectorStruct	Center;							// center of the sphere
	W3dVectorStruct	Extent;							// extent of the sphere

	float					AnimDuration;					// Animation time (in seconds)

	W3dVectorStruct	DefaultColor;
	float					DefaultAlpha;
	W3dVectorStruct	DefaultScale;
	AlphaVectorStruct DefaultVector;

	char					TextureName[2*W3D_NAME_LEN];
	W3dShaderStruct	Shader;

	//
	//	Note this structure is followed by a series of
	// W3dSphereKeyArrayStruct structures which define the
	// variable set of keyframes for each channel.
	//
};

typedef LERPAnimationChannelClass<Vector3>	SphereColorChannelClass;
typedef LERPAnimationChannelClass<float>		SphereAlphaChannelClass;
typedef LERPAnimationChannelClass<Vector3>	SphereScaleChannelClass;
typedef AlphaVectorChannel							SphereVectorChannelClass;
//typedef AlphaVectorChannel									;

class VertexMaterialClass;


class SphereMeshClass
{
friend class SphereRenderObjClass;

public:
	// Constructor
	SphereMeshClass(void);
	SphereMeshClass(float radius, int slices, int stacks);
	// Destructor
	~SphereMeshClass(void);

	void	Generate (float radius, int slices, int stacks);
	int	Get_Num_Polys (void) { return face_ct; };
	void	Set_Alpha_Vector (const AlphaVectorStruct &v, bool inverse, bool is_additive, bool force = false);

private:

	void		Set_DCG (bool is_additive, int index, float value);

	void		Free(void);

	float		Radius;
	int		Slices;
	int		Stacks;			
	int		face_ct;	  		// # of faces

	int		Vertex_ct;		// vertex count
	Vector3	*vtx;				// array of vertices
	Vector3	*vtx_normal;	// array of vertex normals
	Vector2	*vtx_uv;			// array of vertex uv coordinates
	Vector4  *dcg;

	AlphaVectorStruct alpha_vector;	// vector last used to computer vtx_alpha array
	bool		inverse_alpha;	// inverse alpha, or not
	bool		IsAdditive;

	int		strip_ct;		// number of strips
	int		strip_size;		// size of each strip
	int		*strips;			// array of vertex indices for each strip	 (# stripbs x sizeof strip)

	int		fan_ct;			// number of fans
	int		fan_size;		// size of each fan
	int		*fans;			// array of vertex indices for each fan (# of fans by fan_size)

	Vector3i	*tri_poly;		// array of triangle poly's, vertex indices  (can be discard if switched to strips + fans)

};

inline void
SphereMeshClass::Set_DCG (bool is_additive, int index, float value)
{
	if (is_additive) {
		dcg[index].X = value;
		dcg[index].Y = value;
		dcg[index].Z = value;
		dcg[index].W = 0;
	} else {
		dcg[index].X = 1.0F;
		dcg[index].Y = 1.0F;
		dcg[index].Z = 1.0F;
		dcg[index].W = value;
	}

	return ;
}

/*
** SphereRenderObjClass: Procedurally generated render spheres
**
*/
class SphereRenderObjClass : public RenderObjClass
{	

public:

	// These are bit masks, so they should enum 1,2,4,8,10,20,40 etc...
	enum SphereFlags {
		USE_ALPHA_VECTOR	= 0x00000001,
		USE_CAMERA_ALIGN	= 0x00000002,
		USE_INVERSE_ALPHA	= 0x00000004,
		USE_ANIMATION_LOOP= 0x00000008,
	};

	SphereRenderObjClass(void);
	SphereRenderObjClass(const W3dSphereStruct & def);
	SphereRenderObjClass(const SphereRenderObjClass & src);
	SphereRenderObjClass & operator = (const SphereRenderObjClass &);
	~SphereRenderObjClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface 
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);
	virtual void 					Set_Transform(const Matrix3D &m); 
	virtual void 					Set_Position(const Vector3 &v);
   virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;
	virtual void					Set_Hidden(int onoff)				{ RenderObjClass::Set_Hidden (onoff); Update_On_Visibilty (); }
	virtual void					Set_Visible(int onoff)				{ RenderObjClass::Set_Visible (onoff); Update_On_Visibilty (); }
	virtual void					Set_Animation_Hidden(int onoff)	{ RenderObjClass::Set_Animation_Hidden (onoff); Update_On_Visibilty (); }
	virtual void					Set_Force_Visible(int onoff)		{ RenderObjClass::Set_Force_Visible (onoff); Update_On_Visibilty (); }


	const AABoxClass	&			Get_Box(void);

	virtual int					 	Get_Num_Polys(void) const;
	virtual const char *		 	Get_Name(void) const;
	virtual void				 	Set_Name(const char * name);

	unsigned int					Get_Flags(void)  { return Flags; }
	void								Set_Flags(unsigned int flags) { Flags = flags; }
	void								Set_Flag(unsigned int flag, bool onoff) { Flags &= (~flag); if (onoff) Flags |= flag; }

	// Animation access
	bool								Is_Animating (void)		{ return IsAnimating; }
	void								Start_Animating (void)	{ IsAnimating = true; anim_time = 0; }
	void								Stop_Animating (void)	{ IsAnimating = false; anim_time = 0; }

	// Current state access
	void							 	Set_Color(const Vector3 & color)			{ Color = color; }
	void							 	Set_Alpha(float alpha)						{ Alpha = alpha; }
	void							 	Set_Scale(const Vector3 & scale)			{ Scale = scale; }
	void								Set_Vector(const AlphaVectorStruct &v)	{ Vector = v; }

	const Vector3 &				Get_Color(void) const	{ return Color; }
	float								Get_Alpha(void) const	{ return Alpha; }
	const Vector3 &				Get_Scale(void) const	{ return Scale; }
	const AlphaVectorStruct &	Get_Vector(void) const	{ return Vector; }

	Vector3							Get_Default_Color(void) const;
	float								Get_Default_Alpha(void) const;
	Vector3							Get_Default_Scale(void) const;
	AlphaVectorStruct				Get_Default_Vector(void) const;

	// Size/position methods
	void								Set_Extent (const Vector3 &extent);
	void							  	Set_Local_Center_Extent(const Vector3 & center,const Vector3 & extent);
	void							  	Set_Local_Min_Max(const Vector3 & min,const Vector3 & max);

	// Texture access
	void							  	Set_Texture(TextureClass *tf);
	TextureClass	*				Peek_Texture(void)					{return SphereTexture;}
	ShaderClass	&					Get_Shader(void)						{return SphereShader;}
	void								Set_Shader(ShaderClass &shader)	{SphereShader=shader;}

	// Animation control
	float								Get_Animation_Duration (void) const		{ return AnimDuration; }
	void								Set_Animation_Duration (float time)		{ AnimDuration = time; Restart_Animation (); }
	void								Restart_Animation (void)					{ anim_time = 0; }

	// Animatable channel access
	SphereColorChannelClass &			Get_Color_Channel (void)				{ return ColorChannel; }
	const SphereColorChannelClass &	Peek_Color_Channel (void)				{ return ColorChannel; }	
	
	SphereAlphaChannelClass &			Get_Alpha_Channel (void)				{ return AlphaChannel; }
	const SphereAlphaChannelClass &	Peek_Alpha_Channel (void)				{ return AlphaChannel; }

	SphereScaleChannelClass &			Get_Scale_Channel (void)				{ return ScaleChannel; }
	const SphereScaleChannelClass &	Peek_Scale_Channel (void)				{ return ScaleChannel; }

	SphereVectorChannelClass &			Get_Vector_Channel (void)				{ return VectorChannel; }
	const SphereVectorChannelClass &	Peek_Vector_Channel (void)				{ return VectorChannel; }

	void								Set_Color_Channel (const SphereColorChannelClass &data)		{ ColorChannel = data; }
	void								Set_Alpha_Channel (const SphereAlphaChannelClass &data)		{ AlphaChannel = data; }
	void								Set_Scale_Channel (const SphereScaleChannelClass &data)		{ ScaleChannel = data; }
	void								Set_Vector_Channel (const SphereVectorChannelClass &data)	{ VectorChannel = data; }

protected:
	
	virtual void			 		update_cached_box(void);
	virtual void			 		Update_Cached_Bounding_Volumes(void) const;
	void								Update_On_Visibilty(void);

	// Initialization stuff
	void								Init_Material (void);
	static void						Generate_Shared_Mesh_Arrays (const AlphaVectorStruct &alphavector);

	// Animation Stuff
	void								animate(void);		// animation update function
	float								anim_time;			// what time in seconds are we in the animation
	float								AnimDuration;
	bool								IsAnimating;
	
	SphereColorChannelClass		ColorChannel;
	SphereAlphaChannelClass		AlphaChannel;
	SphereScaleChannelClass		ScaleChannel;
	SphereVectorChannelClass	VectorChannel;

	void						 		update_mesh_data(const Vector3 & center,const Vector3 & extent);
	void						 		render_sphere();
	void						 		vis_render_sphere(SpecialRenderInfoClass & rinfo,const Vector3 & center,const Vector3 & extent);

	char						 		Name[2*W3D_NAME_LEN];
	Vector3					 		ObjSpaceCenter;
	Vector3					 		ObjSpaceExtent;

	int						 		CurrentLOD;

	// Current State
	Vector3					 		Color;
	float								Alpha;
	Vector3							Scale;
	AlphaVectorStruct				Vector;

	Quaternion						Orientation;

	// Flags
	unsigned int					Flags;

	VertexMaterialClass	  	  *SphereMaterial;
	ShaderClass					  	SphereShader;
	TextureClass				  *SphereTexture;

	AABoxClass						CachedBox;

	// Friend classes
	friend class SpherePrototypeClass;
};

inline void SphereRenderObjClass::Set_Extent (const Vector3 &extent)
{
	ObjSpaceExtent = extent;
	update_cached_box();
	Update_Cached_Bounding_Volumes();
}

inline void SphereRenderObjClass::Set_Local_Center_Extent(const Vector3 & center,const Vector3 & extent)
{
	ObjSpaceCenter = center;
	ObjSpaceExtent = extent;
	update_cached_box();
}

inline void SphereRenderObjClass::Set_Local_Min_Max(const Vector3 & min,const Vector3 & max)
{
	ObjSpaceCenter = (max + min) / 2.0f;
	ObjSpaceExtent = (max - min) / 2.0f;
	update_cached_box();
}


inline const AABoxClass & SphereRenderObjClass::Get_Box(void) 
{ 
	Validate_Transform();
	update_cached_box();
	return CachedBox; 
}

/*
** Loader for spheres
*/
class SphereLoaderClass : public PrototypeLoaderClass
{
public:
	virtual int						Chunk_Type (void)  { return W3D_CHUNK_SPHERE; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};

/*
** Prototype for Sphere objects
*/
class SpherePrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(SpherePrototypeClass)
public:
	SpherePrototypeClass (void);
	SpherePrototypeClass (SphereRenderObjClass *sphere);

	virtual const char *			Get_Name(void) const;
	virtual int						Get_Class_ID(void) const;
	virtual RenderObjClass *	Create(void);
	virtual void							DeleteSelf()										{ delete this; }

	bool								Load (ChunkLoadClass &cload);
	bool								Save (ChunkSaveClass &csave);

protected:
	~SpherePrototypeClass (void);

private:
	W3dSphereStruct				Definition;

	SphereColorChannelClass		ColorChannel;
	SphereAlphaChannelClass		AlphaChannel;
	SphereScaleChannelClass		ScaleChannel;
	SphereVectorChannelClass	VectorChannel;
};

/*
** Instance of the loader which the asset manager installs
*/
extern SphereLoaderClass			_SphereLoader;

#endif // SPHEREOBJ_H

// EOF - sphereobj,h


