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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/agg_def.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                
 *                                                                                             *
 *                     $Modtime:: 4/05/01 10:21a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "agg_def.h"
#include "htree.h"
#include "w3derr.h"
#include "chunkio.h"
#include "wwdebug.h"
#include "assetmgr.h"
#include "matinfo.h"
#include "texture.h"
#include "wwstring.h"

#include <windows.h>


///////////////////////////////////////////////////////////////////////////////////
//
//	Local constants
//
const char * const EMPTY_STRING			= "";


///////////////////////////////////////////////////////////////////////////////////
//
//	Global variable initialization
//
AggregateLoaderClass	_AggregateLoader;


///////////////////////////////////////////////////////////////////////////////////
//
//	AggregateDefClass
//
AggregateDefClass::AggregateDefClass (void)
	: m_pName (NULL)
{
	// Set our member data to default settings
	::memset (&m_Info, 0, sizeof (m_Info));
	::memset (&m_MiscInfo, 0, sizeof (m_MiscInfo));
	m_MiscInfo.OriginalClassID = RenderObjClass::CLASSID_HLOD;
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	AggregateDefClass
//
AggregateDefClass::AggregateDefClass (const AggregateDefClass &src)
	: m_pName (NULL)
{
	// Set our member data to default settings
	::memset (&m_Info, 0, sizeof (m_Info));
	::memset (&m_MiscInfo, 0, sizeof (m_MiscInfo));
	m_MiscInfo.OriginalClassID = RenderObjClass::CLASSID_HLOD;

	// Invoke the assignment operator
	(*this) = src;
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	AggregateDefClass
//
AggregateDefClass::AggregateDefClass (RenderObjClass &base_model)
	: m_pName (NULL)
{
	// Set our member data to default settings
	::memset (&m_Info, 0, sizeof (m_Info));
	::memset (&m_MiscInfo, 0, sizeof (m_MiscInfo));
	m_MiscInfo.OriginalClassID = RenderObjClass::CLASSID_HLOD;

	Initialize (base_model);
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	~AggregateDefClass
//
AggregateDefClass::~AggregateDefClass (void)
{
	// Free the name buffer if necessary
	if (m_pName != NULL) {
		
		// free() is used because the buffer was allocated with ::_strdup().
		::free (m_pName);
		m_pName = NULL;
	}	

	Free_Subobject_List ();
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
const AggregateDefClass &
AggregateDefClass::operator= (const AggregateDefClass &src)
{
	int index;

	// Free the name buffer if necessary
	if (m_pName != NULL) {		
		::free (m_pName);
		m_pName = NULL;
	}	

	// Start with a fresh set of data
	Free_Subobject_List ();

	// Copy the src object's name and info struct
	Set_Name (src.Get_Name ());
	::memcpy (&m_Info, &src.m_Info, sizeof (m_Info));
	::memcpy (&m_MiscInfo, &src.m_MiscInfo, sizeof (m_MiscInfo));	
	m_Version = src.m_Version;

	// Loop through all the entries in the src object's subobj list
	for (index = 0; index < src.m_SubobjectList.Count (); index ++) {
		W3dAggregateSubobjectStruct *pinfo = src.m_SubobjectList[index];
		if (pinfo != NULL) {

			// Copy the src object's info for this subobj
			W3dAggregateSubobjectStruct *new_info = W3DNEW W3dAggregateSubobjectStruct;
			::memcpy (new_info, pinfo, sizeof (W3dAggregateSubobjectStruct));

			// Add this subobj to our list
			m_SubobjectList.Add (new_info);
		}
	}

	// Return a reference to ourselves
	return *this;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Free_Subobject_List
//
void
AggregateDefClass::Free_Subobject_List (void)
{
	// Delete all the stucture pointers contained in the subobject list
	for (int index = 0; index < m_SubobjectList.Count (); index ++) {
		W3dAggregateSubobjectStruct *pinfo = m_SubobjectList[index];
		if (pinfo) {
			delete pinfo;
		}
	}

	// Reset the lists contents
	m_SubobjectList.Delete_All ();
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
RenderObjClass *
AggregateDefClass::Create (void)
{
	// Attempt to create an instance of the hierarchy
	RenderObjClass *pmodel = Create_Render_Object (m_Info.BaseModelName);
	if (pmodel != NULL) {
		
		// Perform the aggregation
		Attach_Subobjects (*pmodel);

		// Let the new object know what its new name and base name are.
		pmodel->Set_Name (m_pName);
		pmodel->Set_Base_Model_Name (m_Info.BaseModelName);
		pmodel->Set_Sub_Objects_Match_LOD ((m_MiscInfo.Flags & W3D_AGGREGATE_FORCE_SUB_OBJ_LOD) == W3D_AGGREGATE_FORCE_SUB_OBJ_LOD);

	} else {
		WWDEBUG_SAY (("Unable to load aggregate %s.\r\n", m_Info.BaseModelName));
	}

	// Return a pointer to the new aggregate
	return pmodel;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Find_Subobject
//
RenderObjClass *
AggregateDefClass::Find_Subobject
(
	RenderObjClass &model,
	const char mesh_path[MESH_PATH_ENTRIES][MESH_PATH_ENTRY_LEN],
	const char bone_path[MESH_PATH_ENTRIES][MESH_PATH_ENTRY_LEN]
)
{
	RenderObjClass *parent_model = &model;
	parent_model->Add_Ref ();
	
	// Loop through all the models in our "path" until we've either failed
	// or found the exact mesh we were looking for...
	for (int index = 1;
		  (mesh_path[index][0] != 0) && (parent_model != NULL);
		  index ++) {

		// Look one level deeper into the subobject chain...
		RenderObjClass *sub_obj = NULL;
		if (bone_path[index][0] == 0) {
			sub_obj = parent_model->Get_Sub_Object_By_Name (mesh_path[index]);
		} else {
			
			int bone_index = parent_model->Get_Bone_Index (bone_path[index]);
			int subobj_count = parent_model->Get_Num_Sub_Objects_On_Bone (bone_index);
			
			// Loop through all the subobjects on this bone
			for (int subobj_index = 0; (subobj_index < subobj_count) && (sub_obj == NULL); subobj_index ++) {				
				
				// Is this the subobject we were looking for?
				RenderObjClass *ptemp_obj = parent_model->Get_Sub_Object_On_Bone (subobj_index, bone_index);
				if (::lstrcmpi (ptemp_obj->Get_Name (), mesh_path[index]) == 0) {
					sub_obj = ptemp_obj;
				} else {
					REF_PTR_RELEASE (ptemp_obj);
				}
			}
		}

		REF_PTR_RELEASE (parent_model);

		// The parent for the next iteration is the subobject on this one.
		parent_model = sub_obj;
	}

	// Return a pointer to the subobject
	return parent_model;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Attach_Subobjects
//
void
AggregateDefClass::Attach_Subobjects (RenderObjClass &base_model)
{
	// Now loop through all the subobjects and attach them to the appropriate bone
	for (int index = 0; index < m_SubobjectList.Count (); index ++) {
		W3dAggregateSubobjectStruct *psubobj_info = m_SubobjectList[index];
		if (psubobj_info != NULL) {
			
			// Now create this subobject and attach it to its bone.
			RenderObjClass *prender_obj = Create_Render_Object (psubobj_info->SubobjectName);
			if (prender_obj != NULL) {

				// Attach this object to the requested bone
				if (base_model.Add_Sub_Object_To_Bone (prender_obj, psubobj_info->BoneName) == false) {
					WWDEBUG_SAY (("Unable to attach %s to %s.\r\n", psubobj_info->SubobjectName, psubobj_info->BoneName));
				}

				// Release our hold on this pointer
				prender_obj->Release_Ref ();
			} else {
				WWDEBUG_SAY (("Unable to load aggregate subobject %s.\r\n", psubobj_info->SubobjectName));
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Create_Render_Object
//
RenderObjClass *
AggregateDefClass::Create_Render_Object (const char *passet_name)
{
	// Assume failure
	RenderObjClass *prender_obj = NULL;

	// Attempt to get an instance of the render object from the asset manager
	prender_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (passet_name);
	
	// If we couldn't find the render object in the asset manager, then attempt to
	// load it from file
	if ((prender_obj == NULL) &&
	    Load_Assets (passet_name)) {

		// It should be in the asset manager now, so attempt to get it again.
		prender_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (passet_name);
	}
	
	// Return a pointer to the render object
	return prender_obj;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Load_Assets
//
bool
AggregateDefClass::Load_Assets (const char *passet_name)
{
	// Assume failure
	bool retval = false;

	// Param OK?
	if (passet_name != NULL) {
		
		// Determine what the current working directory is
		char path[MAX_PATH];
		::GetCurrentDirectory (sizeof (path), path);

		// Ensure the path is directory delimited
		if (path[::lstrlen(path)-1] != '\\') {
			::lstrcat (path, "\\");
		}

		// Assume the filename is simply the "asset name" + the w3d extension
		::lstrcat (path, passet_name);
		::lstrcat (path, ".w3d");

		// If the file exists, then load it into the asset manager.
		if (::GetFileAttributes (path) != 0xFFFFFFFF) {
			retval = WW3DAssetManager::Get_Instance()->Load_3D_Assets (path);
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
void
AggregateDefClass::Initialize (RenderObjClass &base_model)
{
	// Start with fresh lists
	Free_Subobject_List ();

	// Determine what the render objects original name was.
	const char *orig_model_name = base_model.Get_Base_Model_Name ();
	orig_model_name = (orig_model_name == NULL) ? base_model.Get_Name () : orig_model_name;

	// Record information about this base model
	::lstrcpy (m_Info.BaseModelName, orig_model_name);
	m_Info.SubobjectCount = 0;
	m_MiscInfo.OriginalClassID = base_model.Class_ID ();
	m_MiscInfo.Flags = 0;	
	m_MiscInfo.Flags |= base_model.Is_Sub_Objects_Match_LOD_Enabled () ? W3D_AGGREGATE_FORCE_SUB_OBJ_LOD : 0;
	

	// Pass the aggregate name along
	Set_Name (base_model.Get_Name ());

	// Create a new instance of the model which we can use
	// to compare with the supplied model and determine
	// which 'bones-models' and textures are new.
	RenderObjClass *pvanilla_model = (RenderObjClass *)Create_Render_Object (orig_model_name);

	// Build lists of changes from the delta between the original model and the provided one
	Build_Subobject_List (*pvanilla_model, base_model);

	// Release the model if necessary
	REF_PTR_RELEASE (pvanilla_model);	
	return ;
}



///////////////////////////////////////////////////////////////////////////////////
//
//	Build_Subobject_List
//
void
AggregateDefClass::Build_Subobject_List
(
	RenderObjClass &original_model,
	RenderObjClass &model
)
{
	int index;

	// Loop through all the bones in this render obj
	int bone_count = model.Get_Num_Bones ();
	for (int bone_index = 0; bone_index < bone_count; bone_index ++) {			
		const char *pbone_name = model.Get_Bone_Name (bone_index);
		
		// Build a list of nodes that are contained in the vanilla model
		DynamicVectorClass <RenderObjClass *> orig_node_list;
		for (index = 0;
			  index < original_model.Get_Num_Sub_Objects_On_Bone (bone_index);
			  index ++) {
			RenderObjClass *psubobj = original_model.Get_Sub_Object_On_Bone (index, bone_index);
			if (psubobj != NULL) {
				orig_node_list.Add (psubobj);
			}
		}

		// Build a list of nodes that are contained in this bone
		DynamicVectorClass <RenderObjClass *> node_list;
		for (index = 0;
			  index < model.Get_Num_Sub_Objects_On_Bone (bone_index);
			  index ++) {
			RenderObjClass *psubobj = model.Get_Sub_Object_On_Bone (index, bone_index);
			if (psubobj != NULL) {
				node_list.Add (psubobj);
			}
		}

		int node_count = node_list.Count ();
		if (node_count > 0) {
			
			// Loop through the subobjects and add each one to our internal list
			W3dAggregateSubobjectStruct subobj_info = { 0 };
			for (int node_index = 0; node_index < node_count; node_index ++) {
				RenderObjClass *psubobject = node_list[node_index];
				WWASSERT (psubobject != NULL);
				
				// Is this subobject new?  (i.e. not in a 'vanilla' instance?)
				const char *prototype_name = psubobject->Get_Name ();
				if (psubobject != NULL &&
					 (Is_Object_In_List (prototype_name, orig_node_list) == false)) {
					
					// Add this subobject to our list
					::lstrcpy (subobj_info.SubobjectName, prototype_name);
					::lstrcpy (subobj_info.BoneName, pbone_name);
					Add_Subobject (subobj_info);
					m_Info.SubobjectCount ++;

					// Attach this render object to the 'original' model (this is done
					// so we can do texture compares later)
					RenderObjClass *prender_obj = WW3DAssetManager::Get_Instance ()->Create_Render_Obj (prototype_name);
					((RenderObjClass &)original_model).Add_Sub_Object_To_Bone (prender_obj, pbone_name);
					REF_PTR_RELEASE (prender_obj);
				}
			}
		}

		// Free our hold on the render objs in the original node list
		for (index = 0; index < orig_node_list.Count (); index ++) {
			REF_PTR_RELEASE (orig_node_list[index]);
		}
		orig_node_list.Delete_All ();

		// Free our hold on the render objs in the node list
		for (index = 0; index < node_list.Count (); index ++) {
			REF_PTR_RELEASE (node_list[index]);
		}
		node_list.Delete_All ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Is_Object_In_List
//
bool
AggregateDefClass::Is_Object_In_List
(
	const char *passet_name,
	DynamicVectorClass <RenderObjClass *> &node_list
)
{
	// Assume failure
	bool retval = false;

	// Loop through the nodes in the list until we've found the one
	// were are looking for.
	for (int node_index = 0; (node_index < node_list.Count ()) && (retval == false); node_index ++) {
		RenderObjClass *prender_obj = node_list[node_index];
		
		// Is this the render object we were looking for?
		if (prender_obj != NULL &&
		    ::lstrcmpi (prender_obj->Get_Name (), passet_name) == 0) {
			retval = true;
		}
	}	

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
WW3DErrorType
AggregateDefClass::Load_W3D (ChunkLoadClass &chunk_load)
{
	W3dTextureReplacerHeaderStruct header = { 0 };

	
	while (chunk_load.Open_Chunk()) {

		WW3DErrorType error = WW3D_ERROR_OK;

		switch (chunk_load.Cur_Chunk_ID()) {

			case W3D_CHUNK_AGGREGATE_HEADER:
				error = Read_Header(chunk_load);
				break;

			case W3D_CHUNK_AGGREGATE_INFO:
				error = Read_Info(chunk_load);
				break;

			case W3D_CHUNK_TEXTURE_REPLACER_INFO:
				if (chunk_load.Read (&header, sizeof (header)) == sizeof (header)) {
					if (header.ReplacedTexturesCount > 0) {
						WWDEBUG_SAY(("Obsolete texture replacement chunk encountered in aggregate: %s\r\n",m_pName));
					}
				} 
				break;

			case W3D_CHUNK_AGGREGATE_CLASS_INFO:
				error = Read_Class_Info(chunk_load);
				break;

			default:
				
				// Unknown chunk.
				break;
		}	
		chunk_load.Close_Chunk();
		if (error != WW3D_ERROR_OK) return (error);
	}
	
	return WW3D_ERROR_OK;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Read_Header
//
WW3DErrorType
AggregateDefClass::Read_Header (ChunkLoadClass &chunk_load)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_LOAD_FAILED;

	// Is this the header chunk?
	W3dAggregateHeaderStruct header = { 0 };
	if (chunk_load.Read (&header, sizeof (header)) == sizeof (header)) {

		// Copy the name from the header structure
		m_pName = ::_strdup (header.Name);
		m_Version = header.Version;

		// Success!
		ret_val = WW3D_ERROR_OK;
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Read_Info
//
WW3DErrorType
AggregateDefClass::Read_Info (ChunkLoadClass &chunk_load)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_LOAD_FAILED;

	// Read the chunk straight into our member structure
	::memset (&m_Info, 0, sizeof (m_Info));
	if (chunk_load.Read (&m_Info, sizeof (m_Info)) == sizeof (m_Info)) {

		// Success!
		ret_val = WW3D_ERROR_OK;

		// Read all the subobjects from the file
		for (UINT isubobject = 0;
			  (isubobject < m_Info.SubobjectCount) && (ret_val == WW3D_ERROR_OK);
			  isubobject ++) {

			// Read this subobject's definition from the file
			ret_val = Read_Subobject (chunk_load);
		}				
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Read_Subobject
//
WW3DErrorType
AggregateDefClass::Read_Subobject (ChunkLoadClass &chunk_load)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_LOAD_FAILED;

	// Read the subobject information from the file
	W3dAggregateSubobjectStruct subobj_info = { 0 };
	if (chunk_load.Read (&subobj_info, sizeof (subobj_info)) == sizeof (subobj_info)) {

		// Add this subobject to our list
		Add_Subobject (subobj_info);

		// Success!
		ret_val = WW3D_ERROR_OK;
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Add_Subobject
//
void
AggregateDefClass::Add_Subobject (const W3dAggregateSubobjectStruct &subobj_info)
{
	// Create a new structure and copy the contents of the src
	W3dAggregateSubobjectStruct *pnew_entry = W3DNEW W3dAggregateSubobjectStruct;
	::lstrcpy (pnew_entry->SubobjectName, subobj_info.SubobjectName);
	::lstrcpy (pnew_entry->BoneName, subobj_info.BoneName);

	// Add this new entry to the list
	m_SubobjectList.Add (pnew_entry);
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Read_Class_Info
//
WW3DErrorType
AggregateDefClass::Read_Class_Info (ChunkLoadClass &chunk_load)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_LOAD_FAILED;

	// Read the chunk straight into our header structure
	::memset (&m_MiscInfo, 0, sizeof (m_MiscInfo));
	if (chunk_load.Read (&m_MiscInfo, sizeof (m_MiscInfo)) == sizeof (m_MiscInfo)) {

		// Success!
		ret_val = WW3D_ERROR_OK;
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
WW3DErrorType
AggregateDefClass::Save_W3D (ChunkSaveClass &chunk_save)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_SAVE_FAILED;

	// Begin a chunk that identifies an aggregate
	if (chunk_save.Begin_Chunk (W3D_CHUNK_AGGREGATE) == TRUE) {
		
		// Attempt to save the different sections of the aggregate definition
		if ((Save_Header (chunk_save) == WW3D_ERROR_OK) &&
			 (Save_Info (chunk_save) == WW3D_ERROR_OK) &&
			 (Save_Class_Info (chunk_save) == WW3D_ERROR_OK)) {
			
			// Success!
			ret_val = WW3D_ERROR_OK;
		}

		// Close the aggregate chunk
		chunk_save.End_Chunk ();
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Save_Header
//
WW3DErrorType
AggregateDefClass::Save_Header (ChunkSaveClass &chunk_save)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_SAVE_FAILED;

	// Begin a chunk that identifies the aggregate
	if (chunk_save.Begin_Chunk (W3D_CHUNK_AGGREGATE_HEADER) == TRUE) {
		
		// Fill the header structure
		W3dAggregateHeaderStruct header = { 0 };
		header.Version = W3D_CURRENT_AGGREGATE_VERSION;
		::lstrcpyn (header.Name, m_pName, sizeof (header.Name));
		header.Name[sizeof (header.Name) - 1] = 0;

		// Write the header out to the chunk
		if (chunk_save.Write (&header, sizeof (header)) == sizeof (header)) {
			// Success!
			ret_val = WW3D_ERROR_OK;			
		}

		// End the header chunk
		chunk_save.End_Chunk ();
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Save_Info
//
WW3DErrorType
AggregateDefClass::Save_Info (ChunkSaveClass &chunk_save)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_SAVE_FAILED;

	// Begin a chunk that identifies the aggregate settings
	if (chunk_save.Begin_Chunk (W3D_CHUNK_AGGREGATE_INFO) == TRUE) {
		
		// Write the settings structure out to the chunk
		if (chunk_save.Write (&m_Info, sizeof (m_Info)) == sizeof (m_Info)) {
			// Success!
			ret_val = WW3D_ERROR_OK;

			// Write all the subobjects to the file
			for (int isubobject = 0;
			     (isubobject < m_SubobjectList.Count ()) && (ret_val == WW3D_ERROR_OK);
				  isubobject ++) {

				// Write this object to the file
				ret_val = Save_Subobject (chunk_save, m_SubobjectList[isubobject]);
			}
		}

		// End the settings chunk
		chunk_save.End_Chunk ();
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Save_Subobject
//
WW3DErrorType
AggregateDefClass::Save_Subobject
(
	ChunkSaveClass &chunk_save,
	W3dAggregateSubobjectStruct *psubobject
)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_SAVE_FAILED;

	// Write the subobj structure out to the chunk
	if (chunk_save.Write (psubobject, sizeof (W3dAggregateSubobjectStruct)) == sizeof (W3dAggregateSubobjectStruct)) {

		// Success!
		ret_val = WW3D_ERROR_OK;
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Save_Class_Info
//
WW3DErrorType
AggregateDefClass::Save_Class_Info (ChunkSaveClass &chunk_save)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_SAVE_FAILED;

	// Begin a chunk that identifies the texture replacer header
	if (chunk_save.Begin_Chunk (W3D_CHUNK_AGGREGATE_CLASS_INFO) == TRUE) {
		
		// Write the class information structure out to the chunk
		if (chunk_save.Write (&m_MiscInfo, sizeof (m_MiscInfo)) == sizeof (m_MiscInfo)) {
			
			// Success!
			ret_val = WW3D_ERROR_OK;
		}

		// End the class info chunk
		chunk_save.End_Chunk ();
	}

	// Return the WW3D_ERROR_TYPE return code
	return ret_val;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
PrototypeClass *
AggregateLoaderClass::Load_W3D (ChunkLoadClass &chunk_load)
{
	// Assume failure
	AggregatePrototypeClass *pprototype = NULL;

	// Create a definition object
	AggregateDefClass *pdefinition = W3DNEW AggregateDefClass;
	if (pdefinition != NULL) {
		
		// Ask the definition object to load the aggregate data
		if (pdefinition->Load_W3D (chunk_load) != WW3D_ERROR_OK) {
			
			// Error!  Free the definition
			delete pdefinition;
			pdefinition = NULL;
		} else {

			// Success!  Create a prototype from the definition
			pprototype = W3DNEW AggregatePrototypeClass (pdefinition);
		}
	}

    // Return a pointer to the prototype
	 return pprototype;
}

