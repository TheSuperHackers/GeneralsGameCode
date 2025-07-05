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

/* $Header: /Commando/Code/ww3d2/assetmgr.cpp 43    11/01/01 1:11a Jani_p $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/ww3d2/assetmgr.cpp                           $* 
 *                                                                                             * 
 *                   Org Author:: Greg_h                                                       * 
 *                                                                                             * 
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 08/05/02 10:14a                                              $*
 *                                                                                             * 
 *                    $Revision:: 46                                                          $* 
 *                                                                                             * 
 * 06/27/02 KM Texture class abstraction																			*
 * 07/01/02 KM Shader library integration
 * 08/05/02 KM Texture class redesign (revisited)
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   WW3DAssetManager::WW3DAssetManager -- Constructor                                         *
 *   WW3DAssetManager::~WW3DAssetManager -- Destructor                                         *
 *   WW3DAssetManager::Free -- free all memory (un-needed?)                                    *
 *   WW3DAssetManager::Free_Assets -- Release all loaded assets                                *
 *   WW3DAssetManager::Load_3D_Assets -- Load 3D assets from a .W3D file                       *
 *   WW3DAssetManager::Load_Prototype -- loads a prototype from a W3D chunk                    *
 *   WW3DAssetManager::Create_Render_Obj -- Create a render object for the user                *
 *   WW3DAssetManager::Render_Obj_Exists -- Check whether a render object with the given name  *
 *   WW3DAssetManager::Create_Render_Obj_Iterator -- Create an iterator which can enumerate al *
 *   WW3DAssetManager::Release_Render_Obj_Iterator -- release a render object iterator         *
 *   WW3DAssetManager::Create_HAnim_Iterator -- Creates an HAnim Iterator                      *
 *   WW3DAssetManager::Create_HTree_Iterator -- creates an htree iterator                      *
 *   WW3DAssetManager::Create_Material_Iterator -- Create a material iterator                  *
 *   WW3DAssetManager::Create_Font3DData_Iterator -- Create a Font3DData iterator              *
 *   WW3DAssetManager::Get_HAnim -- Returns a pointer to a names HAnim                         *
 *   WW3DAssetManager::Get_HTree -- Returns a pointer to the named HTree                       *
 *   WW3DAssetManager::Get_Material -- Gets a pointer to a loaded material or creates it       *
 *   WW3DAssetManager::Get_Material -- Gets a pointer to a loaded material or creates the mate *
 *   WW3DAssetManager::Get_Material -- Gets a pointer to a loaded material or creates it       *
 *   WW3DAssetManager::Get_Font3DInstance -- Creates a pointer to a Font3DInstance             *
 *   WW3DAssetManager::Get_Font3DData -- Gets a pointer to a loaded Font3DData or creates it   *
 *   WW3DAssetManager::Release_Material -- Release a material                                  *
 *   WW3DAssetManager::Release_Font3DData -- Release a Font3DData	                             *
 *   WW3DAssetManager::Add_Material -- Add a material to the list                              *
 *   WW3DAssetManager::Add_Font3DData -- Add a Font3DData to the list                          *
 *   WW3DAssetManager::Get_Texture -- get a TextureClass for the specified targa               *
 *   WW3DAssetManager::Release_All_Textures -- release all textures in the system              *
 *   WW3DAssetManager::Register_Prototype_Loader -- add a new loader to the system             *
 *   WW3DAssetManager::Find_Prototype_Loader -- find the loader that handles this chunk type   *
 *   WW3DAssetManager::Add_Prototype -- adds the prototype to the hash table                   *
 *   WW3DAssetManager::Find_Prototype -- searches the hash table for the prototype             *
 *   WW3DAssetManager::Open_Texture_File_Cache -- Turn on the texture cache system.            * 
 *   WW3DAssetManager::Close_Texture_File_Cache -- Turn off the texture cache system.          * 
 *   CachedTextureFileClass::getMipmapData -- get data for texture - check to see if in cache. * 
 *   CachedTextureFileClass::getMipmapLevelPartial -- not yet implemented                      * 
 *   CachedTextureFileClass::setupDefaultValues -- loads texture in to get default data.       * 
 *   WW3DAssetManager::Get_Streaming_Texture -- Gets a streaming texture.                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "assetmgr.h"
#include <assert.h>

#include "bittype.h"
#include "chunkio.h"
#include "realcrc.h"

#include "wwdebug.h"

#include "htreemgr.h"
#include "hanimmgr.h"
#include "texture.h"
#include "font3d.h"
#include "render2dsentence.h"		// for FontCharsClass
#include "proto.h"
#include "hanim.h"
#include "hcanim.h"
#include "htree.h"
#include "collect.h"
#include "ww3d.h"
#include "ffactory.h"
#include "boxrobj.h"
#include "nullrobj.h"
#include "distlod.h"
#include "hlod.h"
#include "agg_def.h"
#include "wwstring.h"
#include "wwmemlog.h"
#include "dazzle.h"
#include "dx8wrapper.h"
#include "dx8renderer.h"
#include "metalmap.h"
#include "w3dexclusionlist.h"
#include <INI.H>
#include <windows.h>
#include <stdio.h>
#include <d3dx8core.h>
#include "texture.h"
#include "wwprofile.h"
#include "assetstatus.h"
#include "ringobj.h"
#include "sphereobj.h"

#include "shdlib.h"

/*
** Static member variable which keeps track of the single instanced asset manager
*/
WW3DAssetManager *		WW3DAssetManager::TheInstance = NULL;

/*
** Static instance of the Null prototype.  This render object is special cased
** to always be available...
*/
static NullPrototypeClass _NullPrototype;

/*
** Iterator for the Render Objects in the asset manager
*/
class RObjIterator : public RenderObjIterator
{
public:
	virtual bool					Is_Done(void);
	virtual const char *			Current_Item_Name(void);
	virtual int						Current_Item_Class_ID(void);
protected:
	friend class WW3DAssetManager;
};


/*
** Iterators for the other types of 3D assets:
** HAnims, HTrees, Textures, Fonts
*/
class HAnimIterator : public AssetIterator
{
public:
	HAnimIterator(void) : Iterator( WW3DAssetManager::Get_Instance()->HAnimManager ) { };

	virtual void			First(void) { Iterator.First(); }
	virtual void			Next(void)	{ Iterator.Next(); }
	virtual bool			Is_Done(void) { return Iterator.Is_Done(); }
	virtual const char *	Current_Item_Name(void) { return Iterator.Get_Current_Anim()->Get_Name(); }

protected:
	HAnimManagerIterator	Iterator;
	friend class WW3DAssetManager;
};

class HTreeIterator : public AssetIterator
{
public:
	virtual bool					Is_Done(void);
	virtual const char *			Current_Item_Name(void);
protected:
	friend class WW3DAssetManager;
};

class Font3DDataIterator : public AssetIterator
{
public:

	virtual void					First(void) { Node = WW3DAssetManager::Get_Instance()->Font3DDatas.Head(); }
	virtual void					Next(void)	{ Node = Node->Next(); }
	virtual bool					Is_Done(void) { return Node==NULL; }
	virtual const char *			Current_Item_Name(void) { return Node->Data()->Name; }

protected:

	Font3DDataIterator(void)	{ Node = WW3DAssetManager::Get_Instance()->Font3DDatas.Head(); }
	SLNode<Font3DDataClass> *		Node;
	friend class WW3DAssetManager;
};

/***********************************************************************************************
 * WW3DAssetManager::WW3DAssetManager -- Constructor                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *   05/10/1999 SKB : Add TextureCache                                                         * 
 *=============================================================================================*/
WW3DAssetManager::WW3DAssetManager(void) :
	PrototypeLoaders		(PROTOLOADERS_VECTOR_SIZE),
	Prototypes				(PROTOTYPES_VECTOR_SIZE),

	WW3D_Load_On_Demand		(false),
	Activate_Fog_On_Load		(false),
	MetalManager(0)
{
	assert(TheInstance == NULL);
	TheInstance = this;

	// set the growth rates
	PrototypeLoaders.Set_Growth_Step(PROTOLOADERS_GROWTH_RATE);
	Prototypes.Set_Growth_Step(PROTOTYPES_GROWTH_RATE);

	// install the default loaders
#ifndef USE_WWSHADE
	Register_Prototype_Loader(&_MeshLoader);
#endif

	Register_Prototype_Loader(&_HModelLoader);
	Register_Prototype_Loader(&_CollectionLoader);
	Register_Prototype_Loader(&_BoxLoader);
	Register_Prototype_Loader(&_HLodLoader);
	Register_Prototype_Loader(&_DistLODLoader);
	Register_Prototype_Loader(&_AggregateLoader);
	Register_Prototype_Loader(&_NullLoader);
	Register_Prototype_Loader(&_DazzleLoader);
	Register_Prototype_Loader (&_RingLoader);
	Register_Prototype_Loader (&_SphereLoader);

	SHD_REG_LOADER;
	
	// allocate the hash table and clear it.
	PrototypeHashTable = W3DNEWARRAY PrototypeClass * [PROTOTYPE_HASH_TABLE_SIZE];
	memset(PrototypeHashTable,0,sizeof(PrototypeClass *) * PROTOTYPE_HASH_TABLE_SIZE);		
}


/***********************************************************************************************
 * WW3DAssetManager::~WW3DAssetManager -- Destructor                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DAssetManager::~WW3DAssetManager(void)
{
	if (MetalManager) delete MetalManager;
	Free();
	TheInstance = NULL;

	// If we need to, free the hash table
	if (PrototypeHashTable != NULL) {
		delete [] PrototypeHashTable;
		PrototypeHashTable = NULL;
	}
}

static void Create_Number_String(StringClass& number, unsigned value)
{
	unsigned miljoonat=value/(1024*1028);
	unsigned tuhannet=(value/1024)%1024;
	unsigned ykkoset=value%1024;
	if (miljoonat) {
		number.Format("%d %3.3d %3.3d",miljoonat,tuhannet,ykkoset);
	}
	else if (tuhannet) {
		number.Format("%d %3.3d",tuhannet,ykkoset);
	}
	else {
		number.Format("%d",ykkoset);
	}
}

void	WW3DAssetManager::Load_Procedural_Textures()
{
	int i,count;
	if (!MetalManager)
	{
		INIClass ini;
		ini.Load("metals.ini");
		MetalManager=W3DNEW MetalMapManagerClass(ini);
	}
	
	count=MetalManager->Metal_Map_Count();		
	for (i=0; i<count; i++)
	{
		TextureClass *tex=MetalManager->Get_Metal_Map(i);
		TextureHash.Insert(tex->Get_Texture_Name(),tex);
	}	
}

static void Log_Textures(bool inited,unsigned& total_count, unsigned& total_mem)
{
	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	for (ite.First();!ite.Is_Done();ite.Next()) {
		TextureClass * tex=ite.Peek_Value();
		if (tex->Is_Initialized()!=inited) continue;

		D3DSURFACE_DESC desc;
		IDirect3DTexture8* d3d_texture=tex->Peek_D3D_Texture();
		if (!d3d_texture) continue;
		DX8_ErrorCode(d3d_texture->GetLevelDesc(0,&desc));

		StringClass tex_format="Unknown";
		switch (desc.Format) {
		case D3DFMT_A8R8G8B8: tex_format="D3DFMT_A8R8G8B8"; break;
		case D3DFMT_R8G8B8: tex_format="D3DFMT_R8G8B8"; break;
		case D3DFMT_A4R4G4B4: tex_format="D3DFMT_A4R4G4B4"; break;
		case D3DFMT_A1R5G5B5: tex_format="D3DFMT_A1R5G5B5"; break;
		case D3DFMT_R5G6B5: tex_format="D3DFMT_R5G6B5"; break;
		case D3DFMT_L8: tex_format="D3DFMT_L8"; break;
		case D3DFMT_A8: tex_format="D3DFMT_A8"; break;
		case D3DFMT_P8: tex_format="D3DFMT_P8"; break;
		case D3DFMT_X8R8G8B8: tex_format="D3DFMT_X8R8G8B8"; break;
		case D3DFMT_X1R5G5B5: tex_format="D3DFMT_X1R5G5B5"; break;
		case D3DFMT_R3G3B2: tex_format="D3DFMT_R3G3B2"; break;
		case D3DFMT_A8R3G3B2: tex_format="D3DFMT_A8R3G3B2"; break;
		case D3DFMT_X4R4G4B4: tex_format="D3DFMT_X4R4G4B4"; break;
		case D3DFMT_A8P8: tex_format="D3DFMT_A8P8"; break;
		case D3DFMT_A8L8: tex_format="D3DFMT_A8L8"; break;
		case D3DFMT_A4L4: tex_format="D3DFMT_A4L4"; break;
		case D3DFMT_V8U8: tex_format="D3DFMT_V8U8"; break;
		case D3DFMT_L6V5U5: tex_format="D3DFMT_L6V5U5"; break;  
		case D3DFMT_X8L8V8U8: tex_format="D3DFMT_X8L8V8U8"; break;
		case D3DFMT_Q8W8V8U8: tex_format="D3DFMT_Q8W8V8U8"; break;
		case D3DFMT_V16U16: tex_format="D3DFMT_V16U16"; break;
		case D3DFMT_W11V11U10: tex_format="D3DFMT_W11V11U10"; break;
		case D3DFMT_UYVY: tex_format="D3DFMT_UYVY"; break;
		case D3DFMT_YUY2: tex_format="D3DFMT_YUY2"; break;
		case D3DFMT_DXT1: tex_format="D3DFMT_DXT1"; break;
		case D3DFMT_DXT2: tex_format="D3DFMT_DXT2"; break;
		case D3DFMT_DXT3: tex_format="D3DFMT_DXT3"; break;
		case D3DFMT_DXT4: tex_format="D3DFMT_DXT4"; break;
		case D3DFMT_DXT5: tex_format="D3DFMT_DXT5"; break;
		case D3DFMT_D16_LOCKABLE: tex_format="D3DFMT_D16_LOCKABLE"; break;
		case D3DFMT_D32: tex_format="D3DFMT_D32"; break;
		case D3DFMT_D15S1: tex_format="D3DFMT_D15S1"; break;
		case D3DFMT_D24S8: tex_format="D3DFMT_D24S8"; break;
		case D3DFMT_D16: tex_format="D3DFMT_D16"; break;
		case D3DFMT_D24X8: tex_format="D3DFMT_D24X8"; break;
		case D3DFMT_D24X4S4: tex_format="D3DFMT_D24X4S4"; break;
		default:	break;
		}

		unsigned texmem=tex->Get_Texture_Memory_Usage();
		total_mem+=texmem;
		total_count++;
		StringClass number;
		Create_Number_String(number,texmem);

		WWDEBUG_SAY(("%32s	%4d * %4d (%15s), init %d, size: %14s bytes, refs: %d",
			tex->Get_Texture_Name().str(),
			desc.Width,
			desc.Height,
			tex_format.str(),
			tex->Is_Initialized(),
			number.str(),
			tex->Num_Refs()));

	}	
}

void WW3DAssetManager::Log_Texture_Statistics()
{
	unsigned total_initialized_tex_mem=0;
	unsigned total_uninitialized_tex_mem=0;
	unsigned total_initialized_count=0;
	unsigned total_uninitialized_count=0;
	StringClass number;

	WWDEBUG_SAY(("\nInitialized textures ------------------------------------------\n"));
	Log_Textures(true,total_initialized_count,total_initialized_tex_mem);

	Create_Number_String(number,total_initialized_tex_mem);
	WWDEBUG_SAY(("\n%d initialized textures, totalling %14s bytes\n",
		total_initialized_count,
		number.str()));

	WWDEBUG_SAY(("\nUn-initialized textures ---------------------------------------\n"));
	Log_Textures(false,total_uninitialized_count,total_uninitialized_tex_mem);

	Create_Number_String(number,total_uninitialized_tex_mem);
	WWDEBUG_SAY(("\n%d un-initialized textures, totalling, totalling %14s bytes\n",
		total_uninitialized_count,
		number.str()));
/*
	RenderObjIterator * rite=WW3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator();
	if (rite) {
		for (rite->First(); !rite->Is_Done(); rite->Next()) {
//			RenderObjClass * robj=Create_Render_Obj(rite->Current_Item_Name());	
//			if (robj) {
//
//				robj->Release_Ref();
//			}
			if (rite->Current_Item_Class_ID()==RenderObjClass::CLASSID_HMODEL) {
				WWDEBUG_SAY(("robj: %s",rite->Current_Item_Name()));
			}
		}

		WW3DAssetManager::Get_Instance()->Release_Render_Obj_Iterator(rite);
	}
*/
}

/***********************************************************************************************
 * WW3DAssetManager::Free -- free all memory (un-needed?)                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Free(void)
{
	Free_Assets();
}


/***********************************************************************************************
 * WW3DAssetManager::Free_Assets -- Release all loaded assets                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *   05/10/1999 SKB : Close down texture cache file.                                           * 
 *=============================================================================================*/
void WW3DAssetManager::Free_Assets(void)
{
	WWPROFILE( "WW3DAssetManager::Free_Assets" );

	// delete all of the prototypes
	int count = Prototypes.Count();
	while (count-- > 0) {

		PrototypeClass * proto = Prototypes[count];
		Prototypes.Delete(count);

		if (proto != NULL) {
			proto->DeleteSelf();
		}
	}
	
	// clear the prototype hash table
	memset(PrototypeHashTable,0,sizeof(PrototypeClass *) * PROTOTYPE_HASH_TABLE_SIZE);	

	// delete all of the anims and trees
	HAnimManager.Free_All_Anims();
	HTreeManager.Free_All_Trees();

	// release all my references to the materials
	Release_All_Textures();
	Release_All_Font3DDatas();
	Release_All_FontChars();

	// Close down cache if it is open.
	// NONONONOO.... Don't close it as we might want to free the assets and still be able to load textures.
//	Close_Texture_File_Cache();
}


/***********************************************************************************************
 * WW3DAssetManager::Free_Unused_Assets -- Release all assets that are referenced only by      *
 *                                         the asset manager.                                  *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/18/99   EHC : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Release_Unused_Assets(void)
{
	// release all references to objects that have only one reference on them
	// and remove them from our lists.
	Release_Unused_Textures();
	Release_Unused_Font3DDatas();
}

/***********************************************************************************************
 * WW3DAssetManager::Free_Assets_With_Exclusion_List -- Release all assets that are not named  *
 *                                                      in the exclusion list.                 *
 *                                                                                             *
 * This function checks if each prototype is named or is a child of something named in the     *
 * given exclusion list.                                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 * exclusion_list - list of names of render object prototypes to not release.                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/12/2002 GH  : Created.                                                                 * 
 *=============================================================================================*/
void WW3DAssetManager::Free_Assets_With_Exclusion_List(const DynamicVectorClass<StringClass> & exclusion_names)
{
	// Reset the dx8 mesh renderer
	TheDX8MeshRenderer.Invalidate();

	// Build an exclusion list object that will do the real filtering work for us
	W3DExclusionListClass exclusion_list(exclusion_names);
   
	// temporary vector to hold the prototypes that get excluded from being deleted.
	// grow by the initial size so we don't waste lots of time re-allocating!
	const int DEFAULT_EXCLUDE_ARRAY_SIZE = 8000;
	DynamicVectorClass<PrototypeClass *> exclude_array(DEFAULT_EXCLUDE_ARRAY_SIZE);
	exclude_array.Set_Growth_Step(DEFAULT_EXCLUDE_ARRAY_SIZE);

	// iterate the array of prototypes saving each one that should be excluded from deletion
	int i=0;
	for (; i<Prototypes.Count(); i++) {

		PrototypeClass * proto = Prototypes[i];
		if (proto != NULL) {		

			// If this prototype is excluded, copy the pointer, otherwise delete it.
			if (exclusion_list.Is_Excluded(proto)) {
				//WWDEBUG_SAY(("excluding %s",proto->Get_Name()));
				exclude_array.Add(proto);
			} else {
				//WWDEBUG_SAY(("deleting %s",proto->Get_Name()));
				proto->DeleteSelf();
			}
			Prototypes[i] = NULL;
		}
	}
	
	// reset the array now that we've gotten rid of everything.
	Prototypes.Reset_Active();

	// clear the prototype hash table
	memset(PrototypeHashTable,0,sizeof(PrototypeClass *) * PROTOTYPE_HASH_TABLE_SIZE);	

	// re-add the prototypes that we saved
	for (i=0; i<exclude_array.Count(); i++) {
		Add_Prototype(exclude_array[i]);
	}

	// delete all of the anims and trees
	HAnimManager.Free_All_Anims_With_Exclusion_List(exclusion_list);
	HTreeManager.Free_All_Trees_With_Exclusion_List(exclusion_list);

	// release references to textures that are not used
	Release_Unused_Textures();

}

/***********************************************************************************************
 * WW3DAssetManager::Create_Asset_List -- Create a list of the W3D files that are loaded       *
 *                                                                                             *
 * This function checks if each prototype is named or is a child of something named in the     *
 * given exclusion list.                                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 * model_list - dynamic vector to populate with names of w3d files loaded                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * As in other places in the code, we are assuming that w3d filenames match the "top-level"    *
 * render object contained within them!                                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/12/2002 GH  : Created.                                                                 * 
 *=============================================================================================*/
void WW3DAssetManager::Create_Asset_List(DynamicVectorClass<StringClass> & model_list)
{
	for (int i=0; i<Prototypes.Count(); i++) 	{
		// ok, we ignore all of the following:
		// - sub objects, these will have a '.' in their name
		// - munged objects, these will have # characters in their name
		PrototypeClass * proto = Prototypes[i];
		if (proto) {
			const char * name = proto->Get_Name();

			if ((strchr(name,'#') == NULL) && (strchr(name,'.') == NULL)) {
				model_list.Add(StringClass(name));
			}
		}
	}

	// Add in the w3d files for all of the animations
	HAnimManager.Create_Asset_List(model_list);
}


/***********************************************************************************************
 * WW3DAssetManager::Load_3D_Assets -- Load 3D assets from a file                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/22/98   BMG : Created.                                                                 *
 *=============================================================================================*/
bool WW3DAssetManager::Load_3D_Assets( const char * filename )
{
	bool result = false;

	FileClass * file = _TheFileFactory->Get_File( filename );
	if ( file ) {
		if ( file->Is_Available() ) {
			result = WW3DAssetManager::Load_3D_Assets( *file );
		} else {
			WWDEBUG_SAY(("Missing asset '%s'.", filename));
		}
		_TheFileFactory->Return_File( file );
	}

	return result;
}


/***********************************************************************************************
 * WW3DAssetManager::Load_3D_Assets -- Load 3D assets from a .W3D file                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
bool WW3DAssetManager::Load_3D_Assets(FileClass & w3dfile)
{
	WWPROFILE( "WW3DAssetManager::Load_3D_Assets" );
	if (!w3dfile.Open()) {
		return false;
	}

	ChunkLoadClass cload(&w3dfile);

	while (cload.Open_Chunk()) {

		switch (cload.Cur_Chunk_ID()) {

			case W3D_CHUNK_HIERARCHY:
				HTreeManager.Load_Tree(cload);
				break;

			case W3D_CHUNK_ANIMATION:
			case W3D_CHUNK_COMPRESSED_ANIMATION:
			case W3D_CHUNK_MORPH_ANIMATION:
				HAnimManager.Load_Anim(cload);
				break;
        
			default:
				Load_Prototype(cload);
				break;
		}

		cload.Close_Chunk();
	}

	w3dfile.Close();

	return true;
}


/***********************************************************************************************
 * WW3DAssetManager::Load_Prototype -- loads a prototype from a W3D chunk                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/98    GTH : Created.                                                                 *
 *   2/19/99    EHC : Now has the Add_Prototype call responsible for adding the prototype to   *
 *                    the Prototypes list object.                                              *
 *=============================================================================================*/
bool WW3DAssetManager::Load_Prototype(ChunkLoadClass & cload)
{
	WWPROFILE( "WW3DAssetManager::Load_Prototype" );
	WWMEMLOG(MEM_GEOMETRY);

	/*
	** Get the chunk id
	*/
	int chunk_id = cload.Cur_Chunk_ID();

	/*
	** Find a loader that handles that type of chunk
	*/
	PrototypeLoaderClass * loader = Find_Prototype_Loader(chunk_id);
	PrototypeClass * newproto = NULL;

	if (loader != NULL) {

		/*
		** Ask it to create a prototype from the contents of the
		** chunk.
		*/
		newproto = loader->Load_W3D(cload);

	} else {

		/*
		** Warn user about an unknown chunk type
		*/
		WWDEBUG_SAY(("Unknown chunk type encountered!  Chunk Id = %d",chunk_id));
		return false;
	}

	/*
	** Now, see if the prototype that we loaded has a duplicate
	** name with any of our currently loaded prototypes (can't have that!)
	*/
	if (newproto != NULL) {

		if (!Render_Obj_Exists(newproto->Get_Name())) {
				
			/*
			** Add the new, unique prototype to our list
			*/
			Add_Prototype(newproto);

		} else {

			/*
			** Warn the user about a name collision with this prototype 
			** and dump it
			*/
			WWDEBUG_SAY(("Render Object Name Collision: %s",newproto->Get_Name()));
			newproto->DeleteSelf();
			newproto = NULL;
			return false;
		}
	
	} else {

		/*
		** Warn user that a prototype was not generated from this 
		** chunk type
		*/
		WWDEBUG_SAY(("Could not generate prototype!  Chunk  = %d",chunk_id));
		return false;
	}
	
	return true;
}


/***********************************************************************************************
 * WW3DAssetManager::Create_Render_Obj -- Create a render object for the user                  *
 *                                                                                             *
 *    This function will create any type of render object.  I.e. if you pass in the name       *
 *    of an HModel, it will create an hmodel for you.                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * WW3DAssetManager::Create_Render_Obj(const char * name)
{
	WWPROFILE( "WW3DAssetManager::Create_Render_Obj" );
	WWMEMLOG(MEM_GEOMETRY);

	// Try to find a prototype
	PrototypeClass * proto = Find_Prototype(name);

	if (WW3D_Load_On_Demand && proto == NULL) {	// If we didn't find one, try to load on demand
		AssetStatusClass::Peek_Instance()->Report_Load_On_Demand_RObj(name);

		char filename [MAX_PATH];
		const char *mesh_name = ::strchr (name, '.');
		if (mesh_name != NULL) {
			::lstrcpyn (filename, name, ((int)mesh_name) - ((int)name) + 1);
			::lstrcat (filename, ".w3d");
		} else {
			sprintf( filename, "%s.w3d", name);
		}

		// If we can't find it, try the parent directory
		if ( Load_3D_Assets( filename ) == false ) {
			StringClass	new_filename(StringClass("..\\"),true);
			new_filename+=filename;
			Load_3D_Assets( new_filename );
		}

		proto = Find_Prototype(name);		// try again
	}

	if (proto == NULL) {
		static int warning_count = 0;
		// Note - objects named "#..." are scaled cached objects, so don't warn...
		if (name[0] != '#') {
			if (++warning_count <= 20) {
				WWDEBUG_SAY(("WARNING: Failed to create Render Object: %s",name));
			}
			AssetStatusClass::Peek_Instance()->Report_Missing_RObj(name);
		}
		return NULL;		// Failed to find a prototype
	}

	return proto->Create();
}


/***********************************************************************************************
 * WW3DAssetManager::Render_Obj_Exists -- Check whether a render object with the given name ex *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
bool WW3DAssetManager::Render_Obj_Exists(const char * name)
{
	if (Find_Prototype(name) == NULL) return false;
	else return true;
}


/***********************************************************************************************
 * WW3DAssetManager::Create_Render_Obj_Iterator -- Create an iterator which can enumerate all  *
 *                                                                                             *
 *    The iterator returned can enumerate all of the loaded render objects for you.            *
 *    The user is responsible for releasing the iterator!                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    User must release the iterator back to the asset manager or there will be a memory leak  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjIterator * WW3DAssetManager::Create_Render_Obj_Iterator(void)
{
	return W3DNEW RObjIterator();
}


/***********************************************************************************************
 * WW3DAssetManager::Release_Render_Obj_Iterator -- release a render object iterator           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/28/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Release_Render_Obj_Iterator(RenderObjIterator * it)
{
	WWASSERT(it != NULL);
	delete it;
}

/***********************************************************************************************
 * WW3DAssetManager::Create_HAnim_Iterator -- Creates an HAnim Iterator                        *
 *                                                                                             *
 *    Creates an iterator which can enumerate each of the named hierarchical animations        *
 *    which are currently loaded.                                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    User must delete the iterator!                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
AssetIterator * WW3DAssetManager::Create_HAnim_Iterator(void)
{
	return W3DNEW HAnimIterator();
}														


/***********************************************************************************************
 * WW3DAssetManager::Create_HTree_Iterator -- creates an htree iterator                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/11/98    GTH : Created.                                                                 *
 *=============================================================================================*/
AssetIterator * WW3DAssetManager::Create_HTree_Iterator(void)
{
	return W3DNEW HTreeIterator();
}


/***********************************************************************************************
 * WW3DAssetManager::Create_Font3DData_Iterator -- Create a Font3DData iterator                *
 *                                                                                             *
 *   Creates an iterator which can enumerate each of the Font3DDatas currently loaded          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    User must delete the iterator!                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
AssetIterator * WW3DAssetManager::Create_Font3DData_Iterator(void)
{
	return W3DNEW Font3DDataIterator();
}

/***********************************************************************************************
 * WW3DAssetManager::Get_HAnim -- Returns a pointer to a names HAnim                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    The implementation has changed since its inception, the asset manager no longer owns the *
 *	   hanim's so they need to be released by the caller.													  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
HAnimClass *	WW3DAssetManager::Get_HAnim(const char * name)
{
	WWPROFILE( "WW3DAssetManager::Get_HAnim" );

	// Try to find the hanim
	HAnimClass * anim = HAnimManager.Get_Anim(name);

	if (WW3D_Load_On_Demand && anim == NULL) {	// If we didn't find it, try to load on demand
		
		if ( !HAnimManager.Is_Missing( name ) ) {	// if this is NOT a known missing anim

			AssetStatusClass::Peek_Instance()->Report_Load_On_Demand_HAnim(name);

			char filename[ MAX_PATH ];
			const char *animname = strchr( name, '.');
			if (animname != NULL) {
				sprintf( filename, "%s.w3d", animname+1);
			} else {
				WWDEBUG_SAY(( "Animation %s has no . in the name", name ));
				WWASSERT( 0 );
				return NULL;
			}

			// If we can't find it, try the parent directory
			if ( Load_3D_Assets( filename ) == false ) {
				StringClass	new_filename = StringClass("..\\") + filename;
				Load_3D_Assets( new_filename );
			}

			anim = HAnimManager.Get_Anim(name);		// Try agai
			if (anim == NULL) {
				HAnimManager.Register_Missing( name );		// This is now a KNOWN missing anim
				AssetStatusClass::Peek_Instance()->Report_Missing_HAnim(name);
			}
		}
	}

	return anim;
}										 


/***********************************************************************************************
 * WW3DAssetManager::Get_HTree -- Returns a pointer to the named HTree                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *    DO NOT DELETE the HTree you get from this function, it is "owned" by the asset manager.  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/97   GTH : Created.                                                                 *
 *=============================================================================================*/
HTreeClass *	WW3DAssetManager::Get_HTree(const char * name)
{
	WWPROFILE( "WW3DAssetManager::Get_HTree" );

	// Try to find the htree
	HTreeClass * htree = HTreeManager.Get_Tree(name);

	if (WW3D_Load_On_Demand && htree == NULL) {	// If we didn't find it, try to load on demand
		
		AssetStatusClass::Peek_Instance()->Report_Load_On_Demand_HTree(name);

		char filename[ MAX_PATH ];
		sprintf( filename, "%s.w3d", name);

		// If we can't find it, try the parent directory
		if ( Load_3D_Assets( filename ) == false ) {
			StringClass	new_filename("..\\",true);
			new_filename+=filename;
			Load_3D_Assets( new_filename );
		}

		htree = HTreeManager.Get_Tree(name);	// Try again

		if (htree == NULL) {
			AssetStatusClass::Peek_Instance()->Report_Missing_HTree(name);
		}
	}

	return htree;
}

/***********************************************************************************************
 * WW3DAssetManager::Get_Texture -- get a TextureClass from the specified file                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/31/2001  NH : Created.                                                                  *
 *=============================================================================================*/
TextureClass * WW3DAssetManager::Get_Texture
(
	const char * filename, 
	MipCountType mip_level_count,
	WW3DFormat texture_format,
	bool allow_compression,
	TextureBaseClass::TexAssetType type,
	bool allow_reduction
)
{
	WWPROFILE( "WW3DAssetManager::Get_Texture 1" );

	/*
	** We cannot currently mip-map bumpmaps
	*/
	if (texture_format==WW3D_FORMAT_U8V8) 
	{
		mip_level_count=MIP_LEVELS_1;
	}

	/*
	** Bail if the user isn't really asking for anything
	*/
	if ((filename == NULL) || (strlen(filename) == 0)) 
	{
		return NULL;
	}

	StringClass lower_case_name(filename,true);
	_strlwr(lower_case_name.Peek_Buffer());

	/*
	** See if the texture has already been loaded.
	*/
	TextureClass* tex = TextureHash.Get(lower_case_name);
	if (tex && (tex->Is_Initialized() == true) && (texture_format!=WW3D_FORMAT_UNKNOWN)) 
	{
		WWASSERT_PRINT(tex->Get_Texture_Format()==texture_format,("Texture %s has already been loaded with different format",filename));
	}

	/*
	** Didn't have it so we have to create a new texture
	*/
	if (!tex) 
	{
		if (type==TextureBaseClass::TEX_REGULAR)
		{
			tex = NEW_REF (TextureClass, (lower_case_name, NULL, mip_level_count, texture_format, allow_compression, allow_reduction));
		}
		else if (type==TextureBaseClass::TEX_CUBEMAP)
		{
			tex = NEW_REF (CubeTextureClass, (lower_case_name, NULL, mip_level_count, texture_format, allow_compression, allow_reduction));
		}
		else if (type==TextureBaseClass::TEX_VOLUME)
		{
			tex = NEW_REF (VolumeTextureClass, (lower_case_name, NULL, mip_level_count, texture_format, allow_compression, allow_reduction));
		}
		else
		{
			WWASSERT_PRINT(false, ("Unhandled case"));
			return NULL;
		}

		TextureHash.Insert(tex->Get_Texture_Name(),tex);
	}

	tex->Add_Ref();
	return tex;
}


/***********************************************************************************************
 * WW3DAssetManager::Release_All_Textures -- release all textures in the system                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/10/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Release_All_Textures(void)
{
	/*
	** for each texture in the list, get it and release ref it
	*/

	HashTemplateIterator<StringClass,TextureClass*> ite(TextureHash);
	for (ite.First();!ite.Is_Done();ite.Next()) {
		TextureClass * tex=ite.Peek_Value();
//		WWASSERT(tex->Num_Refs()==1);	// If asset manager is releasing the texture,
														// nobody should be referencing to it anymore!
		tex->Release_Ref();
	}
	TextureHash.Remove_All();
}


/***********************************************************************************************
 * WW3DAssetManager::Release_Unused_Textures -- release all textures with refcount == 1        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/18/99    EHC : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Release_Unused_Textures(void)
{
	/*
	** for each texture in the list, get it, check it's refcount, and and release ref it if the
	** refcount is one.
	*/

	unsigned count=0;
	TextureClass* temp_textures[256];

	HashTemplateIterator<StringClass,TextureClass*> ite(TextureHash);
	for (ite.First();!ite.Is_Done();ite.Next()) {
		TextureClass* tex=ite.Peek_Value();
		if (tex->Num_Refs() == 1) {
			temp_textures[count++]=tex;
			if (count==256) {
				for (unsigned i=0;i<256;++i) {
					TextureHash.Remove(temp_textures[i]->Get_Texture_Name());
					temp_textures[i]->Release_Ref();
				}
				count=0;
				ite.First();	// iterator doesn't support modifying the hash table while iterating, so start from the
									// beginning.
			}
		}
	}
	for (unsigned i=0;i<count;++i) {
		TextureHash.Remove(temp_textures[i]->Get_Texture_Name());
		temp_textures[i]->Release_Ref();
	}
}

/***********************************************************************************************
 * WW3DAssetManager::Release_Texture -- release a specific texture from the asset manager      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/18/99    EHC : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Release_Texture(TextureClass *tex)
{
	/*
	** Try to find the texture in the list, if found release it and remove it from the list.
	*/

	TextureHash.Remove(tex->Get_Texture_Name());
	tex->Release_Ref();
}

void WW3DAssetManager::Log_All_Textures(void)
{
	Log_Texture_Statistics();

	HashTemplateIterator<StringClass,TextureClass*> ite(TextureHash);

	// Log lightmaps -----------------------------------

	WWDEBUG_SAY((
		"Lightmap textures: %d\n\n"
		"size     name\n"
		"--------------------------------------\n"
		,
		TextureClass::_Get_Total_Lightmap_Texture_Count()));

	for (ite.First();!ite.Is_Done();ite.Next()) {
		TextureClass* t=ite.Peek_Value();
		if (!t->Is_Lightmap()) continue;

		StringClass tmp(true);
		unsigned bytes=t->Get_Texture_Memory_Usage();
		if (!t->Is_Initialized()) {
			tmp+="*";
		}
		else {
			tmp+=" ";
		}
		WWDEBUG_SAY(("%4.4dkb %s%s",bytes/1024,tmp.str(),t->Get_Texture_Name().str()));
	}

	// Log procedural textures -------------------------------

	WWDEBUG_SAY((
		"Procedural textures: %d\n\n"
		"size     name\n"
		"--------------------------------------\n"
		,
		TextureClass::_Get_Total_Procedural_Texture_Count()));

	for (ite.First();!ite.Is_Done();ite.Next()) {
		TextureClass* t=ite.Peek_Value();
		if (!t->Is_Procedural()) continue;

		StringClass tmp(true);
		unsigned bytes=t->Get_Texture_Memory_Usage();
		if (!t->Is_Initialized()) {
			tmp+="*";
		}
		else {
			tmp+=" ";
		}
		WWDEBUG_SAY(("%4.4dkb %s%s",bytes/1024,tmp.str(),t->Get_Texture_Name().str()));
	}

	// Log "ordinary" textures -------------------------------

	WWDEBUG_SAY((
		"Ordinary textures: %d\n\n"
		"size     name\n"
		"--------------------------------------\n"
		,
		TextureClass::_Get_Total_Texture_Count()-TextureClass::_Get_Total_Lightmap_Texture_Count()-TextureClass::_Get_Total_Procedural_Texture_Count()));

	for (ite.First();!ite.Is_Done();ite.Next()) {
		TextureClass* t=ite.Peek_Value();
		if (t->Is_Procedural()) continue;
		if (t->Is_Lightmap()) continue;

		StringClass tmp(true);
		unsigned bytes=t->Get_Texture_Memory_Usage();
		if (!t->Is_Initialized()) {
			tmp+="*";
		}
		else {
			tmp+=" ";
		}
		WWDEBUG_SAY(("%4.4dkb %s%s",bytes/1024,tmp.str(),t->Get_Texture_Name().str()));
	}

}



/***********************************************************************************************
 * WW3DAssetManager::Get_Font3DInstance -- Creates a pointer to a Font3DInstance					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
Font3DInstanceClass * WW3DAssetManager::Get_Font3DInstance( const char *name )
{
	WWPROFILE( "WW3DAssetManager::Get_Font3DInstance" );
	return NEW_REF( Font3DInstanceClass, ( name ));
}


/***********************************************************************************************
 * WW3DAssetManager::Get_Font3DData -- Gets a pointer to a loaded Font3DData or creates it     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
Font3DDataClass * WW3DAssetManager::Get_Font3DData( const char *name )
{
	WWPROFILE( "WW3DAssetManager::Get_Font3DData" );
	// loop through and see if the Font3D we are looking for has already been
	// allocated and thus we can just return it.
	for (	SLNode<Font3DDataClass> *node = Font3DDatas.Head(); node; node = node->Next()) {
		if (!stricmp(name, node->Data()->Name)) {
			node->Data()->Add_Ref();
			return node->Data();
		}
	}

	// if one hasn't been found and a font name has been specified then create it
	Font3DDataClass * font = NEW_REF( Font3DDataClass, ( name ));

	// add it to the asset manager
	Add_Font3DData( font);

	// return it
	return font;
}

/***********************************************************************************************
 * WW3DAssetManager::Add_Font3DData -- Add a Font3DData to the list                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Add_Font3DData(Font3DDataClass * font)
{
	font->Add_Ref();
	Font3DDatas.Add_Head(font);
}

void WW3DAssetManager::Remove_Font3DData(Font3DDataClass * font)
{
	font->Release_Ref();
	Font3DDatas.Remove(font);
}

/***********************************************************************************************
 * WW3DAssetManager::Release_All_Font3DDatas -- Release all Font3DDatas from the asset manager *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void	WW3DAssetManager::Release_All_Font3DDatas( void )
{
	// for each mat in the list, get it and release ref it
	Font3DDataClass *head;
	while ((head = Font3DDatas.Remove_Head()) != NULL )	{
		head->Release_Ref();
	}
}

/***********************************************************************************************
 * WW3DAssetManager::Release_Unused_Font3DDatas -- Release all Font3DDatas with refcount == 1  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void	WW3DAssetManager::Release_Unused_Font3DDatas( void )
{
	/*
	** for each font data in the list, get it, check it's refcount, and and release ref it if the
	** refcount is one.
	*/
	SLNode<Font3DDataClass> *node, * next;
	for (	node = Font3DDatas.Head(); node; node = next) {
		next = node->Next();
		Font3DDataClass *font = node->Data();
		if (font->Num_Refs() == 1) {
			Font3DDatas.Remove(font);
			font->Release_Ref();
		}
	}
}

/***********************************************************************************************
 * WW3DAssetManager::Get_FontChars -- Gets a pointer to a loaded FontChars or creates it       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/01     BMG : Created.                                                                 *
 *=============================================================================================*/
FontCharsClass *	WW3DAssetManager::Get_FontChars( const char * name, int point_size, bool is_bold )
{
	WWPROFILE( "WW3DAssetManager::Get_FontChars" );

	// loop through and see if we already have the font chars and we can just return it.
	for ( int i = 0; i < FontCharsList.Count(); i++ ) {
		if ( FontCharsList[i]->Is_Font( name, point_size, is_bold ) ) {
			FontCharsList[i]->Add_Ref();
			return FontCharsList[i];
		}
	}

	// If one hasn't been found, create it
	FontCharsClass * font = NEW_REF( FontCharsClass, () );
	font->Initialize_GDI_Font( name, point_size, is_bold );
	font->Add_Ref();
	FontCharsList.Add( font );			// add it to the list	
	return font;							// return it
}


/***********************************************************************************************
 * WW3DAssetManager::Release_All_FontChars -- Release all FontChars from the asset manager     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/1/01     BMG : Created.                                                                 *
 *=============================================================================================*/
void	WW3DAssetManager::Release_All_FontChars( void )
{
	// for each fontchars in the list, get it and release ref it
	while ( FontCharsList.Count() ) {
		FontCharsList[0]->Release_Ref();
		FontCharsList.Delete( 0 );
	}
}

/***********************************************************************************************
 * WW3DAssetManager::Register_Prototype_Loader -- add a new loader to the system               *
 *                                                                                             *
 *    The library will automatically install loaders for the "built-in" render object          *
 *    types.  This function exists so that the user can design App-specific render objects,    *
 *    define a chunk format for them, and have the asset manager load them in like everything  *
 *    else.                                                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *    loader - pointer to a global or static instance of your loader type.                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/28/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Register_Prototype_Loader(PrototypeLoaderClass * loader)
{
	WWASSERT(loader != NULL);
	PrototypeLoaders.Add(loader);
}


/***********************************************************************************************
 * WW3DAssetManager::Find_Prototype_Loader -- find the loader that handles this chunk type     *
 *                                                                                             *
 * INPUT:                                                                                      *
 * chunk_id - chunk type that the loader needs to handle                                       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * pointer to the appropriate loader or NULL if one wasn't found                               *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/28/98    GTH : Created.                                                                 *
 *=============================================================================================*/
PrototypeLoaderClass * WW3DAssetManager::Find_Prototype_Loader(int chunk_id)
{
	for (int i=0; i<PrototypeLoaders.Count(); i++) {
		PrototypeLoaderClass * loader = PrototypeLoaders[i];
		if (loader && loader->Chunk_Type() == chunk_id) {
			return loader;
		}
	}
	return NULL;
}


/***********************************************************************************************
 * WW3DAssetManager::Add_Prototype -- adds the prototype to the hash table                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/98    GTH : Created.                                                                 *
 *   12/8/98    GTH : Renamed to simply Add_Prototype                                          *
 *   2/19/99    EHC : Now adds the prototype to the prototype list                             *
 *=============================================================================================*/
void WW3DAssetManager::Add_Prototype(PrototypeClass * newproto)
{
	WWASSERT(newproto != NULL);
	int hash = CRC_Stringi(newproto->Get_Name()) & PROTOTYPE_HASH_MASK;
	newproto->friend_setNextHash(PrototypeHashTable[hash]);
	PrototypeHashTable[hash] = newproto;
	Prototypes.Add(newproto);
}


/***********************************************************************************************
 * WW3DAssetManager::Remove_Prototype -- Removes all references to the protype.					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/4/99	 PDS : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Remove_Prototype(PrototypeClass *proto)
{
	WWASSERT(proto != NULL);
	if (proto != NULL) {

		//
		// Find the prototype in the hash table.
		//
		const char *pname = proto->Get_Name ();
		bool bfound = false;
		PrototypeClass *prev = NULL;
		int hash = CRC_Stringi(pname) & PROTOTYPE_HASH_MASK;				
		for (PrototypeClass *test = PrototypeHashTable[hash];
			  (test != NULL) && (bfound == false);
			  test = test->friend_getNextHash()) {
			
			// Is this the prototype?
			if (::stricmp (test->Get_Name(), pname) == 0) {
				
				// Remove this prototype from the linked list for this hash index.
				if (prev == NULL) {
					PrototypeHashTable[hash] = test->friend_getNextHash();
				} else {
					prev->friend_setNextHash(test->friend_getNextHash());
				}

				// Success!
				bfound = true;
			}

			// Remember who our previous entry is
			prev = test;
		}

		// Now remove this from our vector-array of prototypes
		Prototypes.Delete (proto);
	}

	return;
}


/***********************************************************************************************
 * WW3DAssetManager::Remove_Prototype -- Removes all references to the protype.					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/4/99	 PDS : Created.                                                                 *
 *=============================================================================================*/
void WW3DAssetManager::Remove_Prototype(const char *name)
{
	WWASSERT(name != NULL);
	if (name != NULL) {

		// Lookup the prototype by name
		PrototypeClass *proto = Find_Prototype (name);
		if (proto != NULL) {

			// Remove the prototype from our lists, and free its memory
			Remove_Prototype (proto);
			proto->DeleteSelf();
		}
	}

	return;
}


/***********************************************************************************************
 * WW3DAssetManager::Find_Prototype -- searches the hash table for the prototype               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/29/98    GTH : Created.                                                                 *
 *   12/8/98    GTH : Renamed to simply Find_Prototype                                         *
 *=============================================================================================*/
PrototypeClass * WW3DAssetManager::Find_Prototype(const char * name)
{
	// Special case Null render object.  So we always have it...
	if (stricmp(name,"NULL") == 0) {
		return &(_NullPrototype);
	}
	
	// Find the prototype
	int hash = CRC_Stringi(name) & PROTOTYPE_HASH_MASK;
	PrototypeClass * test = PrototypeHashTable[hash];

	while (test != NULL) {
		if (stricmp(test->Get_Name(),name) == 0) {
			return test;
		}
		test = test->friend_getNextHash();
	}
	return NULL;
}

/*
** Iterator Implementations.
** =====================================================================
** If the user derives a custom asset manager, you will have
** to implement iterators which can walk through your datastructures.
*/

bool RObjIterator::Is_Done(void)
{
	return !(Index < WW3DAssetManager::Get_Instance()->Prototypes.Count());
}

const char * RObjIterator::Current_Item_Name(void)
{
	if (Index < WW3DAssetManager::Get_Instance()->Prototypes.Count()) {
		return WW3DAssetManager::Get_Instance()->Prototypes[Index]->Get_Name();
	} else {
		return NULL;
	}
}

int RObjIterator::Current_Item_Class_ID(void)
{
	if (Index < WW3DAssetManager::Get_Instance()->Prototypes.Count()) {
		return WW3DAssetManager::Get_Instance()->Prototypes[Index]->Get_Class_ID();
	} else {
		return -1;
	}
}

bool HTreeIterator::Is_Done(void)
{
	return !(Index < WW3DAssetManager::Get_Instance()->HTreeManager.Num_Trees());
}

const char * HTreeIterator::Current_Item_Name(void)
{
	return WW3DAssetManager::Get_Instance()->HTreeManager.Get_Tree(Index)->Get_Name();
}


