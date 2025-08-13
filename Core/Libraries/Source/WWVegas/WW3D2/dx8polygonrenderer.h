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
 *                     $Archive:: /Commando/Code/ww3d2/dx8polygonrenderer.h                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/12/01 6:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef DX8_POLYGON_RENDERER_H
#define DX8_POLYGON_RENDERER_H


#include "always.h"
#include "meshmdl.h"
#include "dx8list.h"
#include "sortingrenderer.h"
#include "mesh.h"
#include "dx8wrapper.h"

class DX8PolygonRendererClass;
class DX8TextureCategoryClass;


/**
** DX8PolygonRendererClass
** This is a record of a batch/range of polygons to be rendered.  These hang off of the DX8TextureCategoryClass's
** and are rendered after the system installs a vertex buffer and textures in the DX8 wrapper.
*/
class DX8PolygonRendererClass : public MultiListObjectClass
{
	MeshModelClass *				mmc;
	DX8TextureCategoryClass *	texture_category;
	unsigned							index_offset;				// absolute index of index 0 for our parent mesh
	unsigned							vertex_offset;				// absolute index of vertex 0 for our parent mesh
	unsigned							index_count;				// number of indices
	unsigned							min_vertex_index;			// relative index of the first vertex our polys reference
	unsigned							vertex_index_range;		// range to the last vertex our polys reference
	bool								strip;						// is this a strip?
	unsigned							pass;					// rendering pass

public:
	DX8PolygonRendererClass(
		unsigned index_count,
		MeshModelClass* mmc_,
		DX8TextureCategoryClass* tex_cat,
		unsigned vertex_offset,
		unsigned index_offset,
		bool strip,
		unsigned pass);
	DX8PolygonRendererClass(const DX8PolygonRendererClass& src,MeshModelClass* mmc_);
	~DX8PolygonRendererClass();

	void								Render(/*const Matrix3D & tm,*/int base_vertex_offset);
	void								Render_Sorted(/*const Matrix3D & tm,*/int base_vertex_offset,const SphereClass & bounding_sphere);
	void								Set_Vertex_Index_Range(unsigned min_vertex_index_,unsigned vertex_index_range_);

	unsigned							Get_Vertex_Offset(void)	{ return vertex_offset; }
	unsigned							Get_Index_Offset(void)	{ return index_offset; }
	inline unsigned						Get_Pass(void)	{ return pass; }

	MeshModelClass*				Get_Mesh_Model_Class() { return mmc; }
	DX8TextureCategoryClass*	Get_Texture_Category() { return texture_category; }
	void								Set_Texture_Category(DX8TextureCategoryClass* tc) { texture_category=tc; }

	void Log();
};

// ----------------------------------------------------------------------------

inline void DX8PolygonRendererClass::Set_Vertex_Index_Range(unsigned min_vertex_index_,unsigned vertex_index_range_)
{
//	WWDEBUG_SAY(("Set_Vertex_Index_Range - min: %d, range: %d",min_vertex_index_,vertex_index_range_));
//	if (vertex_index_range_>30000) {
//		int a=0;
//		a++;
//	}
	min_vertex_index=min_vertex_index_;
	vertex_index_range=vertex_index_range_;
}

// ----------------------------------------------------------------------------

inline void DX8PolygonRendererClass::Render(/*const Matrix3D & tm,*/int base_vertex_offset)
{
//	DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
//	SNAPSHOT_SAY(("Set_Transform"));
	SNAPSHOT_SAY(("Set_Index_Buffer_Index_Offset(%d)",base_vertex_offset));

	DX8Wrapper::Set_Index_Buffer_Index_Offset(base_vertex_offset);
	if (strip) {
		SNAPSHOT_SAY(("Draw_Strip(%d,%d,%d,%d)",index_offset,index_count-2,min_vertex_index,vertex_index_range));
		DX8Wrapper::Draw_Strip(
			index_offset,
			index_count-2,
			min_vertex_index,
			vertex_index_range);
	}
	else {
		SNAPSHOT_SAY(("Draw_Triangles(%d,%d,%d,%d)",index_offset,index_count-2,min_vertex_index,vertex_index_range));
		DX8Wrapper::Draw_Triangles(
			index_offset,
			index_count/3,
			min_vertex_index,
			vertex_index_range);
	}
}

inline void DX8PolygonRendererClass::Render_Sorted(/*const Matrix3D & tm,*/int base_vertex_offset,const SphereClass & bounding_sphere)
{
	WWASSERT(!strip);	// Strips can't be sorted for now
//	DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
//	SNAPSHOT_SAY(("Set_Transform"));
	SNAPSHOT_SAY(("Set_Index_Buffer_Index_Offset(%d)",base_vertex_offset));
	SNAPSHOT_SAY(("Insert_Sorting_Triangles(%d,%d,%d,%d)",index_offset,index_count-2,min_vertex_index,vertex_index_range));

	DX8Wrapper::Set_Index_Buffer_Index_Offset(base_vertex_offset);
	SortingRendererClass::Insert_Triangles(
		bounding_sphere,
		index_offset,
		index_count/3,
		min_vertex_index,
		vertex_index_range);

}

#endif
