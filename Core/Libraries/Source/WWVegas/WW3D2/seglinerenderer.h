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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/seglinerenderer.h                      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/08/01 5:23p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SEGLINERENDERER_H
#define SEGLINERENDERER_H

#include "always.h"
#include "shader.h"
#include "texture.h"
#include "matrix3d.h"
#include "vector2.h"

class RenderInfoClass;
class SphereClass;
struct W3dEmitterLinePropertiesStruct;
struct VertexFormatXYZDUV1;


// The maximum allowable level of subdivision. This should be no more than 7 to avoid increasing
// the chunk buffer size too much
#define MAX_SEGLINE_SUBDIV_LEVELS 7



/**
** SegLineRendererClass
** This class implements the low-level line rendering functionality used by both SegmentedLineClass
** and ParticleBufferClass
*/
class SegLineRendererClass
{
public:

	SegLineRendererClass(void);
	SegLineRendererClass(const SegLineRendererClass & that);
	SegLineRendererClass & operator = (const SegLineRendererClass & that);
	~SegLineRendererClass(void);

	enum TextureMapMode {
		UNIFORM_WIDTH_TEXTURE_MAP =	0x00000000,	// Entire line uses one row of texture (constant V)
		UNIFORM_LENGTH_TEXTURE_MAP =	0x00000001, // Entire line uses one row of texture stretched length-wise
		TILED_TEXTURE_MAP =				0x00000002	// Tiled continuously over line
	};

	void					Init(const W3dEmitterLinePropertiesStruct & props);

	// Get properties used to render this line segment
	TextureClass *		Get_Texture(void) const;
	TextureClass *		Peek_Texture(void) const								{ return Texture; }
	ShaderClass			Get_Shader(void) const									{ return Shader; }
	float					Get_Width(void) const									{ return Width; }
	const Vector3 &	Get_Color(void) const 									{ return Color; }
	float					Get_Opacity(void) const									{ return Opacity; }
	float					Get_Noise_Amplitude(void) const						{ return NoiseAmplitude; }
	float					Get_Merge_Abort_Factor(void) const					{ return MergeAbortFactor; }
	unsigned int		Get_Current_Subdivision_Level(void)	const			{ return SubdivisionLevel; }
	TextureMapMode		Get_Texture_Mapping_Mode(void) const;
	float					Get_Texture_Tile_Factor(void) const					{ return TextureTileFactor; }
	Vector2				Get_UV_Offset_Rate(void) const;
	int					Is_Merge_Intersections(void) const					{ return Bits & MERGE_INTERSECTIONS; }
	int					Is_Freeze_Random(void) const							{ return Bits & FREEZE_RANDOM; }
	int					Is_Sorting_Disabled(void) const						{ return Bits & DISABLE_SORTING; }
	int					Are_End_Caps_Enabled(void)	const						{ return Bits & END_CAPS; }

	// Set properties used to render this line segment
	void					Set_Texture(TextureClass *texture);
	void					Set_Shader(ShaderClass shader)						{ Shader = shader; }
	void					Set_Width(float width)									{ Width = width; }
	void					Set_Color(const Vector3 &color)						{ Color = color; }
	void					Set_Opacity(float opacity)								{ Opacity = opacity; }
	void					Set_Noise_Amplitude(float amplitude)				{ NoiseAmplitude = amplitude; }
	void					Set_Merge_Abort_Factor(float factor)				{ MergeAbortFactor = factor; }
	void					Set_Current_Subdivision_Level(unsigned int lv)	{ SubdivisionLevel = lv; }
	void					Set_Texture_Mapping_Mode(TextureMapMode mode);
	void					Set_Texture_Tile_Factor(float factor);	// Might be clamped if too high
	void					Set_Current_UV_Offset(const Vector2 & offset);
	void					Set_UV_Offset_Rate(const Vector2 &rate);
	void					Set_Merge_Intersections(int onoff)					{ if (onoff) { Bits |= MERGE_INTERSECTIONS; } else { Bits &= ~MERGE_INTERSECTIONS; }; }
	void					Set_Freeze_Random(int onoff)							{ if (onoff) { Bits |= FREEZE_RANDOM; } else { Bits &= ~FREEZE_RANDOM; }; }
	void					Set_Disable_Sorting(int onoff)						{ if (onoff) { Bits |= DISABLE_SORTING; } else { Bits &= ~DISABLE_SORTING; }; }
	void					Set_End_Caps(int onoff)									{ if (onoff) { Bits |= END_CAPS; } else { Bits &= ~END_CAPS; }; }
	
	
	void					Render(	RenderInfoClass & rinfo,
										const Matrix3D & transform,
										unsigned int point_count,
										Vector3 * points,
										const SphereClass & obj_sphere,
										Vector4 * rgbas = 0);

	void					Reset_Line(void);
	void					Scale(float scale);

private:

	// Utility functions
	void								subdivision_util(unsigned int point_cnt, const Vector3 *xformed_pts,
											const float *base_tex_v, unsigned int *p_sub_point_cnt,
											Vector3 *xformed_subdiv_pts, float *subdiv_tex_v,
											Vector4 *base_diffuse, Vector4 *subdiv_diffuse);

	// Global properties
	TextureClass *					Texture;
	ShaderClass						Shader;
	float								Width;
	Vector3							Color;
	float								Opacity;
	
	// Subdivision properties
	unsigned int					SubdivisionLevel;	
	float								NoiseAmplitude;

	// If >0, will abort a merge which causes an intersection to move
	// farther away than this factor * line radius from the line point.
	// (defaults to 1.5, has no affect if intersection merging is disabled).
	float								MergeAbortFactor;

	// Affects tiled texture mapping mode only. If this is set too high, performance (and
	// possibly visual) problems will result from excessive tiling over a single polygon, over
	// the entire line, or both.
	float								TextureTileFactor;

	// Used for texture coordinate animation
	unsigned int					LastUsedSyncTime;		// Last sync time used	
	Vector2							CurrentUVOffset;		// Current UV offset
	Vector2							UVOffsetDeltaPerMS;	// Amount to increase offset each millisec
	
	// Various flags
	enum BitShiftOffsets {
		TEXTURE_MAP_MODE_OFFSET = 24	// By how many bits do I need to shift the texture mapping mode?
	};
	enum {
		MERGE_INTERSECTIONS =	0x00000001,	// Merge intersections
		FREEZE_RANDOM =			0x00000002,	// Freeze random (note: offsets are in camera space)
		DISABLE_SORTING =			0x00000004,	// Disable sorting (even if shader has alpha-blending)
		END_CAPS =					0x00000008,	// Draw end caps on the line

		// Some bits are used to select the texture mapping mode:
		TEXTURE_MAP_MODE_MASK =	0xFF000000,	// Must cover all possible TextureMapMode values

		DEFAULT_BITS = MERGE_INTERSECTIONS | (UNIFORM_WIDTH_TEXTURE_MAP << TEXTURE_MAP_MODE_OFFSET)
	};
	unsigned int					Bits;

	friend class SegmentedLineClass;

	VertexFormatXYZDUV1 *getVertexBuffer(unsigned int number);
	unsigned int m_vertexBufferSize;
	VertexFormatXYZDUV1 *m_vertexBuffer;
};



inline SegLineRendererClass::TextureMapMode SegLineRendererClass::Get_Texture_Mapping_Mode(void) const
{ 
	return (TextureMapMode)((Bits & TEXTURE_MAP_MODE_MASK) >> TEXTURE_MAP_MODE_OFFSET); 
}

inline void SegLineRendererClass::Set_Texture_Mapping_Mode(SegLineRendererClass::TextureMapMode mode)
{
	Bits &= ~TEXTURE_MAP_MODE_MASK;
	Bits |= ((mode << TEXTURE_MAP_MODE_OFFSET) & TEXTURE_MAP_MODE_MASK);
}

inline Vector2 SegLineRendererClass::Get_UV_Offset_Rate(void) const
{	
	return UVOffsetDeltaPerMS * 1000.0f;
}

inline void SegLineRendererClass::Set_UV_Offset_Rate(const Vector2 &rate)
{
	UVOffsetDeltaPerMS = rate * 0.001f;
}


#endif //SEGLINERENDERER_H

