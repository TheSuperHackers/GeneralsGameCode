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
 *                     $Archive:: /Commando/Code/ww3d2/part_ldr.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                
 *                                                                                             *
 *                     $Modtime:: 8/01/01 3:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PART_EMT_LDR_H
#define __PART_EMT_LDR_H


#include "proto.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "w3derr.h"
#include "w3d_util.h"
#include "part_emt.h"



// Forward declarations
class ChunkLoadClass;
class ChunkSaveClass;
class Vector3Randomizer;


///////////////////////////////////////////////////////////////////////////////////
//
//	Macros
//
#ifndef SAFE_FREE
#define SAFE_FREE(pointer)	\
{ \
	if (pointer) {	\
		::free (pointer);	\
		pointer = 0; \
	} \
}
#endif //SAFE_FREE


#define BYTE_TO_FLOAT(byte)	\
	(((float)byte) / 255.0F)	\

#define FLOAT_TO_BYTE(fval)			\
	((unsigned char)(max(fval,0.0F) * 255.0F))	\

#define RGBA_TO_VECTOR3(rgba)																			\
	Vector3 (BYTE_TO_FLOAT(rgba.R), BYTE_TO_FLOAT (rgba.G), BYTE_TO_FLOAT (rgba.B))	\

#define VECTOR3_TO_RGBA(vector3, rgba)		\
	rgba.R = FLOAT_TO_BYTE (vector3.X);		\
	rgba.G = FLOAT_TO_BYTE (vector3.Y);		\
	rgba.B = FLOAT_TO_BYTE (vector3.Z);		\


///////////////////////////////////////////////////////////////////////////////////
//
//	ParticleEmitterDefClass
//
//	Description of a general particle emitter object.  Used by the asset manager
// to construct particle emitters.
//
class ParticleEmitterDefClass
{
	public:

		///////////////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//		
		ParticleEmitterDefClass (void);
		ParticleEmitterDefClass (const ParticleEmitterDefClass &src);
		virtual ~ParticleEmitterDefClass (void);


		///////////////////////////////////////////////////////////
		//
		//	Public operators
		//		
		const ParticleEmitterDefClass &operator= (const ParticleEmitterDefClass &src);


		///////////////////////////////////////////////////////////
		//
		//	Public methods
		//		
		virtual WW3DErrorType	Load_W3D (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Save_W3D (ChunkSaveClass &chunk_save);
		const char *				Get_Name (void) const					{ return m_pName; }
		virtual void				Set_Name (const char *pname);

		//
		//	Informational methods
		//
		unsigned int			Get_Version (void) const				{ return m_Version; }

		//
		//	Inline Accessors
		//
		int						Get_Render_Mode (void) const			{ return m_InfoV2.RenderMode; }	// values defined in w3d_file.h
		int						Get_Frame_Mode (void) const			{ return m_InfoV2.FrameMode; } // values in w3d_file.h
		const char *			Get_Texture_Filename (void) const	{ return m_Info.TextureFilename; }
		float						Get_Lifetime (void) const				{ return m_Info.Lifetime; }
		float						Get_Emission_Rate (void) const		{ return m_Info.EmissionRate; }
		float						Get_Max_Emissions (void) const		{ return m_Info.MaxEmissions; }
		float						Get_Fade_Time (void) const				{ return m_Info.FadeTime; }
		float						Get_Gravity (void) const				{ return m_Info.Gravity; }
		float						Get_Elasticity (void) const			{ return m_Info.Elasticity; }
		Vector3					Get_Velocity (void) const				{ return Vector3 (m_Info.Velocity.X, m_Info.Velocity.Y, m_Info.Velocity.Z); }
		Vector3					Get_Acceleration (void) const			{ return Vector3 (m_Info.Acceleration.X, m_Info.Acceleration.Y, m_Info.Acceleration.Z); }
		unsigned int			Get_Burst_Size (void) const			{ return m_InfoV2.BurstSize; }
		float						Get_Outward_Vel (void) const			{ return m_InfoV2.OutwardVel; }
		float						Get_Vel_Inherit (void) const			{ return m_InfoV2.VelInherit; }
		float						Get_Future_Start_Time (void) const	{ return m_ExtraInfo.FutureStartTime; }

		virtual void			Set_Render_Mode (int mode)						{ m_InfoV2.RenderMode = mode; } // values in w3d_file.h
		virtual void			Set_Frame_Mode (int mode)						{ m_InfoV2.FrameMode = mode; } // values in w3d_file.h 
		virtual void			Set_Texture_Filename (const char *pname);
		virtual void			Set_Lifetime (float value)						{ m_Info.Lifetime = value; }
		virtual void			Set_Emission_Rate (float value)				{ m_Info.EmissionRate = value; }
		virtual void			Set_Max_Emissions (float value)				{ m_Info.MaxEmissions = value; }
		virtual void			Set_Fade_Time (float value)					{ m_Info.FadeTime = value; }
		virtual void			Set_Gravity (float value)						{ m_Info.Gravity = value; }
		virtual void			Set_Elasticity (float value)					{ m_Info.Elasticity = value; }
		virtual void			Set_Velocity (const Vector3 &value)			{ m_Info.Velocity.X = value.X; m_Info.Velocity.Y = value.Y; m_Info.Velocity.Z = value.Z; }
		virtual void			Set_Acceleration (const Vector3 &value)	{ m_Info.Acceleration.X = value.X; m_Info.Acceleration.Y = value.Y; m_Info.Acceleration.Z = value.Z; }
		virtual void			Set_Burst_Size (unsigned int count)			{ m_InfoV2.BurstSize = count; }
		virtual void			Set_Outward_Vel (float value)					{ m_InfoV2.OutwardVel = value; }
		virtual void			Set_Vel_Inherit (float value)					{ m_InfoV2.VelInherit = value; }
		virtual void			Set_Future_Start_Time (float value)			{ m_ExtraInfo.FutureStartTime = value; }

		void						Get_Shader (ShaderClass &shader) const		{ W3dUtilityClass::Convert_Shader (m_InfoV2.Shader, &shader); }
		virtual void			Set_Shader (const ShaderClass &shader)		{ W3dUtilityClass::Convert_Shader (shader, &m_InfoV2.Shader); }

		//
		//	Randomizer accessors
		//
		Vector3Randomizer *	Get_Creation_Volume (void) const								{ return m_pCreationVolume->Clone (); }
		Vector3Randomizer *	Get_Velocity_Random (void) const								{ return m_pVelocityRandomizer->Clone (); }
		virtual void			Set_Creation_Volume (Vector3Randomizer *randomizer);
		virtual void			Set_Velocity_Random (Vector3Randomizer *randomizer);

		//
		//	Keyframe accessors
		//
		virtual void			Get_Color_Keyframes (ParticlePropertyStruct<Vector3> &keyframes) const;
		virtual void			Get_Opacity_Keyframes (ParticlePropertyStruct<float> &keyframes) const;
		virtual void			Get_Size_Keyframes (ParticlePropertyStruct<float> &keyframes) const;
		virtual void			Get_Rotation_Keyframes (ParticlePropertyStruct<float> &rotationframes) const;
		virtual void			Get_Frame_Keyframes (ParticlePropertyStruct<float> &frameframes) const;
		virtual void			Get_Blur_Time_Keyframes (ParticlePropertyStruct<float> &blurtimeframes) const;
		virtual float			Get_Initial_Orientation_Random (void) const { return m_InitialOrientationRandom; }

		virtual void			Set_Color_Keyframes (ParticlePropertyStruct<Vector3> &keyframes);
		virtual void			Set_Opacity_Keyframes (ParticlePropertyStruct<float> &keyframes);
		virtual void			Set_Size_Keyframes (ParticlePropertyStruct<float> &keyframes);
		virtual void			Set_Rotation_Keyframes (ParticlePropertyStruct<float> &keyframes, float orient_rnd);
		virtual void			Set_Frame_Keyframes (ParticlePropertyStruct<float> &keyframes);
		virtual void			Set_Blur_Time_Keyframes (ParticlePropertyStruct<float> &keyframes);

		//
		//	User data accessors
		//
		const char *			Get_User_String (void) const					{ return m_pUserString; }
		int						Get_User_Type (void) const						{ return m_iUserType; }
		virtual void			Set_User_String (const char *pstring);
		virtual void			Set_User_Type (int type)						{ m_iUserType = type; }

		//
		// Line Properties accessors
		// NOTE: These properties are only used when Line rendering mode is used.
		//
		const W3dEmitterLinePropertiesStruct * Get_Line_Properties(void) const { return &m_LineProperties; }

		int						Get_Line_Texture_Mapping_Mode(void) const	{ return (m_LineProperties.Flags & W3D_ELINE_TEXTURE_MAP_MODE_MASK) >> W3D_ELINE_TEXTURE_MAP_MODE_OFFSET; }
		int						Is_Merge_Intersections(void) const			{ return m_LineProperties.Flags & W3D_ELINE_MERGE_INTERSECTIONS; }
		int						Is_Freeze_Random(void) const					{ return m_LineProperties.Flags & W3D_ELINE_FREEZE_RANDOM; }
		int						Is_Sorting_Disabled(void) const				{ return m_LineProperties.Flags & W3D_ELINE_DISABLE_SORTING; }
		int						Are_End_Caps_Enabled(void)	const				{ return m_LineProperties.Flags & W3D_ELINE_END_CAPS; }
		int						Get_Subdivision_Level(void) const			{ return m_LineProperties.SubdivisionLevel; }
		float						Get_Noise_Amplitude(void) const				{ return m_LineProperties.NoiseAmplitude; }
		float						Get_Merge_Abort_Factor(void) const			{ return m_LineProperties.MergeAbortFactor; }
		float						Get_Texture_Tile_Factor(void) const			{ return m_LineProperties.TextureTileFactor; }
		Vector2					Get_UV_Offset_Rate(void) const				{ return Vector2(m_LineProperties.UPerSec,m_LineProperties.VPerSec); }
		
		virtual void			Set_Line_Texture_Mapping_Mode(int mode);
		virtual void			Set_Merge_Intersections(int onoff)			{ if (onoff) { m_LineProperties.Flags |= W3D_ELINE_MERGE_INTERSECTIONS; } else { m_LineProperties.Flags &= ~W3D_ELINE_MERGE_INTERSECTIONS; }; }
		virtual void			Set_Freeze_Random(int onoff)					{ if (onoff) { m_LineProperties.Flags |= W3D_ELINE_FREEZE_RANDOM; } else { m_LineProperties.Flags &= ~W3D_ELINE_FREEZE_RANDOM; }; }
		virtual void			Set_Disable_Sorting(int onoff)				{ if (onoff) { m_LineProperties.Flags |= W3D_ELINE_DISABLE_SORTING; } else { m_LineProperties.Flags &= ~W3D_ELINE_DISABLE_SORTING; }; }
		virtual void			Set_End_Caps(int onoff)							{ if (onoff) { m_LineProperties.Flags |= W3D_ELINE_END_CAPS; } else { m_LineProperties.Flags &= ~W3D_ELINE_END_CAPS; }; }
		virtual void			Set_Subdivision_Level(int lvl)				{ m_LineProperties.SubdivisionLevel = lvl; }
		virtual void			Set_Noise_Amplitude(float k)					{ m_LineProperties.NoiseAmplitude = k; }
		virtual void			Set_Merge_Abort_Factor(float k)				{ m_LineProperties.MergeAbortFactor = k; }
		virtual void			Set_Texture_Tile_Factor(float k)				{ m_LineProperties.TextureTileFactor = k; }
		virtual void			Set_UV_Offset_Rate(const Vector2 & rate)	{ m_LineProperties.UPerSec = rate.X; m_LineProperties.VPerSec = rate.Y; }

	protected:
		
		///////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		
		//
		//	Loading methods
		//
		virtual WW3DErrorType	Read_Header (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_User_Data (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_Info (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_InfoV2 (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_Props (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_Line_Properties (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_Rotation_Keyframes (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_Frame_Keyframes (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_Blur_Time_Keyframes (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Read_Extra_Info (ChunkLoadClass &chunk_load);

		virtual bool				Read_Color_Keyframe (ChunkLoadClass &chunk_load, float *key_time, Vector3 *value);
		virtual bool				Read_Opacity_Keyframe (ChunkLoadClass &chunk_load, float *key_time, float *value);
		virtual bool				Read_Size_Keyframe (ChunkLoadClass &chunk_load, float *key_time, float *value);

		//
		//	Saving methods
		//
		virtual WW3DErrorType	Save_Header (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType	Save_User_Data (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType	Save_Info (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType	Save_InfoV2 (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType	Save_Props (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType	Save_Line_Properties (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType	Save_Frame_Keyframes (ChunkSaveClass & chunk_save);
		virtual WW3DErrorType	Save_Rotation_Keyframes (ChunkSaveClass & chunk_save);
		virtual WW3DErrorType	Save_Blur_Time_Keyframes (ChunkSaveClass & chunk_save);
		virtual WW3DErrorType	Save_Extra_Info (ChunkSaveClass & chunk_save);

		virtual WW3DErrorType	Save_Color_Keyframes (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType	Save_Opacity_Keyframes (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType	Save_Size_Keyframes (ChunkSaveClass &chunk_save);

		virtual Vector3Randomizer *Create_Randomizer (W3dVolumeRandomizerStruct &info);
		virtual void				Initialize_Randomizer_Struct (const Vector3Randomizer &randomizer, W3dVolumeRandomizerStruct &info);

		//
		//	Conversion methods
		//
		virtual void				Initialize_To_Ver2 (void);
		virtual void				Convert_To_Ver2 (void);

		//
		//	Misc. methods
		//
		virtual void				Normalize_Filename (void);
		virtual void				Free_Props (void);

	private:

		///////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		///////////////////////////////////////////////////////////
		char * 									m_pName;
		char *									m_pUserString;
		int										m_iUserType;
		unsigned	int							m_Version;
		ShaderClass								m_Shader;
		W3dEmitterInfoStruct					m_Info;
		W3dEmitterInfoStructV2				m_InfoV2;
		W3dEmitterExtraInfoStruct			m_ExtraInfo;
		W3dEmitterLinePropertiesStruct	m_LineProperties;
		ParticlePropertyStruct<Vector3>	m_ColorKeyframes;
		ParticlePropertyStruct<float>		m_OpacityKeyframes;
		ParticlePropertyStruct<float>		m_SizeKeyframes;
		ParticlePropertyStruct<float>		m_RotationKeyframes;
		float										m_InitialOrientationRandom;
		ParticlePropertyStruct<float>		m_FrameKeyframes;
		ParticlePropertyStruct<float>		m_BlurTimeKeyframes;
		Vector3Randomizer *					m_pCreationVolume;
		Vector3Randomizer *					m_pVelocityRandomizer;
};

inline void ParticleEmitterDefClass::Set_Line_Texture_Mapping_Mode(int mode)	
{ 
	m_LineProperties.Flags &= ~W3D_ELINE_TEXTURE_MAP_MODE_MASK;
	m_LineProperties.Flags |= ((mode << W3D_ELINE_TEXTURE_MAP_MODE_OFFSET) & W3D_ELINE_TEXTURE_MAP_MODE_MASK); 
}



///////////////////////////////////////////////////////////////////////////////////
//
//	ParticleEmitterPrototypeClass
//
class ParticleEmitterPrototypeClass : public W3DMPO, public PrototypeClass 
{
	W3DMPO_GLUE(ParticleEmitterPrototypeClass)
	public:

		///////////////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//		
		ParticleEmitterPrototypeClass (ParticleEmitterDefClass *pdef)	{ m_pDefinition = pdef; }
		
		///////////////////////////////////////////////////////////
		//
		//	Public methods
		//
		virtual const char *						Get_Name (void) const						{ return m_pDefinition->Get_Name(); }
		virtual int											Get_Class_ID (void) const				{ return RenderObjClass::CLASSID_PARTICLEEMITTER; }
		virtual RenderObjClass *				Create (void);
		virtual void										DeleteSelf()										{ delete this; }
		virtual ParticleEmitterDefClass *	Get_Definition (void) const		{ return m_pDefinition; }

	protected:
		virtual ~ParticleEmitterPrototypeClass (void)						{ delete m_pDefinition; }

	protected:
		///////////////////////////////////////////////////////////
		//
		//	Protected member data (so derived can access)
		//
		ParticleEmitterDefClass	*	m_pDefinition;
};


///////////////////////////////////////////////////////////////////////////////////
//
//	ParticleEmitterLoaderClass
//
class ParticleEmitterLoaderClass : public PrototypeLoaderClass
{
	public:

		virtual int						Chunk_Type (void)  { return W3D_CHUNK_EMITTER; }
		virtual PrototypeClass *	Load_W3D (ChunkLoadClass &chunk_load);
};


///////////////////////////////////////////////////////////////////////////////////
//
//	Global variables
//
extern ParticleEmitterLoaderClass	_ParticleEmitterLoader;


#endif //__PART_EMT_LDR_H
