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

#pragma once

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
			MAPPER_ID_GRID_WS_CLASSIC_ENVIRONMENT,
			MAPPER_ID_GRID_WS_ENVIRONMENT,
		};

		TextureMapperClass(unsigned int stage=0);
		TextureMapperClass(const TextureMapperClass & src) : Stage(src.Stage) { }

		virtual ~TextureMapperClass(void) { }

		virtual int								Mapper_ID(void) const { return MAPPER_ID_UNKNOWN;}

		virtual TextureMapperClass *		Clone(void) const = 0;

		virtual bool							Is_Time_Variant(void) { return false; }
		virtual void							Apply(int uv_array_index) = 0;
		virtual void							Reset(void) { }
		virtual bool							Needs_Normals(void) { return false; }
		void										Set_Stage(int stage) { Stage = stage; }
		int										Get_Stage(void) const { return Stage; }

		// This is called by Apply(). It should not be called externally except
		// in unusual circumstances.
		virtual void							Calculate_Texture_Matrix(Matrix4x4 &tex_matrix) = 0;

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
	ScaleTextureMapperClass(const Vector2 &scale, unsigned int stage);
	ScaleTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	ScaleTextureMapperClass(const ScaleTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_SCALE;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( ScaleTextureMapperClass, (*this)); }

	virtual void Apply(int uv_array_index);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

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
	LinearOffsetTextureMapperClass(const Vector2 &offset_per_sec, const Vector2 & start_offset,
		bool clamp_fix, const Vector2 &scale, unsigned int stage);
	LinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	LinearOffsetTextureMapperClass(const LinearOffsetTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_LINEAR_OFFSET;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( LinearOffsetTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }

	virtual void Reset(void);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

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
	Vector2			StartingUVOffset;		// Need to store this for copy constructors
	bool				ClampFix;				// Restrict the offset in a correct manner for clamped textures
};

/*
** GridTextureMapperClass
** Animates a texture by divving it up into a grid and using those offsets
*/
class GridTextureMapperClass : public TextureMapperClass
{
	W3DMPO_GLUE(GridTextureMapperClass)
public:
	GridTextureMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, unsigned int stage);
	GridTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	GridTextureMapperClass(const GridTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_GRID;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( GridTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Apply(int uv_array_index);
	virtual void Reset(void);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

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
	unsigned int	Offset;				// Only affects initialization, but need to store it for copy CTors to work

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
	RotateTextureMapperClass(float rad_per_sec, const Vector2& center, const Vector2 &scale, unsigned int stage);
	RotateTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	RotateTextureMapperClass(const RotateTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_ROTATE;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( RotateTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Reset(void);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

private:
	float CurrentAngle;
	float RadiansPerMilliSec;
	Vector2 Center;
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
};

/*
** SineLinearOffsetTextureMapperClass
** Modifies the UV coodinates by a sine linear offset
*/
class SineLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
	W3DMPO_GLUE(SineLinearOffsetTextureMapperClass)
public:
	SineLinearOffsetTextureMapperClass(const Vector3 &uafp, const Vector3 &vafp, const Vector2 &scale, unsigned int stage);
	SineLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	SineLinearOffsetTextureMapperClass(const SineLinearOffsetTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_SINE_LINEAR_OFFSET;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( SineLinearOffsetTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Reset(void);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

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
class StepLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
	W3DMPO_GLUE(StepLinearOffsetTextureMapperClass)
public:
	StepLinearOffsetTextureMapperClass(const Vector2 &step, float steps_per_sec, bool clamp_fix,
		const Vector2 &scale, unsigned int stage);
	StepLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	StepLinearOffsetTextureMapperClass(const StepLinearOffsetTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_STEP_LINEAR_OFFSET;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( StepLinearOffsetTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Reset(void);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

private:
	Vector2 Step;								// Size of step
	float StepsPerMilliSec;					// Steps per millisecond
	Vector2 CurrentStep;						// Current step
	float	Remainder;							// Remainder time
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
	bool	ClampFix;							// Restrict the offset in a correct manner for clamped textures
};

/*
** ZigZagLinearOffsetTextureMapperClass
** Modifies the UV coodinates by a ZigZag linear offset
*/
class ZigZagLinearOffsetTextureMapperClass : public ScaleTextureMapperClass
{
	W3DMPO_GLUE(ZigZagLinearOffsetTextureMapperClass)
public:
	ZigZagLinearOffsetTextureMapperClass(const Vector2 &speed, float period, const Vector2 &scale, unsigned int stage);
	ZigZagLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	ZigZagLinearOffsetTextureMapperClass(const ZigZagLinearOffsetTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_ZIGZAG_LINEAR_OFFSET;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( ZigZagLinearOffsetTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Reset(void);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

private:
	Vector2 Speed;								// Speed of zigzag in units per millisecond
	float Period;								// Time taken for a period	in milliseconds
	float Half_Period;						// Half of period
	float Remainder;							// Remainder time in milliseconds
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
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);
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
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);
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
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

protected:
	unsigned int	LastUsedSyncTime;		// Sync time last used to update offset
	float VSpeed,VOffset;
	bool UseReflect;
};

class WSEnvMapperClass : public TextureMapperClass
{
public:
	enum AxisType { AXISTYPE_X=0, AXISTYPE_Y=1, AXISTYPE_Z=2 };
	WSEnvMapperClass(AxisType axis, unsigned int stage) : TextureMapperClass(stage), Axis(axis) { }
	WSEnvMapperClass(const WSEnvMapperClass & src) : TextureMapperClass(src), Axis(src.Axis) { }
	WSEnvMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	virtual bool Needs_Normals(void) { return true; }
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);
protected:
	AxisType		Axis;
};

class WSClassicEnvironmentMapperClass : public WSEnvMapperClass
{
	W3DMPO_GLUE(WSClassicEnvironmentMapperClass)
public:
	WSClassicEnvironmentMapperClass(AxisType axis, unsigned int stage) : WSEnvMapperClass(axis, stage) { }
	WSClassicEnvironmentMapperClass(const WSClassicEnvironmentMapperClass & src) : WSEnvMapperClass(src) { }
	WSClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) : WSEnvMapperClass(ini, section, stage) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_WS_CLASSIC_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( WSClassicEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
};

class WSEnvironmentMapperClass : public WSEnvMapperClass
{
	W3DMPO_GLUE(WSEnvironmentMapperClass)
public:
	WSEnvironmentMapperClass(AxisType axis, unsigned int stage) : WSEnvMapperClass(axis, stage) { }
	WSEnvironmentMapperClass(const WSClassicEnvironmentMapperClass & src) : WSEnvMapperClass(src) { }
	WSEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) : WSEnvMapperClass(ini, section, stage) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_WS_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( WSEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
};

class GridClassicEnvironmentMapperClass : public GridTextureMapperClass
{
	W3DMPO_GLUE(GridClassicEnvironmentMapperClass)
public:
	GridClassicEnvironmentMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, unsigned int stage) : GridTextureMapperClass(fps, gridwidth_log2, last_frame, offset, stage) { }
	GridClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) : GridTextureMapperClass(ini, section, stage) { }
	GridClassicEnvironmentMapperClass(const GridTextureMapperClass & src) : GridTextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_GRID_CLASSIC_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( GridClassicEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual bool Needs_Normals(void) { return true; }
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);
};

class GridEnvironmentMapperClass : public GridTextureMapperClass
{
	W3DMPO_GLUE(GridEnvironmentMapperClass)
public:
	GridEnvironmentMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, unsigned int stage) : GridTextureMapperClass(fps, gridwidth_log2, last_frame, offset, stage) { }
	GridEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage) : GridTextureMapperClass(ini, section, stage) { }
	GridEnvironmentMapperClass(const GridTextureMapperClass & src) : GridTextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_GRID_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( GridEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual bool Needs_Normals(void) { return true; }
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);
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
	ScreenMapperClass(const Vector2 &offset_per_sec, const Vector2 & start_offset, bool clamp_fix,
		const Vector2 &scale, unsigned int stage) : LinearOffsetTextureMapperClass(offset_per_sec, start_offset, clamp_fix, scale, stage) { }
	ScreenMapperClass(const INIClass &ini, const char *section, unsigned int stage) : LinearOffsetTextureMapperClass(ini, section, stage) { }
	ScreenMapperClass(const ScreenMapperClass & src) : LinearOffsetTextureMapperClass(src) { }
	virtual int	Mapper_ID(void) const { return MAPPER_ID_SCREEN;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( ScreenMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);
};

/**
** RandomTextureMapperClass
** Modifies the textures over time
*/
class RandomTextureMapperClass : public ScaleTextureMapperClass
{
	W3DMPO_GLUE(RandomTextureMapperClass)
public:
	RandomTextureMapperClass(float fps, const Vector2 &scale, unsigned int stage);
	RandomTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	RandomTextureMapperClass(const RandomTextureMapperClass & src);

	virtual int	Mapper_ID(void) const { return MAPPER_ID_RANDOM;}

	virtual TextureMapperClass *Clone(void) const { return NEW_REF( RandomTextureMapperClass, (*this)); }

	virtual bool Is_Time_Variant(void) { return true; }
	virtual void Reset(void);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);

protected:
	void randomize(void);
	float FPMS;									// frames per millisecond
	float Remainder;							// remaining time
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
	BumpEnvTextureMapperClass(float rad_per_sec, float scale_factor, const Vector2 & offset_per_sec,
		const Vector2 & start_offset, bool clamp_fix, const Vector2 &scale, unsigned int stage);
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

class GridWSEnvMapperClass : public GridTextureMapperClass
{
public:
	enum AxisType { AXISTYPE_X=0, AXISTYPE_Y=1, AXISTYPE_Z=2 };
	GridWSEnvMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, AxisType axis, unsigned int stage);
	GridWSEnvMapperClass(const GridWSEnvMapperClass & src);
	GridWSEnvMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	virtual void Calculate_Texture_Matrix(Matrix4x4 &tex_matrix);
	virtual bool Needs_Normals(void) { return true; }
protected:
	AxisType		Axis;
};

class GridWSClassicEnvironmentMapperClass : public GridWSEnvMapperClass
{
public:
	GridWSClassicEnvironmentMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, AxisType axis, unsigned int stage);
	GridWSClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	GridWSClassicEnvironmentMapperClass(const GridWSEnvMapperClass & src);
	virtual int	Mapper_ID(void) const { return MAPPER_ID_GRID_WS_CLASSIC_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( GridWSClassicEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
};

class GridWSEnvironmentMapperClass : public GridWSEnvMapperClass
{
public:
	GridWSEnvironmentMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, AxisType axis, unsigned int stage);
	GridWSEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage);
	GridWSEnvironmentMapperClass(const GridWSEnvMapperClass & src);
	virtual int	Mapper_ID(void) const { return MAPPER_ID_GRID_WS_ENVIRONMENT;}
	virtual TextureMapperClass* Clone() const { return NEW_REF( GridWSEnvironmentMapperClass, (*this)); }
	virtual void Apply(int uv_array_index);
};


/*
** Utility functions
*/
void Reset_All_Texture_Mappers(RenderObjClass *robj, bool make_unique);
