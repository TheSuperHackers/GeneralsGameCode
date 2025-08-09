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
 *                     $Archive:: /Commando/Code/ww3d2/agg_def.h                              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/05/01 9:52a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef AGGREGATE_DEF_H
#define AGGREGATE_DEF_H

#include "proto.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "w3derr.h"
#include "Vector.H"
#include "bittype.h"
#include <string.h>

#ifdef _UNIX
#include "osdep.h"
#endif


// Forward declarations
class ChunkLoadClass;
class ChunkSaveClass;
class IndirectTextureClass;


///////////////////////////////////////////////////////////////////////////////////
//
//	Macros
//
#ifndef SAFE_FREE
#define SAFE_FREE(pointer)	\
{ \
	if (pointer) {	\
		::free (pointer);	\
		pointer = 0; \
	} \
}

#endif //SAFE_FREE


//////////////////////////////////////////////////////////////////////////////////
//
//	AggregateDefClass
//
//	Description of an aggregate object.  Used by the asset manager
// to construct aggregates.  An  'aggregate' is simply a 'shell' that
//	contains references to a hierarchy model and subobjects to attach to its bones.
//
class AggregateDefClass
{
	public:

		///////////////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		AggregateDefClass (void);
		AggregateDefClass (RenderObjClass &base_model);
		AggregateDefClass (const AggregateDefClass &src);
		virtual ~AggregateDefClass (void);


		///////////////////////////////////////////////////////////
		//
		//	Public operators
		//
		const AggregateDefClass &operator= (const AggregateDefClass &src);

		///////////////////////////////////////////////////////////
		//
		//	Public methods
		//
		virtual WW3DErrorType	Load_W3D (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType	Save_W3D (ChunkSaveClass &chunk_save);
		const char *				Get_Name (void) const					{ return m_pName; }
		void							Set_Name (const char *pname)			{ SAFE_FREE (m_pName); m_pName = ::_strdup (pname); }
		RenderObjClass *			Create (void);
		AggregateDefClass *		Clone (void) const						{ return W3DNEW AggregateDefClass (*this); }

		//
		//	Public accessors
		//
		ULONG							Class_ID (void) const					{ return m_MiscInfo.OriginalClassID; }

		//
		//	Initialization
		//
		void							Initialize (RenderObjClass &base_model);

	protected:

		///////////////////////////////////////////////////////////
		//
		//	Protected data types
		//
		typedef struct _TEXTURE_INFO
		{
			W3dTextureReplacerStruct	names;
			IndirectTextureClass *		pnew_texture;

			bool operator == (_TEXTURE_INFO &src) { return false; }
			bool operator != (_TEXTURE_INFO &src) { return true; }
		} TEXTURE_INFO;


		///////////////////////////////////////////////////////////
		//
		//	Protected methods
		//

		//
		//	Loading methods
		//
		virtual WW3DErrorType		Read_Header (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType		Read_Info (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType		Read_Subobject (ChunkLoadClass &chunk_load);
		virtual WW3DErrorType		Read_Class_Info (ChunkLoadClass &chunk_load);

		//
		//	Saving methods
		//
		virtual WW3DErrorType		Save_Header (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType		Save_Info (ChunkSaveClass &chunk_save);
		virtual WW3DErrorType		Save_Subobject (ChunkSaveClass &chunk_save, W3dAggregateSubobjectStruct *psubobject);
		virtual WW3DErrorType		Save_Class_Info (ChunkSaveClass &chunk_save);

		//
		//	Creation methods
		//
		virtual void				Attach_Subobjects (RenderObjClass &base_model);

		//
		//	Search methods
		//
		virtual RenderObjClass *			Find_Subobject (RenderObjClass &model, const char mesh_path[MESH_PATH_ENTRIES][MESH_PATH_ENTRY_LEN], const char bone_path[MESH_PATH_ENTRIES][MESH_PATH_ENTRY_LEN]);

		//
		//	Misc. methods
		//
		virtual void				Free_Subobject_List (void);
		virtual void				Add_Subobject (const W3dAggregateSubobjectStruct &subobj_info);
		virtual bool				Load_Assets (const char *asset_name);
		virtual RenderObjClass *Create_Render_Object (const char *passet_name);
		virtual bool				Is_Object_In_List (const char *passet_name, DynamicVectorClass <RenderObjClass *> &node_list);

		virtual void				Build_Subobject_List (RenderObjClass &original_model, RenderObjClass &model);


	private:

		///////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		DWORD																m_Version;
		DynamicVectorClass<W3dAggregateSubobjectStruct *>	m_SubobjectList;
		W3dAggregateInfoStruct										m_Info;
		W3dAggregateMiscInfo											m_MiscInfo;
		char * 															m_pName;
};


///////////////////////////////////////////////////////////////////////////////////
//
//	AggregatePrototypeClass
//
class AggregatePrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(AggregatePrototypeClass)
	public:

		///////////////////////////////////////////////////////////
		//
		//	Public constructors/destructors
		//
		AggregatePrototypeClass (AggregateDefClass *pdef)		{ m_pDefinition = pdef; }

		///////////////////////////////////////////////////////////
		//
		//	Public methods
		//
		virtual const char *				Get_Name(void) const				{ return m_pDefinition->Get_Name (); }
		virtual int									Get_Class_ID(void) const		{ return m_pDefinition->Class_ID (); }
		virtual RenderObjClass *		Create (void)								{ return m_pDefinition->Create (); }
		virtual void								DeleteSelf()								{ delete this; }
		virtual AggregateDefClass	*	Get_Definition (void) const { return m_pDefinition; }
		virtual void								Set_Definition (AggregateDefClass *pdef) { m_pDefinition = pdef; }

	protected:
		virtual ~AggregatePrototypeClass (void)					{ delete m_pDefinition; }

	private:

		///////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		AggregateDefClass	*	m_pDefinition;
};


///////////////////////////////////////////////////////////////////////////////////
//
//	AggregateLoaderClass
//
class AggregateLoaderClass : public PrototypeLoaderClass
{
	public:

		virtual int						Chunk_Type (void)  { return W3D_CHUNK_AGGREGATE; }
		virtual PrototypeClass *	Load_W3D (ChunkLoadClass &chunk_load);
};


///////////////////////////////////////////////////////////////////////////////////
//
//	Global variables
//
extern AggregateLoaderClass	_AggregateLoader;


#endif //__AGGREGATE_DEF_H
