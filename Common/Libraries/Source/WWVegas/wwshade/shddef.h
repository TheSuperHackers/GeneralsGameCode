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
 *                 Project Name : WWSHADE                                                         *
 *                                                                                             *
 *                     $Archive:: wwshade/shddef.h                           $*
 *                                                                                             *
 *                  $Org Author:: Jani_p
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 6/07/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                          $*
 *                                                                                             *
 *   6/07/02	 KJM : Added validation enum
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHDDEF_H
#define SHDDEF_H

#include "always.h"
#include "editable.h"
#include "refcount.h"

class ShdInterfaceClass;

enum SHDVER
{
	SHDVER_UNDEFINED=0,
	SHDVER_6,
	SHDVER_7,
	SHDVER_8
};

class ShdVersion
{
public:
	ShdVersion() : Version(SHDVER_UNDEFINED) {}

	void		Set(const SHDVER ver) { Version=ver; }
	SHDVER	Get() const  { return Version; }

private:

	SHDVER	Version;
};



/**
** ShdDefClass - This class is the base class for all shader "definition" objects.  
**
** A shader definition object has two responsibilities.  
**
** - It contains a "generic" description (chars, ints, floats, etc) of all of the user-settable parameters used 
** by an instance of this type of shader (e.g. what textures it uses, colors, etc).
**
** - It contains a virtual "Create" function which can create an actual shader for you.  This function is an 
** abstract factory which creates a shader implementation compatible with the current hardware the application
** is running on.
*/
class ShdDefClass : public EditableClass, public RefCountClass
{
public:
	DECLARE_EDITABLE(ShdDefClass, EditableClass);

	ShdDefClass(uint32 class_id);
	ShdDefClass(const ShdDefClass & that);
	virtual ~ShdDefClass(void);
		
	virtual ShdDefClass *			Clone(void) const	= 0;		
	virtual void						Reset(void);

	// Run-Time Type identification (ID's defined in "shdclassids.h")
	WWINLINE uint32					Get_Class_ID (void) const { return ClassID; }

	// Shader Management & Creation (should create a shader compatible with the current hardware/API)
	virtual void						Init()=0;
	virtual void						Shutdown()=0;
	virtual ShdInterfaceClass *	Create (void) const = 0;

	// Name methods
	const char *						Get_Name (void) const;
	void									Set_Name (const char *new_name);	

	// Surface type, used for decal, sound, and emitter creation
	int									Get_Surface_Type(void) const	{ return SurfaceType; }
	void									Set_Surface_Type(int t) { SurfaceType = t; }

	// Validation methods
	virtual bool						Is_Valid_Config (StringClass &message)			{ return true; }

	// Requirements  PRELIMINARY, NEED TO VALIDATE THIS PART OF THE INTERFACE!!!
	virtual bool						Uses_Vertex_Alpha(void) const						{ return false; }
	virtual bool						Uses_UV_Channel(int i) const						{ return (i==0); }
	virtual bool						Uses_Vertex_Colors(void) const					{ return false; }
	virtual bool						Requires_Normals(void) const						{ return false; }
	virtual bool						Requires_Tangent_Space_Vectors(void) const	{ return false; }
	virtual bool						Requires_Sorting(void) const						{ return false; }
	virtual int							Static_Sort_Index(void) const						{ return 0; }

	// From PersistClass
	virtual bool						Save (ChunkSaveClass &csave);
	virtual bool						Load (ChunkLoadClass &cload);

private:

	bool									Save_Variables (ChunkSaveClass &csave);
	bool									Load_Variables (ChunkLoadClass &cload);

	uint32								ClassID;
	StringClass							Name;
	int									SurfaceType;
};



#endif //SHDDEF_H
