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
 *                     $Archive:: /VSS_Sync/ww3d2/mapper.cpp              $* 
 *                                                                         * 
 *                      $Author:: Vss_sync                                $* 
 *                                                                         * 
 *                     $Modtime:: 8/30/01 1:38a                           $* 
 *                                                                         * 
 *                    $Revision:: 31                                      $* 
 *                                                                         * 
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


inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }


// HY 1/26/01
// Rewritten to use DX 8 texture matrices

TextureMapperClass::TextureMapperClass(unsigned int stage)
{
	Stage = stage;
	if (Stage >= MeshMatDescClass::MAX_TEX_STAGES) Stage = MeshMatDescClass::MAX_TEX_STAGES - 1;
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(const Vector2 &offset_per_sec, const Vector2 &scale, unsigned int stage) :
	ScaleTextureMapperClass(scale, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time())
{
	CurrentUVOffset.X = 0.0f;
	CurrentUVOffset.Y = 0.0f;

	// HY 5/16/01
	// This is horrible disgusting legacy from the unmentionable API we used before
	// leaving it unchanged because the artists have worked around it
	UVOffsetDeltaPerMS = offset_per_sec * -0.001f;
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	ScaleTextureMapperClass(ini,section,stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time())
{
	CurrentUVOffset.X = 0.0f;
	CurrentUVOffset.Y = 0.0f;

	float u_offset_per_sec = ini.Get_Float(section, "UPerSec", 0.0f);
	float v_offset_per_sec = ini.Get_Float(section, "VPerSec", 0.0f);
	UVOffsetDeltaPerMS = Vector2(u_offset_per_sec, v_offset_per_sec) * -0.001f;
}

LinearOffsetTextureMapperClass::LinearOffsetTextureMapperClass(const LinearOffsetTextureMapperClass & src) :
	ScaleTextureMapperClass(src),
	UVOffsetDeltaPerMS(src.UVOffsetDeltaPerMS),
	LastUsedSyncTime(WW3D::Get_Sync_Time())
{
	CurrentUVOffset.X = 0.0f;
	CurrentUVOffset.Y = 0.0f;
}

void LinearOffsetTextureMapperClass::Apply(int uv_array_index)
{
	unsigned int delta = WW3D::Get_Sync_Time() - LastUsedSyncTime;
	float del = (float)delta;
	float offset_u = CurrentUVOffset.X + UVOffsetDeltaPerMS.X * del;
	float offset_v = CurrentUVOffset.Y + UVOffsetDeltaPerMS.Y * del;

	// ensure both coordinates of offset are in [0, 1] range:
	offset_u = offset_u - WWMath::Floor(offset_u);
	offset_v = offset_v - WWMath::Floor(offset_v);	

	// Set up the offset matrix
	Matrix3D m(true);
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	m[0].Z=offset_u;
	m[0].X=Scale.X;
	m[1].Z=offset_v;
	m[1].Y=Scale.Y;
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);

	// Update state
	CurrentUVOffset.X = offset_u;
	CurrentUVOffset.Y = offset_v;
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

// Scale mapper
// HY 5/16/01
ScaleTextureMapperClass::ScaleTextureMapperClass(const Vector2 &scale, unsigned int stage) :
	TextureMapperClass(stage),
	Scale(scale)
{
}

ScaleTextureMapperClass::ScaleTextureMapperClass(unsigned int stage) :
	TextureMapperClass(stage),
	Scale(1.0f,1.0f)
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
	// Set up the scale matrix
	Matrix3D m(true);	
	
	m[0].X=Scale.U;
	m[1].Y=Scale.V;
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

// Grid Mapper
// HY 5/16/01
GridTextureMapperClass::GridTextureMapperClass(float fps, unsigned int gridwidth_log2, unsigned int stage) :
	TextureMapperClass(stage)
{
	LastFrame = 0;
	initialize(fps, gridwidth_log2);
}

GridTextureMapperClass::GridTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	TextureMapperClass(stage)
{
	float fps = ini.Get_Float(section,"FPS", 1.0f);
	unsigned int gridwidth_log2 = ini.Get_Int(section,"Log2Width", 1);
	LastFrame=ini.Get_Int(section,"Last",0);
	initialize(fps, gridwidth_log2);
}

GridTextureMapperClass::GridTextureMapperClass(const GridTextureMapperClass & src) :
	TextureMapperClass(src),	
	Sign(src.Sign),
	MSPerFrame(src.MSPerFrame),
	OOGridWidth(src.OOGridWidth),
	GridWidthLog2(src.GridWidthLog2),
	LastFrame(src.LastFrame)
{
	Reset();
}

void GridTextureMapperClass::Apply(int uv_array_index)
{
	update_temporal_state();

	float u_offset, v_offset;
	calculate_uv_offset(&u_offset, &v_offset);

	// Set up the offset matrix
	Matrix3D m(true);
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	m[0].Z = u_offset;
	m[1].Z = v_offset;
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage), m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
}

void GridTextureMapperClass::Reset(void)
{
	Remainder = 0;
	CurrentFrame = Sign == -1 ? (1 << (GridWidthLog2 + GridWidthLog2)) - 1 : 0;
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

void GridTextureMapperClass::Set_Frame_Per_Second(float fps)
{
	initialize(fps, GridWidthLog2);
}

void GridTextureMapperClass::initialize(float fps, unsigned int gridwidth_log2)
{
	unsigned int grid_width = (1 << GridWidthLog2);

	if (LastFrame == 0) LastFrame = (grid_width * grid_width);
	LastUsedSyncTime = WW3D::Get_Sync_Time();
	GridWidthLog2 = gridwidth_log2;
	OOGridWidth = 1.0f / (float)(grid_width);
	if (fps == 0.0f) {
		// Value of MSPerFrame does not matter as long as it is not 0 - sign will multiply results,
		// zeroing them out.
		Sign = 0;
		MSPerFrame = 1;
		CurrentFrame = 0;
	} else if (fps < 0.0f) {
		Sign = -1;
		MSPerFrame = (unsigned int)(1000.0f / fabs(fps));
		CurrentFrame = LastFrame - 1;
	} else {
		Sign = 1;
		MSPerFrame = (unsigned int)(1000.0f / fabs(fps));
		CurrentFrame = 0;
	}
	Remainder = 0;
}

void GridTextureMapperClass::update_temporal_state(void)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta = now - LastUsedSyncTime;
	Remainder += delta;
	int new_frame = (int)CurrentFrame + ((int)(Remainder / MSPerFrame) * Sign);
	CurrentFrame = *((unsigned int *)&new_frame);
	LastUsedSyncTime = now;

	CurrentFrame = CurrentFrame % LastFrame;
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
RotateTextureMapperClass::RotateTextureMapperClass(float rad_per_sec, const Vector2 &center, unsigned int stage) :
	ScaleTextureMapperClass(stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f),
	RadiansPerSec(rad_per_sec),
	Center(center)
{
}

RotateTextureMapperClass::RotateTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :	
	ScaleTextureMapperClass(ini,section,stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f)
{
	RadiansPerSec=2*WWMATH_PI*ini.Get_Float(section,"Speed",0.1f);	
	Center.U=ini.Get_Float(section,"UCenter",0.0f);
	Center.V=ini.Get_Float(section,"VCenter",0.0f);
}

RotateTextureMapperClass::RotateTextureMapperClass(const RotateTextureMapperClass & src) :
	ScaleTextureMapperClass(src),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	RadiansPerSec(src.RadiansPerSec),
	CurrentAngle(0.0f),
	Center(src.Center)
{
}

void RotateTextureMapperClass::Apply(int uv_array_index)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	LastUsedSyncTime=now;

	CurrentAngle+=RadiansPerSec * delta / 1000.0f;
	CurrentAngle=fmodf(CurrentAngle,2*WWMATH_PI);
	if (CurrentAngle<0.0f) CurrentAngle+=2*WWMATH_PI;	

	// Set up the rotation matrix	
	float c,s;
	c=WWMath::Cos(CurrentAngle);
	s=WWMath::Sin(CurrentAngle);
	Matrix4 m(true);

	// subtract center
	// rotate
	// add center
	// then scale
	m[0].Set(Scale.X*c,-Scale.X*s,-Scale.X*(c*Center.U-s*Center.V-Center.U),0.0f);
	m[1].Set(Scale.Y*s,Scale.Y*c,-Scale.Y*(s*Center.U+c*Center.V-Center.V),0.0f);	

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

// SineLinearOffset Mapper
// HY 5/16/01
SineLinearOffsetTextureMapperClass::SineLinearOffsetTextureMapperClass(const Vector3 &uafp, const Vector3 &vafp, unsigned int stage) :
	TextureMapperClass(stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	UAFP(uafp),
	VAFP(vafp),
	CurrentAngle(0.0f)
{
}

SineLinearOffsetTextureMapperClass::SineLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	TextureMapperClass(stage),
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
	TextureMapperClass(src),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	UAFP(src.UAFP),
	VAFP(src.VAFP),
	CurrentAngle(0.0f)
{	
}

void SineLinearOffsetTextureMapperClass::Apply(int uv_array_index)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	LastUsedSyncTime=now;

	CurrentAngle+=(delta / 1000.0f)*WWMATH_PI*2;

	float offset_u=UAFP.X*sin(UAFP.Y*CurrentAngle+UAFP.Z*WWMATH_PI);
	float offset_v=VAFP.X*sin(VAFP.Y*CurrentAngle+VAFP.Z*WWMATH_PI);

	// ensure both coordinates of offset are in [0, 1] range:
	offset_u = offset_u - WWMath::Floor(offset_u);
	offset_v = offset_v - WWMath::Floor(offset_v);	

	// Set up the offset matrix
	Matrix3D m(true);
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	m[0].Z=offset_u;
	m[1].Z=offset_v;
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);

}

// StepLinearOffset Mapper
// HY 5/16/01
StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(const Vector2 &step, float steps_per_sec, unsigned int stage) :
	TextureMapperClass(stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Step(step),
	StepsPerSec(steps_per_sec),
	CurrentStep(0.0f,0.0f)
{
}

StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	TextureMapperClass(stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentStep(0.0f,0.0f)
{
	Step.U = ini.Get_Float(section, "UStep", 0.0f);	
	Step.V = ini.Get_Float(section, "VStep", 0.0f);	
	StepsPerSec = ini.Get_Float(section, "SPS", 0.0f);	
}

StepLinearOffsetTextureMapperClass::StepLinearOffsetTextureMapperClass(const StepLinearOffsetTextureMapperClass & src) :
	TextureMapperClass(src),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Step(src.Step),
	StepsPerSec(src.StepsPerSec),
	CurrentStep(0.0f,0.0f)
{	
}

void StepLinearOffsetTextureMapperClass::Apply(int uv_array_index)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	float ms_per_step=1000.0f / StepsPerSec;

	if (delta>ms_per_step)
	{
		LastUsedSyncTime=now;
		CurrentStep+=Step*StepsPerSec;
	}
	
	// ensure both coordinates of offset are in [0, 1] range:
	CurrentStep.U -= WWMath::Floor(CurrentStep.U);
	CurrentStep.V -= WWMath::Floor(CurrentStep.V);	

	// Set up the offset matrix
	Matrix3D m(true);
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	m[0].Z=CurrentStep.U;
	m[1].Z=CurrentStep.V;
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);

}

void StepLinearOffsetTextureMapperClass::Reset(void)
{
	LastUsedSyncTime = WW3D::Get_Sync_Time();
	CurrentStep.Set(0.0f,0.0f);
}

// ZigZagLinearOffset Mapper
// HY 5/16/01
ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(const Vector2 &speed, float period, unsigned int stage) :
	TextureMapperClass(stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Speed(speed),
	Period(period)	
{
}

ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage) :
	TextureMapperClass(stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time())	
{
	Speed.U = ini.Get_Float(section, "UPerSec", 0.0f);
	Speed.V = ini.Get_Float(section, "VPerSec", 0.0f);
	Period = ini.Get_Float(section, "Period", 0.0f);
}

ZigZagLinearOffsetTextureMapperClass::ZigZagLinearOffsetTextureMapperClass(const ZigZagLinearOffsetTextureMapperClass & src) :
	TextureMapperClass(src),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),	
	Speed(src.Speed),
	Period(src.Period)
{	
}

void ZigZagLinearOffsetTextureMapperClass::Apply(int uv_array_index)
{
	unsigned int now = WW3D::Get_Sync_Time();
	unsigned int delta =  now - LastUsedSyncTime;
	float time=delta/1000.0f;

	if (time>Period)
	{
		LastUsedSyncTime=now;		
	}
	
	float offset_u,offset_v;

	float half_period=0.5f*Period;
	if (time<half_period)
	{
		offset_u=Speed.U * time;
		offset_v=Speed.V * time;
	} else
	{
		offset_u=Speed.U * (Period - time);
		offset_v=Speed.V * (Period - time);
	}

	// ensure both coordinates of offset are in [0, 1] range:
	offset_u = offset_u - WWMath::Floor(offset_u);
	offset_v = offset_v - WWMath::Floor(offset_v);	

	// Set up the offset matrix
	Matrix3D m(true);
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	m[0].Z=offset_u;
	m[1].Z=offset_v;
	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);

}

void ZigZagLinearOffsetTextureMapperClass::Reset(void)
{
	LastUsedSyncTime = WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------
//
// Environment mapper calculates the texture coordinates based on
// transformed normals
//
// ----------------------------------------------------------------------------

void ClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
	// The canonical environment map
	// scale the normal by (.5,.5) and add (.5,.5) to move it to (0,1) range
	// and ignore the Z component
	Matrix3D matenv(	0.5f, 0.0f, 0.0f, 0.5f,
							0.0f, 0.5f, 0.0f, 0.5f,
							0.0f, 0.0f, 1.0f, 0.0f );

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),matenv);

	// Get camera normals
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

void EnvironmentMapperClass::Apply(int uv_array_index)
{
	// The canonical environment map
	// scale the normal by (.25,.25) and add (.5,.5) to move it to (0,1) range
	// the additional half is to fudge the 1+z normalization factor
	// and ignore the Z component
	Matrix3D matenv(	0.25f, 0.0f, 0.0f, 0.5f,
							0.0f, 0.25f, 0.0f, 0.5f,
							0.0f, 0.0f, 1.0f, 0.0f );	

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),matenv);

	// Get camera reflection vector
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

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
	unsigned int now=WW3D::Get_Sync_Time();

	float delta=(now-LastUsedSyncTime)*0.001f;
	LastUsedSyncTime=now;

	VOffset+=delta*VSpeed;
	VOffset-=WWMath::Floor(VOffset);

	// takes the Z component and
	// uses it to index the texture
	Matrix3D matenv(	0.0f, 0.0f, 0.5f, 0.5f,
							0.0f, 0.0f, 0.0f, VOffset,
							0.0f, 0.0f, 1.0f, 0.0f );	

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),matenv);

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

void WSClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
	// The canonical environment map
	// scale the normal by (.5,.5) and add (.5,.5) to move it to (0,1) range
	// and ignore the Z component
	Matrix3D matenv(	0.5f, 0.0f, 0.0f, 0.5f,
							0.0f, 0.5f, 0.0f, 0.5f,
							0.0f, 0.0f, 1.0f, 0.0f );

	// multiply by inverse of view transform	
	Matrix4 mat;
	DX8Wrapper::Get_Transform(D3DTS_VIEW,mat);	
	Matrix3D mat2(mat[0].X,mat[1].X,mat[2].X,0.0f,
					  mat[0].Y,mat[1].Y,mat[2].Y,0.0f,
					  mat[0].Z,mat[1].Z,mat[2].Z,0.0f);		
#ifdef ALLOW_TEMPORARIES
	matenv=matenv*mat2;	
#else
	matenv.postMul(mat2);	
#endif

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),matenv);

	// Get camera normals
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

void WSEnvironmentMapperClass::Apply(int uv_array_index)
{
	// The canonical environment map
	// scale the normal by (.25,.25) and add (.5,.5) to move it to (0,1) range
	// the additional half is to fudge the 1+z normalization factor
	// and ignore the Z component
	Matrix3D matenv(	0.25f, 0.0f, 0.0f, 0.5f,
							0.0f, 0.25f, 0.0f, 0.5f,
							0.0f, 0.0f, 1.0f, 0.0f );		

	// multiply by inverse of view transform	
	Matrix4 mat;	
	DX8Wrapper::Get_Transform(D3DTS_VIEW,mat);		
	Matrix3D mat2(mat[0].X,mat[1].X,mat[2].X,0.0f,
					  mat[0].Y,mat[1].Y,mat[2].Y,0.0f,
					  mat[0].Z,mat[1].Z,mat[2].Z,0.0f);						  	
#ifdef ALLOW_TEMPORARIES
	matenv=matenv*mat2;	
#else
	matenv.postMul(mat2);	
#endif

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),matenv);	

	// Get camera reflection
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	
}

void ScreenMapperClass::Apply(int uv_array_index)
{
	unsigned int delta = WW3D::Get_Sync_Time() - LastUsedSyncTime;
	float del = (float)delta;
	float offset_u = CurrentUVOffset.X + UVOffsetDeltaPerMS.X * del;
	float offset_v = CurrentUVOffset.Y + UVOffsetDeltaPerMS.Y * del;

	// ensure both coordinates of offset are in [0, 1] range:
	offset_u = offset_u - WWMath::Floor(offset_u);
	offset_v = offset_v - WWMath::Floor(offset_v);	

	// multiply by projection matrix	
	// followed by scale and translation
	Matrix4 mat;	
	DX8Wrapper::Get_Transform(D3DTS_PROJECTION,mat);	
	mat[0]*=Scale.X; // entire row since we're pre-multiplying
	mat[1]*=Scale.Y;
	Vector4 last(mat[3]); // this gets the w
	last*=offset_u; // multiply by w because the projected flag will divide by w
	mat[0]+=last;
	last=mat[3];
	last*=offset_v;
	mat[1]+=last;

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),mat);	

	// Get camera space position
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);

	// Tell rasterizer what to expect
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_PROJECTED | D3DTTFF_COUNT3);

	// Update state
	CurrentUVOffset.X = offset_u;
	CurrentUVOffset.Y = offset_v;
	LastUsedSyncTime = WW3D::Get_Sync_Time();

}

void GridClassicEnvironmentMapperClass::Apply(int uv_array_index)
{
	update_temporal_state();

	float u_offset, v_offset;
	calculate_uv_offset(&u_offset, &v_offset);

	float del = 0.5f * OOGridWidth;	
	// Set up the offset matrix		
	Matrix3D tform(	del,	0.0f,	0.0f,	u_offset + del,
							0.0f,	del,	0.0f,	v_offset + del,
							0.0f,	0.0f,	1.0f,	0.0f				);		

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),tform);

	// Get camera normals
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACENORMAL);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

void GridEnvironmentMapperClass::Apply(int uv_array_index)
{
	update_temporal_state();

	float u_offset, v_offset;
	calculate_uv_offset(&u_offset, &v_offset);

	// Set up the offset matrix
	Matrix3D m(true);
	
	// According to the docs this should work since its 2D
	// otherwise change to translate
	m[0].Z = u_offset;
	m[1].Z = v_offset;
	
	float del=0.5f * OOGridWidth;	
	// Set up the offset matrix		
	Matrix3D tform(	del,	0.0f,	0.0f,	u_offset + del,
							0.0f,	del,	0.0f,	v_offset + del,
							0.0f,	0.0f,	1.0f,	0.0f				);		

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),tform);

	// Get camera space reflection
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);

	// Tell rasterizer to expect 2D matrices
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

RandomTextureMapperClass::RandomTextureMapperClass(float fps, unsigned int stage):
	TextureMapperClass(stage),
	FPS(fps),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Speed(0.0f,0.0f)
{
	CurrentAngle=WWMATH_PI*(rand() & 2047)/1024.0f;
	Center.U=(rand() & 2047)/2048.0f;
	Center.V=(rand() & 2047)/2048.0f;
}

RandomTextureMapperClass::RandomTextureMapperClass(const INIClass &ini, const char *section, unsigned int stage):
	TextureMapperClass(stage),	
	LastUsedSyncTime(WW3D::Get_Sync_Time())
{
	FPS = ini.Get_Float(section, "FPS", 0.0f);	
	CurrentAngle=WWMATH_PI*(rand() & 2047)/1024.0f;
	Center.U=(rand() & 2047)/2048.0f;
	Center.V=(rand() & 2047)/2048.0f;

	Speed.U = ini.Get_Float(section, "UPerSec", 0.0f);
	Speed.V = ini.Get_Float(section, "VPerSec", 0.0f);
}

RandomTextureMapperClass::RandomTextureMapperClass(const RandomTextureMapperClass & src):
	TextureMapperClass(src),
	FPS(src.FPS),	
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	Speed(src.Speed)
{
	CurrentAngle=WWMATH_PI*(rand() & 2047)/1024.0f;
	Center.U=(rand() & 2047)/2048.0f;
	Center.V=(rand() & 2047)/2048.0f;
}

void RandomTextureMapperClass::Apply(int uv_array_index)
{
	// Set up the random matrix
	Matrix3D m(true);	

	unsigned int delta=0;
	
	if (FPS!=0.0f)
	{
		float ms_per_frame=1000/FPS;
		unsigned int now = WW3D::Get_Sync_Time();
		delta =  now - LastUsedSyncTime;
		if (delta>ms_per_frame)
		{
			LastUsedSyncTime=now;
			CurrentAngle=WWMATH_PI*(rand() & 2047)/1024.0f;
			Center.U=(rand() & 2047)/2048.0f;
			Center.V=(rand() & 2047)/2048.0f;
		}
	}	

	m.Rotate_Z(CurrentAngle);
	float uoff=Center.U + delta*Speed.U*0.001f;
	float voff=Center.V + delta*Speed.V*0.001f;
	uoff=fmodf(uoff,1.0f);
	voff=fmodf(voff,1.0f);
	m[0].Z=uoff;
	m[1].Z=voff;

	DX8Wrapper::Set_Transform((D3DTRANSFORMSTATETYPE) (D3DTS_TEXTURE0+Stage),m);

	// Disable Texgen
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | uv_array_index);	

	// Tell rasterizer to expect 2D texture coordinates
	DX8Wrapper::Set_DX8_Texture_Stage_State(Stage,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
}

// BumpEnv Mapper
// GTH 8/22/01
BumpEnvTextureMapperClass::BumpEnvTextureMapperClass(float rad_per_sec, float scale_factor, const Vector2 & offset_per_sec, const Vector2 &scale, unsigned int stage) :
	LinearOffsetTextureMapperClass(offset_per_sec,scale, stage),
	LastUsedSyncTime(WW3D::Get_Sync_Time()),
	CurrentAngle(0.0f),
	RadiansPerSecond(rad_per_sec),
	ScaleFactor(scale_factor)
{
}

BumpEnvTextureMapperClass::BumpEnvTextureMapperClass(INIClass &ini, const char *section, unsigned int stage) :	
	LinearOffsetTextureMapperClass(ini,section,stage),
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

void RandomTextureMapperClass::Reset(void)
{
	LastUsedSyncTime = WW3D::Get_Sync_Time();
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
