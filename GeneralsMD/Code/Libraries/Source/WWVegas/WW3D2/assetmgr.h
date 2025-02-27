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

/* $Header: /Commando/Code/ww3d2/assetmgr.h 19    12/17/01 7:55p Jani_p $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/assetmgr.h                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 12/15/01 4:14p                                              $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "Vector.h"
#include "htreemgr.h"
#include "hanimmgr.h"
#include "SLIST.h"
#include "texture.h"
#include "hashtemplate.h"
#include "simplevec.h"

class	HAnimClass;
class	HTreeClass;
class	ChunkLoadClass;

class FileClass;
class FileFactoryClass;
class PrototypeLoaderClass;
class	Font3DDataClass;
class	Font3DInstanceClass;
class	FontCharsClass;
class RenderObjClass;
class HModelClass;
class PrototypeClass;
class HTreeManagerClass;
class HAnimManagerClass;
class HAnimIterator;
class TextureIterator;
class TextureFileCache;
class StreamingTextureClass;
struct StreamingTextureConfig;
class TextureClass;
class MetalMapManagerClass;


/*
** AssetIterator
**	This object can iterate through the 3D assets which
** currently exist in the Asset Manager.  It tells you the names
** of the assets which the manager can create for you.
*/
class AssetIterator
{

public:

	virtual							~AssetIterator(void) { };
	virtual void					First(void) { Index = 0; }
	virtual void					Next(void)	{ Index ++; }
	virtual bool					Is_Done(void) = 0;
	virtual const char *			Current_Item_Name(void) = 0;

protected:

	AssetIterator(void)			{ Index = 0; }
	int								Index;
};

/*
** RenderObjIterator
** The render obj iterator simply adds a method for determining
** the class id of a render object prototype in the system.
*/
class RenderObjIterator : public AssetIterator
{
public:
	virtual int						Current_Item_Class_ID(void) = 0;
};


/*

	WW3DAssetManager

	This object is the manager of all of the 3D data.  Load your meshes, animations,
	etc etc using the Load_3D_Assets function.

	WARNING: hierarchy trees should be loaded before the meshes and animations
	which attach to them.

	-------------------------------------------------------------------------------------
	Dec 11, 1997, Asset Manager Brainstorming:

	- WW3DAssetManager will be diferentiated from other game data asset managers
	(sounds, strings, etc) because they behave differently and serve different
	purposes

	- WW3D creates "clones" from the blueprints it has of render objects whereas
	Our commando data asset manager will provide the data (file images) for the
	blueprints.  Maybe the CommandoDataManager could deal in MemoryFileClasses.
	Or void * and then the ww3d manager could convert to MemoryFiles...

	- Future caching: In the case that we want to implement a caching system,
	assets must be "released" when not in use.

   - CommandoW3d asset manager asks the game data asset manager for assets by name.
	Game data manager must have a "directory" structure which maps each named
	asset to data on disk.  It then returns an image of the file once it has
	been loaded into ram.

	- Assets must be individual files, named with the asset name used in code/scripting
	We will write a tool which chops w3d files up so that all of the individual assets
	are brought out into their own file and named with the actual w3d name.

   - Data Asset Manager will load the file into ram, give it to us and forget about it
	W3d will release_ref it or delete it and the file image will go away.

	- Each time the 3d asset manager is requested for an asset, it will look through
	the render objects it has, if the asset isn't found, it will ask for the asset
	from the generic data asset manager.  When creating the actual render object,
	the 3d asset manager may find that it needs another asset (such as a hierarchy tree).
	It will then recurse, and ask for that asset.

	- Copy Mode will go away.  It will be internally set based on whether the mesh
	contains vertex animation.  All other render objects will simply "Clone".

	- Commando will derive a Cmdo3DAssetManager which knows about the special chunks
	required for the terrains.

	-------------------------------------------------------------------------------------
	July 28, 1998

	- Exposed the prototype system and added prototype loaders (PrototypeClass and
	PrototypeLoaderClass in proto.h).  This now allows the user to install his own
	loaders for new render object types.

	- Simplified the interface by removing the special purpose creation functions,
	leaving only the Create_Render_Obj function.

	- In certain cases some users need to know what kind of render object was created
	so we added a Class_ID mechanism to RenderObjClass.

	- Class_ID for render objects is not enough.  Need the asset iterator to be able
	to tell you the Class_ID of each asset in the asset manager.  This also means that
	the prototype class needs to be able to tell you the class ID.  Actually this
	code only seems to be used by tools such as SView but is needed anyway...

*/


class WW3DAssetManager
{

public:

	/*
	** Constructor and destructor
	*/
	WW3DAssetManager(void);
	virtual ~WW3DAssetManager(void);

	/*
	** Access to the single instance of a WW3DAssetManager.  The user
	** can subclass their own asset manager class but should only
	** create one instance.  (a violation of this will be caught with
	** a run-time assertion)
	**
	** The "official" way to get at the global asset manager is to
	** use a line of code like this:
	**	WW3DAssetManager::Get_Instance();
	*/
	static WW3DAssetManager *		Get_Instance(void) { return TheInstance; }
	static void							Delete_This(void) { delete TheInstance; TheInstance=NULL; }

	/*
	** Load data from any type of w3d file
	*/
	virtual bool						Load_3D_Assets( const char * filename);
	virtual bool						Load_3D_Assets(FileClass & assetfile);

	/*
	** Get rid of all of the currently loaded assets
	*/
	virtual void						Free_Assets(void);

	/*
	**	Release any assets that only the asset manager has a reference to.
	*/
	virtual void						Release_Unused_Assets(void);

	/*
	** Release assets not in the given exclusion list.
	*/
	virtual void						Free_Assets_With_Exclusion_List(const DynamicVectorClass<StringClass> & model_exclusion_list);
	virtual void						Create_Asset_List(DynamicVectorClass<StringClass> & model_exclusion_list);

	/*
	** create me an instance of one of the prototype render objects
	*/
	virtual RenderObjClass *		Create_Render_Obj(const char * name);

	/*
	** query if there is a render object with the specified name
	*/
	virtual bool						Render_Obj_Exists(const char * name);

	/*
	** Iterate through all render objects or through the
	** sub-categories of render objects.  NOTE! the user is responsible
	** for releasing the iterator when finished with it!
	*/
	virtual RenderObjIterator *	Create_Render_Obj_Iterator(void);
	virtual void						Release_Render_Obj_Iterator(RenderObjIterator *);

	/*
	** Access to HAnims, Used by Animatable3DObj's
	** TODO: make HAnims accessible from the HMODELS (or Animatable3DObj...)
	*/
	virtual AssetIterator *			Create_HAnim_Iterator(void);
	virtual HAnimClass *				Get_HAnim(const char * name);
	virtual bool						Add_Anim (HAnimClass *new_anim) { return HAnimManager.Add_Anim (new_anim); }

	/*
	** Access to textures
	*/
//	virtual AssetIterator *			Create_Texture_Iterator(void);

	HashTemplateClass<StringClass,TextureClass*>& Texture_Hash() { return TextureHash; }

	static void Log_Texture_Statistics();

	virtual TextureClass *			Get_Texture
	(
		const char * filename,
		MipCountType mip_level_count=MIP_LEVELS_ALL,
		WW3DFormat texture_format=WW3D_FORMAT_UNKNOWN,
		bool allow_compression=true,
		TextureBaseClass::TexAssetType type=TextureBaseClass::TEX_REGULAR,
		bool allow_reduction=true
	);

	virtual void						Release_All_Textures(void);
	virtual void						Release_Unused_Textures(void);
	virtual void						Release_Texture(TextureClass *);
	virtual void						Load_Procedural_Textures();
	virtual MetalMapManagerClass* Peek_Metal_Map_Manager() { return MetalManager; }

	/*
	** Access to Font3DInstances. (These are not saved, we just use the
	** asset manager as a convienient way to create them.)
	*/
	virtual Font3DInstanceClass * Get_Font3DInstance( const char * name);

	/*
	** Access to FontChars. Used by Render2DSentenceClass
	*/
	virtual FontCharsClass *		Get_FontChars( const char * name, int point_size, bool is_bold = false );

	/*
	** Access to HTrees, Used by Animatable3DObj's
	*/
	virtual AssetIterator *			Create_HTree_Iterator(void);
	virtual HTreeClass *				Get_HTree(const char * name);

	/*
	** Prototype Loaders, The user can register new loaders here.  Note that
	** a the pointer to your loader will be stored inside the asset manager.
	** For this reason, your loader should be a static or global object.
	*/
	virtual void						Register_Prototype_Loader(PrototypeLoaderClass * loader);

	/*
	**	The Add_Prototype is public so that we can add prototypes for procedurally
	** generated objects to the asset manager.
	*/
	void									Add_Prototype(PrototypeClass * newproto);
	void									Remove_Prototype(PrototypeClass *proto);
	void									Remove_Prototype(const char *name);
	PrototypeClass *					Find_Prototype(const char * name);

	/*
	** Load on Demand
	*/
	bool	Get_WW3D_Load_On_Demand( void )			{ return WW3D_Load_On_Demand; }
	void	Set_WW3D_Load_On_Demand( bool on_off )	{ WW3D_Load_On_Demand = on_off; }

	/*
	** Add fog to objects on load
	*/
	bool	Get_Activate_Fog_On_Load( void )				{ return Activate_Fog_On_Load; }
	void	Set_Activate_Fog_On_Load( bool on_off )	{ Activate_Fog_On_Load = on_off; }

	// Log texture statistics
	void Log_All_Textures();

protected:

	/*
	** Access to Font3DData. (These are privately managed/accessed)
	*/
	virtual AssetIterator *			Create_Font3DData_Iterator(void);
	virtual void						Add_Font3DData(Font3DDataClass * font);
	virtual void						Remove_Font3DData(Font3DDataClass * font);
	virtual Font3DDataClass *		Get_Font3DData(const char * name);
	virtual void						Release_All_Font3DDatas( void);
	virtual void						Release_Unused_Font3DDatas( void);

	virtual void						Release_All_FontChars( void );

	void									Free(void);

	PrototypeLoaderClass *			Find_Prototype_Loader(int chunk_id);
	bool									Load_Prototype(ChunkLoadClass & cload);

	/*
	** Compile time control over the dynamic arrays:
	*/
	enum
	{
		PROTOLOADERS_VECTOR_SIZE =	32,
		PROTOLOADERS_GROWTH_RATE =	16,

		PROTOTYPES_VECTOR_SIZE =	256,
		PROTOTYPES_GROWTH_RATE =	32,
	};

	/*
	** Prototype Loaders
	** These objects are responsible for importing certain W3D chunk types and turning
	** them into prototypes.
	*/
	DynamicVectorClass < PrototypeLoaderClass * >			PrototypeLoaders;

	/*
	** Prototypes
	** These objects are abstract factories for named render objects.  Prototypes is
	** a dynamic array of pointers to the currently loaded prototypes.
	*/
	DynamicVectorClass < PrototypeClass * >					Prototypes;

	/*
	** Prototype Hash Table
	** This structure is simply used to speed up the name lookup for prototypes
	*/
	enum
	{
		PROTOTYPE_HASH_TABLE_SIZE =	4096,
		PROTOTYPE_HASH_BITS =			12,
		PROTOTYPE_HASH_MASK =			0x00000FFF
	};

	PrototypeClass * *												PrototypeHashTable;

	/*
	** managers of HTrees, HAnims, Textures....
	*/
	HTreeManagerClass					HTreeManager;
	HAnimManagerClass					HAnimManager;

	/*
	** list of Font3DDatas
	*/
	SList<Font3DDataClass>			Font3DDatas;

	/*
	** list of FontChars
	*/
	SimpleDynVecClass<FontCharsClass*>		FontCharsList;

	/*
	** Should .W3D be loaded if not in memory
	*/
	bool									WW3D_Load_On_Demand;

	/*
	** Should we activate fog on objects while loading them
	*/
	bool									Activate_Fog_On_Load;

	// Metal Map Manager
	MetalMapManagerClass * MetalManager;

	/*
	** Texture hash table for quick texture lookups
	*/
	HashTemplateClass<StringClass, TextureClass *> TextureHash;

	/*
	** The 3d asset manager is a singleton, there should be only
	** one and it is accessible through Get_Instance()
	*/
	static WW3DAssetManager *		TheInstance;

	/*
	** the iterator classes are friends
	*/
	friend class RObjIterator;
	friend class HAnimIterator;
	friend class HTreeIterator;
	friend class Font3DDataIterator;
	friend class TextureIterator;

	// Font3DInstance need access to the Font3DData
	friend class Font3DInstanceClass;
};
