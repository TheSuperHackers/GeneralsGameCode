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

#include "statistics.h"
#include "wwstring.h"
#include "simplevec.h"
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "dx8caps.h"
#include "textureloader.h"
#include "texture.h"
#include <Utility/stdio_adapter.h>

#include <memory.h>
#ifdef _UNIX
#include "osdep.h"
#endif

// ----------------------------------------------------------------------------
//
// Texture memory tracking system
//
// ----------------------------------------------------------------------------

static int texture_memory;
static int texture_count;
static int lightmap_texture_memory;
static int lightmap_texture_count;
static int procedural_texture_memory;
static int procedural_texture_count;
static int record_count;
static int texture_change_count;
static int last_frame_texture_memory;
static int last_frame_texture_count;
static int last_frame_lightmap_texture_memory;
static int last_frame_lightmap_texture_count;
static int last_frame_procedural_texture_memory;
static int last_frame_procedural_texture_count;
static int last_frame_record_count;
static int last_frame_texture_change_count;
static TextureClass* latest_texture;
static Debug_Statistics::RecordTextureMode record_texture_mode;
static StringClass texture_statistics_string;

struct TextureStatisticsStruct
{
	TextureClass* tex;
	int usage_count;
	int change_count;
};

static SimpleDynVecClass<TextureStatisticsStruct> texture_statistics;

static void Record_Texture_Begin()
{
	texture_memory=0;
	texture_count=0;
	lightmap_texture_memory=0;
	lightmap_texture_count=0;
	procedural_texture_memory=0;
	procedural_texture_count=0;
	record_count=0;
	texture_change_count=0;
	latest_texture=NULL;
	texture_statistics.Resize(0);
}

static void Record_Texture_End()
{
	last_frame_texture_memory=texture_memory;
	last_frame_texture_count=texture_count;
	last_frame_lightmap_texture_memory=lightmap_texture_memory;
	last_frame_lightmap_texture_count=lightmap_texture_count;
	last_frame_procedural_texture_memory=procedural_texture_memory;
	last_frame_procedural_texture_count=procedural_texture_count;
	last_frame_record_count=record_count;
	last_frame_texture_change_count=texture_change_count;

	texture_statistics_string="";
	if (record_texture_mode==Debug_Statistics::RECORD_TEXTURE_DETAILS) {
		char tmp_text[1024];
		snprintf(tmp_text,sizeof(tmp_text),
			"Set_DX8_Texture count: %d\nactual changes: %d\n\n"
			"id      refs changes  size      name\n"
			"--------------------------------------\n",

			last_frame_record_count,last_frame_texture_change_count);
		texture_statistics_string+=tmp_text;
		for (int a=0;a<texture_count;++a) {
			StringClass working_string;
			TextureClass* t=texture_statistics[a].tex;

			int id=0;
			StringClass flash="  ";
//			if (t && t->getClassID() == ID_TEXTURE_FILE_CLASS) {
//				TextureFileClass* tfc=static_cast<TextureFileClass*>(t);
//				id=tfc->ID();
//				if (tfc->Get_Texture_Flash()) flash="F";
//			}

			working_string.Format("%4.4d  %3.3d   %3.3d     %s ",id,texture_statistics[a].usage_count,texture_statistics[a].change_count,flash.str());
			texture_statistics_string+=working_string;

			StringClass error="";
			if (t) {
//				int red_factor=t->Get_Current_Reduction_Factor();
				unsigned bytes=t->Get_Texture_Memory_Usage();
//				unsigned non_red_bytes=t->Get_Non_Reduced_Texture_Memory_Usage();
				if (!t->Is_Initialized()) {
//					non_red_bytes=bytes;
					texture_statistics_string+="*";
				}
				else {
					texture_statistics_string+="  ";
				}
//				if (!non_red_bytes) non_red_bytes=1;
//				int percents=100-100*bytes/non_red_bytes;
				working_string.Format("%4.4dkb         ",bytes/1024);
				texture_statistics_string+=working_string;
			}
			else {
				texture_statistics_string+="N/A  ";
			}
			texture_statistics_string+=t->Get_Texture_Name();//getTextureName();
			texture_statistics_string+=error;
			texture_statistics_string+="\n";
		}
		texture_statistics_string+="\nid              = id of texture. Use with command 'flash_texture [id]'\n";
		texture_statistics_string+="refs          = # of times texture is used when rendering\n";
		texture_statistics_string+="changes    = # of times texture change needed - BAD IF HIGH!\n";
		texture_statistics_string+="red         = texture reduction factor\n";
		texture_statistics_string+="size          = amount of memory needed for texture\n";
		texture_statistics_string+="(w/o red)     = size of reduction not used\n";
		texture_statistics_string+="percent    = savings of reduction system, in percents\n";
		texture_statistics_string+="\n* = thumbnail used\n";
		texture_statistics_string+="\n";
//		texture_statistics_string+=TextureFileClass::List_Missing_Files();

	}

}

void Debug_Statistics::Record_Texture_Mode(RecordTextureMode mode)
{
	record_texture_mode=mode;
}

Debug_Statistics::RecordTextureMode Debug_Statistics::Get_Record_Texture_Mode()
{
	return record_texture_mode;
}

static bool Find_Record_Texture(TextureClass* t)
{
	for (int a=0;a<texture_count;++a) {
		if (texture_statistics[a].tex==t) {
			if (record_texture_mode==Debug_Statistics::RECORD_TEXTURE_DETAILS) {
				texture_statistics[a].usage_count++;
				if (t!=latest_texture) {
					texture_statistics[a].change_count++;
				}
			}
		
			return true;
		}
	}
	return false;
}

static void Add_Record_Texture(TextureClass* t)
{
	TextureStatisticsStruct tss;
	tss.tex=t;
	tss.usage_count=1;
	tss.change_count=1;
	texture_statistics.Add(tss);

	texture_count++;
	if (t->Is_Lightmap()) lightmap_texture_count++;
	if (t->Is_Procedural()) procedural_texture_count++;
}

void Debug_Statistics::Record_Texture(TextureClass* t)
{
	record_count++;
	if (t!=latest_texture) {
		texture_change_count++;
	}

	if (record_texture_mode==RECORD_TEXTURE_NONE) {
		latest_texture=t;
		return;
	}
	if (!t) {
		latest_texture=t;
		return;
	}

	if (Find_Record_Texture(t)) {
		latest_texture=t;
		return;
	}
	Add_Record_Texture(t);

	texture_memory+=t->Get_Texture_Memory_Usage();
	latest_texture=t;
	if (t->Is_Lightmap()) lightmap_texture_memory+=t->Get_Texture_Memory_Usage();
	if (t->Is_Procedural()) procedural_texture_memory+=t->Get_Texture_Memory_Usage();

}

int Debug_Statistics::Get_Record_Texture_Size()	// Return total textures used during latest frame, in bytes
{
	return last_frame_texture_memory;
}

int Debug_Statistics::Get_Record_Texture_Count()	// Return total textures used during latest frame, in bytes
{
	return last_frame_texture_count;
}

int Debug_Statistics::Get_Record_Texture_Change_Count()
{
	return last_frame_texture_change_count;
}

int Debug_Statistics::Get_Record_Lightmap_Texture_Size()	// Return total lightmap textures used during latest frame, in bytes
{
	return last_frame_lightmap_texture_memory;
}

int Debug_Statistics::Get_Record_Lightmap_Texture_Count()	// Return total lightmap textures used during latest frame, in bytes
{
	return last_frame_lightmap_texture_count;
}

int Debug_Statistics::Get_Record_Procedural_Texture_Size()	// Return total procedural textures used during latest frame, in bytes
{
	return last_frame_procedural_texture_memory;
}

int Debug_Statistics::Get_Record_Procedural_Texture_Count()	// Return total procedural textures used during latest frame, in bytes
{
	return last_frame_procedural_texture_count;
}

const StringClass& Debug_Statistics::Get_Record_Texture_String()
{
	return texture_statistics_string;
}

// ----------------------------------------------------------------------------

static int dx8_skin_renders;
static int last_frame_dx8_skin_renders;
static int dx8_skin_polygons;
static int last_frame_dx8_skin_polygons;
static int dx8_skin_vertices;
static int last_frame_dx8_skin_vertices;
static int dx8_polygons;
static int last_frame_dx8_polygons;
static int dx8_vertices;
static int last_frame_dx8_vertices;
static int sorting_polygons;
static int last_frame_sorting_polygons;
static int sorting_vertices;
static int last_frame_sorting_vertices;
static int draw_calls;
static int last_frame_draw_calls;

void Debug_Statistics::Record_DX8_Skin_Polys_And_Vertices(int pcount,int vcount)
{
	dx8_skin_polygons+=pcount;
	dx8_skin_vertices+=vcount;
	dx8_skin_renders++;
	draw_calls++;
}

void Debug_Statistics::Record_DX8_Polys_And_Vertices(int pcount,int vcount,const ShaderClass& shader)
{
	if (shader.Get_NPatch_Enable()==ShaderClass::NPATCH_ENABLE && DX8Wrapper::Get_Current_Caps()->Support_NPatches()) {
		unsigned level=WW3D::Get_NPatches_Level();
		level*=level;
		pcount*=level;
	}
	dx8_polygons+=pcount;
	dx8_vertices+=vcount;
	draw_calls++;
}

int Debug_Statistics::Get_DX8_Skin_Renders()
{
	return last_frame_dx8_skin_renders;
}

int Debug_Statistics::Get_DX8_Skin_Polygons()
{
	return last_frame_dx8_skin_polygons;
}

int Debug_Statistics::Get_DX8_Skin_Vertices()
{
	return last_frame_dx8_skin_vertices;
}

int Debug_Statistics::Get_DX8_Polygons()
{
	return last_frame_dx8_polygons;
}

int Debug_Statistics::Get_DX8_Vertices()
{
	return last_frame_dx8_vertices;
}

void Debug_Statistics::Record_Sorting_Polys_And_Vertices(int pcount,int vcount)
{
	sorting_polygons+=pcount;
	sorting_vertices+=vcount;
	draw_calls++;
}

int Debug_Statistics::Get_Sorting_Polygons()
{
	return last_frame_sorting_polygons;
}

int Debug_Statistics::Get_Sorting_Vertices()
{
	return last_frame_sorting_vertices;
}

int Debug_Statistics::Get_Draw_Calls()
{
	return last_frame_draw_calls;
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void Debug_Statistics::Begin_Statistics()
{
	dx8_polygons=0;
	dx8_vertices=0;
	dx8_skin_polygons=0;
	dx8_skin_vertices=0;
	dx8_skin_renders=0;
	sorting_polygons=0;
	sorting_vertices=0;
	draw_calls=0;
	Record_Texture_Begin();
	DX8Wrapper::Begin_Statistics();
//	DX8MeshRendererClass::Begin_Statistics();
}

void Debug_Statistics::End_Statistics()
{
	Record_Texture_End();
	last_frame_dx8_skin_polygons=dx8_skin_polygons;
	last_frame_dx8_skin_vertices=dx8_skin_vertices;
	last_frame_dx8_skin_renders=dx8_skin_renders;
	last_frame_dx8_polygons=dx8_polygons;
	last_frame_dx8_vertices=dx8_vertices;
	last_frame_sorting_polygons=sorting_polygons;
	last_frame_sorting_vertices=sorting_vertices;
	last_frame_draw_calls=draw_calls;
//	DX8MeshRendererClass::End_Statistics();
	DX8Wrapper::End_Statistics();
}

void Debug_Statistics::Shutdown_Statistics()
{
	texture_statistics_string.Release_Resources();
}
// ----------------------------------------------------------------------------

