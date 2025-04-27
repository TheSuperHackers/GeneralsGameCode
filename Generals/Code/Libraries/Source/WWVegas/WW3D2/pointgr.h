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
 *                     $Archive:: /Commando/Code/ww3d2/pointgr.h          $* 
 *                                                                         * 
 *                      $Author:: Naty_h                                  $* 
 *                                                                         * 
 *                     $Modtime:: 8/02/01 8:34p                           $* 
 *                                                                         * 
 *                    $Revision:: 10                                      $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef POINTGR_H
#define POINTGR_H

#include "sharebuf.h"
#include "shader.h"
#include "vector4.h"
#include "vector3.h"
#include "vector2.h"
#include "Vector.H"

class VertexMaterialClass;
class RenderInfoClass;
class TextureClass;

/*
** PointGroupClass -- a custom object for rendering 
** groups of points (such as particle systems).
** It is possible to change mode/number of points/shader/etc. but these
** changes tend to be expensive if done often. Expected usage is to set the
** point location/color/active arrays each frame the object is visible (with
** the same number of points) and perform other changes relatively rarely.
** NOTE: Currently it is implemented using general triangles (1 or 2 per
** point), so it is probably suboptimal for software rasterization devices
** (which would probably perform better with some kind of blit/sprite code).
*/ 
class PointGroupClass
{
public:

	enum PointModeEnum {
		TRIS,			// each point is a triangle
		QUADS,		// each point is a quad formed out of two triangles
		SCREENSPACE	// each point is a tri placed to affect certain pixels (should be used with 2D camera)
	};

	enum FlagsType {
		TRANSFORM,	// transform points w. modelview matrix (worldspace points)
	};

	PointGroupClass(void);
	virtual ~PointGroupClass(void);
	PointGroupClass & operator = (const PointGroupClass & that);

	// PointGroupClass interface:
	void						Set_Arrays(ShareBufferClass<Vector3> *locs,
									ShareBufferClass<Vector4> *diffuse = NULL,																		
									ShareBufferClass<unsigned int> *apt = NULL,
									ShareBufferClass<float> *sizes = NULL,
									ShareBufferClass<unsigned char> *orientations = NULL,
									ShareBufferClass<unsigned char> *frames = NULL,
									int active_point_count = -1,
									float vpxmin = 0.0f, float vpymin = 0.0f,
									float vpxmax = 0.0f, float vpymax = 0.0f);
	void						Set_Point_Size(float size);
	float						Get_Point_Size(void);
	void						Set_Point_Color(Vector3 color);
	Vector3					Get_Point_Color(void);
	void						Set_Point_Alpha(float alpha);
	float						Get_Point_Alpha(void);
	void						Set_Point_Orientation(unsigned char orientation);
	unsigned char		Get_Point_Orientation(void);
	void						Set_Point_Frame(unsigned char frame);
	unsigned char		Get_Point_Frame(void);
	void						Set_Point_Mode(PointModeEnum mode);
	PointModeEnum		Get_Point_Mode(void);
	void						Set_Flag(FlagsType flag, bool onoff);
	int							Get_Flag(FlagsType flag);
	void						Set_Texture(TextureClass* texture);
	TextureClass * 	Get_Texture(void);
	TextureClass * 	Peek_Texture(void);
	void						Set_Shader(ShaderClass shader);
	ShaderClass			Get_Shader(void);
	void						Set_Billboard(bool shouldBillboard);
	bool						Get_Billboard(void);

	// The frame property is taken from a set of possible frames. The rows/columns in the frame
	// texture determine the number of possible frames. Since it must be a power of 2, we represent
	// it as its log base 2. This number cannot be greater than 4 (which corresponds to a 16x16
	// square of frames, i.e. 256 frames).
	unsigned char			Get_Frame_Row_Column_Count_Log2(void);
	void						Set_Frame_Row_Column_Count_Log2(unsigned char frccl2);

	int						Get_Polygon_Count(void);

	void						Render(RenderInfoClass &rinfo);
	void						RenderVolumeParticle(RenderInfoClass &rinfo, unsigned int depth);

protected:
	// Update arrays.
	void						Update_Arrays(Vector3 *point_loc,
									Vector4 *point_diffuse,									
									float *point_size,
									unsigned char *point_orientation,
									unsigned char *point_frame, 
									int active_points,
									int total_points, 
									int &vnum, 
									int &pnum);

	// These shared buffers are used for communication to the point group - to
	// pass point locations, colors and enables. The location and color arrays
	// are 'compressed' using the active point table (if present) and then 
	// are processed into other arrays which are passed to the GERD.
	// SR rather than WWMath types are used so Vector Processors can be used.
	// The arrays override the default value if present.
	// The orientation and frame properties are "index properties": they select one out of a small
	// group of possibilities for each point.
	// Orientation: this is the 2D rotation of the point about its center. There are 256 discrete
	// orientations. The unit circle is evenly subdivided 256 times to create the set of
	// orientations. The reason we discretize orientation is for performance: this way we can
	// precalculate vertex offsets.
	// Frame: the texture is divided into a 2D square grid, and each point uses one grid square
	// instead of the whole texture. The number of possible frames is not invariant - it depends
	// on the number of rows / columns in the grid. The reason we do this instead of having
	// different textures is to avoid texture state changes. Also for performance reasons, the
	// number of possible frames must be a power of two - for this reason the number of frame rows
	// and columns, orientations, etc. are represented as the log base 2 of the actual number.
	ShareBufferClass<Vector3> *			PointLoc;	// World/cameraspace point locs
	ShareBufferClass<Vector4> *			PointDiffuse; // (NULL if not used) RGBA values
	ShareBufferClass<unsigned int> *		APT;			// (NULL if not used) active point table
	ShareBufferClass<float> *				PointSize;	// (NULL if not used) size override table
	ShareBufferClass<unsigned char> *	PointOrientation; // (NULL if not used) orientation indices
	ShareBufferClass<unsigned char> *	PointFrame; // (NULL if not used) frame indices
	int											PointCount;	// Active (if APT) or total point count

	// See comments for Get/Set_Frame_Row_Column_Count_Log2 above
	unsigned char			FrameRowColumnCountLog2;		// MUST be equal or lesser than 4

	// These parameters are passed to the GERD:
	TextureClass*			Texture;
	ShaderClass				Shader;					// (default created in CTor)

	// Internal state:
	PointModeEnum			PointMode;					// are points tris or quads?
	unsigned int			Flags;						// operation control flags
	float						DefaultPointSize;			// point size (size array overrides if present)
	Vector3					DefaultPointColor;		// point color (color array overrides if present)
	float						DefaultPointAlpha;		// point alpha (alpha array overrides if present)	
	unsigned char			DefaultPointOrientation;// point orientation (orientation array overrides if present)
	unsigned char			DefaultPointFrame;		// point texture frame (frame array overrides if present)

	// View plane rectangle (only used in SCREENSPACE mode - set by Set_Arrays
	// and used in Update_GERD_Arrays).
	float						VPXMin;
	float						VPYMin;
	float						VPXMax;
	float						VPYMax;

	bool						Billboard;

	// Static stuff:
	// For performance / memory reasons we prepare vertex location and UV
	// arrays for various orientations and texture frames, as static data
	// members of this class. This avoids the need to create such arrays over
	// again for each object. The values in the UV arrays are used as-is, the
	// values in the location arrays normally need to be scaled to the point
	// sizes and added to the point location before use. We have distinct sets
	// of arrays for each point mode.
	// There are tables of vertex positions for each of triangle and quad mode
	// with 256 discrete orientations. Each entry contains 3 (for triangle
	// mode) or 4 (for quad mode) Vector3s.
	// There are five tables of texture UVs for each of triangle and quad mode:
	// with 1x1(1), 2x2(4), 4x4(16), 8x8(64) and 16x16(256) frames. Each entry
	// contains 3 (for triangle mode) or 4 (for quad mode) Vector2s.
	// In addition, there is one array of vertex positions for screenspace mode
	// with 2 entries (for size 1 and size 2). Each entry contains 3 Vector3s.
	// the Init function (which is called by WW3D::Init()) creates these
	// arrays, and the Shutdown function (which is called by WW3D::Shutdown()
	// releases them.
public:
	static void				_Init(void);
	static void				_Shutdown(void);

private:
	static Vector3 _TriVertexLocationOrientationTable[256][3];
	static Vector3 _QuadVertexLocationOrientationTable[256][4];
	static Vector3 _ScreenspaceVertexLocationSizeTable[2][3];
	static Vector2 *_TriVertexUVFrameTable[5];
	static Vector2 *_QuadVertexUVFrameTable[5];
	static VertexMaterialClass *PointMaterial;

	// Static arrays for intermediate calcs (never resized down, just up):
	static VectorClass<Vector3>		compressed_loc;		// point locations 'compressed' by APT
	static VectorClass<Vector4>		compressed_diffuse;	// point colors 'compressed' by APT
	static VectorClass<float>			compressed_size;		// point sizes 'compressed' by APT
	static VectorClass<unsigned char>	compressed_orient;	// point orientations 'compressed' by APT
	static VectorClass<unsigned char>	compressed_frame;		// point frames 'compressed' by APT
	static VectorClass<Vector3>		transformed_loc;		// transformed point locations
};


class SegmentGroupClass : public PointGroupClass
{
public:
	SegmentGroupClass(void);
	virtual ~SegmentGroupClass(void);

};



#endif