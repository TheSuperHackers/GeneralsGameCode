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
 *                     $Archive:: /Commando/Code/ww3d2/mapper.cpp         $* 
 *                                                                         * 
 *                  $Org Author:: Hector_y                                $* 
 *                                                                         * 
 *                      $Author:: Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                         * 
 *                    $Revision:: 33                                      $* 
 *                                                                         * 
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "mapper.h"
#include "ww3d.h"
#include "INI.H"
#include "chunkio.h"
#include "w3derr.h"
#include "meshmatdesc.h"
#include "dx8wrapper.h"
#include "wwdebug.h"
#include "matinfo.h"
#include "rendobj.h"
#include "mesh.h"
#include <RANDOM.H>
#include <bound.h>

Random4Class rand4;

inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }


// HY 1/26/01
// Rewritten to use DX 8 texture matrices

TextureMapperClass::TextureMapperClass(unsigned int stage)
{
	Stage = stage;
	if (Stage >= MeshMatDescClass::MAX_TEX_STAGES) Stage = MeshMatDescClass::MAX_TEX_STAGES - 1;
}


// Scale mapper
// HY 5/16/01
ScaleTextureMapperClass::ScaleTextureMapperClass(const Vector2 &scale, unsigned int stage) :
	TextureMapperClass(stage),
	Scale(scale)
{
}

ScaleTextureMapperClass::ScaleTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	TextureMapperClass(stage)	
{
	Scale.U = ini.Get_Float(section, "UScale", 1.0f);
	Scale.V = ini.Get_Float(section, "VScale", 1.0f);	
}

ScaleTextureMapperClass::ScaleTextureMapperClass(const ScaleTextureMapperClass & src) :
	TextureMapperClass(src),
	Scale(src.Scale)
{
}

void ScaleTextureMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

void ScaleTextureMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	tex_matrix.Make_Identity();
	tex_matrix[0].X = Scale.U;
	tex_matrix[1].Y = Scale.V;
}

// Linear Offset Mapper
LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(const Vector2 &offset_per_sec,
		const Vector2 & start_offset, bool clamp_fix, const Vector2 &scale, unsigned int stage) :
	ScaleTextureMapperClass(scale, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	StartingUVOffset(start_offset),
	ClampFix(clamp_fix)
{
	CurrentUVOffset = StartingUVOffset;

	// HY 5/16/01
	// This is horrible disgusting legacy from the unmentionable API we used before
	// leaving it unchanged because the artists have worked around it
	UVOffsetDeltaPerMS = offset_per_sec * -0.001f;
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	ScaleTextureMapperClass(ini,section,stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time())
{
	float u_offset_per_sec = ini.Get_Float(section, "UPerSec", 0.0f);
	float v_offset_per_sec = ini.Get_Float(section, "VPerSec", 0.0f);
	UVOffsetDeltaPerMS = Vector2(u_offset_per_sec, v_offset_per_sec) * -0.001f;

	float u_start_offset = ini.Get_Float(section, "UOffset", 0.0f);
	float v_start_offset = ini.Get_Float(section, "VOffset", 0.0f);
	StartingUVOffset.Set(u_start_offset,	v_start_offset);
	CurrentUVOffset = StartingUVOffset;

	ClampFix = ini.Get_Bool(section, "ClampFix", false);
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(const LinearOffsetTextureMapperClass & src) :
	ScaleTextureMapperClass(src),
	UVOffsetDeltaPerMS(src.UVOffsetDeltaPerMS),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	StartingUVOffset(src.StartingUVOffset),
	ClampFix(src.ClampFix)
{
	CurrentUVOffset = StartingUVOffset;	// Reset a mapper as it is created
}

void LinearOffsetTextureMapperClass::Reset(void)
{
	Set_Current_UV_Offset(Vector2(0.0f, 0.0f));
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

void LinearOffsetTextureMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	unsigned int delta = WW3D::Get_Sync_Time() - LastUsedSyncTime;
	float del = (float)delta;
	float offset_u = CurrentUVOffset.X + UVOffsetDeltaPerMS.X * del;
	float offset_v = CurrentUVOffset.Y + UVOffsetDeltaPerMS.Y * del;

	// We need to clamp these texture coordinates to a reasonable range so the hardware doesn't
	// choke on them. We do this in one of two ways:
	// If ClampFix is not TRUE we use the fractional part of the offset, restricting it between
	// 0 and 1 with wraparound. This works well for tiled textures.
	// If ClampFix is TRUE we clamp the offsets between -Scale and +Scale with no wraparound.
	// This works well for clamped textures.
	if (!ClampFix) {
		offset_u = offset_u - WWMath::Floor(offset_u);
		offset_v = offset_v - WWMath::Floor(offset_v);	
	} else {
		offset_u = WWMath::Clamp(offset_u, -Scale.X, Scale.X);
		offset_v = WWMath::Clamp(offset_v, -Scale.Y, Scale.Y);
	}

	// Set up the offset matrix
	tex_matrix.Make_Identity();
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	tex_matrix[0].Z=offset_u;
	tex_matrix[0].X=Scale.X;
	tex_matrix[1].Z=offset_v;
	tex_matrix[1].Y=Scale.Y;

	// Update state
	CurrentUVOffset.X = offset_u;
	CurrentUVOffset.Y = offset_v;
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

// Grid Mapper
// HY 5/16/01
GridTextureMapperClass::GridTextureMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, unsigned int stage) :
	TextureMapperClass(stage),
	LastFrame(last_frame),
	Offset(offset)
{
	initialize(fps, gridwidth_log2);
}

GridTextureMapperClass::GridTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	TextureMapperClass(stage)
{
	float fps = ini.Get_Float(section, "FPS", 1.0f);
	unsigned int gridwidth_log2 = ini.Get_Int(section, "Log2Width", 1);
	LastFrame = ini.Get_Int(section, "Last", 0);
	Offset = ini.Get_Int(section, "Offset", 0);
	initialize(fps, gridwidth_log2);
}

GridTextureMapperClass::GridTextureMapperClass(const GridTextureMapperClass & src) :
	TextureMapperClass(src),	
	Sign(src.Sign),
	MSPerFrame(src.MSPerFrame),
	OOGridWidth(src.OOGridWidth),
	GridWidthLog2(src.GridWidthLog2),
	LastFrame(src.LastFrame),
	Offset(src.Offset)
{
	Reset();
}

void GridTextureMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage), m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
}

void GridTextureMapperClass::Reset(void)
{
	Remainder = 0;
	if (Sign >= 0) {
		CurrentFrame = Offset;
	} else {
		CurrentFrame = (LastFrame - 1) - Offset;
	}
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

void GridTextureMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	update_temporal_state();

	float u_offset, v_offset;
	calculate_uv_offset(&u_offset, &v_offset);

	// Set up the offset matrix
	tex_matrix.Make_Identity();
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	tex_matrix[0].Z = u_offset;
	tex_matrix[1].Z = v_offset;
}

void GridTextureMapperClass::Set_Frame_Per_Second(float fps)
{
	initialize(fps, GridWidthLog2);
}

void GridTextureMapperClass::initialize(float fps, unsigned int gridwidth_log2)
{
	unsigned int grid_width = (1 << gridwidth_log2);

	if (LastFrame == 0) LastFrame = (grid_width * grid_width);
	Offset = Offset % LastFrame;
	LastUsedSyncTime = WW3D::Get_Sync_Time();
	GridWidthLog2 = gridwidth_log2;
	OOGridWidth = 1.0f / (float)(grid_width);
	if (fps == 0.0f) {
		// Value of MSPerFrame does not matter as long as it is not 0 - sign will multiply results,
		// zeroing them out.
		Sign = 0;
		MSPerFrame = 1;
		CurrentFrame = Offset;
	} else if (fps < 0.0f) {
		Sign = -1;
		MSPerFrame = (unsigned int)(1000.0f / fabs(fps));
		CurrentFrame = (LastFrame - 1) - Offset;
	} else {
		Sign = 1;
		MSPerFrame = (unsigned int)(1000.0f / fabs(fps));
		CurrentFrame = Offset;
	}
	Remainder = 0;
}

void GridTextureMapperClass::update_temporal_state(void)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta = now - LastUsedSyncTime;
	Remainder += delta;
	LastUsedSyncTime = now;

	int new_frame = (int)CurrentFrame + ((int)(Remainder / MSPerFrame) * Sign);
	new_frame=new_frame % LastFrame;

	if (new_frame<0) {
		CurrentFrame=LastFrame+new_frame;
	} else {
		CurrentFrame=(unsigned int) new_frame;
	}
	Remainder = Remainder % MSPerFrame;
}

void GridTextureMapperClass::calculate_uv_offset(float * u_offset, float * v_offset)
{
	unsigned int row_mask = ~(0xFFFFFFFF << GridWidthLog2);
	unsigned int col_mask = row_mask << GridWidthLog2;
	unsigned int x = CurrentFrame & row_mask;
	unsigned int y = (CurrentFrame & col_mask) >> GridWidthLog2;
	*u_offset = x * OOGridWidth;
	*v_offset = y * OOGridWidth;
}

// Rotate Mapper
// HY 5/16/01
RotateTextureMapperClass::RotateTextureMapperClass(float rad_per_sec, const Vector2 &center, const Vector2 &scale, unsigned int stage) :
	ScaleTextureMapperClass(scale, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f),
	RadiansPerMilliSec(rad_per_sec/1000.0f),
	Center(center)
{
}

RotateTextureMapperClass::RotateTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :	
	ScaleTextureMapperClass(ini, section, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f)
{
	RadiansPerMilliSec=2*WWMATH_PI*ini.Get_Float(section,"Speed",0.1f)/1000.0f;
	Center.U=ini.Get_Float(section,"UCenter",0.0f);
	Center.V=ini.Get_Float(section,"VCenter",0.0f);
}

RotateTextureMapperClass::RotateTextureMapperClass(const RotateTextureMapperClass & src) :
	ScaleTextureMapperClass(src),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	RadiansPerMilliSec(src.RadiansPerMilliSec),
	CurrentAngle(0.0f),
	Center(src.Center)
{
}

void RotateTextureMapperClass::Reset(void)
{
	CurrentAngle = 0.0f;
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

void RotateTextureMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	LastUsedSyncTime=now;

	CurrentAngle+=RadiansPerMilliSec * delta;
	CurrentAngle=fmodf(CurrentAngle,2*WWMATH_PI);
	if (CurrentAngle<0.0f) CurrentAngle+=2*WWMATH_PI;	

	// Set up the rotation matrix	
	float c,s;
	c=WWMath::Cos(CurrentAngle);
	s=WWMath::Sin(CurrentAngle);
	tex_matrix.Make_Identity();

	// subtract center
	// rotate
	// add center
	// then scale
	tex_matrix[0].Set(Scale.X * c, -Scale.X * s, -Scale.X * (c * Center.U - s * Center.V - Center.U), 0.0f);
	tex_matrix[1].Set(Scale.Y * s, Scale.Y * c, -Scale.Y * (s * Center.U + c * Center.V - Center.V), 0.0f);	
}

// SineLinearOffset Mapper
// HY 5/16/01
SineLinearOffsetTextureMapperClass::SineLinearOffsetTextureMapperClass(const Vector3 &uafp, const Vector3 &vafp, const Vector2 &scale, unsigned int stage) :
	ScaleTextureMapperClass(scale, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	UAFP(uafp),
	VAFP(vafp),
	CurrentAngle(0.0f)
{
}

SineLinearOffsetTextureMapperClass::SineLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	ScaleTextureMapperClass(ini, section, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f)
{
	UAFP.X = ini.Get_Float(section, "UAmp", 1.0f);	
	UAFP.Y = ini.Get_Float(section, "UFreq", 1.0f);
	UAFP.Z = ini.Get_Float(section, "UPhase", 0.0f);	

	VAFP.X = ini.Get_Float(section, "VAmp", 1.0f);	
	VAFP.Y = ini.Get_Float(section, "VFreq", 1.0f);
	VAFP.Z = ini.Get_Float(section, "VPhase", 0.0f);
}

SineLinearOffsetTextureMapperClass::SineLinearOffsetTextureMapperClass(const SineLinearOffsetTextureMapperClass & src) :
	ScaleTextureMapperClass(src),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	UAFP(src.UAFP),
	VAFP(src.VAFP),
	CurrentAngle(0.0f)
{	
}

void SineLinearOffsetTextureMapperClass::Reset(void)
{
	CurrentAngle = 0.0f;
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

void SineLinearOffsetTextureMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	LastUsedSyncTime=now;

	const float ms_to_radians=2*WWMATH_PI/1000.0f;

	CurrentAngle+=delta*ms_to_radians;

	float offset_u=UAFP.X*sin(UAFP.Y*CurrentAngle+UAFP.Z*WWMATH_PI);
	float offset_v=VAFP.X*sin(VAFP.Y*CurrentAngle+VAFP.Z*WWMATH_PI);

	// Set up the offset matrix
	tex_matrix.Make_Identity();
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	tex_matrix[0].Z = offset_u;
	tex_matrix[0].X = Scale.X;
	tex_matrix[1].Z = offset_v;
	tex_matrix[1].Y = Scale.Y;
}

// StepLinearOffset Mapper
// HY 5/16/01
StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(const Vector2 &step,
		float steps_per_sec, bool clamp_fix, const Vector2 &scale, unsigned int stage) :
	ScaleTextureMapperClass(scale, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Step(step),
	StepsPerMilliSec(steps_per_sec/1000.0f),
	CurrentStep(0.0f,0.0f),
	Remainder(0),
	ClampFix(clamp_fix)
{
}

StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	ScaleTextureMapperClass(ini, section, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentStep(0.0f,0.0f),
	Remainder(0)
{
	Step.U = ini.Get_Float(section, "UStep", 0.0f);	
	Step.V = ini.Get_Float(section, "VStep", 0.0f);	
	StepsPerMilliSec = ini.Get_Float(section, "SPS", 0.0f)/1000.0f;
	ClampFix = ini.Get_Bool(section, "ClampFix", false);
}

StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(const StepLinearOffsetTextureMapperClass & src) :
	ScaleTextureMapperClass(src),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Step(src.Step),
	StepsPerMilliSec(src.StepsPerMilliSec),
	CurrentStep(0.0f,0.0f),
	Remainder(0),
	ClampFix(src.ClampFix)
{	
}

void StepLinearOffsetTextureMapperClass::Reset(void)
{
	LastUsedSyncTime = WW3D::Get_Sync_Time();
	CurrentStep.Set(0.0f,0.0f);
	Remainder=0;
}

void StepLinearOffsetTextureMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	LastUsedSyncTime=now;

	Remainder+=delta;
	int num_steps=(int) (StepsPerMilliSec*Remainder);	

	if (num_steps!=0)
	{
		CurrentStep+=Step*num_steps;
		Remainder-=num_steps/(float)StepsPerMilliSec;
	}
	
	// We need to clamp these texture coordinates to a reasonable range so the hardware doesn't
	// choke on them. We do this in one of two ways:
	// If ClampFix is not TRUE we use the fractional part of the offset, restricting it between
	// 0 and 1 with wraparound. This works well for tiled textures.
	// If ClampFix is TRUE we clamp the offsets between -Scale and +Scale with no wraparound.
	// This works well for clamped textures.
	if (!ClampFix) {
		CurrentStep.U -= WWMath::Floor(CurrentStep.U);
	  	CurrentStep.V -= WWMath::Floor(CurrentStep.V);	
	} else {
		CurrentStep.U = WWMath::Clamp(CurrentStep.U, -Scale.X, Scale.X);
		CurrentStep.V = WWMath::Clamp(CurrentStep.V, -Scale.Y, Scale.Y);
	}

	// Set up the offset matrix
	tex_matrix.Make_Identity();
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	tex_matrix[0].Z = CurrentStep.U;
	tex_matrix[0].X = Scale.X;
	tex_matrix[1].Z = CurrentStep.V;
	tex_matrix[1].Y = Scale.Y;
}

// ZigZagLinearOffset Mapper
// HY 5/16/01
ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(const Vector2 &speed, float period, const Vector2 &scale, unsigned int stage) :
	ScaleTextureMapperClass(scale, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Speed(speed/1000.0f),
	Period(period*1000.0f),
	Remainder(0)
{
	// since we're zigzagging, a negative period is the same as a positive one
	if (Period<0.0f) Period=-Period;
	Half_Period=0.5f*Period;
}

ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	ScaleTextureMapperClass(ini, section, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Remainder(0)
{
	Speed.U = ini.Get_Float(section, "UPerSec", 0.0f)/1000.0f;
	Speed.V = ini.Get_Float(section, "VPerSec", 0.0f)/1000.0f;
	Period = ini.Get_Float(section, "Period", 0.0f)*1000.0f;
	if (Period<0.0f) Period=-Period;
	Half_Period=0.5f*Period;
}

ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(const ZigZagLinearOffsetTextureMapperClass & src) :
	ScaleTextureMapperClass(src),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),	
	Speed(src.Speed),
	Period(src.Period),
	Half_Period(src.Half_Period),
	Remainder(0)
{		
}

void ZigZagLinearOffsetTextureMapperClass::Reset(void)
{
	LastUsedSyncTime = WW3D::Get_Sync_Time();
	Remainder=0;
}

void ZigZagLinearOffsetTextureMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	LastUsedSyncTime=now;
	Remainder+=delta;

	float offset_u=0.0f;
	float offset_v=0.0f;

	if (Period>0.0f)
	{
		// figure out the fractional number of periods
		int num_periods=(int) (Remainder/Period);		
		Remainder-=num_periods*Period;

		float time=0.0f;		
		if (Remainder>Half_Period) {
			time=Period-Remainder;
		} else {
			time=Remainder;
		}
		offset_u=Speed.U * time;
		offset_v=Speed.V * time;
	}	

	// Set up the offset matrix
	tex_matrix.Make_Identity();
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	tex_matrix[0].Z = offset_u;
	tex_matrix[0].X = Scale.X;
	tex_matrix[1].Z = offset_v;
	tex_matrix[1].Y = Scale.Y;
}

// ----------------------------------------------------------------------------
//
// Environment mapper calculates the texture coordinates based on
// transformed normals
//
// ----------------------------------------------------------------------------

void ClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Get camera normals
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

void ClassicEnvironmentMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	// The canonical environment map
	// scale the normal by (.5,.5) and add (.5,.5) to move it to (0,1) range
	// and ignore the Z component
	tex_matrix.Init(	0.5f, 0.0f, 0.0f, 0.5f,
							0.0f, 0.5f, 0.0f, 0.5f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f);
}

void EnvironmentMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Get camera reflection vector
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

void EnvironmentMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	// The canonical environment map
	// scale the normal by (.5,.5) and add (.5,.5) to move it to (0,1) range	
	tex_matrix.Init(	0.5f, 0.0f, 0.0f, 0.5f,
							0.0f, 0.5f, 0.0f, 0.5f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f );
}

// Edge mapper
EdgeMapperClass::EdgeMapperClass(unsigned int stage) :
	TextureMapperClass(stage),
	VSpeed(0.0f),
	UseReflect(false),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	VOffset(0.0f)
{
}

EdgeMapperClass::EdgeMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	TextureMapperClass(stage),
	VSpeed(0.0f),
	UseReflect(false),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	VOffset(0.0f)
{
	VSpeed=ini.Get_Float(section, "VPerSec", 0.0f);
	VOffset=ini.Get_Float(section, "VStart", 0.0f);
	UseReflect=ini.Get_Bool(section, "UseReflect", false);
}

EdgeMapperClass::EdgeMapperClass(const EdgeMapperClass & src):
	TextureMapperClass(src.Stage),
	VSpeed(src.VSpeed),
	UseReflect(src.UseReflect),
	VOffset(src.VOffset),
	LastUsedSyncTime(WW3D::Get_Sync_Time())
{
}

void EdgeMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Get camera reflection vector
	if (UseReflect)
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
	else
		DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

void EdgeMapperClass::Reset(void)
{
	LastUsedSyncTime = WW3D::Get_Sync_Time();
	VOffset = 0.0f;
}

void EdgeMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	unsigned int now=WW3D::Get_Sync_Time();

	float delta=(now-LastUsedSyncTime)*0.001f;
	LastUsedSyncTime=now;

	VOffset+=delta*VSpeed;
	VOffset-=WWMath::Floor(VOffset);

	// takes the Z component and
	// uses it to index the texture
	tex_matrix.Init(	0.0f, 0.0f, 0.5f, 0.5f,
							0.0f, 0.0f, 0.0f, VOffset,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f );	
}

WSEnvMapperClass::WSEnvMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	TextureMapperClass(stage),
	Axis(AXISTYPE_Z)
{
	char temp[2];
	ini.Get_String(section, "Axis", "Z", &temp[0], 2);
	switch(temp[0]) {
		case 'X':
		case 'x':
			Axis = AXISTYPE_X;
			break;
		case 'Y':
		case 'y':
			Axis = AXISTYPE_Y;
			break;
		case 'Z':
		case 'z':
			Axis = AXISTYPE_Z;
			break;
		default:
			Axis = AXISTYPE_Z;
			break;
	}
}

void WSEnvMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	// The canonical environment map
	// scale the normal by (.5,.5) and add (.5,.5) to move it to (0,1) range
	switch (Axis) {
		case AXISTYPE_X:
			tex_matrix.Init(	0.0f, 0.5f, 0.0f, 0.5f,
									0.0f, 0.0f, 0.5f, 0.5f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f );
			break;
		case AXISTYPE_Y:
			tex_matrix.Init(	0.5f, 0.0f, 0.0f, 0.5f,
									0.0f, 0.0f, 0.5f, 0.5f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f );
			break;
		case AXISTYPE_Z:
		default:
			tex_matrix.Init(	0.5f, 0.0f, 0.0f, 0.5f,
									0.0f, 0.5f, 0.0f, 0.5f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f );
			break;
	}
	// multiply by inverse of view transform	
	Matrix4x4 mat;	
	DX8Wrapper::Get_Transform(D3DTS_VIEW,mat);		
	Matrix4x4 mat2(	mat[0].X, mat[1].X, mat[2].X, 0.0f,
						mat[0].Y, mat[1].Y, mat[2].Y, 0.0f,
						mat[0].Z, mat[1].Z, mat[2].Z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f );						  	
	tex_matrix = tex_matrix * mat2;	
}

void WSClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Get camera normals
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

void WSEnvironmentMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);	

	// Get camera reflection
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

void GridClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Get camera normals
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

void GridClassicEnvironmentMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	update_temporal_state();

	float u_offset, v_offset;
	calculate_uv_offset(&u_offset, &v_offset);

	float del = 0.5f * OOGridWidth;	
	// Set up the offset matrix		
	tex_matrix.Init(	del,	0.0f,	0.0f,	u_offset + del,
							0.0f,	del,	0.0f,	v_offset + del,
							0.0f,	0.0f,	1.0f,	0.0f,
							0.0f, 0.0f, 0.0f, 1.0f );		
}

void GridEnvironmentMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Get camera space reflection
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

void GridEnvironmentMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	update_temporal_state();

	float u_offset, v_offset;
	calculate_uv_offset(&u_offset, &v_offset);

	float del=0.5f * OOGridWidth;	
	// Set up the offset matrix		
	tex_matrix.Init(	del,	0.0f,	0.0f,	u_offset + del,
							0.0f,	del,	0.0f,	v_offset + del,
							0.0f,	0.0f,	1.0f,	0.0f,
							0.0f, 0.0f, 0.0f, 1.0f );		
}

void ScreenMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);	

	// Get camera space position
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);

	// Tell rasterizer what to expect
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_PROJECTED | D3DTTFF_COUNT3);
}

void ScreenMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	unsigned int delta = WW3D::Get_Sync_Time() - LastUsedSyncTime;
	float del = (float)delta;
	float offset_u = CurrentUVOffset.X + UVOffsetDeltaPerMS.X * del;
	float offset_v = CurrentUVOffset.Y + UVOffsetDeltaPerMS.Y * del;

	// We need to clamp these texture coordinates to a reasonable range so the hardware doesn't
	// choke on them. We do this in one of two ways:
	// If ClampFix is not TRUE we use the fractional part of the offset, restricting it between
	// 0 and 1 with wraparound. This works well for tiled textures.
	// If ClampFix is TRUE we clamp the offsets between -Scale and +Scale with no wraparound.
	// This works well for clamped textures.
	if (!ClampFix) {
		offset_u = offset_u - WWMath::Floor(offset_u);
		offset_v = offset_v - WWMath::Floor(offset_v);	
	} else {
		offset_u = WWMath::Clamp(offset_u, -Scale.X, Scale.X);
		offset_v = WWMath::Clamp(offset_v, -Scale.Y, Scale.Y);
	}

	// multiply by projection matrix	
	// followed by scale and translation
	DX8Wrapper::Get_Transform(D3DTS_PROJECTION, tex_matrix);	
	tex_matrix[0] *= Scale.X; // entire row since we're pre-multiplying
	tex_matrix[1] *= Scale.Y;
	Vector4 last(tex_matrix[3]); // this gets the w
	last *= offset_u; // multiply by w because the projected flag will divide by w
	tex_matrix[0] += last;
	last = tex_matrix[3];
	last *= offset_v;
	tex_matrix[1] += last;

	// Update state
	CurrentUVOffset.X = offset_u;
	CurrentUVOffset.Y = offset_v;
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

RandomTextureMapperClass::RandomTextureMapperClass(float fps, const Vector2 &scale, unsigned int stage):
	ScaleTextureMapperClass(scale, stage),
	FPMS(fps/1000.0f),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Speed(0.0f,0.0f),
	Remainder(0)
{
	randomize();
}

RandomTextureMapperClass::RandomTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage):
	ScaleTextureMapperClass(ini, section, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Remainder(0)
{
	FPMS = ini.Get_Float(section, "FPS", 0.0f)/1000.0f;
	Speed.U = ini.Get_Float(section, "UPerSec", 0.0f)/1000.0f;
	Speed.V = ini.Get_Float(section, "VPerSec", 0.0f)/1000.0f;
	randomize();
}

RandomTextureMapperClass::RandomTextureMapperClass(const RandomTextureMapperClass & src):
	ScaleTextureMapperClass(src),
	FPMS(src.FPMS),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Speed(src.Speed),
	Remainder(0)
{
	randomize();
}

void RandomTextureMapperClass::randomize(void)
{
	CurrentAngle=2*WWMATH_PI*rand4.Get_Float();
	Center.U=rand4.Get_Float();
	Center.V=rand4.Get_Float();
}

void RandomTextureMapperClass::Reset(void)
{
	LastUsedSyncTime = WW3D::Get_Sync_Time();
	Remainder=0;
}

void RandomTextureMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta=now-LastUsedSyncTime;
	LastUsedSyncTime=now;
	Remainder+=delta;
	
	if (FPMS!=0.0f) {

		int num_frames=(int) (Remainder*FPMS);

		if (num_frames!=0) {
			randomize();
			Remainder-=num_frames/FPMS;
		}		
	}	

	// Set up the random matrix - start with a rotation matrix of 'CurrentAngle' about the Z-axis.
	// We apply the scale matrix to the right of the unscaled rotate/offset matrix, because we
	// don't want the scaling to affect the offset. So we get:
	// [  c -s  uoff ]     [ Sx  0   0 ]     [ cSx -sSy uoff ]
	// [  s  c  voff ]  *  [ 0   Sy  0 ]  =  [ sSx  cSy voff ]
	// [  0  0   1   ]     [ 0   0   1 ]     [  0    0    1  ]
	float c = cosf(CurrentAngle);
	float s = sinf(CurrentAngle);
	tex_matrix.Make_Identity();
	tex_matrix[0][0] = c * Scale.X;
	tex_matrix[0][1] = -s * Scale.Y;
	tex_matrix[1][0] = s * Scale.X;
	tex_matrix[1][1] = c * Scale.Y;

	// Offset matrix
	float uoff = Center.U + Remainder * Speed.U;
	float voff = Center.V + Remainder * Speed.V;
	uoff = fmodf(uoff, 1.0f);
	voff = fmodf(voff, 1.0f);
	tex_matrix[0].Z = uoff;
	tex_matrix[1].Z = voff;
}

// BumpEnv Mapper
// GTH 8/22/01
BumpEnvTextureMapperClass::BumpEnvTextureMapperClass(float rad_per_sec, float scale_factor,
		const Vector2 & offset_per_sec, const Vector2 & start_offset, bool clamp_fix,
		const Vector2 &scale, unsigned int stage) :
	LinearOffsetTextureMapperClass(offset_per_sec, start_offset, clamp_fix, scale, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f),
	RadiansPerSecond(rad_per_sec),
	ScaleFactor(scale_factor)
{
}

BumpEnvTextureMapperClass::BumpEnvTextureMapperClass(INIClass &ini, const char *section, unsigned int stage) :	
	LinearOffsetTextureMapperClass(ini, section, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f)
{
	RadiansPerSecond = 2*WWMATH_PI*ini.Get_Float(section,"BumpRotation",0.0f);	
	ScaleFactor = ini.Get_Float(section,"BumpScale",1.0f);
}

BumpEnvTextureMapperClass::BumpEnvTextureMapperClass(const BumpEnvTextureMapperClass & src) :
	LinearOffsetTextureMapperClass(src),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f),
	RadiansPerSecond(src.RadiansPerSecond),
	ScaleFactor(src.ScaleFactor)
{
}

void BumpEnvTextureMapperClass::Apply(int uv_array_index)
{
	LinearOffsetTextureMapperClass::Apply(uv_array_index);

	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	LastUsedSyncTime=now;

	CurrentAngle+=RadiansPerSecond * delta * 0.001f;
	CurrentAngle=fmodf(CurrentAngle,2*WWMATH_PI);

	// Compute the sine and cosine for the bump matrix
	float c,s;
	c=ScaleFactor * WWMath::Fast_Cos(CurrentAngle);
	s=ScaleFactor * WWMath::Fast_Sin(CurrentAngle);

	// Set the Bump Environment Matrix
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_BUMPENVMAT00, F2DW(c));
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_BUMPENVMAT01, F2DW(-s));
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_BUMPENVMAT10, F2DW(s));
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_BUMPENVMAT11, F2DW(c));
}

/*
** Utility functions
*/
void Reset_All_Texture_Mappers(RenderObjClass *robj, bool make_unique)
{

	if (robj->Class_ID()==RenderObjClass::CLASSID_MESH) {
		MeshClass *mesh=(MeshClass*) robj;	
		MaterialInfoClass *minfo = robj->Get_Material_Info();
		if (minfo && minfo->Has_Time_Variant_Texture_Mappers()) {
			if (make_unique) {
				mesh->Make_Unique();
				minfo->Make_Vertex_Materials_Unique();
			}
			minfo->Reset_Texture_Mappers();
			minfo->Release_Ref();
		}
	} else {
		int num_obj = robj->Get_Num_Sub_Objects();
		RenderObjClass *sub_obj;

		for (int i = 0; i < num_obj; i++) {
			sub_obj = robj->Get_Sub_Object(i);
			if (sub_obj) {
				Reset_All_Texture_Mappers(sub_obj, make_unique);
				sub_obj->Release_Ref();
			}
		}
	}
}


GridWSEnvMapperClass::GridWSEnvMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, AxisType axis, unsigned int stage):
	GridTextureMapperClass(fps, gridwidth_log2, last_frame, offset, stage),
	Axis(axis)
{
}

GridWSEnvMapperClass::GridWSEnvMapperClass(const GridWSEnvMapperClass & src) :
	GridTextureMapperClass(src),
	Axis(src.Axis)
{
}

GridWSEnvMapperClass::GridWSEnvMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	GridTextureMapperClass(ini, section, stage),
	Axis(AXISTYPE_Z)
{
	char temp[2];
	ini.Get_String(section, "Axis", "Z", &temp[0], 2);
	switch(temp[0]) {
		case 'X':
		case 'x':
			Axis = AXISTYPE_X;
			break;
		case 'Y':
		case 'y':
			Axis = AXISTYPE_Y;
			break;
		case 'Z':
		case 'z':
			Axis = AXISTYPE_Z;
			break;
		default:
			Axis = AXISTYPE_Z;
			break;
	}
}

void GridWSEnvMapperClass::Calculate_Texture_Matrix(Matrix4x4 &tex_matrix)
{
	// multiply by inverse of view transform	
	Matrix4x4 mat;	
	DX8Wrapper::Get_Transform(D3DTS_VIEW,mat);		
	Matrix4x4 mv (	mat[0].X, mat[1].X, mat[2].X, 0.0f,
						mat[0].Y, mat[1].Y, mat[2].Y, 0.0f,
						mat[0].Z, mat[1].Z, mat[2].Z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f );	

	update_temporal_state();

	float u_offset, v_offset;
	calculate_uv_offset(&u_offset, &v_offset);

	float del=0.5f * OOGridWidth;	
	// Set up the offset matrix		
	Matrix4x4 md;	

	switch (Axis) {
		case AXISTYPE_X:
			md.Init(				0.0f, del, 0.0f, u_offset + del,
									0.0f, 0.0f, del, v_offset + del,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f );
			break;
		case AXISTYPE_Y:
			md.Init(				del, 0.0f, 0.0f, u_offset + del,
									0.0f, 0.0f, del, v_offset + del,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f );
			break;
		case AXISTYPE_Z:
		default:
			md.Init(				del, 0.0f, 0.0f, u_offset + del,
									0.0f, del, 0.0f, v_offset + del,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f );			
			break;
	}	
	// multiply by inverse of view transform, then
	// change the world space reflection vector to a UV coordinate
	// then offset by the grid coordinate

	tex_matrix = md * mv;
}

/***********************************************************************************************
 * GridWSClassicEnvironmentMapperClass::GridWSClassicEnvironmentMapperClass -- Grid WS Env Map *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
GridWSClassicEnvironmentMapperClass::GridWSClassicEnvironmentMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, AxisType axis, unsigned int stage) :
	GridWSEnvMapperClass(fps, gridwidth_log2, last_frame, offset, axis, stage)
{	
}

GridWSClassicEnvironmentMapperClass::GridWSClassicEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage):
	GridWSEnvMapperClass(ini,section,stage)
{	
}

GridWSClassicEnvironmentMapperClass::GridWSClassicEnvironmentMapperClass(const GridWSEnvMapperClass & src):
	GridWSEnvMapperClass(src)	
{
}

void GridWSClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Get camera normals
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

/***********************************************************************************************
 * GridWSEnvironmentMapperClass::GridWSEnvironmentMapperClass -- grid ws env                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/29/2002  hy : Created.                                                                  *
 *=============================================================================================*/
GridWSEnvironmentMapperClass::GridWSEnvironmentMapperClass(float fps, unsigned int gridwidth_log2, unsigned int last_frame, unsigned int offset, AxisType axis, unsigned int stage) :
	GridWSEnvMapperClass(fps, gridwidth_log2, last_frame, offset, axis, stage)
{	
}

GridWSEnvironmentMapperClass::GridWSEnvironmentMapperClass(const INIClass &ini, const char *section, unsigned int stage):
	GridWSEnvMapperClass(ini, section, stage)
{
}

GridWSEnvironmentMapperClass::GridWSEnvironmentMapperClass(const GridWSEnvMapperClass & src):
	GridWSEnvMapperClass(src)
{
}

void GridWSEnvironmentMapperClass::Apply(int uv_array_index)
{
	// Set up the texture matrix
	Matrix4x4 m;
	Calculate_Texture_Matrix(m);
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Get camera space reflection
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}
