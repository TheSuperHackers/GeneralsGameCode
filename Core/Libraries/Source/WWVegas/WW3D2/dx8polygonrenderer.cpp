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
 *                     $Archive:: /Commando/Code/ww3d2/dx8polygonrenderer.cpp                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/22/01 6:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#include "dx8polygonrenderer.h"
#include "dx8renderer.h"


// ----------------------------------------------------------------------------

DX8PolygonRendererClass::DX8PolygonRendererClass(
	unsigned index_count_,
	MeshModelClass* mmc_,
	DX8TextureCategoryClass* tex_cat,
	unsigned vertex_offset_,
	unsigned index_offset_,
	bool strip_,
	unsigned pass_)
	:
	mmc(mmc_), 
	texture_category(tex_cat),
	index_offset(index_offset_),
	vertex_offset(vertex_offset_),
	min_vertex_index(0),
	vertex_index_range(0),
	index_count(index_count_),
	strip(strip_),
	pass(pass_)
{
	WWASSERT(index_count);
	mmc->PolygonRendererList.Add_Tail(this);
}

DX8PolygonRendererClass::DX8PolygonRendererClass(const DX8PolygonRendererClass& src,MeshModelClass* mmc_)
	:
	mmc(mmc_), 
	texture_category(src.texture_category),
	index_offset(src.index_offset),
	vertex_offset(src.vertex_offset),
	min_vertex_index(src.min_vertex_index),
	vertex_index_range(src.vertex_index_range),
	index_count(src.index_count),
	strip(src.strip),
	pass(src.pass)
{
	mmc->PolygonRendererList.Add_Tail(this);
}

DX8PolygonRendererClass::~DX8PolygonRendererClass()
{
	if (texture_category) texture_category->Remove_Polygon_Renderer(this);
}

// ----------------------------------------------------------------------------

void DX8PolygonRendererClass::Log()
{
	StringClass work(true);

	work.Format("	%8d %8d %6d %6d %6d %s\n",
		index_count,
		index_count/3,
		index_offset,
		min_vertex_index,
		vertex_index_range,
		mmc->Get_Name());

/*	work.Format(
		"		Index count: %d (%d polys) i_offset: %d min_vi: %d vi_range: %d ident: %d (%s)\n",
		index_count,
		index_count/3,
		index_offset,
		min_vertex_index,
		vertex_index_range,
		mmc->ident,
		mmc->Get_Name());
*/	WWDEBUG_SAY((work));
}

