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

/*************************************************************************** 
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     *** 
 *************************************************************************** 
 *                                                                         * 
 *                 Project Name : G                                        * 
 *                                                                         * 
 *                     $Archive:: /Commando/Code/ww3d2/mapper.h           $* 
 *                                                                         * 
 *                     $Org Author:: Greg_h                                  $* 
 *                                                                         * 
 *                       $Author:: Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                         * 
 *                    $Revision:: 26                                      $* 
 *                                                                         * 
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VERTEXMAPPER_H
#define VERTEXMAPPER_H

#include "refcount.h"
#include "w3d_file.h"
#include "w3derr.h"
#include "wwdebug.h"
#include "vector2.h"
#include "vector3.h"
#include "ww3d.h"
#include "matrix4.h"

class INIClass;

/*
** TextureMapperClass
** Base class for all texture mappers.
*/
class TextureMapperClass : public W3DMPO, public RefCountClass
{
	public:

		enum {
			MAPPER_ID_UNKNOWN,
			MAPPER_ID_LINEAR_OFFSET,
			MAPPER_ID_CLASSIC_ENVIRONMENT,
			MAPPER_ID_ENVIRONMENT,
			MAPPER_ID_SCREEN,
			MAPPER_ID_ANIMATING_1D,
			MAPPER_ID_AXIAL,
			MAPPER_ID_SILHOUETTE,
			MAPPER_ID_SCALE,
			MAPPER_ID_GRID,
			MAPPER_ID_ROTATE,
			MAPPER_ID_SINE_LINEAR_OFFSET,
			MAPPER_ID_STEP_LINEAR_OFFSET,
			MAPPER_ID_ZIGZAG_LINEAR_OFFSET,
			MAPPER_ID_WS_CLASSIC_ENVIRONMENT,
			MAPPER_ID_WS_ENVIRONMENT,
			MAPPER_ID_GRID_CLASSIC_ENVIRONMENT,
			MAPPER_ID_GRID_ENVIRONMENT,
			MAPPER_ID_RANDOM,
			MAPPER_ID_EDGE,
			MAPPER_ID_BUMPENV,
		};

		TextureMapperClass(unsigned int stage=0);
		TextureMapperClass(const TextureMapperClass & src) : Stage(src.Stage) { }


		virtual int								Mapper_ID(void) const { return MAPPER_ID_UNKNOWN;}

		virtual TextureMapperClass *		Clone(void) const = 0;

		virtual bool							Is_Time_Variant(void) { return false; }
		virtual void							Apply(int uv_array_index) = 0;
		virtual void							Reset(void) { }
		virtual bool							Needs_Normals(void) { return false; }
		void										Set_Stage(int stage) { Stage = stage; }
		int										Get_Stage(void) const { return Stage; }

	protected:
		unsigned int							Stage;
};

/*
** ScaleTextureMapperClass
** Scales UV coordinates
*/
class ScaleTextureMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(ScaleTextureMapperClass)
public:	
	ScaleTextureMapperClass(unsigned int stage);
	ScaleTextureMapperClass(const Vector2 &scale, unsigned int stage);
	ScaleTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	ScaleTextureMapperClass(const ScaleTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_SCALE;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( ScaleTextureMapperClass, (*this)); }

	virtual void Apply(int uv_array_index);

protected:
	Vector2			Scale;		// Scale
};

/*
** LinearOffsetTextureMapperClass
** Modifies the UV coordinates by a linear offset
*/
class LinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
	W3DMPO_GLUE(LinearOffsetTextureMapperClass)
public:
	LinearOffsetTextureMapperClass(const Vector2 &offset_per_sec, const Vector2 &scale, unsigned int stage);
	LinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	LinearOffsetTextureMapperClass(const LinearOffsetTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_LINEAR_OFFSET;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( LinearOffsetTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }

	virtual void Apply(int uv_array_index);
	virtual void Reset(void) { Set_Current_UV_Offset(Vector2(0.0f, 0.0f)); }

	void Set_Current_UV_Offset(const Vector2 &cur)  {
		CurrentUVOffset = cur;
	}
	void Set_UV_Offset_Delta(const Vector2 &per_sec)  {
		UVOffsetDeltaPerMS = per_sec;
		UVOffsetDeltaPerMS *= -0.001f;
	}
	void Get_Current_UV_Offset(Vector2 &cur)
	{	cur= CurrentUVOffset;
	}
	void Set_LastUsedSyncTime(unsigned int time) { LastUsedSyncTime = time;}
	unsigned int Get_LastUsedSyncTime() { return LastUsedSyncTime;}
	
protected:
	Vector2			CurrentUVOffset;		// Current UV offset
	Vector2			UVOffsetDeltaPerMS;	// Amount to increase offset each millisec
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
};

/*
** GridTextureMapperClass
** Animates a texture by divving it up into a grid and using those offsets
*/
class GridTextureMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(GridTextureMapperClass)
public:
	GridTextureMapperClass(float fps, unsigned int gridwidth_log2, unsigned int stage);
	GridTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	GridTextureMapperClass(const GridTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_GRID;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( GridTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Apply(int uv_array_index);
	virtual void Reset(void);

	void Set_Frame(unsigned int frame) { CurrentFrame=frame; }
	void Set_Frame_Per_Second(float fps);
	
protected:	
	// Utility functions
	void initialize(float fps, unsigned int gridwidth_log2);
	void update_temporal_state(void);
	void calculate_uv_offset(float * u_offset, float * v_offset);

	// Constant properties
	int				Sign;					// +1 if frame rate positive, -1 otherwise
	unsigned int	MSPerFrame;			// milliseconds per frame
	float				OOGridWidth;		// 1.0f / size of the side of the grid)
	unsigned int	GridWidthLog2;		// log base 2 of size of the side of the grid
	unsigned int	LastFrame;			// Last frame to use

	// Temporal state
	unsigned int	Remainder;			// used for timing calculations
	unsigned int	CurrentFrame;		// current frame
	unsigned int	LastUsedSyncTime;	// Sync time last used to update offset
};

/*
** RotateTextureMapperClass
** Modifies the textures over time
*/
class RotateTextureMapperClass : public ScaleTextureMapperClass
{
	W3DMPO_GLUE(RotateTextureMapperClass)
public:
	RotateTextureMapperClass(float rad_per_sec, const Vector2& center, unsigned int stage);
	RotateTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	RotateTextureMapperClass(const RotateTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_ROTATE;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( RotateTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Apply(int uv_array_index);
	virtual void Reset(void) { CurrentAngle = 0.0f; }

private:
	float CurrentAngle;
	float RadiansPerSec;
	Vector2 Center;
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
};

/*
** SineLinearOffsetTextureMapperClass
** Modifies the UV coodinates by a sine linear offset
*/
class SineLinearOffsetTextureMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(SineLinearOffsetTextureMapperClass)
public:
	SineLinearOffsetTextureMapperClass(const Vector3 &uafp, const Vector3 &vafp, unsigned int stage);
	SineLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	SineLinearOffsetTextureMapperClass(const SineLinearOffsetTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_SINE_LINEAR_OFFSET;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( SineLinearOffsetTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Apply(int uv_array_index);
	virtual void Reset(void) { CurrentAngle = 0.0f; }

private:
	Vector3 UAFP;								// U Coordinate Amplitude frequency phase
	Vector3 VAFP;								// V Coordinate Amplitude frequency phase
	float CurrentAngle;
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
};

/*
** StepLinearOffsetTextureMapperClass
** Modifies the UV coodinates by a Step linear offset
*/
class StepLinearOffsetTextureMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(StepLinearOffsetTextureMapperClass)
public:
	StepLinearOffsetTextureMapperClass(const Vector2 &step, float steps_per_sec, unsigned int stage);
	StepLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	StepLinearOffsetTextureMapperClass(const StepLinearOffsetTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_STEP_LINEAR_OFFSET;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( StepLinearOffsetTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Apply(int uv_array_index);
	virtual void Reset(void);

private:
	Vector2 Step;								// Size of step
	float StepsPerSec;						// Steps per second
	Vector2 CurrentStep;						// Current step
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
};

/*
** ZigZagLinearOffsetTextureMapperClass
** Modifies the UV coodinates by a ZigZag linear offset
*/
class ZigZagLinearOffsetTextureMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(ZigZagLinearOffsetTextureMapperClass)
public:
	ZigZagLinearOffsetTextureMapperClass(const Vector2 &speed, float period, unsigned int stage);
	ZigZagLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	ZigZagLinearOffsetTextureMapperClass(const ZigZagLinearOffsetTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_ZIGZAG_LINEAR_OFFSET;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( ZigZagLinearOffsetTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Apply(int uv_array_index);
	virtual void Reset(void);

private:
	Vector2 Speed;								// Speed of zigzag
	float Period;								// Time taken for a period	
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
};

// ----------------------------------------------------------------------------
//
// Environment Mapper calculates the texture coordinates based on
// transformed normals
//
// ----------------------------------------------------------------------------

class ClassicEnvironmentMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(ClassicEnvironmentMapperClass)
public:
	ClassicEnvironmentMapperClass(unsigned int stage) : TextureMapperClass(stage) { }
	ClassicEnvironmentMapperClass(const ClassicEnvironmentMapperClass & src) : TextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_CLASSIC_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( ClassicEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual bool Needs_Normals(void) { return true; }
};

class EnvironmentMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(EnvironmentMapperClass)
public:
	EnvironmentMapperClass(unsigned int stage) : TextureMapperClass(stage) { }
	EnvironmentMapperClass(const EnvironmentMapperClass & src) : TextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( EnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual bool Needs_Normals(void) { return true; }
};

class EdgeMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(EdgeMapperClass)
public:
	EdgeMapperClass(unsigned int stage);
	EdgeMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	EdgeMapperClass(const EdgeMapperClass & src);
	virtual int	Mapper_ID(void) const { return MAPPER_ID_EDGE;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( EdgeMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual void Reset(void);
	virtual bool Is_Time_Variant(void) { return true; }
	virtual bool Needs_Normals(void) { return true; }

protected:
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
	float VSpeed,VOffset;
	bool UseReflect;
};

class WSClassicEnvironmentMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(WSClassicEnvironmentMapperClass)
public:
	WSClassicEnvironmentMapperClass(unsigned int stage) : TextureMapperClass(stage) { }
	WSClassicEnvironmentMapperClass(const WSClassicEnvironmentMapperClass & src) : TextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_WS_CLASSIC_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( WSClassicEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual bool Needs_Normals(void) { return true; }
};

class WSEnvironmentMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(WSEnvironmentMapperClass)
public:
	WSEnvironmentMapperClass(unsigned int stage) : TextureMapperClass(stage) { }
	WSEnvironmentMapperClass(const WSEnvironmentMapperClass & src) : TextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_WS_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( WSEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual bool Needs_Normals(void) { return true; }
};

class GridClassicEnvironmentMapperClass : public GridTextureMapperClass
{
	W3DMPO_GLUE(GridClassicEnvironmentMapperClass)
public:
	GridClassicEnvironmentMapperClass(float fps,unsigned int gridwidth, unsigned int stage):GridTextureMapperClass(fps,gridwidth,stage) { }
	GridClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) : GridTextureMapperClass(ini,section,stage) { }
	GridClassicEnvironmentMapperClass(const GridTextureMapperClass & src) : GridTextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_GRID_CLASSIC_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( GridClassicEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual bool Needs_Normals(void) { return true; }
};

class GridEnvironmentMapperClass : public GridTextureMapperClass
{
	W3DMPO_GLUE(GridEnvironmentMapperClass)
public:	
	GridEnvironmentMapperClass(float fps,unsigned int gridwidth, unsigned int stage):GridTextureMapperClass(fps,gridwidth,stage) { }
	GridEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) : GridTextureMapperClass(ini,section,stage) { }
	GridEnvironmentMapperClass(const GridTextureMapperClass & src) : GridTextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_GRID_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( GridEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual bool Needs_Normals(void) { return true; }
};

// ----------------------------------------------------------------------------
//
// Screen Mapper calculates texture coordinates based on the projected screen
// coordinates of vertices
//
// ----------------------------------------------------------------------------
class ScreenMapperClass : public LinearOffsetTextureMapperClass
{
	W3DMPO_GLUE(ScreenMapperClass)
public:
	ScreenMapperClass(const Vector2 &offset_per_sec, const Vector2 &scale, unsigned int stage):LinearOffsetTextureMapperClass(offset_per_sec,scale,stage) { }
	ScreenMapperClass(const INIClass &ini, const char *section, unsigned int stage):LinearOffsetTextureMapperClass(ini,section,stage) { }
	ScreenMapperClass(const LinearOffsetTextureMapperClass & src):LinearOffsetTextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_SCREEN;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( ScreenMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
};

/**
** RandomTextureMapperClass
** Modifies the textures over time
*/
class RandomTextureMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(RandomTextureMapperClass)
public:
	RandomTextureMapperClass(float fps, unsigned int stage);
	RandomTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	RandomTextureMapperClass(const RandomTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_RANDOM;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( RandomTextureMapperClass, (*this)); }

	virtual void Apply(int uv_array_index);
	virtual void Reset(void);
	virtual bool Is_Time_Variant(void) { return true; }

protected:
	float FPS;
	float CurrentAngle;
	Vector2 Center;
	Vector2 Speed;
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
};

/**
** BumpEnvTextureMapperClass
** Modifies the bump transform as a function of time.  This mapper is derived
** from the LinearOffset mapper so that you can scroll and scale the bump map.
*/
class BumpEnvTextureMapperClass : public LinearOffsetTextureMapperClass
{
	W3DMPO_GLUE(BumpEnvTextureMapperClass)
public:
	BumpEnvTextureMapperClass(float rad_per_sec, float scale_factor, const Vector2 & offset_per_sec, const Vector2 &scale, unsigned int stage);
	BumpEnvTextureMapperClass(INIClass &ini, const char *section, unsigned int stage);
	BumpEnvTextureMapperClass(const BumpEnvTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_BUMPENV;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( BumpEnvTextureMapperClass, (*this)); }

	virtual void Apply(int uv_array_index);

protected:

	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
	float				CurrentAngle;
	float				RadiansPerSecond;
	float				ScaleFactor;
};


/*
** Utility functions
*/
void Reset_All_Texture_Mappers(RenderObjClass *robj, bool make_unique);

#endif