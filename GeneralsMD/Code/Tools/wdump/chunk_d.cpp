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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WDump                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DShellExt/External/chunk_d.cpp       $*
 *                                                                                             *
 *                      $Author:: Moumine_ballo                                               $*
 *                                                                                             *
 *                     $Modtime:: 1/02/02 1:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 66                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "w3d_file.h"
#include "stdafx.h"
#include "wdump.h"
#include "Chunk_D.h"
#include "rawfilem.h"
#include "finddialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static int Get_Bit(void const * array, int bit);
static void Set_Bit(void * array, int bit, int value);


ChunkTableClass ChunkItem::ChunkTable;

ChunkTableClass::~ChunkTableClass() {
	ChunkType *chunktype;
	POSITION p = Types.GetStartPosition();
	while(p) {
		void *key;
		Types.GetNextAssoc(p, key, (void *&) chunktype);
		delete chunktype;
	}
}
void ChunkTableClass::NewType(int ID, const char *name, void (*callback)(ChunkItem *item, CListCtrl *list), bool wrapper) {
	ChunkType *chunktype = new ChunkType(name, callback, wrapper);
	Types.SetAt((void *) ID, (void *) chunktype);
}
void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, const char *Value, const char *Type) {

	if (List != NULL) {
		int list_item = List->InsertItem(Counter++, Name);
		List->SetItemText(list_item, 1, Type);
		List->SetItemText(list_item, 2, Value);

	} else {
		FindDialog::Compare (Value);
	}
}

void ChunkTableClass::AddItemVersion(CListCtrl *List,int &Counter,uint32 version)
{
	char buf[64];
	sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(version),W3D_GET_MINOR_VERSION(version));
	AddItem(List,Counter,"Version",buf);
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, uint32 Value) {
	char buf[256];
	sprintf(buf, "%d", Value);
	AddItem(List, Counter, Name, buf, "int32");
}
void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, uint16 Value) {
	char buf[256];
	sprintf(buf, "%d", Value);
	AddItem(List, Counter, Name, buf, "int16");
}
void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, uint8 Value) {
	char buf[256];
	sprintf(buf, "%d", Value);
	AddItem(List, Counter, Name, buf, "int8");
}
void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, uint8 *Value, int Count) {
	CString buffer;
	CString temp;
	int counter = 0;
	while(counter < Count) {
		temp.Format("%d ", Value[counter++]);
		buffer += temp;
	}
	char type[256];
	sprintf(type, "int8[%d]", Count);
	AddItem(List, Counter, Name, (LPCTSTR) buffer, type);
}
void ChunkTableClass::AddItem(CListCtrl *List,int &Counter,  const char *Name, float32 Value) {
	char buf[256];
	sprintf(buf, "%f", Value);
	AddItem(List, Counter, Name, buf, "float");
}
void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, uint32 *Value, int Count) {
	CString buffer;
	CString temp;
	int counter = 0;
	while(counter < Count) {
		temp.Format("%d ", Value[counter++]);
		buffer += temp;
	}
	char type[256];
	sprintf(type, "int32[%d]", Count);
	AddItem(List, Counter, Name, (LPCTSTR) buffer, type);
}
void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, float32 *Value, int Count) {
	CString buffer;
	CString temp;
	int counter = 0;
	while(counter < Count) {
		temp.Format("%f ", Value[counter++]);
		buffer += temp;
	}
	char type[256];
	sprintf(type, "float[%d]", Count);
	AddItem(List, Counter, Name, (LPCTSTR) buffer, type);
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, IOVector3Struct *Value) {
	char buf[256];
	sprintf(buf, "%f %f %f", Value->X, Value->Y, Value->Z);
	AddItem(List, Counter, Name, buf, "vector");
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, IOVector4Struct *Value) {
	char buf[256];
	sprintf(buf, "%f %f %f %f", Value->X, Value->Y, Value->Z, Value->W);
	AddItem(List, Counter, Name, buf, "vector4");
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, W3dQuaternionStruct *Value) {
	char buf[256];
	sprintf(buf, "%f %f %f %f", Value->Q[0], Value->Q[1], Value->Q[2], Value->Q[3]);
	AddItem(List, Counter, Name, buf, "quaternion");
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, W3dRGBStruct *Value) {
	CString buffer;
	buffer.Format( "(%d %d %d) ", Value->R, Value->G, Value->B);
	AddItem(List, Counter, Name, (LPCTSTR) buffer, "RGB");
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, W3dRGBStruct *Value, int Count) {
	CString buffer;
	CString temp;
	int counter = 0;
	while(counter < Count) {
		temp.Format( "(%d %d %d) ", Value[counter].R, Value[counter].G, Value[counter].B);
		counter++;
		buffer += temp;
	}
	char type[256];
	sprintf(type, "RGB[%d]", Count);
	AddItem(List, Counter, Name, (LPCTSTR) buffer, type);
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, W3dRGBAStruct *Value) {
	CString buffer;
	buffer.Format( "(%d %d %d %d) ", Value->R, Value->G, Value->B, Value->A);
	AddItem(List, Counter, Name, (LPCTSTR) buffer, "RGBA");
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, W3dTexCoordStruct *Value, int Count) {
	CString temp;
	int counter = 0;
	while(counter < Count) {
		char type[256];
		sprintf(type, "%s.TexCoord[%d]", Name, counter);
		AddItem(List, Counter, type, &Value[counter]);
		counter++;
	}
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, W3dTexCoordStruct *Value) 
{
	char buf[256];
	sprintf(buf, "%f %f", Value->U, Value->V);
	AddItem(List, Counter, Name, buf, "UV");
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *name, W3dShaderStruct * shader) 
{
	static const char * _depth_compare[] = { "Pass Never","Pass Less","Pass Equal","Pass Less or Equal", "Pass Greater","Pass Not Equal","Pass Greater or Equal","Pass Always" };
	static const char * _depth_mask[] = { "Write Disable", "Write Enable" };
	static const char * _color_mask[] = { "Write Disable", "Write Enable" };
	static const char * _destblend[] = { "Zero","One","Src Color","One Minus Src Color","Src Alpha","One Minus Src Alpha","Src Color Prefog" };
	static const char * _fogfunc[] = { "Disable","Enable","Scale Fragment","Replace Fragment" };
	static const char * _prigradient[] = { "Disable","Modulate","Add","Bump-Environment" };
	static const char * _secgradient[] = { "Disable","Enable" };
	static const char * _srcblend[] = { "Zero","One","Src Alpha","One Minus Src Alpha" };
	static const char * _texturing[] = { "Disable","Enable" };
	static const char * _detailcolor[] = { "Disable","Detail","Scale","InvScale","Add","Sub","SubR","Blend","DetailBlend" };
	static const char * _detailalpha[] = { "Disable","Detail","Scale","InvScale" };
	static const char * _dithermask[] = { "Disable", "Enable" };
	static const char * _shademodel[] = { "Smooth", "Flat" };
	static const char * _alphatest[] = { "Alpha Test Disable", "Alpha Test Enable" };

	int counter = 0;
	char label[256];

	sprintf(label,"%s.DepthCompare",name);
	AddItem(List, Counter, label, _depth_compare[W3d_Shader_Get_Depth_Compare(shader)]);
	sprintf(label,"%s.DepthMask",name);
	AddItem(List, Counter, label, _depth_mask[W3d_Shader_Get_Depth_Mask(shader)]);
	sprintf(label,"%s.DestBlend",name);
	AddItem(List, Counter, label, _destblend[W3d_Shader_Get_Dest_Blend_Func(shader)]);
	sprintf(label,"%s.PriGradient",name);
	AddItem(List, Counter, label, _prigradient[W3d_Shader_Get_Pri_Gradient(shader)]);
	sprintf(label,"%s.SecGradient",name);
	AddItem(List, Counter, label, _secgradient[W3d_Shader_Get_Sec_Gradient(shader)]);
	sprintf(label,"%s.SrcBlend",name);
	AddItem(List, Counter, label, _srcblend[W3d_Shader_Get_Src_Blend_Func(shader)]);
	sprintf(label,"%s.Texturing",name);
	AddItem(List, Counter, label, _texturing[W3d_Shader_Get_Texturing(shader)]);
	sprintf(label,"%s.DetailColor",name);
	AddItem(List, Counter, label, _detailcolor[W3d_Shader_Get_Detail_Color_Func(shader)]);
	sprintf(label,"%s.DetailAlpha",name);
	AddItem(List, Counter, label, _detailalpha[W3d_Shader_Get_Detail_Alpha_Func(shader)]);
	sprintf(label,"%s.AlphaTest",name);
	AddItem(List, Counter, label, _alphatest[W3d_Shader_Get_Alpha_Test(shader)]);	

	counter++;
	shader++;
}

void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *name, W3dPS2ShaderStruct * shader) 
{
	static const char * _depth_compare[] = { "Pass Never","Pass Less","Pass Always","Pass Less or Equal"};
	static const char * _depth_mask[] = { "Write Disable", "Write Enable" };
	static const char * _color_mask[] = { "Write Disable", "Write Enable" };
	static const char * _ablend[] = { "Src Color","Dest Color","Zero"};
	static const char * _cblend[] = { "Src Alpha","Dest Alpha","One"};
	static const char * _fogfunc[] = { "Disable","Enable","Scale Fragment","Replace Fragment" };
	static const char * _prigradient[] = { "Disable","Modulate","Highlight","Highlight2" };
	static const char * _secgradient[] = { "Disable","Enable" };
	static const char * _texturing[] = { "Disable","Enable" };
	static const char * _detailcolor[] = { "Disable","Detail","Scale","InvScale","Add","Sub","SubR","Blend","DetailBlend" };
	static const char * _detailalpha[] = { "Disable","Detail","Scale","InvScale" };
	static const char * _dithermask[] = { "Disable", "Enable" };
	static const char * _shademodel[] = { "Smooth", "Flat" };

	int counter = 0;
	char label[256];

	sprintf(label,"%s.DepthCompare",name);
	AddItem(List, Counter, label, _depth_compare[W3d_Shader_Get_Depth_Compare(shader)]);
	sprintf(label,"%s.DepthMask",name);
	AddItem(List, Counter, label, _depth_mask[W3d_Shader_Get_Depth_Mask(shader)]);
	sprintf(label,"%s.PriGradient",name);
	AddItem(List, Counter, label, _prigradient[W3d_Shader_Get_Pri_Gradient(shader)]);
	sprintf(label,"%s.Texturing",name);
	AddItem(List, Counter, label, _texturing[W3d_Shader_Get_Texturing(shader)]);

	sprintf(label,"%s.AParam",name);
	AddItem(List, Counter, label, _ablend[W3d_Shader_Get_PS2_Param_A(shader)]);

	sprintf(label,"%s.BParam",name);
	AddItem(List, Counter, label, _ablend[W3d_Shader_Get_PS2_Param_B(shader)]);

	sprintf(label,"%s.CParam",name);
	AddItem(List, Counter, label, _cblend[W3d_Shader_Get_PS2_Param_C(shader)]);

	sprintf(label,"%s.DParam",name);
	AddItem(List, Counter, label, _ablend[W3d_Shader_Get_PS2_Param_D(shader)]);

	counter++;
	shader++;
}


void ChunkTableClass::AddItem(CListCtrl *List, int &Counter, const char *Name, Vector3i *Value) {
	char buf[256];
	sprintf(buf, "%d %d %d", Value->I, Value->J, Value->K);
	AddItem(List, Counter, Name, buf, "IJK");
}

void ChunkTableClass::List_Subitems(ChunkItem *Item, CListCtrl *List) {
	int counter = 0;
	POSITION p = Item->Chunks.GetHeadPosition();
	while(p) {
		ChunkItem *subitem = Item->Chunks.GetNext(p);
		if(subitem->Type) {
			AddItem(List, counter,subitem->Type->Name,  "", "chunk");
		}
	}
}
void ChunkTableClass::List_W3D_CHUNK_MESH(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}
void ChunkTableClass::List_W3D_CHUNK_MESH_HEADER(ChunkItem *Item, CListCtrl *List) {

	struct W3dMeshHeaderStruct *data;
	data = (W3dMeshHeaderStruct *) Item->Data;
	int Counter = 0;

	char buf[64];
	sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(data->Version),W3D_GET_MINOR_VERSION(data->Version));
	AddItem(List, Counter,"Version", buf);
	AddItem(List, Counter,"MeshName", data->MeshName);
	AddItem(List,Counter,"Attributes",data->Attributes);
	if (data->Attributes & W3D_MESH_FLAG_COLLISION_BOX)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_COLLISION_BOX");
	if (data->Attributes & W3D_MESH_FLAG_SKIN)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_SKIN");
	if (data->Attributes & W3D_MESH_FLAG_SHADOW)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_SHADOW");
	if (data->Attributes & W3D_MESH_FLAG_ALIGNED)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_ALIGNED");
	if (data->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL");
	if (data->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE");
	AddItem(List, Counter,"NumTris", data->NumTris);
	AddItem(List, Counter,"NumQuads", data->NumQuads);
	AddItem(List, Counter,"NumSrTris", data->NumSrTris);
	AddItem(List, Counter,"NumPovQuads", data->NumPovQuads);
	AddItem(List, Counter,"NumVertices", data->NumVertices);
	AddItem(List, Counter,"NumNormals", data->NumNormals);
	AddItem(List, Counter,"NumSrNormals", data->NumSrNormals);
	AddItem(List, Counter,"NumTexCoords", data->NumTexCoords);
	AddItem(List, Counter,"NumMaterials", data->NumMaterials);
	AddItem(List, Counter,"NumVertColors", data->NumVertColors);
	AddItem(List, Counter,"NumVertInfluences", data->NumVertInfluences);
	AddItem(List, Counter,"NumDamageStages", data->NumDamageStages);
	AddItem(List, Counter,"FutureCounts", data->FutureCounts, 5);
	AddItem(List, Counter,"LODMin", data->LODMin);
	AddItem(List, Counter,"LODMax", data->LODMax);
	AddItem(List, Counter,"Min", &data->Min);
	AddItem(List, Counter,"Max", &data->Max);
	AddItem(List, Counter,"SphCenter", &data->SphCenter);
	AddItem(List, Counter,"SphRadius", data->SphRadius);
	AddItem(List, Counter,"Translation", &data->Translation);
	AddItem(List, Counter,"Rotation", data->Rotation, 9);
	AddItem(List, Counter,"MassCenter", &data->MassCenter);
	AddItem(List, Counter,"Inertia", data->Inertia, 9);
	AddItem(List, Counter,"Volume", data->Volume);
	AddItem(List, Counter,"HierarchyTreeName", data->HierarchyTreeName);
	AddItem(List, Counter,"HierarchyModelName", data->HierarchyModelName);
	AddItem(List, Counter,"FutureUse", data->FutureUse, 24);
}
void ChunkTableClass::List_W3D_CHUNK_VERTICES(ChunkItem *Item, CListCtrl *List) {
	W3dVectorStruct *data;
	data = (W3dVectorStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {
		sprintf(buf, "Vertex[%d]", counter++);
		AddItem(List, Counter,buf, data);
		data++;
	}
}
void ChunkTableClass::List_W3D_CHUNK_VERTEX_NORMALS(ChunkItem *Item, CListCtrl *List) {
	W3dVectorStruct *data;
	data = (W3dVectorStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {
		sprintf(buf, "Normal[%d]", counter++);
		AddItem(List, Counter,buf, data);
		data++;
	}
}
void ChunkTableClass::List_W3D_CHUNK_SURRENDER_NORMALS(ChunkItem *Item, CListCtrl *List) {
	W3dVectorStruct *data;
	data = (W3dVectorStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {
		sprintf(buf, "SRNormal[%d]", counter++);
		AddItem(List, Counter,buf, data);
		data++;
	}
}
void ChunkTableClass::List_W3D_CHUNK_TEXCOORDS(ChunkItem *Item, CListCtrl *List) {
	
	W3dTexCoordStruct *data;
	data = (W3dTexCoordStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {
		sprintf(buf, "TexCoord[%d]", counter++);
		AddItem(List, Counter, buf, data);
		data++;
	}
}
void ChunkTableClass::List_O_W3D_CHUNK_MATERIALS(ChunkItem *Item, CListCtrl *List) {

	struct W3dMaterialStruct *data;
	data = (W3dMaterialStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {

		sprintf(buf, "Material[%d].MaterialName", counter);
		AddItem(List, Counter,buf, data->MaterialName);

		sprintf(buf, "Material[%d].PrimaryName", counter);
		AddItem(List, Counter,buf, data->PrimaryName);

		sprintf(buf, "Material[%d].SecondaryName", counter);
		AddItem(List, Counter,buf, data->SecondaryName);

		sprintf(buf, "Material[%d].RenderFlags", counter);
		AddItem(List, Counter,buf, data->RenderFlags);

		sprintf(buf, "Material[%d].Red", counter);
		AddItem(List, Counter,buf, data->Red);

		sprintf(buf, "Material[%d].Green", counter);
		AddItem(List, Counter,buf, data->Green);

		sprintf(buf, "Material[%d].Blue", counter);
		AddItem(List, Counter,buf, data->Blue);

		counter++;
		data++;
	}

}


void ChunkTableClass::List_O_W3D_CHUNK_TRIANGLES(ChunkItem *Item, CListCtrl *List) {

	int Counter = 0;
	AddItem(List, Counter, "Obsolete structure", "");
}
void ChunkTableClass::List_O_W3D_CHUNK_QUADRANGLES(ChunkItem *Item, CListCtrl *List) {
		int Counter = 0;
		AddItem(List, Counter, "Outdated structure", "");
}
void ChunkTableClass::List_O_W3D_CHUNK_SURRENDER_TRIANGLES(ChunkItem *Item, CListCtrl *List) {
	struct W3dSurrenderTriStruct *data;
	data = (W3dSurrenderTriStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {

		sprintf(buf, "Triangle[%d].Attributes", counter);
		AddItem(List, Counter,buf, data->Attributes);

		sprintf(buf, "Triangle[%d].Gouraud", counter);
		AddItem(List, Counter,buf, data->Gouraud, 3);

		sprintf(buf, "Triangle[%d].VertexIndices", counter);
		AddItem(List, Counter,buf, data->Vindex, 3);

		sprintf(buf, "Triangle[%d].MaterialIdx", counter);
		AddItem(List, Counter,buf, data->MaterialIdx);

		sprintf(buf, "Triangle[%d].Normal", counter);
		AddItem(List, Counter,buf, &data->Normal);

		sprintf(buf, "Triangle[%d].TexCoord", counter);
		AddItem(List, Counter,buf, data->TexCoord, 3);

		counter++;
		data++;
	}

}
void ChunkTableClass::List_O_W3D_CHUNK_POV_TRIANGLES(ChunkItem *Item, CListCtrl *List) {
	int Counter = 0;
	AddItem(List, Counter,"Contact Greg if you need to look at this!", "unsupported");
}
void ChunkTableClass::List_O_W3D_CHUNK_POV_QUADRANGLES(ChunkItem *Item, CListCtrl *List) {
	int Counter = 0;
	AddItem(List, Counter,"Contact Greg if you need to look at this!", "unsupported");
}
void ChunkTableClass::List_W3D_CHUNK_MESH_USER_TEXT(ChunkItem *Item, CListCtrl *List) {
	int Counter = 0;
	AddItem(List, Counter,"UserText", (char *) Item->Data);
}
void ChunkTableClass::List_W3D_CHUNK_VERTEX_COLORS(ChunkItem *Item, CListCtrl *List) {

	struct W3dRGBStruct *data;
	data = (W3dRGBStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];

	int sz = sizeof(W3dRGBStruct);

	while(data < max) {
	
		sprintf(buf, "Vertex[%d].RGB", counter);
		AddItem(List, Counter,buf, data);

		counter++;
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_VERTEX_INFLUENCES(ChunkItem *Item, CListCtrl *List) {

	struct W3dVertInfStruct *data;
	data = (W3dVertInfStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {
	
		sprintf(buf, "VertexInfluence[%d].BoneIdx", counter);
		AddItem(List, Counter,buf, data->BoneIdx);
		sprintf(buf, "VertexInfluence[%d].Pad", counter);
		AddItem(List, Counter,buf, data->Pad, 6);

		counter++;
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_DAMAGE(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}
void ChunkTableClass::List_W3D_CHUNK_DAMAGE_HEADER(ChunkItem *Item, CListCtrl *List) {

	struct W3dMeshDamageStruct *data;
	data = (W3dMeshDamageStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {
	
		sprintf(buf, "DamageStruct[%d].NumDamageMaterials", counter);
		AddItem(List, Counter,buf, data->NumDamageMaterials);
		sprintf(buf, "DamageStruct[%d].NumDamageVerts", counter);
		AddItem(List, Counter,buf, data->NumDamageVerts);
		sprintf(buf, "DamageStruct[%d].NumDamageColors", counter);
		AddItem(List, Counter,buf, data->NumDamageColors);
		sprintf(buf, "DamageStruct[%d].DamageIndex", counter);
		AddItem(List, Counter,buf, data->DamageIndex);

		counter++;
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_DAMAGE_VERTICES(ChunkItem *Item, CListCtrl *List) {

	struct W3dMeshDamageVertexStruct *data;
	data = (W3dMeshDamageVertexStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {
	
		sprintf(buf, "DamageVertexStruct[%d].VertexIndex", counter);
		AddItem(List, Counter,buf, data->VertexIndex);

		sprintf(buf, "DamageVertexStruct[%d].NewVertex", counter);
		AddItem(List, Counter,buf, data->VertexIndex);

		counter++;
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_DAMAGE_COLORS(ChunkItem *Item, CListCtrl *List) {

	struct W3dMeshDamageColorStruct *data;
	data = (W3dMeshDamageColorStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {
	
		sprintf(buf, "DamageColorStruct[%d].VertexIndex", counter);
		AddItem(List, Counter,buf, data->VertexIndex);

		sprintf(buf, "DamageColorStruct[%d].NewColor", counter);
		AddItem(List, Counter,buf, &data->NewColor);

		counter++;
		data++;
	}
}

void ChunkTableClass::List_O_W3D_CHUNK_MATERIALS2(ChunkItem *Item, CListCtrl *List) {
	struct W3dMaterial2Struct *data;
	data = (W3dMaterial2Struct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {

		sprintf(buf, "Material[%d].MaterialName", counter);
		AddItem(List, Counter,buf, data->MaterialName);

		sprintf(buf, "Material[%d].PrimaryName", counter);
		AddItem(List, Counter,buf, data->PrimaryName);

		sprintf(buf, "Material[%d].SecondaryName", counter);
		AddItem(List, Counter,buf, data->SecondaryName);

		sprintf(buf, "Material[%d].RenderFlags", counter);
		AddItem(List, Counter,buf, data->RenderFlags);

		sprintf(buf, "Material[%d].Red", counter);
		AddItem(List, Counter,buf, data->Red);

		sprintf(buf, "Material[%d].Green", counter);
		AddItem(List, Counter,buf, data->Green);

		sprintf(buf, "Material[%d].Blue", counter);
		AddItem(List, Counter,buf, data->Blue);
	
		sprintf(buf, "Material[%d].Alpha", counter);
		AddItem(List, Counter,buf, data->Alpha);

		sprintf(buf, "Material[%d].PrimaryNumFrames", counter);
		AddItem(List, Counter,buf, data->PrimaryNumFrames);

		sprintf(buf, "Material[%d].SecondaryNumFrames", counter);
		AddItem(List, Counter,buf, data->SecondaryNumFrames);

		counter++;
		data++;
	}

}

void ChunkTableClass::List_W3D_CHUNK_MATERIALS3(ChunkItem *Item, CListCtrl *List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_MATERIAL3(ChunkItem *Item, CListCtrl *List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_MATERIAL3_NAME(ChunkItem *Item, CListCtrl *List)
{
	int Counter = 0;
	char * data = (char *)Item->Data;
	AddItem(List, Counter,"Material Name:", data);
}

void ChunkTableClass::List_W3D_CHUNK_MATERIAL3_INFO(ChunkItem *Item, CListCtrl *List)
{
	int Counter = 0;
	struct W3dMaterial3Struct *data;
	data = (W3dMaterial3Struct *) Item->Data;
	AddItem(List,Counter,"Attributes",data->Attributes);
	if (data->Attributes & W3DMATERIAL_USE_ALPHA) AddItem(List,Counter,"Attributes","W3DMATERIAL_USE_ALPHA");
	if (data->Attributes & W3DMATERIAL_USE_SORTING) AddItem(List,Counter,"Attributes","W3DMATERIAL_USE_SORTING");
	if (data->Attributes & W3DMATERIAL_HINT_DIT_OVER_DCT) AddItem(List,Counter,"Attributes","W3DMATERIAL_HINT_DIT_OVER_DCT");
	if (data->Attributes & W3DMATERIAL_HINT_SIT_OVER_SCT) AddItem(List,Counter,"Attributes","W3DMATERIAL_HINT_SIT_OVER_SCT");
	if (data->Attributes & W3DMATERIAL_HINT_DIT_OVER_DIG) AddItem(List,Counter,"Attributes","W3DMATERIAL_HINT_DIT_OVER_DIG");
	if (data->Attributes & W3DMATERIAL_HINT_SIT_OVER_SIG) AddItem(List,Counter,"Attributes","W3DMATERIAL_HINT_SIT_OVER_SIG");
	if (data->Attributes & W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA) AddItem(List,Counter,"Attributes","W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA");
	if (data->Attributes & W3DMATERIAL_PSX_TRANS_100) AddItem(List,Counter,"Attributes","W3DMATERIAL_PSX_TRANS_100");
	if (data->Attributes & W3DMATERIAL_PSX_TRANS_50) AddItem(List,Counter,"Attributes","W3DMATERIAL_PSX_TRANS_50");
	if (data->Attributes & W3DMATERIAL_PSX_TRANS_25) AddItem(List,Counter,"Attributes","W3DMATERIAL_PSX_TRANS_25");
	if (data->Attributes & W3DMATERIAL_PSX_TRANS_MINUS_100) AddItem(List,Counter,"Attributes","W3DMATERIAL_PSX_TRANS_MINUS_100");
	if (data->Attributes & W3DMATERIAL_PSX_NO_RT_LIGHTING) AddItem(List,Counter,"Attributes","W3DMATERIAL_PSX_NO_RT_LIGHTING");
	AddItem(List, Counter,"Diffuse Color", &data->DiffuseColor);
	AddItem(List, Counter,"Specular Color", &data->SpecularColor);
	AddItem(List, Counter,"Emissive Coefficients", &data->EmissiveCoefficients);
	AddItem(List, Counter,"Ambient Coefficients", &data->AmbientCoefficients);
	AddItem(List, Counter,"Diffuse Coefficients", &data->DiffuseCoefficients);
	AddItem(List, Counter,"Specular Coefficients", &data->SpecularCoefficients);
	AddItem(List, Counter,"Shininess", data->Shininess);
	AddItem(List, Counter,"Opacity", data->Opacity);
	AddItem(List, Counter,"Translucency", data->Translucency);
	AddItem(List, Counter,"Fog Coefficient", data->FogCoeff);
}

void ChunkTableClass::List_W3D_CHUNK_MAP3_FILENAME(ChunkItem * Item,CListCtrl * List)
{
	int Counter = 0;
	char * data = (char *)Item->Data;
	AddItem(List, Counter, "Texture Filename:", data);
}

void ChunkTableClass::List_W3D_CHUNK_MAP3_INFO(ChunkItem * Item,CListCtrl * List)
{
	int Counter = 0;
	struct W3dMap3Struct * data;
	data = (W3dMap3Struct *)Item->Data;

	AddItem(List, Counter, "Mapping Type",data->MappingType);
	AddItem(List, Counter, "Frame Count",data->FrameCount);
	AddItem(List, Counter, "Frame Rate",data->FrameRate);
}

void ChunkTableClass::List_W3D_CHUNK_MATERIAL3_DC_MAP(ChunkItem *Item, CListCtrl *List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_MATERIAL3_DI_MAP(ChunkItem *Item, CListCtrl *List)
{
	List_Subitems(Item,List);
}

void ChunkTableClass::List_W3D_CHUNK_MATERIAL3_SC_MAP(ChunkItem *Item, CListCtrl *List)
{
	List_Subitems(Item,List);
}

void ChunkTableClass::List_W3D_CHUNK_MATERIAL3_SI_MAP(ChunkItem *Item, CListCtrl *List)
{
	List_Subitems(Item,List);
}

void ChunkTableClass::List_W3D_CHUNK_MESH_HEADER3(ChunkItem *Item, CListCtrl *List) {

	struct W3dMeshHeader3Struct *data;
	data = (W3dMeshHeader3Struct *) Item->Data;
	int Counter = 0;

	char buf[64];
	sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(data->Version),W3D_GET_MINOR_VERSION(data->Version));
	AddItem(List, Counter,"Version", buf);
	AddItem(List, Counter,"MeshName", data->MeshName);
	AddItem(List, Counter,"ContainerName", data->ContainerName);
	AddItem(List, Counter,"Attributes",data->Attributes);
	switch(data->Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK)
	{
		case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
			AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL_MESH","flag");
			break;
		case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
			AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED","flag");
			break;
		case W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN:
			AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN","flag");
			break;
		case W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX:
			AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX","flag");
			break;
		case W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX:
			AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX","flag");
			break;
	};
	
	if (data->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL","flag");
	if (data->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE","flag");
	if (data->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_VIS)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_VIS","flag");
	if (data->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_CAMERA)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_CAMERA","flag");
	if (data->Attributes & W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE)	AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE","flag");

	if (data->Attributes & W3D_MESH_FLAG_HIDDEN) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_HIDDEN","flag");
	if (data->Attributes & W3D_MESH_FLAG_TWO_SIDED) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_TWO_SIDED","flag");
	if (data->Attributes & W3D_MESH_FLAG_CAST_SHADOW) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_CAST_SHADOW");
	if (data->Attributes & W3D_MESH_FLAG_SHATTERABLE) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_SHATTERABLE");
	if (data->Attributes & W3D_MESH_FLAG_NPATCHABLE) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_NPATCHABLE");

	if (data->Attributes & W3D_MESH_FLAG_PRELIT_UNLIT) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_PRELIT_UNLIT","flag");
	if (data->Attributes & W3D_MESH_FLAG_PRELIT_VERTEX) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_PRELIT_VERTEX","flag");
	if (data->Attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS","flag");
	if (data->Attributes & W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE) AddItem(List, Counter,"Attributes", "W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE","flag");

	AddItem(List, Counter,"NumTris", data->NumTris);
	AddItem(List, Counter,"NumVertices", data->NumVertices);
	AddItem(List, Counter,"NumMaterials", data->NumMaterials);
	AddItem(List, Counter,"NumDamageStages", data->NumDamageStages);
	
	if (data->SortLevel == SORT_LEVEL_NONE) {
		AddItem(List, Counter, "SortLevel", "NONE");
	} else {
		AddItem(List, Counter, "SortLevel", (uint8)data->SortLevel);
	}

	if ((data->Attributes & W3D_MESH_FLAG_PRELIT_MASK) != 0x0) {
		if (data->PrelitVersion != 0) {
			sprintf (buf, "%d.%d", W3D_GET_MAJOR_VERSION (data->PrelitVersion), W3D_GET_MINOR_VERSION (data->PrelitVersion));
		} else {
			sprintf (buf, "UNKNOWN");
		}
	} else {	  
		sprintf (buf, "N/A");
	}
	AddItem (List, Counter, "PrelitVersion", buf);

	AddItem(List, Counter, "FutureCounts", data->FutureCounts, 1);
	AddItem(List, Counter, "VertexChannels", data->VertexChannels);
	if (data->VertexChannels & W3D_VERTEX_CHANNEL_LOCATION) AddItem(List,Counter,"VertexChannels","W3D_VERTEX_CHANNEL_LOCATION","flag");
	if (data->VertexChannels & W3D_VERTEX_CHANNEL_NORMAL) AddItem(List,Counter,"VertexChannels","W3D_VERTEX_CHANNEL_NORMAL","flag");
	if (data->VertexChannels & W3D_VERTEX_CHANNEL_TEXCOORD) AddItem(List,Counter,"VertexChannels","W3D_VERTEX_CHANNEL_TEXCOORD","flag");
	if (data->VertexChannels & W3D_VERTEX_CHANNEL_COLOR) AddItem(List,Counter,"VertexChannels","W3D_VERTEX_CHANNEL_COLOR","flag");
	if (data->VertexChannels & W3D_VERTEX_CHANNEL_BONEID) AddItem(List,Counter,"VertexChannels","W3D_VERTEX_CHANNEL_BONEID","flag");
	AddItem(List, Counter,"FaceChannels", data->FaceChannels);
	if (data->FaceChannels & W3D_FACE_CHANNEL_FACE) AddItem(List,Counter,"FaceChannels","W3D_FACE_CHANNEL_FACE","flag");
	AddItem(List, Counter,"Min", &data->Min);
	AddItem(List, Counter,"Max", &data->Max);
	AddItem(List, Counter,"SphCenter", &data->SphCenter);
	AddItem(List, Counter,"SphRadius", data->SphRadius);
}

void ChunkTableClass::List_W3D_CHUNK_TRIANGLES(ChunkItem *Item, CListCtrl *List) {
	struct W3dTriStruct *data;
	data = (W3dTriStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {

		sprintf(buf, "Triangle[%d].VertexIndices", counter);
		AddItem(List, Counter,buf, data->Vindex, 3);

		sprintf(buf, "Triangle[%d].Attributes", counter);
		AddItem(List, Counter,buf, data->Attributes);

		sprintf(buf, "Triangle[%d].Normal", counter);
		AddItem(List, Counter,buf, &data->Normal);

		sprintf(buf, "Triangle[%d].Dist", counter);
		AddItem(List, Counter,buf, data->Dist);

		counter++;
		data++;
	}

}

void ChunkTableClass::List_W3D_CHUNK_PER_TRI_MATERIALS(ChunkItem * Item,CListCtrl *List)
{
	unsigned short *data;
	data = (unsigned short *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];

	while(data < max) {
		sprintf(buf, "Triangle[%d].MaterialIdx", counter);
		AddItem(List, Counter,buf, *data);
		counter++;
		data++;
	}
}


void	ChunkTableClass::List_W3D_CHUNK_VERTEX_SHADE_INDICES(ChunkItem * Item,CListCtrl *List)
{
	uint32 * data = (uint32 *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	
	char buf[256];
	while(data < max) {
		sprintf(buf, "Index[%d]", counter);
		AddItem(List, Counter, buf, *data);
		counter++;
		data++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_MATERIAL_INFO(ChunkItem * Item,CListCtrl *List)
{
	W3dMaterialInfoStruct * matinfo = (W3dMaterialInfoStruct *)Item->Data;
	int counter = 0;

	AddItem(List, counter,"PassCount", matinfo->PassCount);
	AddItem(List, counter,"VertexMaterialCount", matinfo->VertexMaterialCount);
	AddItem(List, counter,"ShaderCount", matinfo->ShaderCount);
	AddItem(List, counter,"TextureCount",matinfo->TextureCount);
}

void ChunkTableClass::List_W3D_CHUNK_VERTEX_MATERIALS(ChunkItem *Item,CListCtrl *List)
{
	List_Subitems(Item,List);
}

void ChunkTableClass::List_W3D_CHUNK_VERTEX_MATERIAL(ChunkItem *Item,CListCtrl *List)
{
	List_Subitems(Item,List);
}

void ChunkTableClass::List_W3D_CHUNK_VERTEX_MATERIAL_NAME(ChunkItem * Item,CListCtrl *List)
{
	int Counter = 0;
	char * data = (char *)Item->Data;
	AddItem(List, Counter,"Material Name:", data);
}

void	ChunkTableClass::List_W3D_CHUNK_VERTEX_MATERIAL_INFO(ChunkItem * Item,CListCtrl *List)
{
	struct W3dVertexMaterialStruct *data = (W3dVertexMaterialStruct *)Item->Data;
	int Counter = 0;

	if (data->Attributes & W3DVERTMAT_USE_DEPTH_CUE)				AddItem (List, Counter, "Material.Attributes", "W3DVERTMAT_USE_DEPTH_CUE", "flag");
	if (data->Attributes & W3DVERTMAT_ARGB_EMISSIVE_ONLY)			AddItem (List, Counter, "Material.Attributes", "W3DVERTMAT_ARGB_EMISSIVE_ONLY", "flag");
	if (data->Attributes & W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE)	AddItem (List, Counter, "Material.Attributes", "W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE", "flag");
	if (data->Attributes & W3DVERTMAT_DEPTH_CUE_TO_ALPHA)			AddItem (List, Counter, "Material.Attributes", "W3DVERTMAT_DEPTH_CUE_TO_ALPHA", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_UV)						AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_UV", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT)	AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_SCREEN)					AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SCREEN", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET)		AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE", "flag");

	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_SCALE)							AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SCALE", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_GRID)							AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_ROTATE)							AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ROTATE", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET)		AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV)				AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT)				AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_RANDOM)							AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_RANDOM", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK) == W3DVERTMAT_STAGE0_MAPPING_BUMPENV)						AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE0_MAPPING_BUMPENV", "flag");

	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_UV)						AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_UV", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT)	AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_SCREEN)					AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SCREEN", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET)		AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE", "flag");

	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_SCALE)							AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SCALE", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_GRID)							AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_ROTATE)							AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ROTATE", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET)		AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV)				AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT)				AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_RANDOM)							AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_RANDOM", "flag");
	if ((data->Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK) == W3DVERTMAT_STAGE1_MAPPING_BUMPENV)						AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_STAGE1_MAPPING_BUMPENV", "flag");

	if (data->Attributes & W3DVERTMAT_PSX_MASK) {
		if (data->Attributes & W3DVERTMAT_PSX_NO_RT_LIGHTING) {
			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_PSX_NO_RT_LIGHTING", "flag");
		}
		else {
			if ((data->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_NONE)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_NONE", "flag");
			if ((data->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_100)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_100", "flag");
			if ((data->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_50)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_50", "flag");
			if ((data->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_25)			AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_25", "flag");
			if ((data->Attributes & W3DVERTMAT_PSX_TRANS_MASK) == W3DVERTMAT_PSX_TRANS_MINUS_100)	AddItem(List, Counter, "Material.Attributes", "W3DVERTMAT_PSX_TRANS_MINUS_100", "flag");
		}
	}
 
	AddItem(List, Counter, "Material.Attributes", data->Attributes);
	AddItem(List, Counter, "Material.Ambient", &(data->Ambient));
	AddItem(List, Counter, "Material.Diffuse", &(data->Diffuse));
	AddItem(List, Counter, "Material.Specular", &(data->Specular));
	AddItem(List, Counter, "Material.Emissive", &(data->Emissive));
	AddItem(List, Counter, "Material.Shininess", data->Shininess);
	AddItem(List, Counter, "Material.Opacity", data->Opacity);
	AddItem(List, Counter, "Material.Translucency", data->Translucency);

}

void ChunkTableClass::List_W3D_CHUNK_VERTEX_MAPPER_ARGS0(ChunkItem *Item, CListCtrl *List)
{
	int Counter = 0;
	char * data = (char *)Item->Data;
	AddItem(List, Counter, "Stage0 Mapper Args:", data);
}

void ChunkTableClass::List_W3D_CHUNK_VERTEX_MAPPER_ARGS1(ChunkItem *Item, CListCtrl *List)
{
	int Counter = 0;
	char * data = (char *)Item->Data;
	AddItem(List, Counter, "Stage1 Mapper Args:", data);
}

void	ChunkTableClass::List_W3D_CHUNK_SHADERS(ChunkItem * Item,CListCtrl *List)
{
	struct W3dShaderStruct *shader = (W3dShaderStruct *)Item->Data;
	
	void *max = (char *)Item->Data + Item->Length;
	int counter = 0;
	char label[256];

	while(shader < max) {
		sprintf(label,"shader[%d]",counter);
		int counter2 = 0;
		AddItem(List,counter2,label,shader);
		counter++;
		shader++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_PS2_SHADERS(ChunkItem * Item,CListCtrl *List)
{
	struct W3dPS2ShaderStruct *shader = (W3dPS2ShaderStruct *)Item->Data;
	
	void *max = (char *)Item->Data + Item->Length;
	int counter = 0;
	char label[256];

	while(shader < max) {
		sprintf(label,"shader[%d]",counter);
		int counter2 = 0;
		AddItem(List,counter2,label,shader);
		counter++;
		shader++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_TEXTURES(ChunkItem * Item,CListCtrl *List)
{
	List_Subitems(Item,List);
}

void	ChunkTableClass::List_W3D_CHUNK_TEXTURE(ChunkItem * Item,CListCtrl *List)
{
	List_Subitems(Item,List);
}

void	ChunkTableClass::List_W3D_CHUNK_TEXTURE_NAME(ChunkItem * Item,CListCtrl *List)
{
	int Counter = 0;
	char * data = (char *)Item->Data;
	AddItem(List, Counter,"Texture Name:", data);
}

void	ChunkTableClass::List_W3D_CHUNK_TEXTURE_INFO(ChunkItem * Item,CListCtrl *List)
{
	struct W3dTextureInfoStruct *data = (W3dTextureInfoStruct *)Item->Data;
	int Counter = 0;

	AddItem(List, Counter, "Texture.Attributes", data->Attributes);
	
	if (data->Attributes & W3DTEXTURE_PUBLISH)		AddItem(List, Counter,"Attributes", "W3DTEXTURE_PUBLISH","flag");
	if (data->Attributes & W3DTEXTURE_NO_LOD)			AddItem(List, Counter,"Attributes", "W3DTEXTURE_NO_LOD","flag");
	if (data->Attributes & W3DTEXTURE_CLAMP_U)		AddItem(List, Counter,"Attributes", "W3DTEXTURE_CLAMP_U","flag");
	if (data->Attributes & W3DTEXTURE_CLAMP_V)		AddItem(List, Counter,"Attributes", "W3DTEXTURE_CLAMP_V","flag");
	if (data->Attributes & W3DTEXTURE_ALPHA_BITMAP)	AddItem(List, Counter,"Attributes", "W3DTEXTURE_ALPHA_BITMAP","flag");

	AddItem(List, Counter, "Texture.FrameCount", data->FrameCount);
	AddItem(List, Counter, "Texture.FrameRate", data->FrameRate);
}

void	ChunkTableClass::List_W3D_CHUNK_MATERIAL_PASS(ChunkItem * Item,CListCtrl *List)
{
	List_Subitems(Item,List);
}

void	ChunkTableClass::List_W3D_CHUNK_VERTEX_MATERIAL_IDS(ChunkItem * Item,CListCtrl *List)
{
	int Counter = 0;
	int counter = 0;
	uint32 *data = (uint32 *)Item->Data;
	void *max = (char *) Item->Data + Item->Length;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Vertex[%d] Vertex Material Index", counter);
		AddItem(List, Counter,buf, *data);

		counter++;
		data++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_SHADER_IDS(ChunkItem * Item,CListCtrl *List)
{
	int Counter = 0;
	int counter = 0;
	uint32 *data = (uint32 *)Item->Data;
	void *max = (char *) Item->Data + Item->Length;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Face[%d] Shader Index", counter);
		AddItem(List, Counter, buf, *data);

		counter++;
		data++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_DCG(ChunkItem * Item,CListCtrl *List)
{
	W3dRGBAStruct *data = (W3dRGBAStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Vertex[%d].DCG", counter);
		AddItem(List, Counter,buf, data);

		counter++;
		data++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_DIG(ChunkItem * Item,CListCtrl *List)
{
	W3dRGBStruct *data = (W3dRGBStruct *)Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Vertex[%d].DIG", counter);
		AddItem(List, Counter,buf, data);

		counter++;
		data++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_SCG(ChunkItem * Item,CListCtrl *List)
{
	W3dRGBStruct *data = (W3dRGBStruct *)Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Vertex[%d].SCG", counter);
		AddItem(List, Counter,buf, data);

		counter++;
		data++;
	}
}
		
void	ChunkTableClass::List_W3D_CHUNK_TEXTURE_STAGE(ChunkItem * Item,CListCtrl *List)
{
	List_Subitems(Item,List);
}

void	ChunkTableClass::List_W3D_CHUNK_TEXTURE_IDS(ChunkItem * Item,CListCtrl *List)
{	
	int Counter = 0;
	int counter = 0;
	uint32 *data = (uint32 *)Item->Data;
	void *max = (char *) Item->Data + Item->Length;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Face[%d] Texture Index", counter);
		AddItem(List, Counter, buf, *data);

		counter++;
		data++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_STAGE_TEXCOORDS(ChunkItem * Item,CListCtrl *List)
{
	W3dTexCoordStruct *data = (W3dTexCoordStruct *)Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Vertex[%d].UV", counter);
		AddItem(List, Counter, buf, data);

		counter++;
		data++;
	}
}

void	ChunkTableClass::List_W3D_CHUNK_PER_FACE_TEXCOORD_IDS(ChunkItem * Item,CListCtrl *List)
{
	int Counter = 0;
	int counter = 0;
	Vector3i *data = (Vector3i *)Item->Data;
	void *max = (char *) Item->Data + Item->Length;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Face[%d] UV Indices", counter);
		AddItem(List, Counter, buf, data);

		counter++;
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_AABTREE(ChunkItem * Item,CListCtrl *List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_AABTREE_HEADER(ChunkItem * Item,CListCtrl *List)
{
	W3dMeshAABTreeHeader * data = (W3dMeshAABTreeHeader*)Item->Data;
	int Counter = 0;
	AddItem(List, Counter, "NodeCount", data->NodeCount);
	AddItem(List, Counter, "PolyCount", data->PolyCount);
}

void ChunkTableClass::List_W3D_CHUNK_AABTREE_POLYINDICES(ChunkItem * Item,CListCtrl *List)
{
	int Counter = 0;
	int counter = 0;
	uint32 *data = (uint32 *)Item->Data;
	void *max = (char *) Item->Data + Item->Length;
	char buf[256];

	while(data < max) {
	
		sprintf(buf, "Polygon Index[%d]", counter);
		AddItem(List, Counter, buf, *data);

		counter++;
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_AABTREE_NODES(ChunkItem * Item,CListCtrl *List)
{
	W3dMeshAABTreeNode * data = (W3dMeshAABTreeNode *)Item->Data;
	
	int Counter = 0;
	int index = 0;
	void *max = (char *)Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	
	while(data < max) {

		sprintf(buf, "Node[%d].Min", counter);
		AddItem(List, Counter,buf, &data->Min);

		sprintf(buf, "Node[%d].Max", counter);
		AddItem(List, Counter,buf, &data->Max);

		if (data->FrontOrPoly0 & 0x80000000) {
			sprintf(buf, "Node[%d].Poly0",counter);
			AddItem(List, Counter, buf, data->FrontOrPoly0 & 0x7FFFFFFF);
			sprintf(buf, "Node[%d].PolyCount",counter);
			AddItem(List, Counter, buf, data->BackOrPolyCount);
		} else {
			sprintf(buf, "Node[%d].Front",counter);
			AddItem(List, Counter, buf, data->FrontOrPoly0);
			sprintf(buf, "Node[%d].Back",counter);
			AddItem(List, Counter, buf, data->BackOrPolyCount);
		}

		counter++;
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_HIERARCHY(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_HIERARCHY_HEADER(ChunkItem *Item, CListCtrl *List) {
	struct W3dHierarchyStruct *data;
	data = (W3dHierarchyStruct *) Item->Data;

	int Counter = 0;
	char buf[64];
	sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(data->Version),W3D_GET_MINOR_VERSION(data->Version));
	AddItem(List, Counter,"Version", buf);
	AddItem(List, Counter, "Name", data->Name);
	AddItem(List, Counter, "NumPivots", data->NumPivots);
	AddItem(List, Counter, "Center", &data->Center);
}
void ChunkTableClass::List_W3D_CHUNK_PIVOTS(ChunkItem *Item, CListCtrl *List) {
	struct W3dPivotStruct *data;
	data = (W3dPivotStruct *) Item->Data;
	int Counter = 0;
	int index = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];
	while(data < max) {

		sprintf(buf, "Pivot[%d].Name", counter);
		AddItem(List, Counter,buf, data->Name);

		sprintf(buf, "Pivot[%d].ParentIdx", counter);
		AddItem(List, Counter,buf, data->ParentIdx);

		sprintf(buf, "Pivot[%d].Translation", counter);
		AddItem(List, Counter,buf, &data->Translation);

		sprintf(buf, "Pivot[%d].EulerAngles", counter);
		AddItem(List, Counter,buf, &data->EulerAngles);

		sprintf(buf, "Pivot[%d].Rotation", counter);
		AddItem(List, Counter,buf, &data->Rotation);

		counter++;
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_PIVOT_FIXUPS(ChunkItem *Item, CListCtrl *List) {
	W3dPivotFixupStruct *data = (W3dPivotFixupStruct *) Item->Data;
	
	int Counter = 0;
	int pivot_counter = 0;

	while ((char*)data < (char*)Item->Data + Item->Length) {
		char tmp[256];
		for (int i=0;i<4;i++) {
			sprintf(tmp,"Transform %d, Row[%d]", pivot_counter,i);
			AddItem(List, Counter, tmp, data->TM[i], 3);
		}
		data++;
		pivot_counter++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_ANIMATION(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}
void ChunkTableClass::List_W3D_CHUNK_ANIMATION_HEADER(ChunkItem *Item, CListCtrl *List) {

	W3dAnimHeaderStruct *data = (W3dAnimHeaderStruct *) Item->Data;
	
	int Counter = 0;
	char buf[64];
	sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(data->Version),W3D_GET_MINOR_VERSION(data->Version));
	AddItem(List, Counter,"Version", buf);
	AddItem(List, Counter, "Name", data->Name);
	AddItem(List, Counter, "HierarchyName", data->HierarchyName);
	AddItem(List, Counter, "NumFrames", data->NumFrames);
	AddItem(List, Counter, "FrameRate", data->FrameRate);
}

void ChunkTableClass::List_W3D_CHUNK_ANIMATION_CHANNEL(ChunkItem *Item, CListCtrl *List) 
{
	static const char * _chntypes[] = {
		"X Translation",
		"Y Translation",
		"Z Translation",
		"X Rotation",
		"Y Rotation",
		"Z Rotation",
		"Quaternion"
	};


	W3dAnimChannelStruct *data = (W3dAnimChannelStruct *) Item->Data;
	int Counter = 0;
	AddItem(List, Counter, "FirstFrame", data->FirstFrame);
	AddItem(List, Counter, "LastFrame", data->LastFrame);

	if ((data->Flags >= ANIM_CHANNEL_X)&&(data->Flags <= ANIM_CHANNEL_Q)) {
		AddItem(List, Counter, "ChannelType",_chntypes[data->Flags]);
	} else {
		AddItem(List, Counter, "ChannelType",data->Flags);
	}

	AddItem(List, Counter, "Pivot", data->Pivot);
	AddItem(List, Counter, "VectorLen", data->VectorLen);

	CString name;
	for (int frameidx=0; frameidx <= data->LastFrame - data->FirstFrame; frameidx++) {
		for (int vidx = 0; vidx < data->VectorLen; vidx++) {
			name.Format("Data[%d][%d]", frameidx, vidx);
			AddItem(List, Counter, name, data->Data[frameidx * data->VectorLen + vidx]);
		}
	}
}

void ChunkTableClass::List_W3D_CHUNK_BIT_CHANNEL(ChunkItem *Item, CListCtrl *List) 
{
	static const char * _chntypes[] = 
	{
		"Visibility",
	};


	W3dBitChannelStruct *data = (W3dBitChannelStruct *) Item->Data;
	int Counter = 0;
	unsigned char * bits = &(data->Data[0]);

	AddItem(List, Counter, "FirstFrame", data->FirstFrame);
	AddItem(List, Counter, "LastFrame", data->LastFrame);

	if ((data->Flags >= BIT_CHANNEL_VIS)&&(data->Flags <= BIT_CHANNEL_VIS)) {
		AddItem(List, Counter, "ChannelType",_chntypes[data->Flags]);
	} else {
		AddItem(List, Counter, "ChannelType",data->Flags);
	}

	AddItem(List, Counter, "Pivot", data->Pivot);
	AddItem(List, Counter, "Default Value", data->DefaultVal);

	CString name;
	for (int frameidx=0; frameidx <= data->LastFrame - data->FirstFrame; frameidx++) {
		name.Format("Data[%d]",frameidx + data->FirstFrame);
		AddItem(List, Counter, name, (uint8)Get_Bit(bits,frameidx));
	}
}

void ChunkTableClass::List_W3D_CHUNK_HMODEL(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}
void ChunkTableClass::List_W3D_CHUNK_HMODEL_HEADER(ChunkItem *Item, CListCtrl *List) {
	W3dHModelHeaderStruct *data = (W3dHModelHeaderStruct *) Item->Data;
	
	int Counter = 0;
	char buf[64];
	sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(data->Version),W3D_GET_MINOR_VERSION(data->Version));
	AddItem(List, Counter,"Version", buf);
	AddItem(List, Counter, "Name", data->Name);
	AddItem(List, Counter, "HierarchyName", data->HierarchyName);
	AddItem(List, Counter, "NumConnections", data->NumConnections);
}
void ChunkTableClass::List_W3D_CHUNK_NODE(ChunkItem *Item, CListCtrl *List) {
	W3dHModelNodeStruct *data = (W3dHModelNodeStruct *) Item->Data;
	
	int Counter = 0;
	AddItem(List, Counter, "RenderObjName", data->RenderObjName);
	AddItem(List, Counter, "PivotIdx", data->PivotIdx);
}

void ChunkTableClass::List_W3D_CHUNK_COLLISION_NODE(ChunkItem *Item, CListCtrl *List) {
	W3dHModelNodeStruct *data = (W3dHModelNodeStruct *) Item->Data;
	
	int Counter = 0;
	AddItem(List, Counter, "CollisionMeshName", data->RenderObjName);
	AddItem(List, Counter, "PivotIdx", data->PivotIdx);
}

void ChunkTableClass::List_W3D_CHUNK_SKIN_NODE(ChunkItem *Item, CListCtrl *List) {
	W3dHModelNodeStruct *data = (W3dHModelNodeStruct *) Item->Data;
	
	int Counter = 0;
	AddItem(List, Counter, "SkinMeshName", data->RenderObjName);
	AddItem(List, Counter, "PivotIdx", data->PivotIdx);
}

void ChunkTableClass::List_W3D_CHUNK_HMODEL_AUX_DATA(ChunkItem *Item, CListCtrl *List) {

	W3dHModelAuxDataStruct *data = (W3dHModelAuxDataStruct *) Item->Data;
	
	int Counter = 0;
	AddItem(List, Counter, "Attributes", data->Attributes);
	AddItem(List, Counter, "MeshCount", data->MeshCount);
	AddItem(List, Counter, "CollisionCount", data->CollisionCount);
	AddItem(List, Counter, "SkinCount", data->SkinCount);
	AddItem(List, Counter, "FutureCounts", data->FutureCounts, 8);
	AddItem(List, Counter, "LODMin", data->LODMin);
	AddItem(List, Counter, "LODMax", data->LODMax);
	AddItem(List, Counter, "FutureUse", data->FutureUse, 32);
}

void ChunkTableClass::List_W3D_CHUNK_SHADOW_NODE(ChunkItem *Item, CListCtrl *List) {
	W3dHModelNodeStruct *data = (W3dHModelNodeStruct *) Item->Data;
	
	int Counter = 0;
	AddItem(List, Counter, "ShadowMeshName", data->RenderObjName);
	AddItem(List, Counter, "PivotIdx", data->PivotIdx);
}

void ChunkTableClass::List_W3D_CHUNK_LODMODEL(ChunkItem *Item, CListCtrl *List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_LODMODEL_HEADER(ChunkItem *Item, CListCtrl *List)
{
	W3dLODModelHeaderStruct *data = (W3dLODModelHeaderStruct *) Item->Data;
	int Counter = 0;

	char buf[64];
	sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(data->Version),W3D_GET_MINOR_VERSION(data->Version));
	AddItem(List, Counter,"Version", buf);
	AddItem(List, Counter, "Name", data->Name);
	AddItem(List, Counter, "NumLODs", data->NumLODs);
}

void ChunkTableClass::List_W3D_CHUNK_LOD(ChunkItem * Item,CListCtrl * List)
{
	W3dLODStruct *data = (W3dLODStruct *) Item->Data;
	int Counter = 0;

	AddItem(List, Counter, "Render Object Name", data->RenderObjName);
	AddItem(List, Counter, "LOD Min Distance", data->LODMin);
	AddItem(List, Counter, "LOD Max Distance", data->LODMax);
}

void ChunkTableClass::List_W3D_CHUNK_COLLECTION(ChunkItem * Item,CListCtrl * List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_COLLECTION_HEADER(ChunkItem * Item,CListCtrl * List)
{
	W3dCollectionHeaderStruct *data = (W3dCollectionHeaderStruct *) Item->Data;
	int Counter = 0;

	char buf[64];
	sprintf(buf,"%d.%d",W3D_GET_MAJOR_VERSION(data->Version),W3D_GET_MINOR_VERSION(data->Version));
	AddItem(List, Counter,"Version", buf);
	AddItem(List, Counter, "Name", data->Name);
	AddItem(List, Counter, "RenderObjectCount", data->RenderObjectCount);
}

void ChunkTableClass::List_W3D_CHUNK_COLLECTION_OBJ_NAME(ChunkItem * Item,CListCtrl * List)
{
	char * name = (char *)Item->Data;
	int Counter = 0;
	AddItem(List,Counter,"Render Object Name",name);	
}

void ChunkTableClass::List_W3D_CHUNK_PLACEHOLDER(ChunkItem * Item,CListCtrl * List)
{
	W3dPlaceholderStruct * data = (W3dPlaceholderStruct *)(Item->Data);
	int Counter = 0;

	AddItemVersion(List,Counter,data->version);
	AddItem(List,Counter,"Transform",&(data->transform[0][0]),3);
	AddItem(List,Counter,"Transform",&(data->transform[1][0]),3);
	AddItem(List,Counter,"Transform",&(data->transform[2][0]),3);
	AddItem(List,Counter,"Transform",&(data->transform[3][0]),3);
	AddItem(List,Counter,"Name",(char *)(data + 1));
}

void ChunkTableClass::List_W3D_CHUNK_TRANSFORM_NODE(ChunkItem * Item,CListCtrl * List)
{
	W3dTransformNodeStruct * data = (W3dTransformNodeStruct *)(Item->Data);
	int Counter = 0;

	AddItemVersion(List,Counter,data->version);
	AddItem(List,Counter,"Transform",&(data->transform[0][0]),3);
	AddItem(List,Counter,"Transform",&(data->transform[1][0]),3);
	AddItem(List,Counter,"Transform",&(data->transform[2][0]),3);
	AddItem(List,Counter,"Transform",&(data->transform[3][0]),3);
	AddItem(List,Counter,"Name",(char *)(data + 1));
}

void ChunkTableClass::List_W3D_CHUNK_POINTS(ChunkItem * Item,CListCtrl * List)
{
	W3dVectorStruct *data;
	data = (W3dVectorStruct *) Item->Data;

	int Counter = 0;
	void *max = (char *) Item->Data + Item->Length;
	int counter = 0;
	char buf[256];

	while (data < max) {
		sprintf(buf, "Point[%d]", counter++);
		AddItem(List, Counter,buf, data);
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_LIGHT(ChunkItem * Item,CListCtrl * List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_LIGHT_INFO(ChunkItem * Item,CListCtrl * List)
{
	W3dLightStruct * data = (W3dLightStruct *)Item->Data;
	int counter = 0;

	if ((data->Attributes & W3D_LIGHT_ATTRIBUTE_TYPE_MASK) == W3D_LIGHT_ATTRIBUTE_POINT) AddItem (List, counter, "Attributes", "W3D_LIGHT_ATTRIBUTE_POINT");
	if ((data->Attributes & W3D_LIGHT_ATTRIBUTE_TYPE_MASK) == W3D_LIGHT_ATTRIBUTE_SPOT) AddItem (List, counter, "Attributes", "W3D_LIGHT_ATTRIBUTE_SPOT");
	if ((data->Attributes & W3D_LIGHT_ATTRIBUTE_TYPE_MASK) == W3D_LIGHT_ATTRIBUTE_DIRECTIONAL) AddItem (List, counter, "Attributes", "W3D_LIGHT_ATTRIBUTE_DIRECTIONAL");
	if (data->Attributes & W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS)	AddItem (List, counter, "Attributes", "W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS", "flag");
	AddItem(List,counter,"Ambient",&(data->Ambient),1);
	AddItem(List,counter,"Diffuse",&(data->Diffuse),1);
	AddItem(List,counter,"Specular",&(data->Specular),1);
	AddItem(List,counter,"Intensity",data->Intensity);
}

void ChunkTableClass::List_W3D_CHUNK_SPOT_LIGHT_INFO(ChunkItem * Item,CListCtrl * List)
{
	W3dSpotLightStruct * data = (W3dSpotLightStruct*)Item->Data;
	int counter = 0;
	AddItem(List,counter,"SpotDirection",&(data->SpotDirection));
	AddItem(List,counter,"SpotAngle",data->SpotAngle);
	AddItem(List,counter,"SpotExponent",data->SpotExponent);
}

void ChunkTableClass::List_W3D_CHUNK_NEAR_ATTENUATION(ChunkItem * Item,CListCtrl * List)
{
	W3dLightAttenuationStruct * data = (W3dLightAttenuationStruct *)Item->Data;
	int counter = 0;
	AddItem(List,counter,"Near Atten Start",data->Start);
	AddItem(List,counter,"Near Atten End",data->End);
}

void ChunkTableClass::List_W3D_CHUNK_FAR_ATTENUATION(ChunkItem * Item,CListCtrl * List)
{
	W3dLightAttenuationStruct * data = (W3dLightAttenuationStruct *)Item->Data;
	int counter = 0;
	AddItem(List,counter,"Far Atten Start",data->Start);
	AddItem(List,counter,"Far Atten End",data->End);
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER(ChunkItem * Item,CListCtrl * List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_HEADER(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterHeaderStruct * data = (W3dEmitterHeaderStruct*)Item->Data;
	int counter = 0;
	AddItem(List,counter,"Version",data->Version);
	AddItem(List,counter,"Name",data->Name);
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_USER_DATA(ChunkItem * Item,CListCtrl * List)
{
	char * data = (char *)Item->Data;
	int counter = 0;
	AddItem(List,counter,"User Data",data);
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_INFO(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterInfoStruct * data = (W3dEmitterInfoStruct *)Item->Data;
	int counter = 0;
	AddItem(List,counter,"Texture Name",data->TextureFilename);
	AddItem(List,counter,"StartSize", data->StartSize);
	AddItem(List,counter,"EndSize",data->EndSize);
	AddItem(List,counter,"Lifetime",data->Lifetime);
	AddItem(List,counter,"EmissionRate",data->EmissionRate);
	AddItem(List,counter,"MaxEmissions",data->MaxEmissions);
	AddItem(List,counter,"VelocityRandom",data->VelocityRandom);
	AddItem(List,counter,"PositionRandom",data->PositionRandom);
	AddItem(List,counter,"FadeTime",data->FadeTime);
	AddItem(List,counter,"Gravity",data->Gravity);
	AddItem(List,counter,"Elasticity",data->Elasticity);
	AddItem(List,counter,"Velocity",&(data->Velocity));
	AddItem(List,counter,"Acceleration",&(data->Acceleration));
	AddItem(List,counter,"StartColor",&(data->StartColor));
	AddItem(List,counter,"EndColor",&(data->EndColor));
}	

void ChunkTableClass::List_W3D_CHUNK_EMITTER_INFOV2(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterInfoStructV2 * data = (W3dEmitterInfoStructV2 *)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"BurstSize",data->BurstSize);
	AddItem(List,counter,"CreationVolume.ClassID",data->CreationVolume.ClassID);
	AddItem(List,counter,"CreationVolume.Value1",data->CreationVolume.Value1);
	AddItem(List,counter,"CreationVolume.Value2",data->CreationVolume.Value2);
	AddItem(List,counter,"CreationVolume.Value3",data->CreationVolume.Value3);
	AddItem(List,counter,"VelRandom.ClassID",data->VelRandom.ClassID);
	AddItem(List,counter,"VelRandom.Value1",data->VelRandom.Value1);
	AddItem(List,counter,"VelRandom.Value2",data->VelRandom.Value2);
	AddItem(List,counter,"VelRandom.Value3",data->VelRandom.Value3);
	AddItem(List,counter,"OutwardVel",data->OutwardVel);
	AddItem(List,counter,"VelInherit",data->VelInherit);
	AddItem(List,counter,"Shader",&(data->Shader));
	AddItem(List,counter,"RenderMode",data->RenderMode);
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_PROPS(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterPropertyStruct * data = (W3dEmitterPropertyStruct *)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"ColorKeyframes",data->ColorKeyframes);
	AddItem(List,counter,"OpacityKeyframes",data->OpacityKeyframes);
	AddItem(List,counter,"SizeKeyframes",data->SizeKeyframes);
	AddItem(List,counter,"ColorRandom",&(data->ColorRandom));
	AddItem(List,counter,"OpacityRandom",data->OpacityRandom);
	AddItem(List,counter,"SizeRandom",data->SizeRandom);
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_COLOR_KEYFRAME(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterColorKeyframeStruct * data = (W3dEmitterColorKeyframeStruct *)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"Time",data->Time);
	AddItem(List,counter,"Color",&(data->Color));
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterOpacityKeyframeStruct * data = (W3dEmitterOpacityKeyframeStruct *)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"Time",data->Time);
	AddItem(List,counter,"Opacity",data->Opacity);
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_SIZE_KEYFRAME(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterSizeKeyframeStruct * data = (W3dEmitterSizeKeyframeStruct *)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"Time",data->Time);
	AddItem(List,counter,"Size",data->Size);
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterRotationHeaderStruct * header = (W3dEmitterRotationHeaderStruct*)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"KeyframeCount",header->KeyframeCount);
	AddItem(List,counter,"Random",header->Random);

	W3dEmitterRotationKeyframeStruct * key = (W3dEmitterRotationKeyframeStruct *)((char*)Item->Data + sizeof(W3dEmitterRotationHeaderStruct));
	char buf[256];
	for (unsigned int i=0; i<header->KeyframeCount+1; i++) {
		sprintf(buf,"Time[%d]",i);
		AddItem(List,counter,buf,key[i].Time);
		sprintf(buf,"Rotation[%d]",i);
		AddItem(List,counter,buf,key[i].Rotation);
	}
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterFrameHeaderStruct * header = (W3dEmitterFrameHeaderStruct*)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"KeyframeCount",header->KeyframeCount);
	AddItem(List,counter,"Random",header->Random);

	W3dEmitterFrameKeyframeStruct * key = (W3dEmitterFrameKeyframeStruct *)((char *)Item->Data + sizeof(W3dEmitterFrameHeaderStruct));
	char buf[256];
	for (unsigned int i=0; i<header->KeyframeCount+1; i++) {
		sprintf(buf,"Time[%d]",i);
		AddItem(List,counter,buf,key[i].Time);
		sprintf(buf,"Frame[%d]",i);
		AddItem(List,counter,buf,key[i].Frame);
	}
}

void ChunkTableClass::List_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES(ChunkItem * Item,CListCtrl * List)
{
	W3dEmitterBlurTimeHeaderStruct * header = (W3dEmitterBlurTimeHeaderStruct*)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"KeyframeCount",header->KeyframeCount);
	AddItem(List,counter,"Random",header->Random);

	W3dEmitterBlurTimeKeyframeStruct * key = (W3dEmitterBlurTimeKeyframeStruct *)((char *)Item->Data + sizeof(W3dEmitterBlurTimeHeaderStruct));
	char buf[256];
	for (unsigned int i=0; i<header->KeyframeCount+1; i++) {
		sprintf(buf,"Time[%d]",i);
		AddItem(List,counter,buf,key[i].Time);
		sprintf(buf,"BlurTime[%d]",i);
		AddItem(List,counter,buf,key[i].BlurTime);
	}
}

void ChunkTableClass::List_W3D_CHUNK_AGGREGATE(ChunkItem * Item,CListCtrl * List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_AGGREGATE_HEADER(ChunkItem * Item,CListCtrl * List)
{
	W3dAggregateHeaderStruct * data = (W3dAggregateHeaderStruct*)Item->Data;
	int counter = 0;
	AddItem(List,counter,"Version",data->Version);
	AddItem(List,counter,"Name",data->Name);
}

void ChunkTableClass::List_W3D_CHUNK_AGGREGATE_INFO(ChunkItem * Item,CListCtrl * List)
{
	W3dAggregateInfoStruct * info = (W3dAggregateInfoStruct *)Item->Data;
	int counter = 0;
	AddItem(List,counter,"BaseModelName",info->BaseModelName);
	AddItem(List,counter,"SubobjectCount",info->SubobjectCount);

	char label[256];
	W3dAggregateSubobjectStruct * defs = (W3dAggregateSubobjectStruct *)((char*)Item->Data + sizeof(W3dAggregateInfoStruct));
	
	for (unsigned int subobj=0; subobj<info->SubobjectCount; subobj++) {
		counter = 0;
		
		sprintf(label,"SubObject[%d].SubobjectName",subobj);
		AddItem(List,counter,label,defs[subobj].SubobjectName);
		sprintf(label,"SubObject[%d].BoneName",subobj);
		AddItem(List,counter,label,defs[subobj].BoneName);

	}
}

void ChunkTableClass::List_W3D_CHUNK_TEXTURE_REPLACER_INFO(ChunkItem * Item,CListCtrl * List)
{
	W3dTextureReplacerHeaderStruct * header = (W3dTextureReplacerHeaderStruct *)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"ReplacedTexturesCount",header->ReplacedTexturesCount);

	W3dTextureReplacerStruct * data = (W3dTextureReplacerStruct *)(header + 1);

	for (uint32 replaceidx=0; replaceidx<header->ReplacedTexturesCount; replaceidx++) {
		int pathidx = 0;
		char label[256];

		for (pathidx=0; pathidx<MESH_PATH_ENTRIES; pathidx++){
			sprintf(label,"Replacer[%d].MeshPath[%d]",replaceidx,pathidx);
			AddItem(List,counter,label,data->MeshPath[pathidx]);
		}

		for (pathidx=0; pathidx<MESH_PATH_ENTRIES; pathidx++){
			sprintf(label,"Replacer[%d].BonePath[%d]",replaceidx,pathidx);
			AddItem(List,counter,label,data->BonePath[pathidx]);
		}
		
		AddItem(List,counter,"OldTextureName",data->OldTextureName);
		AddItem(List,counter,"NewTextureName",data->NewTextureName);
		AddItem(List,counter,"TextureParams.Attributes", data->TextureParams.Attributes);
		AddItem(List,counter,"TextureParams.FrameCount", data->TextureParams.FrameCount);
		AddItem(List,counter,"TextureParams.FrameRate", data->TextureParams.FrameRate);
		data++;
	}
}

void ChunkTableClass::List_W3D_CHUNK_AGGREGATE_CLASS_INFO(ChunkItem * Item,CListCtrl * List)
{
	W3dAggregateMiscInfo * data = (W3dAggregateMiscInfo *)(Item->Data);
	int counter = 0;
	AddItem(List,counter,"OriginalClassID",data->OriginalClassID);
	AddItem(List,counter,"Flags",data->Flags);
}

void ChunkTableClass::List_W3D_CHUNK_HLOD(ChunkItem * Item,CListCtrl * List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_HLOD_HEADER(ChunkItem * Item,CListCtrl * List)
{
	W3dHLodHeaderStruct * header = (W3dHLodHeaderStruct *)Item->Data;
	int counter = 0;
	
	AddItem(List,counter,"Version",header->Version);
	AddItem(List,counter,"LodCount", header->LodCount);
	AddItem(List,counter,"Name",header->Name);
	AddItem(List,counter,"HTree Name", header->HierarchyName);
}

void ChunkTableClass::List_W3D_CHUNK_HLOD_LOD_ARRAY(ChunkItem * Item,CListCtrl * List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_HLOD_LOD_ARRAY_HEADER(ChunkItem * Item,CListCtrl * List)
{
	W3dHLodArrayHeaderStruct * header = (W3dHLodArrayHeaderStruct *)Item->Data;
	int counter = 0;

	AddItem(List,counter,"ModelCount",header->ModelCount);
	AddItem(List,counter,"MaxScreenSize",header->MaxScreenSize);
}

void ChunkTableClass::List_W3D_CHUNK_HLOD_SUB_OBJECT(ChunkItem * Item,CListCtrl * List)
{
	W3dHLodSubObjectStruct * data = (W3dHLodSubObjectStruct *)Item->Data;
	int counter = 0;
	AddItem(List,counter,"Name",data->Name);
	AddItem(List,counter,"BoneIndex",data->BoneIndex);
}

void ChunkTableClass::List_W3D_CHUNK_BOX(ChunkItem * Item,CListCtrl * List)
{
	W3dBoxStruct * box = (W3dBoxStruct *)Item->Data;
	int counter = 0;
	AddItem(List,counter,"Version",box->Version);
	AddItem(List,counter,"Attributes",box->Attributes);

	if (box->Attributes & W3D_BOX_ATTRIBUTE_ORIENTED) {
		AddItem(List,counter,"Attributes","W3D_BOX_ATTRIBUTE_ORIENTED","flag");
	}
	if (box->Attributes & W3D_BOX_ATTRIBUTE_ALIGNED) {
		AddItem(List,counter,"Attributes","W3D_BOX_ATTRIBUTE_ALIGNED","flag");
	}
	if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL) {
		AddItem(List,counter,"Attributes","W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL","flag");
	}
	if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE) {
		AddItem(List,counter,"Attributes","W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE","flag");
	}
	if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS) {
		AddItem(List,counter,"Attributes","W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS","flag");
	}
	if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA) {
		AddItem(List,counter,"Attributes","W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA","flag");
	}
	if (box->Attributes & W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE) {
		AddItem(List,counter,"Attributes","W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE","flag");
	}
	
	AddItem(List,counter,"Name",box->Name);
	AddItem(List,counter,"Color",&(box->Color));
	AddItem(List,counter,"Center",&(box->Center));
	AddItem(List,counter,"Extent",&(box->Extent));
}

void ChunkTableClass::List_W3D_CHUNK_NULL_OBJECT(ChunkItem * Item,CListCtrl * List)
{
	W3dNullObjectStruct * null = (W3dNullObjectStruct *)Item->Data;
	int counter = 0;
	AddItemVersion(List,counter,null->Version);
	AddItem(List,counter,"Attributes",null->Attributes);

	// No attributes are currently used

	AddItem(List,counter,"Name",null->Name);
}

void ChunkTableClass::List_W3D_CHUNK_PRELIT_UNLIT(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_PRELIT_VERTEX(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_LIGHTSCAPE(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_LIGHTSCAPE_LIGHT(ChunkItem *Item, CListCtrl *List) {
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_LIGHT_TRANSFORM(ChunkItem *Item, CListCtrl *List) {

	W3dLightTransformStruct *data = (W3dLightTransformStruct*) (Item->Data);

	int counter = 0;

	AddItem (List, counter, "Transform", &(data->Transform [0][0]), 4);
	AddItem (List, counter, "Transform", &(data->Transform [1][0]), 4);
	AddItem (List, counter, "Transform", &(data->Transform [2][0]), 4);
}

void ChunkTableClass::List_W3D_CHUNK_DAZZLE(ChunkItem *Item, CListCtrl *List)
{
	List_Subitems(Item, List);
}

void ChunkTableClass::List_W3D_CHUNK_DAZZLE_NAME(ChunkItem *Item, CListCtrl *List)
{
	int counter = 0;
	AddItem (List, counter, "Dazzle Name", (char *)(Item->Data));
}

void ChunkTableClass::List_W3D_CHUNK_DAZZLE_TYPENAME(ChunkItem *Item, CListCtrl *List)
{
	int counter = 0;
	AddItem (List, counter, "Dazzle Type Name", (char *)(Item->Data));
}

ChunkTableClass::ChunkTableClass() {

	NewType( W3D_CHUNK_MESH, "CHUNK_MESH", List_W3D_CHUNK_MESH, true);
	NewType( W3D_CHUNK_MESH_HEADER, "W3D_CHUNK_MESH_HEADER", List_W3D_CHUNK_MESH_HEADER);
	NewType( W3D_CHUNK_VERTICES, "W3D_CHUNK_VERTICES", List_W3D_CHUNK_VERTICES);
	NewType( W3D_CHUNK_VERTEX_NORMALS, "W3D_CHUNK_VERTEX_NORMALS", List_W3D_CHUNK_VERTEX_NORMALS);
	NewType( W3D_CHUNK_SURRENDER_NORMALS, "W3D_CHUNK_SURRENDER_NORMALS", List_W3D_CHUNK_SURRENDER_NORMALS);
	NewType( W3D_CHUNK_TEXCOORDS, "W3D_CHUNK_TEXCOORDS", List_W3D_CHUNK_TEXCOORDS);
	NewType( O_W3D_CHUNK_MATERIALS, "O_W3D_CHUNK_MATERIALS", List_O_W3D_CHUNK_MATERIALS);
	NewType( O_W3D_CHUNK_TRIANGLES, "O_W3D_CHUNK_TRIANGLES", List_O_W3D_CHUNK_TRIANGLES);
	NewType( O_W3D_CHUNK_QUADRANGLES, "O_W3D_CHUNK_QUADRANGLES", List_O_W3D_CHUNK_QUADRANGLES);
	NewType( O_W3D_CHUNK_SURRENDER_TRIANGLES, "O_W3D_CHUNK_SURRENDER_TRIANGLES", List_O_W3D_CHUNK_SURRENDER_TRIANGLES);
	NewType( O_W3D_CHUNK_POV_TRIANGLES, "O_W3D_CHUNK_POV_TRIANGLES", List_O_W3D_CHUNK_POV_TRIANGLES);
	NewType( O_W3D_CHUNK_POV_QUADRANGLES, "O_W3D_CHUNK_POV_QUADRANGLES", List_O_W3D_CHUNK_POV_QUADRANGLES);
	NewType( W3D_CHUNK_MESH_USER_TEXT, "W3D_CHUNK_MESH_USER_TEXT", List_W3D_CHUNK_MESH_USER_TEXT);
	NewType( W3D_CHUNK_VERTEX_COLORS, "W3D_CHUNK_VERTEX_COLORS", List_W3D_CHUNK_VERTEX_COLORS);
	NewType( W3D_CHUNK_VERTEX_INFLUENCES, "W3D_CHUNK_VERTEX_INFLUENCES", List_W3D_CHUNK_VERTEX_INFLUENCES);
	
	NewType( W3D_CHUNK_DAMAGE, "W3D_CHUNK_DAMAGE", List_W3D_CHUNK_DAMAGE, true);
	NewType( W3D_CHUNK_DAMAGE_HEADER, "W3D_CHUNK_DAMAGE_HEADER", List_W3D_CHUNK_DAMAGE_HEADER);
	NewType( W3D_CHUNK_DAMAGE_VERTICES, "W3D_CHUNK_DAMAGE_VERTICES", List_W3D_CHUNK_DAMAGE_VERTICES);
	NewType( W3D_CHUNK_DAMAGE_COLORS, "W3D_CHUNK_DAMAGE_COLORS", List_W3D_CHUNK_DAMAGE_COLORS);

	NewType( O_W3D_CHUNK_MATERIALS2, "O_W3D_CHUNK_MATERIALS2", List_O_W3D_CHUNK_MATERIALS2);

	NewType( W3D_CHUNK_MATERIALS3, "W3D_CHUNK_MATERIALS3", List_W3D_CHUNK_MATERIALS3, true);
	NewType( W3D_CHUNK_MATERIAL3, "W3D_CHUNK_MATERIAL3", List_W3D_CHUNK_MATERIAL3, true);
	NewType( W3D_CHUNK_MATERIAL3_NAME, "W3D_CHUNK_MATERIAL3_NAME", List_W3D_CHUNK_MATERIAL3_NAME);
	NewType( W3D_CHUNK_MATERIAL3_INFO, "W3D_CHUNK_MATERIAL3_INFO", List_W3D_CHUNK_MATERIAL3_INFO);
	NewType( W3D_CHUNK_MATERIAL3_DC_MAP, "W3D_CHUNK_MATERIAL3_DC_MAP", List_W3D_CHUNK_MATERIAL3_DC_MAP,true);
	NewType( W3D_CHUNK_MATERIAL3_DI_MAP, "W3D_CHUNK_MATERIAL3_DI_MAP", List_W3D_CHUNK_MATERIAL3_DI_MAP,true);
	NewType( W3D_CHUNK_MATERIAL3_SC_MAP, "W3D_CHUNK_MATERIAL3_SC_MAP", List_W3D_CHUNK_MATERIAL3_SC_MAP,true);
	NewType( W3D_CHUNK_MATERIAL3_SI_MAP, "W3D_CHUNK_MATERIAL3_SI_MAP", List_W3D_CHUNK_MATERIAL3_SI_MAP,true);
	NewType( W3D_CHUNK_MAP3_FILENAME, "W3D_CHUNK_MAP3_FILENAME", List_W3D_CHUNK_MAP3_FILENAME);
	NewType( W3D_CHUNK_MAP3_INFO, "W3D_CHUNK_MAP3_INFO", List_W3D_CHUNK_MAP3_INFO);

	NewType( W3D_CHUNK_MESH_HEADER3, "W3D_CHUNK_MESH_HEADER3",List_W3D_CHUNK_MESH_HEADER3);
	NewType( W3D_CHUNK_TRIANGLES, "W3D_CHUNK_TRIANGLES",List_W3D_CHUNK_TRIANGLES);
	NewType( W3D_CHUNK_PER_TRI_MATERIALS,"W3D_CHUNK_PER_TRI_MATERIALS",List_W3D_CHUNK_PER_TRI_MATERIALS);

	NewType( W3D_CHUNK_VERTEX_SHADE_INDICES, "W3D_CHUNK_VERTEX_SHADE_INDICES",List_W3D_CHUNK_VERTEX_SHADE_INDICES);
	NewType( W3D_CHUNK_MATERIAL_INFO,"W3D_CHUNK_MATERIAL_INFO",List_W3D_CHUNK_MATERIAL_INFO);
	NewType( W3D_CHUNK_SHADERS,"W3D_CHUNK_SHADERS",List_W3D_CHUNK_SHADERS);
	NewType( W3D_CHUNK_PS2_SHADERS,"W3D_CHUNK_PS2_SHADERS",List_W3D_CHUNK_PS2_SHADERS);

	NewType( W3D_CHUNK_VERTEX_MATERIALS, "W3D_CHUNK_VERTEX_MATERIALS",List_W3D_CHUNK_VERTEX_MATERIALS,true);
	NewType( W3D_CHUNK_VERTEX_MATERIAL, "W3D_CHUNK_VERTEX_MATERIAL",List_W3D_CHUNK_VERTEX_MATERIAL,true);
	NewType( W3D_CHUNK_VERTEX_MATERIAL_NAME, "W3D_CHUNK_VERTEX_MATERIAL_NAME",List_W3D_CHUNK_VERTEX_MATERIAL_NAME);
	NewType( W3D_CHUNK_VERTEX_MATERIAL_INFO, "W3D_CHUNK_VERTEX_MATERIAL_INFO",List_W3D_CHUNK_VERTEX_MATERIAL_INFO);
	NewType( W3D_CHUNK_VERTEX_MAPPER_ARGS0, "W3D_CHUNK_VERTEX_MAPPER_ARGS0",List_W3D_CHUNK_VERTEX_MAPPER_ARGS0);
	NewType( W3D_CHUNK_VERTEX_MAPPER_ARGS1, "W3D_CHUNK_VERTEX_MAPPER_ARGS1",List_W3D_CHUNK_VERTEX_MAPPER_ARGS1);

	NewType( W3D_CHUNK_TEXTURES, "W3D_CHUNK_TEXTURES", List_W3D_CHUNK_TEXTURES,true);
	NewType( W3D_CHUNK_TEXTURE, "W3D_CHUNK_TEXTURE", List_W3D_CHUNK_TEXTURE,true);
	NewType( W3D_CHUNK_TEXTURE_NAME, "W3D_CHUNK_TEXTURE_NAME", List_W3D_CHUNK_TEXTURE_NAME);
	NewType( W3D_CHUNK_TEXTURE_INFO, "W3D_CHUNK_TEXTURE_INFO", List_W3D_CHUNK_TEXTURE_INFO);
	
	NewType( W3D_CHUNK_MATERIAL_PASS, "W3D_CHUNK_MATERIAL_PASS", List_W3D_CHUNK_MATERIAL_PASS,true);
	NewType( W3D_CHUNK_VERTEX_MATERIAL_IDS, "W3D_CHUNK_VERTEX_MATERIAL_IDS", List_W3D_CHUNK_VERTEX_MATERIAL_IDS);
	NewType( W3D_CHUNK_SHADER_IDS, "W3D_CHUNK_SHADER_IDS", List_W3D_CHUNK_SHADER_IDS);
	NewType( W3D_CHUNK_DCG, "W3D_CHUNK_DCG", List_W3D_CHUNK_DCG);
	NewType( W3D_CHUNK_DIG, "W3D_CHUNK_DIG", List_W3D_CHUNK_DIG);
	NewType( W3D_CHUNK_SCG, "W3D_CHUNK_SCG", List_W3D_CHUNK_SCG);
	
	NewType( W3D_CHUNK_TEXTURE_STAGE, "W3D_CHUNK_TEXTURE_STAGE", List_W3D_CHUNK_TEXTURE_STAGE,true);
	NewType( W3D_CHUNK_TEXTURE_IDS, "W3D_CHUNK_TEXTURE_IDS", List_W3D_CHUNK_TEXTURE_IDS);
	NewType( W3D_CHUNK_STAGE_TEXCOORDS, "W3D_CHUNK_STAGE_TEXCOORDS", List_W3D_CHUNK_STAGE_TEXCOORDS);
	NewType( W3D_CHUNK_PER_FACE_TEXCOORD_IDS, "W3D_CHUNK_PER_FACE_TEXCOORD_IDS", List_W3D_CHUNK_PER_FACE_TEXCOORD_IDS);
	
	NewType( W3D_CHUNK_AABTREE, "W3D_CHUNK_AABTREE", List_W3D_CHUNK_AABTREE,true);
	NewType( W3D_CHUNK_AABTREE_HEADER, "W3D_CHUNK_AABTREE_HEADER", List_W3D_CHUNK_AABTREE_HEADER);
	NewType( W3D_CHUNK_AABTREE_POLYINDICES, "W3D_CHUNK_AABTREE_POLYINDICES", List_W3D_CHUNK_AABTREE_POLYINDICES);
	NewType( W3D_CHUNK_AABTREE_NODES, "W3D_CHUNK_AABTREE_NODES", List_W3D_CHUNK_AABTREE_NODES);
	
	NewType( W3D_CHUNK_HIERARCHY, "W3D_CHUNK_HIERARCHY", List_W3D_CHUNK_HIERARCHY, true);
	NewType( W3D_CHUNK_HIERARCHY_HEADER, "W3D_CHUNK_HIERARCHY_HEADER", List_W3D_CHUNK_HIERARCHY_HEADER);
	NewType( W3D_CHUNK_PIVOTS, "W3D_CHUNK_PIVOTS", List_W3D_CHUNK_PIVOTS);
	NewType( W3D_CHUNK_PIVOT_FIXUPS, "W3D_CHUNK_PIVOT_FIXUPS", List_W3D_CHUNK_PIVOT_FIXUPS);

	NewType( W3D_CHUNK_ANIMATION, "W3D_CHUNK_ANIMATION", List_W3D_CHUNK_ANIMATION, true);
	NewType( W3D_CHUNK_ANIMATION_HEADER, "W3D_CHUNK_ANIMATION_HEADER", List_W3D_CHUNK_ANIMATION_HEADER);
	NewType( W3D_CHUNK_ANIMATION_CHANNEL, "W3D_CHUNK_ANIMATION_CHANNEL", List_W3D_CHUNK_ANIMATION_CHANNEL);
	NewType( W3D_CHUNK_BIT_CHANNEL, "W3D_CHUNK_BIT_CHANNEL", List_W3D_CHUNK_BIT_CHANNEL);

	NewType( W3D_CHUNK_HMODEL, "W3D_CHUNK_HMODEL", List_W3D_CHUNK_HMODEL,true);
	NewType( W3D_CHUNK_HMODEL_HEADER, "W3D_CHUNK_HMODEL_HEADER", List_W3D_CHUNK_HMODEL_HEADER);
	NewType( W3D_CHUNK_NODE, "W3D_CHUNK_NODE", List_W3D_CHUNK_NODE);
	NewType( W3D_CHUNK_COLLISION_NODE, "W3D_CHUNK_COLLISION_NODE", List_W3D_CHUNK_COLLISION_NODE);
	NewType( W3D_CHUNK_SKIN_NODE, "W3D_CHUNK_SKIN_NODE", List_W3D_CHUNK_SKIN_NODE);
	NewType( OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA, "W3D_CHUNK_HMODEL_AUX_DATA", List_W3D_CHUNK_HMODEL_AUX_DATA);
	NewType( OBSOLETE_W3D_CHUNK_SHADOW_NODE, "OBSOLETE_W3D_CHUNK_SHADOW_NODE", List_W3D_CHUNK_SHADOW_NODE);

	NewType( W3D_CHUNK_LODMODEL, "W3D_CHUNK_LODMODEL", List_W3D_CHUNK_LODMODEL,true);
	NewType( W3D_CHUNK_LODMODEL_HEADER, "W3D_CHUNK_LODMODEL_HEADER", List_W3D_CHUNK_LODMODEL_HEADER);
	NewType( W3D_CHUNK_LOD, "W3D_CHUNK_LOD", List_W3D_CHUNK_LOD);

	NewType( W3D_CHUNK_COLLECTION, "W3D_CHUNK_COLLECTION", List_W3D_CHUNK_COLLECTION,true);
	NewType( W3D_CHUNK_COLLECTION_HEADER, "W3D_CHUNK_COLLECTION_HEADER", List_W3D_CHUNK_COLLECTION_HEADER);
	NewType( W3D_CHUNK_COLLECTION_OBJ_NAME, "W3D_CHUNK_COLLECTION_OBJ_NAME", List_W3D_CHUNK_COLLECTION_OBJ_NAME);
	NewType( W3D_CHUNK_PLACEHOLDER,"W3D_CHUNK_PLACEHOLDER", List_W3D_CHUNK_PLACEHOLDER);
	NewType( W3D_CHUNK_TRANSFORM_NODE,"W3D_CHUNK_TRANSFORM_NODE", List_W3D_CHUNK_TRANSFORM_NODE);

	NewType( W3D_CHUNK_POINTS, "W3D_CHUNK_POINTS", List_W3D_CHUNK_POINTS);

	NewType( W3D_CHUNK_LIGHT, "W3D_CHUNK_LIGHT", List_W3D_CHUNK_LIGHT,true);
	NewType( W3D_CHUNK_LIGHT_INFO, "W3D_CHUNK_LIGHT_INFO",List_W3D_CHUNK_LIGHT_INFO);
	NewType( W3D_CHUNK_SPOT_LIGHT_INFO, "W3D_CHUNK_SPOT_LIGHT_INFO",List_W3D_CHUNK_SPOT_LIGHT_INFO);
	NewType( W3D_CHUNK_NEAR_ATTENUATION, "W3D_CHUNK_NEAR_ATTENUATION", List_W3D_CHUNK_NEAR_ATTENUATION);
	NewType( W3D_CHUNK_FAR_ATTENUATION, "W3D_CHUNK_FAR_ATTENUATION", List_W3D_CHUNK_FAR_ATTENUATION);

	NewType( W3D_CHUNK_EMITTER, "W3D_CHUNK_EMITTER", List_W3D_CHUNK_EMITTER,true);
	NewType( W3D_CHUNK_EMITTER_HEADER, "W3D_CHUNK_EMITTER_HEADER",List_W3D_CHUNK_EMITTER_HEADER);
	NewType( W3D_CHUNK_EMITTER_USER_DATA, "W3D_CHUNK_EMITTER_USER_DATA", List_W3D_CHUNK_EMITTER_USER_DATA);
	NewType( W3D_CHUNK_EMITTER_INFO, "W3D_CHUNK_EMITTER_INFO", List_W3D_CHUNK_EMITTER_INFO);
	NewType( W3D_CHUNK_EMITTER_INFOV2, "W3D_CHUNK_EMITTER_INFOV2",List_W3D_CHUNK_EMITTER_INFOV2);
	NewType( W3D_CHUNK_EMITTER_PROPS, "W3D_CHUNK_EMITTER_PROPS", List_W3D_CHUNK_EMITTER_PROPS);
	NewType( OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME, "OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME", List_W3D_CHUNK_EMITTER_COLOR_KEYFRAME);
	NewType( OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME, "OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME", List_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME);
	NewType( OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME, "OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME", List_W3D_CHUNK_EMITTER_SIZE_KEYFRAME);
	NewType( W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES, "W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES", List_W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES);
	NewType( W3D_CHUNK_EMITTER_FRAME_KEYFRAMES, "W3D_CHUNK_EMITTER_FRAME_KEYFRAMES", List_W3D_CHUNK_EMITTER_FRAME_KEYFRAMES);
	NewType( W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES, "W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES", List_W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES);

	NewType( W3D_CHUNK_AGGREGATE, "W3D_CHUNK_AGGREGATE", List_W3D_CHUNK_AGGREGATE,true);
	NewType( W3D_CHUNK_AGGREGATE_HEADER, "W3D_CHUNK_AGGREGATE_HEADER", List_W3D_CHUNK_AGGREGATE_HEADER);
	NewType( W3D_CHUNK_AGGREGATE_INFO, "W3D_CHUNK_AGGREGATE_INFO",List_W3D_CHUNK_AGGREGATE_INFO);
	NewType( W3D_CHUNK_TEXTURE_REPLACER_INFO, "W3D_CHUNK_TEXTURE_REPLACER_INFO", List_W3D_CHUNK_TEXTURE_REPLACER_INFO);
	NewType( W3D_CHUNK_AGGREGATE_CLASS_INFO, "W3D_CHUNK_AGGREGATE_CLASS_INFO", List_W3D_CHUNK_AGGREGATE_CLASS_INFO);
	
	NewType( W3D_CHUNK_HLOD, "W3D_CHUNK_HLOD", List_W3D_CHUNK_HLOD, true);
	NewType( W3D_CHUNK_HLOD_HEADER, "W3D_CHUNK_HLOD_HEADER", List_W3D_CHUNK_HLOD_HEADER);
	NewType( W3D_CHUNK_HLOD_LOD_ARRAY, "W3D_CHUNK_HLOD_LOD_ARRAY", List_W3D_CHUNK_HLOD_LOD_ARRAY, true);
	NewType( W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER, "W3D_CHUNK_HLOD_LOD_ARRAY_HEADER", List_W3D_CHUNK_HLOD_LOD_ARRAY_HEADER);
	NewType( W3D_CHUNK_HLOD_SUB_OBJECT, "W3D_CHUNK_HLOD_SUB_OBJECT", List_W3D_CHUNK_HLOD_SUB_OBJECT);
	NewType( W3D_CHUNK_HLOD_AGGREGATE_ARRAY, "W3D_CHUNK_HLOD_AGGREGATE_ARRAY", List_W3D_CHUNK_HLOD_LOD_ARRAY, true);
	NewType( W3D_CHUNK_HLOD_PROXY_ARRAY, "W3D_CHUNK_HLOD_PROXY_ARRAY", List_W3D_CHUNK_HLOD_LOD_ARRAY, true);

	NewType( W3D_CHUNK_BOX, "W3D_CHUNK_BOX", List_W3D_CHUNK_BOX);

	NewType( W3D_CHUNK_NULL_OBJECT, "W3D_CHUNK_NULL_OBJECT", List_W3D_CHUNK_NULL_OBJECT);

	NewType( W3D_CHUNK_PRELIT_UNLIT,	"W3D_CHUNK_PRELIT_UNLIT", List_W3D_CHUNK_PRELIT_UNLIT, true);
	NewType( W3D_CHUNK_PRELIT_VERTEX, "W3D_CHUNK_PRELIT_VERTEX", List_W3D_CHUNK_PRELIT_VERTEX, true);
	NewType( W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS, "W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS", List_W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS, true);
	NewType( W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE, "W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE", List_W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE, true);
	NewType( W3D_CHUNK_LIGHTSCAPE, "W3D_CHUNK_LIGHTSCAPE", List_W3D_CHUNK_LIGHTSCAPE, true);
	NewType( W3D_CHUNK_LIGHTSCAPE_LIGHT, "W3D_CHUNK_LIGHTSCAPE_LIGHT", List_W3D_CHUNK_LIGHTSCAPE_LIGHT, true);
	NewType( W3D_CHUNK_LIGHT_TRANSFORM, "W3D_CHUNK_LIGHT_TRANSFORM", List_W3D_CHUNK_LIGHT_TRANSFORM);

	NewType( W3D_CHUNK_DAZZLE, "W3D_CHUNK_DAZZLE", List_W3D_CHUNK_DAZZLE, true);
	NewType( W3D_CHUNK_DAZZLE_NAME, "W3D_CHUNK_DAZZLE_NAME" , List_W3D_CHUNK_DAZZLE_NAME);
	NewType( W3D_CHUNK_DAZZLE_TYPENAME, "W3D_CHUNK_DAZZLE_TYPENAME" , List_W3D_CHUNK_DAZZLE_TYPENAME);

}

ChunkType *ChunkTableClass::Lookup(int ID) {
	ChunkType *chunktype;
	if(Types.Lookup((void *) ID, (void *&) chunktype)) 
		return chunktype;
	return 0;
}

ChunkItem::ChunkItem(ChunkLoadClass &cload) {
	ID = cload.Cur_Chunk_ID();
	Length = cload.Cur_Chunk_Length();
	Type = ChunkTable.Lookup(ID);

	// if the chunktype indicates that it has member chunks then do not load this chunk's data. Have the external caller do that.
	if(Type != 0 && Type->Wrapper) {
		Data = 0;
		return;
	}
	if(Type != 0) {
		TRACE("%s %d\n", Type->Name, Length);
	}

	if(Length == 0) {
		Data = 0;
	} else {
		Data = new char[Length];
		cload.Read(Data, Length);
	}
}

ChunkItem::~ChunkItem() {
	if(Data != 0) 
		delete [] Data;
	while(!Chunks.IsEmpty()) {
		ChunkItem *item = Chunks.RemoveHead();
		delete item;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ChunkData::ChunkData()
{

}

ChunkData::~ChunkData()
{
	while(!Chunks.IsEmpty()) {
		ChunkItem *item = Chunks.RemoveHead();
		delete item;
	}
}

bool ChunkData::Load(const char *filename)
{

	Release_Data();

	RawFileMClass chunk_file;

	if (!chunk_file.Open(filename)) {
		return false;
	}

	ChunkLoadClass cload(&chunk_file);

	while (cload.Open_Chunk()) {
		Add_Chunk(cload);
		cload.Close_Chunk();
	}
/*
		switch (cload.Cur_Chunk_ID()) {

			case W3D_CHUNK_MESH:
				MeshManager.Load_Mesh(cload);
				break;

			case W3D_CHUNK_HIERARCHY:
				HTreeManager.Load_Tree(cload);
				break;

			case W3D_CHUNK_ANIMATION:
				HAnimManager.Load_Anim(cload);
				break;

			case W3D_CHUNK_MESH_CONNECTIONS:
				HModelManager.Load_Model_Definition(cload);
				break;
		}
		cload.Close_Chunk();
	}
*/

	chunk_file.Close();

	return true;
}

void ChunkData::Add_Chunk(ChunkLoadClass & cload, ChunkItem *Parent)
{
	ChunkItem *item = new ChunkItem(cload);
	if(Parent) 
		Parent->Chunks.AddTail(item);
	else 
		Chunks.AddTail(item);
//Moumine 1/2/2002    1:21:26 PM
#if ! defined _W3DSHELLEXT
	if(theApp.DumpTextures) {
		if(item->ID == W3D_CHUNK_TEXTURE_NAME) {
			static CMapStringToString existing;

			char * data = (char *)item->Data;
			
			CString value;
			if(existing.Lookup(data, value) == FALSE) {

				existing.SetAt(data, data);

				if(theApp.TextureDumpFile != 0) 
					fprintf(theApp.TextureDumpFile, "%s,%s\n", theApp.Filename, data);
				TRACE("%s,%s\n", theApp.Filename, data);
			}
		}
	}
#endif
	if((item->Type != 0) && item->Type->Wrapper) {
		while(cload.Open_Chunk()) {
			Add_Chunk(cload, item);
			cload.Close_Chunk();
		}
	}
}


void ChunkData::Release_Data()
{
	while(Chunks.IsEmpty() == 0) {
		ChunkItem *item = Chunks.RemoveHead();
		delete item;
	}
}



int Get_Bit(void const * array, int bit)
{
	unsigned char mask = (unsigned char)(1 << (bit % 8));
	return((*((unsigned char *)array + (bit/8)) & mask) != 0);
}

void Set_Bit(void * array, int bit, int value)
{
	unsigned char mask = (unsigned char)(1 << (bit % 8));

	if (value != 0) {
		*((unsigned char *)array + (bit/8)) |= mask;
	} else {
		*((unsigned char *)array + (bit/8)) &= (unsigned char)~mask;
	}
}

