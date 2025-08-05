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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwsaveload/parameter.h                       $*
 *                                                                                             *
 *                   Org Author:: Patrick Smith                                                *
 *                                                                                             *
 *                       Author:: Kenny Mitchell                                                *
 *                                                                                             *
 *                     $Modtime:: 5/29/02 11:00a                                              $*
 *                                                                                             *
 *                    $Revision:: 38                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __PARAMETER_H
#define __PARAMETER_H

#include "always.h"
#include <string.h>
#include <stdlib.h>
#include "parametertypes.h"
#include "Vector.H"
#include "wwstring.h"
#include "bittype.h"
#include "obbox.h"


//////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////
class DefParameterClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	ParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class ParameterClass
{
public:

	typedef enum
	{
		TYPE_INT					= 0,
		TYPE_FLOAT,
		TYPE_STRING,
		TYPE_VECTOR3,
		TYPE_MATRIX3D,
		TYPE_BOOL,
		TYPE_TRANSITION,
		TYPE_MODELDEFINITIONID,
		TYPE_FILENAME,
		TYPE_ENUM,
		TYPE_GAMEOBJDEFINITIONID,
		TYPE_SCRIPT,
		TYPE_SOUND_FILENAME,
		TYPE_ANGLE,
		TYPE_WEAPONOBJDEFINITIONID,
		TYPE_AMMOOBJDEFINITIONID,
		TYPE_SOUNDDEFINITIONID,
		TYPE_COLOR,
		TYPE_PHYSDEFINITIONID,
		TYPE_EXPLOSIONDEFINITIONID,
		TYPE_DEFINITIONIDLIST,
		TYPE_ZONE,
		TYPE_FILENAMELIST,
		TYPE_SEPARATOR,
		TYPE_GENERICDEFINITIONID,
		TYPE_SCRIPTLIST,
		TYPE_VECTOR2,
		TYPE_RECT,
		TYPE_TEXTURE_FILENAME,
		TYPE_STRINGSDB_ID

	}	Type;

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	ParameterClass (void);
	ParameterClass (const ParameterClass &src);
	virtual ~ParameterClass (void);

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const ParameterClass &	operator= (const ParameterClass &src);
	virtual bool				operator== (const ParameterClass &src) = 0;

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// RTTI
	virtual DefParameterClass *	As_DefParameterClass (void)	{ return NULL; }

	// Type identification (see paramtypes.h in wwsaveload)
	virtual Type				Get_Type (void) const = 0;
	virtual bool				Is_Type (Type type) const { return false; }

	// Modification
	virtual bool				Is_Modifed (void) const				{ return IsModified; }
	virtual void				Set_Modified (bool onoff = true)	{ IsModified = onoff; }

	// Display name methods
	virtual const char *		Get_Name (void) const;
	virtual void				Set_Name (const char *new_name);

	// Units display methods
	virtual const char *		Get_Units_Name (void) const;
	virtual void				Set_Units_Name (const char *units_name);

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src) { };

	//////////////////////////////////////////////////////////////////////////////
	//	Static methods
	//////////////////////////////////////////////////////////////////////////////

	// Virtual constructor used to create a new instance of any parameter type
	static ParameterClass *	Construct (ParameterClass::Type type, void *data, const char *param_name);

private:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	bool				IsModified;
	const char *	m_Name;
	StringClass		m_UnitsName;
};


//////////////////////////////////////////////////////////////////////////////////
//	ParameterClass
//////////////////////////////////////////////////////////////////////////////////
inline
ParameterClass::ParameterClass (void)
	:	m_Name (NULL),
		IsModified (false)
{
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//	ParameterClass
//////////////////////////////////////////////////////////////////////////////////
inline
ParameterClass::ParameterClass (const ParameterClass &src)
	:	m_Name (NULL),
		IsModified (false)
{
	(*this) = src;
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//	~ParameterClass
//////////////////////////////////////////////////////////////////////////////////
inline
ParameterClass::~ParameterClass (void)
{
	Set_Name (NULL);
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//	operator=
//////////////////////////////////////////////////////////////////////////////////
inline const ParameterClass &
ParameterClass::operator= (const ParameterClass &src)
{
	IsModified = src.IsModified;
	Set_Name (src.m_Name);
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
//	Get_Name
//////////////////////////////////////////////////////////////////////////////////
inline const char *
ParameterClass::Get_Name (void) const
{
	return m_Name;
}

//////////////////////////////////////////////////////////////////////////////////
//	Set_Name
//////////////////////////////////////////////////////////////////////////////////
inline void
ParameterClass::Set_Name (const char *new_name)
{
	if (m_Name != NULL) {
		::free ((void *)m_Name);
		m_Name = NULL;
	}

	if (new_name != NULL) {
		m_Name = ::strdup (new_name);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//	Get_Units_Name
//////////////////////////////////////////////////////////////////////////////////
inline const char *
ParameterClass::Get_Units_Name (void) const
{
	return m_UnitsName;
}

//////////////////////////////////////////////////////////////////////////////////
//	Set_Units_Name
//////////////////////////////////////////////////////////////////////////////////
inline void
ParameterClass::Set_Units_Name (const char *new_name)
{
	m_UnitsName = new_name;
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//
//	StringParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class StringParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	StringParameterClass (StringClass *string);
	StringParameterClass (const StringParameterClass &src);
	virtual ~StringParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const StringParameterClass &	operator= (const StringParameterClass &src);
	bool									operator== (const StringParameterClass &src);
	bool									operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_STRING; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_STRING) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual const char *		Get_String (void) const;
	virtual void				Set_String (const char *string);

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	StringClass *	m_String;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	FilenameParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class FilenameParameterClass : public StringParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	FilenameParameterClass (StringClass *string);
	FilenameParameterClass (const FilenameParameterClass &src);
	virtual ~FilenameParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const FilenameParameterClass &	operator= (const FilenameParameterClass &src);
	bool										operator== (const FilenameParameterClass &src);
	bool										operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type			Get_Type (void) const { return TYPE_FILENAME; }
	virtual bool			Is_Type (Type type) const { return (type == TYPE_FILENAME) || StringParameterClass::Is_Type (type); }

	// Copy methods
	virtual void			Copy_Value (const ParameterClass &src);

	virtual void			Set_Extension (const char *extension)	{ m_Extension = extension; }
	virtual const char *	Get_Extension (void) const					{ return m_Extension; }

	virtual void			Set_Description (const char *desc)		{ m_Description = desc; }
	virtual const char *	Get_Description (void) const				{ return m_Description; }

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////////////////////
	StringClass				m_Extension;
	StringClass				m_Description;
};

//////////////////////////////////////////////////////////////////////////////////
//
//	TextureFilenameParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class TextureFilenameParameterClass : public FilenameParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	TextureFilenameParameterClass(StringClass *string);
	TextureFilenameParameterClass(const TextureFilenameParameterClass& src);
	virtual ~TextureFilenameParameterClass() {}


	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type			Get_Type (void) const { return TYPE_TEXTURE_FILENAME; }
	virtual bool			Is_Type (Type type) const { return (type == TYPE_TEXTURE_FILENAME) || StringParameterClass::Is_Type (type); }

	void						Set_Show_Alpha(bool show) { Show_Alpha=show; }
	bool						Get_Show_Alpha() const { return Show_Alpha; }

	void						Set_Show_Texture(bool show) { Show_Texture=show; }
	bool						Get_Show_Texture() const { return Show_Texture; }

	// Copy methods
	virtual void			Copy_Value (const ParameterClass &src);

protected:

	bool						Show_Alpha;
	bool						Show_Texture;
};



//////////////////////////////////////////////////////////////////////////////////
//
//	SoundFilenameParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class SoundFilenameParameterClass : public FilenameParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	SoundFilenameParameterClass (StringClass *string);
	SoundFilenameParameterClass (const SoundFilenameParameterClass &src);
	virtual ~SoundFilenameParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const SoundFilenameParameterClass &	operator= (const SoundFilenameParameterClass &src);
	bool											operator== (const SoundFilenameParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type			Get_Type (void) const { return TYPE_SOUND_FILENAME; }
	virtual bool			Is_Type (Type type) const { return (type == TYPE_SOUND_FILENAME) || FilenameParameterClass::Is_Type (type); }
};

//////////////////////////////////////////////////////////////////////////////////
//
//	EnumParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class EnumParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	EnumParameterClass (int *value);
	EnumParameterClass (const EnumParameterClass &src);
	virtual ~EnumParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const EnumParameterClass &	operator= (const EnumParameterClass &src);
	bool								operator== (const EnumParameterClass &src);
	bool								operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_ENUM; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_ENUM) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual void __cdecl		Add_Values (const char *first_name, int first_value, ...);
	virtual void				Add_Value (const char *display_name, int value);
	virtual int					Get_Count (void) const					{ return m_List.Count (); }
	virtual const char *		Get_Entry_Name (int index) const		{ return m_List[index].name; }
	virtual int					Get_Entry_Value (int index) const	{ return m_List[index].value; }

	virtual void				Set_Selected_Value (int value)	{ (*m_Value) = value; Set_Modified (); }
	virtual int					Get_Selected_Value (void) const	{ return (*m_Value); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private data types
	//////////////////////////////////////////////////////////////////////////////
	typedef struct _ENUM_VALUE
	{
		StringClass		name;
		int				value;

		_ENUM_VALUE (const char *_name=NULL, int _value=0) : name (_name), value (_value) {}
		bool operator== (const _ENUM_VALUE &) { return false; }
		bool operator!= (const _ENUM_VALUE &) { return true; }
	} ENUM_VALUE;

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	DynamicVectorClass<ENUM_VALUE>		m_List;
	int *											m_Value;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	PhysDefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class PhysDefParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	PhysDefParameterClass (int *id);
	PhysDefParameterClass (const PhysDefParameterClass &src);
	virtual ~PhysDefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const PhysDefParameterClass &	operator= (const PhysDefParameterClass &src);
	bool									operator== (const PhysDefParameterClass &src);
	bool									operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_PHYSDEFINITIONID; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_PHYSDEFINITIONID) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual void				Set_Value (int id)						{ (*m_Value) = id; Set_Modified (); }
	virtual int					Get_Value (void) const					{ return (*m_Value); }
	virtual void				Set_Base_Class (const char *name)	{ m_BaseClass = name; }
	virtual const char *		Get_Base_Class (void) const			{ return m_BaseClass; }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	int *							m_Value;
	StringClass					m_BaseClass;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	ModelDefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class ModelDefParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	ModelDefParameterClass (int *id);
	ModelDefParameterClass (const ModelDefParameterClass &src);
	virtual ~ModelDefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const ModelDefParameterClass &	operator= (const ModelDefParameterClass &src);
	bool									operator== (const ModelDefParameterClass &src);
	bool									operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_MODELDEFINITIONID; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_MODELDEFINITIONID) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual void				Set_Value (int id)						{ (*m_Value) = id; Set_Modified (); }
	virtual int					Get_Value (void) const					{ return (*m_Value); }
	virtual void				Set_Base_Class (const char *name)	{ m_BaseClass = name; }
	virtual const char *		Get_Base_Class (void) const			{ return m_BaseClass; }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	int *							m_Value;
	StringClass					m_BaseClass;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	DefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class DefParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	DefParameterClass (int *id);
	DefParameterClass (const DefParameterClass &src);
	virtual ~DefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const DefParameterClass &	operator= (const DefParameterClass &src);
	bool								operator== (const DefParameterClass &src);
	bool								operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	//	RTTI
	virtual DefParameterClass *	As_DefParameterClass (void)	{ return this; }

	// Data manipulation
	virtual void				Set_Value (int id)						{ (*m_Value) = id; Set_Modified (); }
	virtual int					Get_Value (void) const					{ return (*m_Value); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	int *							m_Value;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	GenericDefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class GenericDefParameterClass : public DefParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	GenericDefParameterClass (int *id);
	GenericDefParameterClass (const GenericDefParameterClass &src);
	virtual ~GenericDefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const GenericDefParameterClass &	operator= (const GenericDefParameterClass &src);
	bool										operator== (const GenericDefParameterClass &src);
	bool										operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_GENERICDEFINITIONID; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_GENERICDEFINITIONID) || ParameterClass::Is_Type (type); }

	// Class ID control
	virtual void				Set_Class_ID (int class_id)			{ m_ClassID = class_id; Set_Modified (); }
	virtual int					Get_Class_ID (void) const				{ return m_ClassID; }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	int							m_ClassID;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	GameObjDefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class GameObjDefParameterClass : public DefParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	GameObjDefParameterClass (int *id);
	GameObjDefParameterClass (const GameObjDefParameterClass &src);
	virtual ~GameObjDefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const GameObjDefParameterClass &	operator= (const GameObjDefParameterClass &src);
	bool										operator== (const GameObjDefParameterClass &src);
	bool										operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_GAMEOBJDEFINITIONID; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_GAMEOBJDEFINITIONID) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual void				Set_Base_Class (const char *name)	{ m_BaseClass = name; Set_Modified (); }
	virtual const char *		Get_Base_Class (void) const			{ return m_BaseClass; }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	StringClass					m_BaseClass;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	AmmoObjDefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class AmmoObjDefParameterClass : public GameObjDefParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	AmmoObjDefParameterClass (int *id);
	AmmoObjDefParameterClass (const AmmoObjDefParameterClass &src);
	virtual ~AmmoObjDefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const AmmoObjDefParameterClass &	operator= (const AmmoObjDefParameterClass &src);
	bool										operator== (const AmmoObjDefParameterClass &src);
	bool										operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_AMMOOBJDEFINITIONID; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_AMMOOBJDEFINITIONID) || GameObjDefParameterClass::Is_Type (type); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);
};


//////////////////////////////////////////////////////////////////////////////////
//
//	WeaponObjDefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class WeaponObjDefParameterClass : public GameObjDefParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	WeaponObjDefParameterClass (int *id);
	WeaponObjDefParameterClass (const WeaponObjDefParameterClass &src);
	virtual ~WeaponObjDefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const WeaponObjDefParameterClass &	operator= (const WeaponObjDefParameterClass &src);
	bool										operator== (const WeaponObjDefParameterClass &src);
	bool										operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_WEAPONOBJDEFINITIONID; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_WEAPONOBJDEFINITIONID) || GameObjDefParameterClass::Is_Type (type); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);
};


//////////////////////////////////////////////////////////////////////////////////
//
//	ExplosionObjDefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class ExplosionObjDefParameterClass : public GameObjDefParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	ExplosionObjDefParameterClass (int *id);
	ExplosionObjDefParameterClass (const ExplosionObjDefParameterClass &src);
	virtual ~ExplosionObjDefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const ExplosionObjDefParameterClass &	operator= (const ExplosionObjDefParameterClass &src);
	bool										operator== (const ExplosionObjDefParameterClass &src);
	bool										operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_EXPLOSIONDEFINITIONID; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_EXPLOSIONDEFINITIONID) || GameObjDefParameterClass::Is_Type (type); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);
};



//////////////////////////////////////////////////////////////////////////////////
//
//	SoundDefParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class SoundDefParameterClass : public DefParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	SoundDefParameterClass (int *id);
	SoundDefParameterClass (const SoundDefParameterClass &src);
	virtual ~SoundDefParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const SoundDefParameterClass &	operator= (const SoundDefParameterClass &src);
	bool										operator== (const SoundDefParameterClass &src);
	bool										operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const { return TYPE_SOUNDDEFINITIONID; }
	virtual bool				Is_Type (Type type) const { return (type == TYPE_SOUNDDEFINITIONID) || ParameterClass::Is_Type (type); }
};


//////////////////////////////////////////////////////////////////////////////////
//
//	ScriptParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class ScriptParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	ScriptParameterClass (StringClass *name, StringClass *params);
	ScriptParameterClass (const ScriptParameterClass &src);
	virtual ~ScriptParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const ScriptParameterClass &	operator= (const ScriptParameterClass &src);
	bool									operator== (const ScriptParameterClass &src);
	bool									operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const					{ return TYPE_SCRIPT; }
	virtual bool				Is_Type (Type type) const				{ return (type == TYPE_SCRIPT) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual void				Set_Script_Name (const char *name)	{ (*m_ScriptName) = name; Set_Modified (); }
	virtual const char *		Get_Script_Name (void) const			{ return (*m_ScriptName); }
	virtual void				Set_Params (const char *params)		{ (*m_ScriptParams) = params; Set_Modified (); }
	virtual const char *		Get_Params (void) const					{ return (*m_ScriptParams); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	StringClass *				m_ScriptName;
	StringClass *				m_ScriptParams;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	DefIDListParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class DefIDListParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	DefIDListParameterClass (DynamicVectorClass<int> *list);
	DefIDListParameterClass (const DefIDListParameterClass &src);
	virtual ~DefIDListParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const DefIDListParameterClass &	operator= (const DefIDListParameterClass &src);
	bool									operator== (const DefIDListParameterClass &src);
	bool									operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const				{ return TYPE_DEFINITIONIDLIST; }
	virtual bool				Is_Type (Type type) const			{ return (type == TYPE_DEFINITIONIDLIST) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual void				Set_Selected_Class_ID (uint32 *id)	{ m_SelectedClassID = id; }
	virtual uint32 *			Get_Selected_Class_ID (void) const	{ return m_SelectedClassID; }
	virtual void				Set_Class_ID (uint32 id)				{ m_ClassID = id; }
	virtual uint32 			Get_Class_ID (void) const				{ return m_ClassID; }

	virtual DynamicVectorClass<int> &Get_List (void) const	{ return (*m_IDList); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	DynamicVectorClass<int> *	m_IDList;
	uint32							m_ClassID;
	uint32 *							m_SelectedClassID;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	ZoneParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class ZoneParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	ZoneParameterClass (OBBoxClass *box);
	ZoneParameterClass (const ZoneParameterClass &src);
	virtual ~ZoneParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const ZoneParameterClass &		operator= (const ZoneParameterClass &src);
	bool									operator== (const ZoneParameterClass &src);
	bool									operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const				{ return TYPE_ZONE; }
	virtual bool				Is_Type (Type type) const			{ return (type == TYPE_ZONE) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual void					Set_Zone (const OBBoxClass &box)	{ (*m_OBBox) = box; Set_Modified (); }
	virtual const OBBoxClass &	Get_Zone (void) const				{ return (*m_OBBox); }


	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	OBBoxClass		*m_OBBox;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	FilenameListParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class FilenameListParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	FilenameListParameterClass (DynamicVectorClass<StringClass> *list);
	FilenameListParameterClass (const FilenameListParameterClass &src);
	virtual ~FilenameListParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const FilenameListParameterClass &	operator= (const FilenameListParameterClass &src);
	bool									operator== (const FilenameListParameterClass &src);
	bool									operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const				{ return TYPE_FILENAMELIST; }
	virtual bool				Is_Type (Type type) const			{ return (type == TYPE_FILENAMELIST) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual DynamicVectorClass<StringClass> &Get_List (void) const	{ return (*m_FilenameList); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	DynamicVectorClass<StringClass> *	m_FilenameList;
};



//////////////////////////////////////////////////////////////////////////////////
//
//	ScriptListParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class ScriptListParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	ScriptListParameterClass (DynamicVectorClass<StringClass> *name_list, DynamicVectorClass<StringClass> *param_list);
	ScriptListParameterClass (const ScriptListParameterClass &src);
	virtual ~ScriptListParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const ScriptListParameterClass &	operator= (const ScriptListParameterClass &src);
	bool										operator== (const ScriptListParameterClass &src);
	bool										operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const				{ return TYPE_SCRIPTLIST; }
	virtual bool				Is_Type (Type type) const			{ return (type == TYPE_SCRIPTLIST) || ParameterClass::Is_Type (type); }

	// Data manipulation
	virtual DynamicVectorClass<StringClass> &Get_Name_List (void) const	{ return (*m_NameList); }
	virtual DynamicVectorClass<StringClass> &Get_Param_List (void) const	{ return (*m_ParamList); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);

protected:

	//////////////////////////////////////////////////////////////////////////////
	//	Protected members
	//////////////////////////////////////////////////////////////////////////////
	bool							Are_Lists_Identical (DynamicVectorClass<StringClass> &list1, DynamicVectorClass<StringClass> &list2);

	//////////////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////////////
	DynamicVectorClass<StringClass> *	m_NameList;
	DynamicVectorClass<StringClass> *	m_ParamList;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	SeparatorParameterClass
//
//////////////////////////////////////////////////////////////////////////////////
class SeparatorParameterClass : public ParameterClass
{
public:

	//////////////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////////////////
	SeparatorParameterClass (void) {}
	SeparatorParameterClass (const SeparatorParameterClass &src);
	virtual ~SeparatorParameterClass (void) {}

	//////////////////////////////////////////////////////////////////////////////
	//	Public operators
	//////////////////////////////////////////////////////////////////////////////
	const SeparatorParameterClass &	operator= (const SeparatorParameterClass &src);
	bool									operator== (const SeparatorParameterClass &src);
	bool									operator== (const ParameterClass &src);

	//////////////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////////////

	// Type identification
	virtual Type				Get_Type (void) const				{ return TYPE_SEPARATOR; }
	virtual bool				Is_Type (Type type) const			{ return (type == TYPE_SEPARATOR) || ParameterClass::Is_Type (type); }

	// Copy methods
	virtual void				Copy_Value (const ParameterClass &src);
};


#endif //__PARAMETER_H

