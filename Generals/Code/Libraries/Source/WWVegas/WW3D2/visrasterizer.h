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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/visrasterizer.h                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/17/01 10:41a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VISRASTERIZER_H
#define VISRASTERIZER_H

#include "always.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "Vector3i.h"
#include "vector3.h"
#include "simplevec.h"
#include "bittype.h"
#include "plane.h"
#include "meshgeometry.h"


class CameraClass;
class AABoxClass;
struct GradientsStruct;
struct EdgeStruct;

/**
** IDBufferClass
** This class manages the ID buffer and the Z buffer.  It provides the low level
** rasterization code and stats about how many pixels and triangles are drawn.
*/
class IDBufferClass
{
public:
	IDBufferClass(void);
	~IDBufferClass(void);

	/*
	** State interface
	*/
	void						Set_Resolution(int w,int h);
	void						Get_Resolution(int * get_w,int * get_h);
	
	void						Set_Backface_ID(uint32 id)		{ BackfaceID = id; }
	void						Set_Frontface_ID(uint32 id)	{ FrontfaceID = id; }
	uint32					Get_Backface_ID(void)			{ return BackfaceID; }
	uint32					Get_Frontface_ID(void)			{ return FrontfaceID; }

	enum ModeType { OCCLUDER_MODE = 0, NON_OCCLUDER_MODE };
	void						Set_Render_Mode(ModeType mode) { RenderMode = mode; }
	ModeType					Get_Render_Mode(void)			{ return RenderMode; }

	void						Reset_Pixel_Counter(void)		{ PixelCounter = 0; }
	int						Get_Pixel_Counter(void)			{ return PixelCounter; }

	/*
	** Rendering interface
	*/
	void						Clear(void);
	bool						Render_Triangle(const Vector3 & p0,const Vector3 & p1,const Vector3 & p2);
	const uint32 *			Get_Pixel_Row(int y,int min_x,int max_x);

protected:
	void						Reset(void);
	void						Allocate_Buffers(void);
	bool						Is_Backfacing(const Vector3 & p0,const Vector3 & p1,const Vector3 & p2);
	int						Render_Occluder_Scanline(GradientsStruct & grads,EdgeStruct * left,EdgeStruct * right);
	int						Render_Non_Occluder_Scanline(GradientsStruct & grads,EdgeStruct * left,EdgeStruct * right);
	int						Pixel_Coords_To_Address(int x,int y)	{ return y*ResWidth + x; }

	uint32					BackfaceID;
	uint32					FrontfaceID;
	uint32					CurID;
	int						PixelCounter;
	ModeType					RenderMode;

	int						ResWidth;
	int						ResHeight;
	uint32 *					IDBuffer;
	float *					ZBuffer;		// actually a 1/z buffer...
};

inline const uint32 * IDBufferClass::Get_Pixel_Row(int y,int min_x,int max_x)
{
	WWASSERT(y>=0);
	WWASSERT(y<ResHeight);
	WWASSERT(min_x>=0);
	WWASSERT(max_x<=ResWidth);

	int addr = Pixel_Coords_To_Address(min_x,y);
	return &(IDBuffer[addr]);
}

inline bool IDBufferClass::Is_Backfacing(const Vector3 & p0,const Vector3 & p1,const Vector3 & p2)
{
	float x1=p1[0]-p0[0];
	float y1=p1[1]-p0[1];
	float x2=p2[0]-p0[0];
	float y2=p2[1]-p0[1];
	float r=x1*y2-x2*y1;
	if (r<0.0f) return true;
	return false;
}




/**
** VisRasterizerClass
** This class encapsulates the "ID buffer rasterization" code needed by the vis system.  Basically
** it is a floating point z-buffer and an id buffer which is used by the visiblity precalculation system.
** The VisRasterizer will transform and clip triangles into homogeneous view space; then the clipped
** triangles will be passed on to the IDBufferClass which will scan convert them.
*/ 
class VisRasterizerClass
{
public:

	VisRasterizerClass(void);
	~VisRasterizerClass(void);

	/*
	** ID Buffer Interface
	*/
	void					Set_Render_Mode(IDBufferClass::ModeType mode) { IDBuffer.Set_Render_Mode(mode); }
	IDBufferClass::ModeType	Get_Render_Mode(void)	{ return IDBuffer.Get_Render_Mode(); }

	void					Set_Backface_ID(uint32 id)		{ IDBuffer.Set_Backface_ID(id); }
	void					Set_Frontface_ID(uint32 id)	{ IDBuffer.Set_Frontface_ID(id); }
	uint32				Get_Backface_ID(void)			{ return IDBuffer.Get_Backface_ID(); }
	uint32				Get_Frontface_ID(void)			{ return IDBuffer.Get_Frontface_ID(); }

	void					Set_Resolution(int width,int height);
	void					Get_Resolution(int * set_width,int * set_height);

	void					Reset_Pixel_Counter(void)		{ IDBuffer.Reset_Pixel_Counter(); }
	int					Get_Pixel_Counter(void)			{ return IDBuffer.Get_Pixel_Counter(); }

	/*
	** Rendering Interface
	*/
	void					Set_Model_Transform(const Matrix3D & model);
	void					Set_Camera(CameraClass * camera);

	const Matrix3D &	Get_Model_Transform(void);
	CameraClass *		Get_Camera(void);
	CameraClass *		Peek_Camera(void);

	void					Clear(void)							{ IDBuffer.Clear(); }
	bool					Render_Triangles(const Vector3 * verts,int vcount,const TriIndex * tris, int tcount,const AABoxClass & bounds);
	const uint32 *		Get_Pixel_Row(int y,int min_x,int max_x) { return IDBuffer.Get_Pixel_Row(y,min_x,max_x); }

protected:
	
	void					Update_MV_Transform(void);
	const Matrix3D &	Get_MV_Transform(void);
	Vector3 *			Get_Temp_Vertex_Buffer(int count);
	bool					Render_Triangles_Clip(const Vector3 * verts,int vcount,const TriIndex * tris, int tcount);
	bool					Render_Triangles_No_Clip(const Vector3 * verts,int vcount,const TriIndex * tris, int tcount);
	
	Matrix3D				ModelTransform;			// AKA "World Transform"
	CameraClass *		Camera;
	Matrix3D				MVTransform;

	IDBufferClass		IDBuffer;	

	SimpleVecClass<Vector3>	TempVertexBuffer;
};

#endif //VISRASTERIZER_H

