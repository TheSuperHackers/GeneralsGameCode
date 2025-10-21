/*
**	Command & Conquer Renegade(tm)
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

// Chunk_D.h: interface for the ChunkData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHUNK_D_H__41C157F9_5631_11D1_8CDB_006097C6A583__INCLUDED_)
#define AFX_CHUNK_D_H__41C157F9_5631_11D1_8CDB_006097C6A583__INCLUDED_

#pragma once
class ChunkItem;

class ChunkType {
public:
	ChunkType(const char *name, void (*callback)(ChunkItem *item, CListCtrl *List), bool wrapper = false) {Name = name;Callback = callback;	Wrapper = wrapper;};
	const char *Name;
	bool Wrapper;
	void (*Callback)(ChunkItem *, CListCtrl *list);
};

class ChunkTableClass {
public:
	ChunkTableClass();
	~ChunkTableClass();
	ChunkType *Lookup(int ID);
	void NewType(int ID, const char *name, void (*callback)(ChunkItem *item, CListCtrl *list) = 0, bool wrapper = false);
protected:
	CMapPtrToPtr Types;

	static void AddItem(CListCtrl *List, int &Counter, const char *Name, const char *Value, const char *Type="string");
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, uint32 Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, uint16 Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, uint8 Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, uint8 *Value, int Count);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, float32 Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, uint32 *Value, int Count);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, float32 *Value, int Count);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, IOVector3Struct *Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, IOVector4Struct *Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, W3dQuaternionStruct *Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, W3dRGBStruct *Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, W3dRGBStruct *Value, int count);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, W3dRGBAStruct *Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, W3dTexCoordStruct *Value, int Count);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, W3dTexCoordStruct *Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *Name, Vector3i *Value);
	static void AddItem(CListCtrl *List, int &Counter, const char *name, W3dShaderStruct * shader);
	static void AddItem(CListCtrl *List, int &Counter, const char *name, W3dPS2ShaderStruct * shader);

	static void AddItemVersion(CListCtrl *List,int &Counter,uint32 version);


	static void List_Subitems(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_MESH(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MESH_HEADER(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_VERTICES(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_NORMALS(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_SURRENDER_NORMALS(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_TEXCOORDS(ChunkItem *Item, CListCtrl *List);
	static void List_O_W3D_CHUNK_MATERIALS(ChunkItem *Item, CListCtrl *List);
	static void List_O_W3D_CHUNK_TRIANGLES(ChunkItem *Item, CListCtrl *List);
	static void List_O_W3D_CHUNK_QUADRANGLES(ChunkItem *Item, CListCtrl *List);
	static void List_O_W3D_CHUNK_SURRENDER_TRIANGLES(ChunkItem *Item, CListCtrl *List);
	static void List_O_W3D_CHUNK_POV_TRIANGLES(ChunkItem *Item, CListCtrl *List);
	static void List_O_W3D_CHUNK_POV_QUADRANGLES(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MESH_USER_TEXT(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_COLORS(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_INFLUENCES(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_DAMAGE(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_DAMAGE_HEADER(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_DAMAGE_VERTICES(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_DAMAGE_COLORS(ChunkItem *Item, CListCtrl *List);

	static void List_O_W3D_CHUNK_MATERIALS2(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_MATERIALS3(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MATERIAL3(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MATERIAL3_NAME(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MATERIAL3_INFO(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MATERIAL3_DC_MAP(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MAP3_FILENAME(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MAP3_INFO(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MATERIAL3_DI_MAP(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MATERIAL3_SC_MAP(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_MATERIAL3_SI_MAP(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_MESH_HEADER3(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_TRIANGLES(ChunkItem * Item, CListCtrl *List);
	static void List_W3D_CHUNK_PER_TRI_MATERIALS(ChunkItem * Item,CListCtrl *List);

	static void List_W3D_CHUNK_VERTEX_SHADE_INDICES(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_MATERIAL_INFO(ChunkItem * Item,CListCtrl *List);

	static void List_W3D_CHUNK_VERTEX_TANGENTS(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_BINORMALS(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_SHADERS(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_PS2_SHADERS(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_FX_SHADERS(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_FX_SHADER(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_FX_SHADER_INFO(ChunkItem* Item, CListCtrl* List);
	static void List_W3D_CHUNK_FX_SHADER_CONSTANT(ChunkItem* Item, CListCtrl* List);

	static void List_W3D_CHUNK_VERTEX_MATERIALS(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_MATERIAL(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_MATERIAL_NAME(ChunkItem *Item,CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_MATERIAL_INFO(ChunkItem *Item,CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_MAPPER_ARGS0(ChunkItem *Item,CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_MAPPER_ARGS1(ChunkItem *Item,CListCtrl *List);

	static void List_W3D_CHUNK_TEXTURES(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_TEXTURE(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_TEXTURE_NAME(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_TEXTURE_INFO(ChunkItem * Item,CListCtrl *List);

	static void List_W3D_CHUNK_MATERIAL_PASS(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_VERTEX_MATERIAL_IDS(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_SHADER_IDS(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_DCG(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_DIG(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_SCG(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_FXSHADER_IDS(ChunkItem* Item, CListCtrl* List);

	static void List_W3D_CHUNK_TEXTURE_STAGE(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_TEXTURE_IDS(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_STAGE_TEXCOORDS(ChunkItem * Item,CListCtrl *List);
	static void	List_W3D_CHUNK_PER_FACE_TEXCOORD_IDS(ChunkItem * Item,CListCtrl *List);

	static void List_W3D_CHUNK_AABTREE(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_AABTREE_HEADER(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_AABTREE_POLYINDICES(ChunkItem * Item,CListCtrl *List);
	static void List_W3D_CHUNK_AABTREE_NODES(ChunkItem * Item,CListCtrl *List);

	static void List_W3D_CHUNK_HIERARCHY(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_HIERARCHY_HEADER(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_PIVOTS(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_PIVOT_FIXUPS(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_ANIMATION(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_ANIMATION_HEADER(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_ANIMATION_CHANNEL(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_BIT_CHANNEL(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_HMODEL(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_HMODEL_HEADER(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_NODE(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_COLLISION_NODE(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_SKIN_NODE(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_HMODEL_AUX_DATA(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_SHADOW_NODE(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_LODMODEL(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_LODMODEL_HEADER(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_LOD(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_COLLECTION(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_COLLECTION_HEADER(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_COLLECTION_OBJ_NAME(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_PLACEHOLDER(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_TRANSFORM_NODE(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_POINTS(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_LIGHT(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_LIGHT_INFO(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_SPOT_LIGHT_INFO(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_NEAR_ATTENUATION(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_FAR_ATTENUATION(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_EMITTER(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_HEADER(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_USER_DATA(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_INFO(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_INFOV2(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_PROPS(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_COLOR_KEYFRAME(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_SIZE_KEYFRAME(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_AGGREGATE(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_AGGREGATE_HEADER(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_AGGREGATE_INFO(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_TEXTURE_REPLACER_INFO(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_AGGREGATE_CLASS_INFO(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_HLOD(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_HLOD_HEADER(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_HLOD_LOD_ARRAY(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_HLOD_LOD_ARRAY_HEADER(ChunkItem * Item,CListCtrl * List);
	static void List_W3D_CHUNK_HLOD_SUB_OBJECT(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_BOX(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_NULL_OBJECT(ChunkItem * Item,CListCtrl * List);

	static void List_W3D_CHUNK_PRELIT_UNLIT(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_PRELIT_VERTEX(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_LIGHTSCAPE(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_LIGHTSCAPE_LIGHT(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_LIGHT_TRANSFORM(ChunkItem *Item, CListCtrl *List);

	static void List_W3D_CHUNK_DAZZLE(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_DAZZLE_NAME(ChunkItem *Item, CListCtrl *List);
	static void List_W3D_CHUNK_DAZZLE_TYPENAME(ChunkItem *Item, CListCtrl *List);

};

class ChunkItem {
public:
	ChunkItem(ChunkLoadClass &cload); // constructor copies header and data into buffer
	~ChunkItem();
	int ID;
	ChunkType *Type;
	int Length;
	void *Data;
	CList<ChunkItem *, ChunkItem *> Chunks; // wrapper chunks will have members here.
protected:
	static ChunkTableClass ChunkTable;
};


class ChunkData
{
public:
	void Release_Data();
	ChunkData();
	virtual ~ChunkData();
	bool Load(const char *Filename);
	CList<ChunkItem *, ChunkItem *> Chunks;

protected:
	void Add_Chunk(ChunkLoadClass &cload, ChunkItem *Parent = 0); // add a chunk to the list of chunks maintained by ChunkData
};

#endif // !defined(AFX_CHUNK_D_H__41C157F9_5631_11D1_8CDB_006097C6A583__INCLUDED_)
