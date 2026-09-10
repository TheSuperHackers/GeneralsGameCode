import Graphics.Frame.RenderSettings;
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

#include "W3DDevice/GameClient/W3DMeshResource.h"
#include "W3DDevice/GameClient/W3DMeshDrawing.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "WWMath/vp.h"
#include "W3DDevice/GameClient/W3DCamera.h"



/*
**
** W3DMeshResource Implementation
**
*/


W3DMeshResource::W3DMeshResource() :
	CurMatDesc(&DefMatDesc),
	MatInfo(nullptr)
{
	Set_Flag(DIRTY_BOUNDS,true);

	MatInfo = std::make_shared<Graphics::ModelMaterials<RefCountPtr<W3DTextureHandle>>>();
}

W3DMeshResource::W3DMeshResource(const W3DMeshResource & that) :
	W3DMeshGeometry(that),
	DefMatDesc(that.DefMatDesc),
	CurMatDesc(&DefMatDesc),
	MatInfo(nullptr)
{
	if (that.AlternateMatDesc) {
		AlternateMatDesc = std::make_unique<MaterialDescription>(*that.AlternateMatDesc);
	}

	clone_materials(that);
}

W3DMeshResource::~W3DMeshResource()
{

	Reset(0,0,0);
	MatInfo.reset();

}

W3DMeshResource & W3DMeshResource::operator = (const W3DMeshResource & that)
{
	if (this != &that) {
        GraphicsMeshes.reset();

		W3DMeshGeometry::operator = (that);

		DefMatDesc = that.DefMatDesc;
		CurMatDesc = &DefMatDesc;

		AlternateMatDesc.reset();
		if (that.AlternateMatDesc) {
			AlternateMatDesc = std::make_unique<MaterialDescription>(*that.AlternateMatDesc);
		}

		clone_materials(that);
	}
	return * this;
}

void W3DMeshResource::Reset(int polycount,int vertcount,int passcount)
{
    GraphicsMeshes.reset();
	Reset_Geometry(polycount,vertcount);

	// Release everything we have and reset to initial state


	MatInfo->Reset();
	DefMatDesc.Reset(polycount,vertcount,passcount);

	AlternateMatDesc.reset();

	CurMatDesc = &DefMatDesc;
}


void W3DMeshResource::Replace_Texture(W3DTextureHandle* texture,W3DTextureHandle* new_texture)
{
	WWASSERT(texture);
	WWASSERT(new_texture);
	for (int stage=0;stage<MaterialDescription::MAX_TEX_STAGES;++stage) {
		for (int pass=0;pass<Get_Pass_Count();++pass) {
			if (Has_Texture_Array(pass,stage)) {
				for (int i=0;i<Get_Polygon_Count();++i) {
					if (Peek_Texture(i,pass,stage)==texture) {
						Set_Texture(i,new_texture,pass,stage);
					}
				}
			}
			else {
				if (Peek_Single_Texture(pass,stage)==texture) {
					Set_Single_Texture(new_texture,pass,stage);
				}
			}
		}
	}
}

void W3DMeshResource::Replace_VertexMaterial(Graphics::MeshMaterial* vmat,const std::shared_ptr<Graphics::MeshMaterial>& new_vmat)
{
	WWASSERT(vmat);
	WWASSERT(new_vmat);

	for (int pass=0;pass<Get_Pass_Count();++pass) {
		if (Has_Material_Array(pass)) {
			for (int i=0;i<Get_Vertex_Count();++i) {
				if (Peek_Material(i,pass)==vmat) {
					Set_Material(i,new_vmat,pass);
				}
			}
		}
		else {
			if (Peek_Single_Material(pass)==vmat) {
				Set_Single_Material(new_vmat,pass);
			}
		}
	}
}


void W3DMeshResource::Make_Geometry_Unique()
{
    Geometry.Detach_Positions_And_Normals();
}

void W3DMeshResource::Make_UV_Array_Unique(int pass,int stage)
{
	CurMatDesc->Make_UV_Array_Unique(pass,stage);
}

void W3DMeshResource::Make_Color_Array_Unique(int array_index)
{
	CurMatDesc->Make_Color_Array_Unique(array_index);
}

void W3DMeshResource::Enable_Alternate_Material_Description(bool onoff)
{
	if ((onoff == true) && AlternateMatDesc) {
		if (CurMatDesc != AlternateMatDesc.get()) {
			CurMatDesc = AlternateMatDesc.get();

			if (Get_Flag(SORT) && Graphics::Get_Render_Settings().Is_Munge_Sort_On_Load_Enabled())
				compute_static_sort_levels();

			if (Graphics::Get_Render_Settings().Is_Overbright_Modify_On_Load_Enabled())
				modify_for_overbright();

			// TODO: Invalidate just this meshes DX9 data!!!
		}
	} else {
		if (CurMatDesc != &DefMatDesc) {
			CurMatDesc = &DefMatDesc;

			if (Get_Flag(SORT) && Graphics::Get_Render_Settings().Is_Munge_Sort_On_Load_Enabled())
				compute_static_sort_levels();

			if (Graphics::Get_Render_Settings().Is_Overbright_Modify_On_Load_Enabled())
				modify_for_overbright();

		}
	}
}

bool W3DMeshResource::Is_Alternate_Material_Description_Enabled()
{
	return AlternateMatDesc && CurMatDesc == AlternateMatDesc.get();
}
bool W3DMeshResource::Needs_Vertex_Normals()
{
	if (Get_Flag(W3DMeshResource::PRELIT_MASK) == 0) {
		return true;
	}
	return CurMatDesc->Do_Mappers_Need_Normals();
}
