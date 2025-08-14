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
 *                 Project Name : Linegroup.cpp                                                *
 *                                                                                             *
 *                     $Archive::                                                             $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                            $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sharebuf.h"
#include "linegrp.h"
#include "texture.h"
#include "vertmaterial.h"
#include "dx8wrapper.h"
#include "wwmath.h"
#include "rinfo.h"
#include "camera.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "sortingrenderer.h"

// Line groups are a rendering primitive similar to point groups
// They are tetrahedra which are aligned with the view plane with their centers
// at StartLineLoc. The apex of the tetrahedron is at EndLineLoc.
// They can be individually colored LineDiffuse
// and the LineUCoord determines the U coordinate of the texture to use
// the V coordinate is always 0 at the flat end of the tetrahedron
// and 1 at the apex
LineGroupClass::LineGroupClass(void) :
	StartLineLoc(NULL),
	EndLineLoc(NULL),
	LineDiffuse(NULL),
	TailDiffuse(NULL),
	ALT(NULL),
	LineSize(NULL),
	LineUCoord(NULL),
	LineCount(0),
	Texture(NULL),
	Flags(0),
	Shader(ShaderClass::_PresetAdditiveSpriteShader),
	DefaultLineSize(0.0f),
	DefaultLineColor(1.0f, 1.0f, 1.0f),
	DefaultLineAlpha(1.0f),
	DefaultLineUCoord(0.0f),
	DefaultTailDiffuse(0.0f, 0.0f, 0.0f, 0.0f),
	LineMode(TETRAHEDRON)
{
}

LineGroupClass::~LineGroupClass(void)
{
	REF_PTR_RELEASE(StartLineLoc);
	REF_PTR_RELEASE(EndLineLoc);
	REF_PTR_RELEASE(LineDiffuse);
	REF_PTR_RELEASE(TailDiffuse);
	REF_PTR_RELEASE(ALT);
	REF_PTR_RELEASE(LineSize);
	REF_PTR_RELEASE(LineUCoord);
	REF_PTR_RELEASE(Texture);
}

void LineGroupClass::Set_Arrays(
	ShareBufferClass<Vector3> *startlocs,
	ShareBufferClass<Vector3> *endlocs,
	ShareBufferClass<Vector4> *diffuse,
	ShareBufferClass<Vector4> *taildiffuse,
	ShareBufferClass<unsigned int> *alt,
	ShareBufferClass<float> *sizes,
	ShareBufferClass<float> *ucoords,
	int active_line_count
	)
{
	// The Line locations arrays are NOT optional!
	WWASSERT(startlocs);
	WWASSERT(endlocs);

	// Ensure lengths of all arrays are the same:
	WWASSERT(startlocs->Get_Count() == endlocs->Get_Count());
	WWASSERT(!diffuse || startlocs->Get_Count() == diffuse->Get_Count());
	WWASSERT(!alt || startlocs->Get_Count() == alt->Get_Count());
	WWASSERT(!sizes || startlocs->Get_Count() == sizes->Get_Count());
	WWASSERT(!ucoords || startlocs->Get_Count() == ucoords->Get_Count());
	WWASSERT(!taildiffuse || startlocs->Get_Count() == taildiffuse->Get_Count());

	REF_PTR_SET(StartLineLoc,startlocs);
	REF_PTR_SET(EndLineLoc,endlocs);
	REF_PTR_SET(LineDiffuse,diffuse);
	REF_PTR_SET(TailDiffuse,taildiffuse);
	REF_PTR_SET(ALT,alt);
	REF_PTR_SET(LineSize,sizes);
	REF_PTR_SET(LineUCoord,ucoords);

	if (ALT) {
		LineCount = active_line_count;
	} else {
		LineCount = (active_line_count >= 0) ? active_line_count : StartLineLoc->Get_Count();
	}

}

void LineGroupClass::Set_Line_Size(float size)
{
	DefaultLineSize = size;
}

float LineGroupClass::Get_Line_Size(void)
{
	return DefaultLineSize;
}

void LineGroupClass::Set_Line_Color(const Vector3 &color)
{
	DefaultLineColor = color;
}

Vector3 LineGroupClass::Get_Line_Color(void)
{
	return DefaultLineColor;
}

void LineGroupClass::Set_Tail_Diffuse(const Vector4 &tdiffuse)
{
	DefaultTailDiffuse = tdiffuse;
}

Vector4 LineGroupClass::Get_Tail_Diffuse(void)
{
	return DefaultTailDiffuse;
}

void LineGroupClass::Set_Line_Alpha(float alpha)
{
	DefaultLineAlpha = alpha;
}

float LineGroupClass::Get_Line_Alpha(void)
{
	return DefaultLineAlpha;
}

void LineGroupClass::Set_Line_UCoord(float ucoord)
{
	DefaultLineUCoord = ucoord;
}

float LineGroupClass::Get_Line_UCoord(void)
{
	return DefaultLineUCoord;
}

void LineGroupClass::Set_Flag(FlagsType flag, bool on)
{
	if (on) Flags |= 1 << flag;
	else
		Flags &= ~(1 << flag);
}

int LineGroupClass::Get_Flag(FlagsType flag)
{
	return (Flags >> flag) & 0x1;
}

void LineGroupClass::Set_Texture(TextureClass* texture)
{
	REF_PTR_SET(Texture,texture);
}

TextureClass * LineGroupClass::Get_Texture(void)
{
	if (Texture) Texture->Add_Ref();
	return Texture;
}

TextureClass * LineGroupClass::Peek_Texture(void)
{
	return Texture;
}

void LineGroupClass::Set_Shader(const ShaderClass &shader)
{
	Shader = shader;
}

ShaderClass LineGroupClass::Get_Shader(void)
{
	return Shader;
}

void LineGroupClass::Set_Line_Mode(LineModeType linemode)
{
	LineMode = linemode;
}

LineGroupClass::LineModeType LineGroupClass::Get_Line_Mode(void)
{
	return LineMode;
}

void	LineGroupClass::Render(RenderInfoClass &rinfo)
{
	int i;

	// If no lines, do nothing:
	if (LineCount == 0) return;

	// Shader handling
	Shader.Set_Cull_Mode(ShaderClass::CULL_MODE_ENABLE);

	// If there is a color or alpha array enable gradient in shader - otherwise disable.
   float value_255 = 0.9961f;	//254 / 255
	bool default_white_opaque = (	DefaultLineColor.X > value_255 &&
											DefaultLineColor.Y > value_255 &&
											DefaultLineColor.Z > value_255 &&
											DefaultLineAlpha > value_255);

	if (LineDiffuse || !default_white_opaque || !Texture) {
		Shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
	} else {
		Shader.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);
	}

	// If Texture is non-NULL enable texturing in shader - otherwise disable.
	if (Texture) {
		Shader.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
	} else {
		Shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
	}

	VertexMaterialClass * linemat = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(linemat);
	DX8Wrapper::Set_Shader(Shader);
	DX8Wrapper::Set_Texture(0, Texture);
	REF_PTR_RELEASE(linemat);

	WWASSERT(StartLineLoc && StartLineLoc->Get_Array());
	WWASSERT(EndLineLoc && EndLineLoc->Get_Array());

	// Enable sorting if the primitives are translucent and alpha testing is not enabled.
	const bool sort = (Shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO) && (Shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_DISABLE) && (WW3D::Is_Sorting_Enabled());

	// the 3 offsets in view space
	const static Vector3 offset_a = Vector3(WWMath::Cos(WWMATH_PI / 2),			WWMath::Sin(WWMATH_PI /2 ), 0);
	const static Vector3 offset_b = Vector3(WWMath::Cos(7 * WWMATH_PI / 6),		WWMath::Sin(7 * WWMATH_PI / 6), 0);
	const static Vector3 offset_c = Vector3(WWMath::Cos(11 * WWMATH_PI / 6),	WWMath::Sin(11 * WWMATH_PI / 6), 0);

	static Vector3 offset[3];

	offset[0].Set(offset_a);
	offset[1].Set(offset_b);
	offset[2].Set(offset_c);

	// Save off the view matrix
	Matrix4x4 view;
	DX8Wrapper::Get_Transform(D3DTS_VIEW, view);

	Matrix4x4 identity(true);
	DX8Wrapper::Set_Transform(D3DTS_WORLD, identity);

	// if the points are in world space, transform the offsets
	if (Get_Flag(TRANSFORM)) {
		Matrix3D xform_mat;
		xform_mat = rinfo.Camera.Get_Transform();
		xform_mat.Set_Translation(Vector3(0, 0, 0));
		xform_mat.Get_Orthogonal_Inverse(xform_mat);
		for (i = 0; i < 3; i++) {
			Matrix3D::Transform_Vector(xform_mat, offset[i], &offset[i]);
		}
	} else {
		DX8Wrapper::Set_Transform(D3DTS_VIEW, identity);
	}

	int num_tris=0;
	int num_indices=0;
	int num_vertices=0;

	switch (LineMode)	{
		case TETRAHEDRON:
			num_tris			=4 * LineCount;
			num_indices		=3 * num_tris;
			num_vertices	=4 * LineCount;
			break;
		case PRISM:
			num_tris			=8 * LineCount;
			num_indices		=3 * num_tris;
			num_vertices	=6 * LineCount;
			break;
	}

	// construct the tetrahedra in the index buffers
	// assume first vertex is the apex, followed by offset[0-3]

	DynamicIBAccessClass iba(sort?BUFFER_TYPE_DYNAMIC_SORTING:BUFFER_TYPE_DYNAMIC_DX8,num_indices);

	{
		DynamicIBAccessClass::WriteLockClass lock(&iba);
		unsigned short *ibptr = lock.Get_Index_Array();
		unsigned short j, idx;
		switch (LineMode)	{
			case TETRAHEDRON:
				for (j=0; j<LineCount; j++) {
					idx = 4 * j;
					// apex, offset[1], offset[0]
					*ibptr++	= idx + 0;
					*ibptr++	= idx + 2;
					*ibptr++	= idx + 1;
					// apex, offset[2], offset[1]
					*ibptr++	= idx + 0;
					*ibptr++	= idx + 3;
					*ibptr++	= idx + 2;
					// apex, offset[0], offset[2]
					*ibptr++	= idx + 0;
					*ibptr++	= idx + 1;
					*ibptr++	= idx + 3;
					// offset[0-3]
					*ibptr++	= idx + 1;
					*ibptr++	= idx + 2;
					*ibptr++	= idx + 3;
				}
				break;
			case PRISM:
				for (j=0; j<LineCount; j++) {
					idx = 6 * j;
					// starting cap 0,1,2
					*ibptr++ = idx + 0;
					*ibptr++ = idx + 1;
					*ibptr++ = idx + 2;
					// left side
					*ibptr++ = idx + 0;
					*ibptr++ = idx + 3;
					*ibptr++ = idx + 1;
					*ibptr++ = idx + 1;
					*ibptr++ = idx + 3;
					*ibptr++ = idx + 4;
					// bottom side
					*ibptr++ = idx + 1;
					*ibptr++ = idx + 4;
					*ibptr++ = idx + 5;
					*ibptr++ = idx + 1;
					*ibptr++ = idx + 5;
					*ibptr++ = idx + 2;
					// right side
					*ibptr++ = idx + 0;
					*ibptr++ = idx + 2;
					*ibptr++ = idx + 5;
					*ibptr++ = idx + 0;
					*ibptr++ = idx + 5;
					*ibptr++ = idx + 3;
					// end cap
					*ibptr++ = idx + 3;
					*ibptr++ = idx + 5;
					*ibptr++ = idx + 4;
				}
				break;
		}
	}	// writing to ib

	// make the vertex buffers

	DynamicVBAccessClass vba(sort ? BUFFER_TYPE_DYNAMIC_SORTING : BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,num_vertices);

	{
		DynamicVBAccessClass::WriteLockClass lock(&vba);

		VertexFormatXYZNDUV2 *vb = lock.Get_Formatted_Vertex_Array();

		Vector3 loc, start, end;
		int point, j;
		float size = DefaultLineSize;
		Vector4 diffuse(DefaultLineColor.X, DefaultLineColor.Y, DefaultLineColor.Z, DefaultLineAlpha);
		float ucoord = DefaultLineUCoord;
		Vector4 taildiffuse = DefaultTailDiffuse;

		for (i = 0; i < LineCount; i++)
		{
			point = (ALT) ? ALT->Get_Element(i) : i;
			if (LineSize)		size			= LineSize->Get_Element(point);
			if (LineDiffuse)	diffuse		= LineDiffuse->Get_Element(point);
			if (LineUCoord)	ucoord		= LineUCoord->Get_Element(point);
			if (TailDiffuse)	taildiffuse	= TailDiffuse->Get_Element(point);

			end.Set(EndLineLoc->Get_Element(point));
			start.Set(StartLineLoc->Get_Element(point));

			switch (LineMode) {
				case TETRAHEDRON:
					// apex
					vb->x			= end.X;
					vb->y			= end.Y;
					vb->z			= end.Z;
					vb->diffuse	= DX8Wrapper::Convert_Color(taildiffuse);
					vb->u1		= ucoord;
					vb->v1		= 1.0f;
					vb++;

					for (j=0; j<3; j++) {
						loc.Set(start + size * offset[j]);
						vb->x			= loc.X;
						vb->y			= loc.Y;
						vb->z			= loc.Z;
						vb->diffuse	= DX8Wrapper::Convert_Color(diffuse);
						vb->u1		= ucoord;
						vb->v1		= 0.0f;
						vb++;
					}
					break;
			case PRISM:
					// start cap
					for (j = 0; j < 3; j++) {
						loc.Set(start + size * offset[j]);
						vb->x			= loc.X;
						vb->y			= loc.Y;
						vb->z			= loc.Z;
						vb->diffuse	= DX8Wrapper::Convert_Color(diffuse);
						vb->u1		= ucoord;
						vb->v1		= 0.0f;
						vb++;
					}
					// Do not merge loops. The vb has to be written in a specific order
					// (This is to optimize AGP memory write)

					// end cap
					for (j=0; j<3; j++) {
						loc.Set(end + size * offset[j]);
						vb->x			= loc.X;
						vb->y			= loc.Y;
						vb->z			= loc.Z;
						vb->diffuse	= DX8Wrapper::Convert_Color(taildiffuse);
						vb->u1		= ucoord;
						vb->v1		= 1.0f;
						vb++;
					}
					break;
			}

		}
	} // writing to vb

	DX8Wrapper::Set_Index_Buffer(iba, 0);
	DX8Wrapper::Set_Vertex_Buffer(vba);

	if (sort) {
		SortingRendererClass::Insert_Triangles(0, num_tris, 0, num_vertices);
	} else {
		DX8Wrapper::Draw_Triangles(0, num_tris, 0, num_vertices);
	}

	// restore the matrices
	DX8Wrapper::Set_Transform(D3DTS_VIEW, view);
}

int LineGroupClass::Get_Polygon_Count(void)
{
	switch (LineMode) {
		case TETRAHEDRON:
			return LineCount * 4;
			break;
		case PRISM:
			return LineCount * 8;
			break;
	}
	WWASSERT(0);
	return 0;
}
