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
 *                     $Archive:: /Commando/Code/ww3d2/line3d.cpp         $* 
 *                                                                         * 
 *                      $Author:: Jani_p                                  $* 
 *                                                                         * 
 *                     $Modtime:: 7/05/01 4:15p                           $* 
 *                                                                         * 
 *                    $Revision:: 11                                      $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 *   Line3DClass::Line3DClass -- Constructor                               * 
 *   Line3DClass::Line3DClass -- Copy constructor.                         * 
 *   Line3DClass::operator = -- assignment operator                        * 
 *   Line3DClass::~Line3DClass -- Destructor.                              * 
 *   Line3DClass::Clone -- Creates a clone of this Line3D                  * 
 *   Line3DClass::Scale -- Scale object                                    * 
 *   Line3DClass::Scale -- Scale object                                    * 
 *   Line3DClass::Update_Cached_Bounding_Volumes -- update bounding vols   *
 *   Line3DClass::Reset -- Reset line start and end points.                * 
 *   Line3DClass::Reset -- Reset line start and end points, and line width.* 
 *   Re_Color -- Reset the line color.                                     * 
 *	  Set_Opacity -- Reset the line opacity.                                * 
 *   Line3DClass::Render -- render the 3d line                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "line3d.h"
#include "vertmaterial.h"
#include "shader.h"
#include "wwdebug.h"
#include "ww3d.h"
#include "rinfo.h"
#include "dx8wrapper.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"

// 12 Triangles for index buffer
const unsigned short Indices[]=
{
	3,5,1,
	7,5,3,
	1,5,0,
	5,4,0,
	4,2,0,
	4,6,2,
	7,3,2,
	6,7,2,
	7,6,5,
	5,6,4,
	2,3,1,
	2,1,0
};


/************************************************************************** 
 * Line3DClass::Line3DClass -- Constructor                                * 
 *                                                                        * 
 * INPUT:	Vector3 start, end - start, end points of line (world coords).* 
 *       	float width - width of line (in world units).                 * 
 *       	float r, g, b - R, G, B components of line color.             * 
 *       	float opacity - opacity of line.                              * 
 *                                                                        * 
 * OUTPUT:	none.                                                         * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/15/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   * 
 *	  02/16/2001 HY  : Ported to DX8													  *
 *========================================================================*/
Line3DClass::Line3DClass (const Vector3 & start, const Vector3 & end,
	float width, float r, float g, float b, float opacity)
{
	Length = (end - start).Length();
   Width = width;

	// Create box model with origin at start point (X is major axis).	

	// 8 Vertices	
	float halfw = Width * 0.5f;

	vert[0].X = 0.0f;
	vert[0].Y = -halfw;
	vert[0].Z = -halfw;
	vert[1].X = 0.0f;
	vert[1].Y = halfw;
	vert[1].Z = -halfw;
	vert[2].X = 0.0f;
	vert[2].Y = -halfw;
	vert[2].Z = halfw;
	vert[3].X = 0.0f;
	vert[3].Y = halfw;
	vert[3].Z = halfw;
	vert[4].X = Length;
	vert[4].Y = -halfw;
	vert[4].Z = -halfw;
	vert[5].X = Length;
	vert[5].Y = halfw;
	vert[5].Z = -halfw;
	vert[6].X = Length;
	vert[6].Y = -halfw;
	vert[6].Z = halfw;
	vert[7].X = Length;
	vert[7].Y = halfw;
	vert[7].Z = halfw;
	
	Color.X=r;
	Color.Y=g;
	Color.Z=b;
	Set_Opacity(opacity);

	// Set box transform so that the origin is at the start point and it
	// 'looks towards' the endpoint.
	Matrix3D transform(true);
	transform.Obj_Look_At(start, end, 0.0);
	Set_Transform(transform);
}


/************************************************************************** 
 * Line3DClass::Line3DClass -- Copy constructor.                          * 
 *                                                                        * 
 * INPUT:	const Line3DClass & src - source to copy from.                * 
 *                                                                        * 
 * OUTPUT:	none.                                                         * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/15/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   * 
 *	  02/16/2001 HY  : Ported to DX8													  *
 *========================================================================*/
Line3DClass::Line3DClass(const Line3DClass & src) :
	RenderObjClass(src),
	Length(src.Length),
   Width(src.Width),
	Shader(src.Shader),
	Color(src.Color)
{
		for (int i=0; i<8; i++) vert[i]=src.vert[i];
}


/************************************************************************** 
 * Line3DClass::operator = -- assignment operator                         * 
 *                                                                        * 
 * INPUT:	const Line3DClass & that - source to copy from.               * 
 *                                                                        * 
 * OUTPUT:	Line3DClass & - result of assignment.                         * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/15/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   *
 *	  02/16/2001 HY  : Ported to DX8													  *
 *========================================================================*/
Line3DClass & Line3DClass::operator = (const Line3DClass & that)
{
	// Naty: need to add MatInfo and remapper to do this 	Byon
	WWASSERT(0);

	RenderObjClass::operator = (that);

	if (this != &that) {
		Length = that.Length;
      Width = that.Width;		
		Shader=that.Shader;
		Color=that.Color;
		for (int i=0; i<8; i++)
			vert[i]=that.vert[i];
	}

	return * this;
}


/************************************************************************** 
 * Line3DClass::~Line3DClass -- Destructor.                               * 
 *                                                                        * 
 * INPUT:	none.                                                         * 
 *                                                                        * 
 * OUTPUT:	none.                                                         * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/15/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   *
 *	  02/16/2001 HY  : Ported to DX8													  *
 *========================================================================*/
Line3DClass::~Line3DClass(void)
{	
}


/************************************************************************** 
 * Line3DClass::Clone -- Creates a clone of this Line3D                   * 
 *                                                                        * 
 * INPUT:	none.                                                         * 
 *                                                                        * 
 * OUTPUT:	RenderObjClass * - pointer to cloned object.                  * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/15/1998 NH  : Created.                                            *
 *========================================================================*/
RenderObjClass * Line3DClass::Clone(void) const
{
	return NEW_REF( Line3DClass, (*this));
}

/***********************************************************************************************
 * Line3DClass::Render -- render the 3d line                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *	  02/16/2001 HY  : Ported to DX8													                       *
 *=============================================================================================*/

void Line3DClass::Render(RenderInfoClass & rinfo)
{
	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	// If static sort lists are enabled and this mesh has a sort level, put it on the list instead
	// of rendering it.
	unsigned int sort_level = (unsigned int)Get_Sort_Level();

	if (WW3D::Are_Static_Sort_Lists_Enabled() && sort_level != SORT_LEVEL_NONE) 
	{	
		WW3D::Add_To_Static_Sort_List(this, sort_level);
		return;
	}

	DX8Wrapper::Set_Shader(Shader);
	DX8Wrapper::Set_Texture(0,NULL);	
	VertexMaterialClass *vm=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(vm);
	REF_PTR_RELEASE(vm);

	DX8Wrapper::Set_Transform(D3DTS_WORLD,Transform);	

	DynamicVBAccessClass vb(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,8);
	{
		DynamicVBAccessClass::WriteLockClass Lock(&vb);
		const FVFInfoClass &fi=vb.FVF_Info();
		unsigned char *vb=(unsigned char*)Lock.Get_Formatted_Vertex_Array();
		int i;
		unsigned int color=DX8Wrapper::Convert_Color(Color);

		for (i=0; i<8; i++)
		{			
			*(Vector3*)(vb+fi.Get_Location_Offset())=vert[i];
			*(unsigned int*)(vb+fi.Get_Diffuse_Offset())=color;
			vb+=fi.Get_FVF_Size();
		}		
	}

	DynamicIBAccessClass ib(BUFFER_TYPE_DYNAMIC_DX8,36);
	{
		DynamicIBAccessClass::WriteLockClass Lock(&ib);
		unsigned short *mem=Lock.Get_Index_Array();
		try {
		for (int i=0; i<36; i++)
			mem[i]=Indices[i];
		IndexBufferExceptionFunc();
		} catch(...) {
			IndexBufferExceptionFunc();
		}
	}	

	DX8Wrapper::Set_Vertex_Buffer(vb);
	DX8Wrapper::Set_Index_Buffer(ib,0);
	DX8Wrapper::Draw_Triangles(0,36/3,0,8);
}

/************************************************************************** 
 * Line3DClass::Scale -- Scale object                                     * 
 *                                                                        * 
 * INPUT:	float scale - uniform scale factor.                           * 
 *                                                                        * 
 * OUTPUT:	none.                                                         * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/27/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   * 
 *	  02/16/2001 HY  : Ported to DX8													  *
 *========================================================================*/
void Line3DClass::Scale(float scale)
{	
	for (int i=0; i<8; i++) vert[i]*=scale;
	Length *= scale;
   Width *= scale;

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   RenderObjClass *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}


/************************************************************************** 
 * Line3DClass::Scale -- Scale object                                     * 
 *                                                                        * 
 * INPUT:	float scalex, scaley, scalez - axis scale factors.            * 
 *                                                                        * 
 * OUTPUT:	none.                                                         * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/27/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   *
 *	  02/16/2001 HY  : Ported to DX8													  *
 *========================================================================*/
void Line3DClass::Scale(float scalex, float scaley, float scalez)
{
	// The line width is always the same in the y and z axes (the line
	// approximates a cylinder).
	Vector3 scale(scalex,scaley,scalez);
	for (int i=0; i<8; i++) vert[i].Scale(scale);	
	Length *= scalex;
   Width *= scaley;

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   RenderObjClass *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}


void Line3DClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	float half_l = Length * 0.5f;
	sphere.Center.Set(half_l, 0.0f, 0.0f);
	sphere.Radius = half_l;
}


void Line3DClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	float half_l = Length * 0.5f;
	box.Center.Set(half_l, 0.0f, 0.0f);
	box.Extent.Set(half_l, 0.0f, 0.0f);
}

/************************************************************************** 
 * Line3DClass::Reset -- Reset line start and end points.                 * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/19/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   * 
 *========================================================================*/
void Line3DClass::Reset(const Vector3 & new_start, const Vector3 & new_end)
{
	// Adjust length of line:
	float new_length = (new_end - new_start).Length();
	if (new_length == 0) {
		new_length = 0.001f;			// make sure we don't have a zero length BMG
	}
	Scale((new_length / Length), 1.0f, 1.0f);
	Length = new_length;

	// Adjust transform of line:
	Matrix3D transform(true);
	transform.Obj_Look_At(new_start, new_end, 0.0);
	Set_Transform(transform);

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   RenderObjClass *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}


/************************************************************************** 
 * Line3DClass::Reset -- Reset line start and end points, and line width. * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/19/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   * 
 *========================================================================*/
void Line3DClass::Reset(const Vector3 & new_start, const Vector3 & new_end, float new_width)
{
	// Adjust length and width of line:
	float new_length = (new_end - new_start).Length();
	if (new_length == 0) {
		new_length = 0.001f;			// make sure we don't have a zero length BMG
	}
   float width_scale = new_width / Width;
	Scale((new_length / Length), width_scale, width_scale);
	Length = new_length;
   Width = new_width;

	// Adjust transform of line:
	Matrix3D transform(true);
	transform.Obj_Look_At(new_start, new_end, 0.0);
	Set_Transform(transform);
   Matrix3D inv;
	transform.Get_Orthogonal_Inverse(inv);
#ifdef ALLOW_TEMPORARIES
//   Vector3 test = inv * Vector3(new_end);
#else
//		Vector3 test;
//		inv.mulVector3(new_end, test);
#endif

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   RenderObjClass *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}


/************************************************************************** 
 * Re_Color -- Reset the line color.                                      * 
 *                                                                        * 
 * INPUT:	float r, g, b - components of the new color.                  * 
 *                                                                        * 
 * OUTPUT:	none.                                                         * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   01/26/1998 NH  : Created.                                            * 
 *   04/21/1998 NH  : Ported to SR 1.3.                                   * 
 *========================================================================*/
void Line3DClass::Re_Color(float r, float g, float b)
{
	Color=Vector4(r,g,b,Color.W);
}


/************************************************************************** 
 * Set_Opacity -- Reset the line opacity.                                 * 
 *                                                                        * 
 * INPUT:	float opacity - new opacity.                                  * 
 *                                                                        * 
 * OUTPUT:	none.                                                         * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   11/03/1998 NH  : Created.                                            * 
 *========================================================================*/
void Line3DClass::Set_Opacity(float opacity)
{
	if (opacity < 1.0f)
	{	Shader=ShaderClass::_PresetAlphaSolidShader;
		Set_Sort_Level(1);
	}
	else
	{	Shader=ShaderClass::_PresetOpaqueSolidShader;
		Set_Sort_Level(SORT_LEVEL_NONE);
	}
	Color.W=opacity;
}

/*
**
*/
int Line3DClass::Get_Num_Polys(void) const
{ 
	return 12;
}

