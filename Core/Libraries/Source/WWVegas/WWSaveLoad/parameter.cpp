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
 *                     $Archive:: /Commando/Code/wwsaveload/parameter.cpp                     $*
 *                                                                                             *
 *                   Org Author:: Patrick Smith                                                *
 *                                                                                             *
 *                       Author:: Kenny Mitchell                                                *
 *                                                                                             *
 *                     $Modtime:: 5/29/02 11:00a                                              $*
 *                                                                                             *
 *                    $Revision:: 33                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "parameter.h"
#include "parametertypes.h"
#include "simpleparameter.h"
#include "wwstring.h"
#include "definitionclassids.h"


/////////////////////////////////////////////////////////////////////
//
// Construct
//
//	This is a virtual constructor that is capable of creating a new
// instance of any type of parameter used in the Editable system.
//
/////////////////////////////////////////////////////////////////////
ParameterClass *
ParameterClass::Construct (Type type, void *data, const char *name)
{
	ParameterClass *new_param = NULL;
	switch (type) {
		
		case TYPE_INT:
			new_param = W3DNEW IntParameterClass (data, name);
			break;

		case TYPE_FLOAT:
			new_param = W3DNEW FloatParameterClass (data, name);
			break;

		case TYPE_VECTOR2:
			new_param = W3DNEW Vector2ParameterClass (data, name);
			break;

		case TYPE_VECTOR3:
			new_param = W3DNEW Vector3ParameterClass (data, name);
			break;

		case TYPE_RECT:
			new_param = W3DNEW RectParameterClass (data, name);
			break;

		case TYPE_COLOR:
			new_param = W3DNEW ColorParameterClass (data, name);
			break;

		case TYPE_MATRIX3D:
			new_param = W3DNEW Matrix3DParameterClass (data, name);
			break;

		case TYPE_BOOL:
			new_param = W3DNEW BoolParameterClass (data, name);
			break;

		case TYPE_STRINGSDB_ID:
			new_param = W3DNEW StringsDBEntryParameterClass (data, name);
			break;

		case TYPE_ANGLE:
			new_param = W3DNEW AngleParameterClass (data, name);
			break;

		case TYPE_STRING:
			new_param = W3DNEW StringParameterClass ((StringClass *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_FILENAME:
			new_param = W3DNEW FilenameParameterClass ((StringClass *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_TEXTURE_FILENAME:
			new_param = new TextureFilenameParameterClass ((StringClass *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_SOUND_FILENAME:
			new_param = W3DNEW SoundFilenameParameterClass ((StringClass *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_ENUM:
			new_param = W3DNEW EnumParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_GENERICDEFINITIONID:
			new_param = W3DNEW GenericDefParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_GAMEOBJDEFINITIONID:
			new_param = W3DNEW GameObjDefParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_WEAPONOBJDEFINITIONID:
			new_param = W3DNEW WeaponObjDefParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_AMMOOBJDEFINITIONID:
			new_param = W3DNEW AmmoObjDefParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_EXPLOSIONDEFINITIONID:
			new_param = W3DNEW ExplosionObjDefParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;		

		case TYPE_SOUNDDEFINITIONID:
			new_param = W3DNEW SoundDefParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_MODELDEFINITIONID:
			new_param = W3DNEW ModelDefParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_PHYSDEFINITIONID:
			new_param = W3DNEW PhysDefParameterClass ((int *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_DEFINITIONIDLIST:
			new_param = W3DNEW DefIDListParameterClass ((DynamicVectorClass<int> *)data);
			new_param->Set_Name (name);
			((DefIDListParameterClass *)new_param)->Set_Class_ID (CLASSID_GAME_OBJECTS);
			break;

		case TYPE_ZONE:
			new_param = W3DNEW ZoneParameterClass ((OBBoxClass  *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_FILENAMELIST:
			new_param = W3DNEW FilenameListParameterClass ((DynamicVectorClass<StringClass> *)data);
			new_param->Set_Name (name);
			break;

		case TYPE_SEPARATOR:
			new_param = W3DNEW SeparatorParameterClass;
			new_param->Set_Name (name);
			break;
	}

	return new_param;
}

//*******************************************************************************************//
//
//	Start of StringParameterClass
//
//*******************************************************************************************//

/////////////////////////////////////////////////////////////////////
//
//	StringParameterClass
//
/////////////////////////////////////////////////////////////////////
StringParameterClass::StringParameterClass (StringClass *string)
	:	m_String (string)
{	
	return ;
}

/////////////////////////////////////////////////////////////////////
//
//	StringParameterClass
//
/////////////////////////////////////////////////////////////////////
StringParameterClass::StringParameterClass (const StringParameterClass &src)
	:	m_String (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const StringParameterClass &
StringParameterClass::operator= (const StringParameterClass &src)
{
	m_String = src.m_String;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
StringParameterClass::operator== (const StringParameterClass &src)
{
	bool retval = false;

	if (m_String != NULL && src.m_String != NULL &&
		 (m_String->Compare (*(src.m_String)) == 0)) {
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
StringParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = StringParameterClass::operator== ((const StringParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
StringParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_STRING)) {
		(*m_String) = ((StringParameterClass &)src).Get_String ();
	}

	ParameterClass::Copy_Value (src);
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Get_String
//
/////////////////////////////////////////////////////////////////////
const char *
StringParameterClass::Get_String (void) const
{
	const char * string = NULL;
	if (m_String != NULL) {
		string = (*m_String);
	}
	return string;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_String
//
/////////////////////////////////////////////////////////////////////
void
StringParameterClass::Set_String (const char * string)
{
	if (m_String != NULL) {
		Set_Modified ();
		(*m_String) = string;
	}

	return ;
}


//*******************************************************************************************//
//
//	Start of FilenameParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	FilenameParameterClass
//
/////////////////////////////////////////////////////////////////////
FilenameParameterClass::FilenameParameterClass (StringClass *string)
	:	StringParameterClass (string)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	FilenameParameterClass
//
/////////////////////////////////////////////////////////////////////
FilenameParameterClass::FilenameParameterClass (const FilenameParameterClass &src)
	:	StringParameterClass (src)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const FilenameParameterClass &
FilenameParameterClass::operator= (const FilenameParameterClass &src)
{
	StringParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
FilenameParameterClass::operator== (const FilenameParameterClass &src)
{
	return StringParameterClass::operator== (src);
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
FilenameParameterClass::operator== (const ParameterClass &src)
{
	return StringParameterClass::operator== (src);
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
FilenameParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_FILENAME)) {
		Set_String (((FilenameParameterClass &)src).Get_String ());
	}

	StringParameterClass::Copy_Value (src);
	return ;
}

//*******************************************************************************************//
//
//	Start of TextureFilenameParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	TextureFilenameParameterClass
//
/////////////////////////////////////////////////////////////////////
TextureFilenameParameterClass::TextureFilenameParameterClass (StringClass *string)
:	FilenameParameterClass (string),
	Show_Alpha(false),
	Show_Texture(false)
{	
}


/////////////////////////////////////////////////////////////////////
//
//	TextureFilenameParameterClass
//
/////////////////////////////////////////////////////////////////////
TextureFilenameParameterClass::TextureFilenameParameterClass (const TextureFilenameParameterClass &src)
:	FilenameParameterClass (src),
	Show_Alpha(false),
	Show_Texture(false)
{
}

/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void TextureFilenameParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_TEXTURE_FILENAME)) 
	{
		Set_String (((FilenameParameterClass &)src).Get_String ());
	}

	StringParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of SoundFilenameParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	SoundFilenameParameterClass
//
/////////////////////////////////////////////////////////////////////
SoundFilenameParameterClass::SoundFilenameParameterClass (StringClass *string)
	:	FilenameParameterClass (string)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	SoundFilenameParameterClass
//
/////////////////////////////////////////////////////////////////////
SoundFilenameParameterClass::SoundFilenameParameterClass (const SoundFilenameParameterClass &src)
	:	FilenameParameterClass (src)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const SoundFilenameParameterClass &
SoundFilenameParameterClass::operator= (const SoundFilenameParameterClass &src)
{
	FilenameParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
SoundFilenameParameterClass::operator== (const SoundFilenameParameterClass &src)
{
	return FilenameParameterClass::operator== (src);
}



//*******************************************************************************************//
//
//	Start of EnumParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	EnumParameterClass
//
/////////////////////////////////////////////////////////////////////
EnumParameterClass::EnumParameterClass (int *value)
	:	m_Value (value)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	EnumParameterClass
//
/////////////////////////////////////////////////////////////////////
EnumParameterClass::EnumParameterClass (const EnumParameterClass &src)
	:	m_Value (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const EnumParameterClass &
EnumParameterClass::operator= (const EnumParameterClass &src)
{
	m_List.Delete_All ();

	m_Value	= src.m_Value;
	m_List	= src.m_List;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
EnumParameterClass::operator== (const EnumParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
EnumParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = EnumParameterClass::operator== ((const EnumParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
EnumParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_ENUM)) {
		(*m_Value) = ((EnumParameterClass &)src).Get_Selected_Value ();
	}

	ParameterClass::Copy_Value (src);
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Add_Value
//
/////////////////////////////////////////////////////////////////////
void
EnumParameterClass::Add_Value (const char *display_name, int value)
{
	m_List.Add (ENUM_VALUE(display_name, value));
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Add_Value
//
/////////////////////////////////////////////////////////////////////
void __cdecl 
EnumParameterClass::Add_Values (const char *first_name, int first_value, ...)
{
	m_List.Add (ENUM_VALUE(first_name, first_value));

	va_list arg_list;
	va_start (arg_list, first_value);

	//
	//	Add all the params on the stack (until we found
	// the terminator)
	//
	bool more_params = true;	
	while (more_params) {

		//
		//	Get the string param
		//
		const char *name = va_arg (arg_list, const char *);
		if (name == NULL) {
			more_params = false;
		} else {
			
			//
			//	Add the string/id pair to the enum list
			//
			int value = va_arg (arg_list, int);
			m_List.Add (ENUM_VALUE(name, value));
		}		
	}
	
	va_end (arg_list);
	return ;
}



//*******************************************************************************************//
//
//	Start of PhysDefParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	PhysDefParameterClass
//
/////////////////////////////////////////////////////////////////////
PhysDefParameterClass::PhysDefParameterClass (int *id)
	:	m_Value (id)
{	
	return ;
}

/////////////////////////////////////////////////////////////////////
//
//	PhysDefParameterClass
//
/////////////////////////////////////////////////////////////////////
PhysDefParameterClass::PhysDefParameterClass (const PhysDefParameterClass &src)
	:	m_Value (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const PhysDefParameterClass &
PhysDefParameterClass::operator= (const PhysDefParameterClass &src)
{
	m_Value = src.m_Value;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
PhysDefParameterClass::operator== (const PhysDefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
PhysDefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = PhysDefParameterClass::operator== ((const PhysDefParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
PhysDefParameterClass::Copy_Value (const ParameterClass &/*src*/)
{
	//
	//	We don't allow the value to be copied
	//
	return ;
}


//*******************************************************************************************//
//
//	Start of ModelDefParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	ModelDefParameterClass
//
/////////////////////////////////////////////////////////////////////
ModelDefParameterClass::ModelDefParameterClass (int *id)
	:	m_Value (id)
{	
	return ;
}

/////////////////////////////////////////////////////////////////////
//
//	ModelDefParameterClass
//
/////////////////////////////////////////////////////////////////////
ModelDefParameterClass::ModelDefParameterClass (const ModelDefParameterClass &src)
	:	m_Value (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const ModelDefParameterClass &
ModelDefParameterClass::operator= (const ModelDefParameterClass &src)
{
	m_Value = src.m_Value;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ModelDefParameterClass::operator== (const ModelDefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ModelDefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = ModelDefParameterClass::operator== ((const ModelDefParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
ModelDefParameterClass::Copy_Value (const ParameterClass &/*src*/)
{
	//
	//	We don't allow the value to be copied
	//
	return ;
}


//*******************************************************************************************//
//
//	Start of DefParameterClass
//
//*******************************************************************************************//

/////////////////////////////////////////////////////////////////////
//
//	DefParameterClass
//
/////////////////////////////////////////////////////////////////////
DefParameterClass::DefParameterClass (int *id)
	:	m_Value (id)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	DefParameterClass
//
/////////////////////////////////////////////////////////////////////
DefParameterClass::DefParameterClass (const DefParameterClass &src)
	:	m_Value (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const DefParameterClass &
DefParameterClass::operator= (const DefParameterClass &src)
{
	m_Value = src.m_Value;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
DefParameterClass::operator== (const DefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
DefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = DefParameterClass::operator== ((const DefParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
DefParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Get_Type () == Get_Type ()) {
		(*m_Value) = ((DefParameterClass &)src).Get_Value ();
	}

	ParameterClass::Copy_Value (src);
	return ;
}



//*******************************************************************************************//
//
//	Start of GenericDefParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	GenericDefParameterClass
//
/////////////////////////////////////////////////////////////////////
GenericDefParameterClass::GenericDefParameterClass (int *id)
	:	m_ClassID (0),
		DefParameterClass (id)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	GenericDefParameterClass
//
/////////////////////////////////////////////////////////////////////
GenericDefParameterClass::GenericDefParameterClass (const GenericDefParameterClass &src)
	:	m_ClassID (0),
		DefParameterClass (src)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const GenericDefParameterClass &
GenericDefParameterClass::operator= (const GenericDefParameterClass &src)
{
	DefParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
GenericDefParameterClass::operator== (const GenericDefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
GenericDefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = GenericDefParameterClass::operator== ((const GenericDefParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
GenericDefParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_GENERICDEFINITIONID)) {
		(*m_Value) = ((GenericDefParameterClass &)src).Get_Value ();
	}

	ParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of GameObjDefParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	GameObjDefParameterClass
//
/////////////////////////////////////////////////////////////////////
GameObjDefParameterClass::GameObjDefParameterClass (int *id)
	:	DefParameterClass (id)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	GameObjDefParameterClass
//
/////////////////////////////////////////////////////////////////////
GameObjDefParameterClass::GameObjDefParameterClass (const GameObjDefParameterClass &src)
	:	DefParameterClass (src)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const GameObjDefParameterClass &
GameObjDefParameterClass::operator= (const GameObjDefParameterClass &src)
{
	DefParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
GameObjDefParameterClass::operator== (const GameObjDefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
GameObjDefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = GameObjDefParameterClass::operator== ((const GameObjDefParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
GameObjDefParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_GAMEOBJDEFINITIONID)) {
		(*m_Value) = ((GameObjDefParameterClass &)src).Get_Value ();
	}

	ParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of WeaponObjDefParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	WeaponObjDefParameterClass
//
/////////////////////////////////////////////////////////////////////
WeaponObjDefParameterClass::WeaponObjDefParameterClass (int *id)
	:	GameObjDefParameterClass (id)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	WeaponObjDefParameterClass
//
/////////////////////////////////////////////////////////////////////
WeaponObjDefParameterClass::WeaponObjDefParameterClass (const WeaponObjDefParameterClass &src)
	:	GameObjDefParameterClass (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const WeaponObjDefParameterClass &
WeaponObjDefParameterClass::operator= (const WeaponObjDefParameterClass &src)
{
	m_Value = src.m_Value;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
WeaponObjDefParameterClass::operator== (const WeaponObjDefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
WeaponObjDefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = WeaponObjDefParameterClass::operator== ((const WeaponObjDefParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
WeaponObjDefParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_WEAPONOBJDEFINITIONID)) {
		(*m_Value) = ((WeaponObjDefParameterClass &)src).Get_Value ();
	}

	GameObjDefParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of AmmoObjDefParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	AmmoObjDefParameterClass
//
/////////////////////////////////////////////////////////////////////
AmmoObjDefParameterClass::AmmoObjDefParameterClass (int *id)
	:	GameObjDefParameterClass (id)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	AmmoObjDefParameterClass
//
/////////////////////////////////////////////////////////////////////
AmmoObjDefParameterClass::AmmoObjDefParameterClass (const AmmoObjDefParameterClass &src)
	:	GameObjDefParameterClass (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const AmmoObjDefParameterClass &
AmmoObjDefParameterClass::operator= (const AmmoObjDefParameterClass &src)
{
	m_Value = src.m_Value;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
AmmoObjDefParameterClass::operator== (const AmmoObjDefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
AmmoObjDefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = AmmoObjDefParameterClass::operator== ((const AmmoObjDefParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
AmmoObjDefParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_AMMOOBJDEFINITIONID)) {
		(*m_Value) = ((AmmoObjDefParameterClass &)src).Get_Value ();
	}

	GameObjDefParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of ExplosionObjDefParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	ExplosionObjDefParameterClass
//
/////////////////////////////////////////////////////////////////////
ExplosionObjDefParameterClass::ExplosionObjDefParameterClass (int *id)
	:	GameObjDefParameterClass (id)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	ExplosionObjDefParameterClass
//
/////////////////////////////////////////////////////////////////////
ExplosionObjDefParameterClass::ExplosionObjDefParameterClass (const ExplosionObjDefParameterClass &src)
	:	GameObjDefParameterClass (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const ExplosionObjDefParameterClass &
ExplosionObjDefParameterClass::operator= (const ExplosionObjDefParameterClass &src)
{
	m_Value = src.m_Value;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ExplosionObjDefParameterClass::operator== (const ExplosionObjDefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ExplosionObjDefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = ExplosionObjDefParameterClass::operator== ((const ExplosionObjDefParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
ExplosionObjDefParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_AMMOOBJDEFINITIONID)) {
		(*m_Value) = ((ExplosionObjDefParameterClass &)src).Get_Value ();
	}

	GameObjDefParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of SoundDefParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	SoundDefParameterClass
//
/////////////////////////////////////////////////////////////////////
SoundDefParameterClass::SoundDefParameterClass (int *id)
	:	DefParameterClass (id)
{	
	return ;
}

/////////////////////////////////////////////////////////////////////
//
//	SoundDefParameterClass
//
/////////////////////////////////////////////////////////////////////
SoundDefParameterClass::SoundDefParameterClass (const SoundDefParameterClass &src)
	:	DefParameterClass (src)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const SoundDefParameterClass &
SoundDefParameterClass::operator= (const SoundDefParameterClass &src)
{
	DefParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
SoundDefParameterClass::operator== (const SoundDefParameterClass &src)
{
	bool retval = false;

	if (m_Value != NULL && src.m_Value != NULL &&
		 (*m_Value) == (*src.m_Value))
	{
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
SoundDefParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = SoundDefParameterClass::operator== ((const SoundDefParameterClass &)src);
	}

	return retval;
}


//*******************************************************************************************//
//
//	Start of ScriptParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	ScriptParameterClass
//
/////////////////////////////////////////////////////////////////////
ScriptParameterClass::ScriptParameterClass (StringClass *name, StringClass *params)
	:	m_ScriptName (name),
		m_ScriptParams (params)
{	
	return ;
}

/////////////////////////////////////////////////////////////////////
//
//	ScriptParameterClass
//
/////////////////////////////////////////////////////////////////////
ScriptParameterClass::ScriptParameterClass (const ScriptParameterClass &src)
	:	m_ScriptName (NULL),
		m_ScriptParams (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const ScriptParameterClass &
ScriptParameterClass::operator= (const ScriptParameterClass &src)
{
	m_ScriptName	= src.m_ScriptName;
	m_ScriptParams = src.m_ScriptParams;
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ScriptParameterClass::operator== (const ScriptParameterClass &src)
{
	bool retval = false;

	//
	//	Data valid?
	//
	if (	(m_ScriptName != NULL) && (src.m_ScriptName != NULL) &&
			(m_ScriptParams != NULL) && (src.m_ScriptParams != NULL))
	{
	
		//
		//	Simple string compares should workd
		//
		if (	(m_ScriptName->Compare (*(src.m_ScriptName)) == 0) &&
				(m_ScriptParams->Compare (*(src.m_ScriptParams)) == 0))
		{
			retval = true;
		}
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ScriptParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = ScriptParameterClass::operator== ((const ScriptParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
ScriptParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_SCRIPT)) {
		(*m_ScriptName)	= ((ScriptParameterClass &)src).Get_Script_Name ();
		(*m_ScriptParams)	= ((ScriptParameterClass &)src).Get_Params ();
	}

	ParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of DefIDListParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	DefIDListParameterClass
//
/////////////////////////////////////////////////////////////////////
DefIDListParameterClass::DefIDListParameterClass (DynamicVectorClass<int> *list)
	:	m_IDList (list),
		m_ClassID (0),
		m_SelectedClassID (NULL)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	DefIDListParameterClass
//
/////////////////////////////////////////////////////////////////////
DefIDListParameterClass::DefIDListParameterClass (const DefIDListParameterClass &src)
	:	m_IDList (NULL),
		m_ClassID (0),
		m_SelectedClassID (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const DefIDListParameterClass &
DefIDListParameterClass::operator= (const DefIDListParameterClass &src)
{
	m_IDList				= src.m_IDList;
	m_ClassID			= src.m_ClassID;
	m_SelectedClassID	= src.m_SelectedClassID;

	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
DefIDListParameterClass::operator== (const DefIDListParameterClass &src)
{
	bool retval = false;

	//
	//	Data valid?
	//
	if ((m_IDList != NULL) && (src.m_IDList != NULL))
	{
		//
		//	Class IDs the same?
		//
		if (m_ClassID == src.m_ClassID) {

			int count1 = m_IDList->Count ();
			int count2 = src.m_IDList->Count ();

			//
			//	Are the lists the same?
			//
			retval = (count1 == count2);
			for (int index = 0; (index < count1) && retval; index ++) {
				int value1 = (*m_IDList)[index];
				int value2 = (*src.m_IDList)[index];
				retval &= (value1 == value2);
			}
		}
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
DefIDListParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = DefIDListParameterClass::operator== ((const DefIDListParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
DefIDListParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_DEFINITIONIDLIST)) {
		DefIDListParameterClass real_src = (DefIDListParameterClass &)src;
		
		m_ClassID				= real_src.m_ClassID;		
		(*m_IDList)				= (*real_src.m_IDList);

		if (m_SelectedClassID != NULL && real_src.m_SelectedClassID != NULL) {
			(*m_SelectedClassID)	= (*real_src.m_SelectedClassID);
		}
	}

	ParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of ZoneParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	ZoneParameterClass
//
/////////////////////////////////////////////////////////////////////
ZoneParameterClass::ZoneParameterClass (OBBoxClass *box)
	:	m_OBBox (box)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	ZoneParameterClass
//
/////////////////////////////////////////////////////////////////////
ZoneParameterClass::ZoneParameterClass (const ZoneParameterClass &src)
	:	m_OBBox (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const ZoneParameterClass &
ZoneParameterClass::operator= (const ZoneParameterClass &src)
{
	m_OBBox = src.m_OBBox;

	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ZoneParameterClass::operator== (const ZoneParameterClass &src)
{
	bool retval = false;

	//
	//	Are the OBBoxes the same?
	//
	if ((m_OBBox != NULL) && (src.m_OBBox != NULL)) {
		retval = (*m_OBBox) == (*src.m_OBBox);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ZoneParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = ZoneParameterClass::operator== ((const ZoneParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
ZoneParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_ZONE)) {
		ZoneParameterClass real_src = (ZoneParameterClass &)src;
		
		(*m_OBBox)	= (*real_src.m_OBBox);
	}

	ParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of FilenameListParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	FilenameListParameterClass
//
/////////////////////////////////////////////////////////////////////
FilenameListParameterClass::FilenameListParameterClass (DynamicVectorClass<StringClass> *list)
	:	m_FilenameList (list)		
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	FilenameListParameterClass
//
/////////////////////////////////////////////////////////////////////
FilenameListParameterClass::FilenameListParameterClass (const FilenameListParameterClass &src)
	:	m_FilenameList (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const FilenameListParameterClass &
FilenameListParameterClass::operator= (const FilenameListParameterClass &src)
{
	m_FilenameList	= src.m_FilenameList;

	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
FilenameListParameterClass::operator== (const FilenameListParameterClass &src)
{
	bool retval = false;

	//
	//	Data valid?
	//
	if ((m_FilenameList != NULL) && (src.m_FilenameList != NULL))
	{
		int count1 = m_FilenameList->Count ();
		int count2 = src.m_FilenameList->Count ();

		//
		//	Are the lists the same?
		//
		retval = (count1 == count2);
		for (int index = 0; (index < count1) && retval; index ++) {
			StringClass &filename1 = (*m_FilenameList)[index];
			StringClass &filename2 = (*src.m_FilenameList)[index];
			retval &= (::stricmp (filename1, filename2) == 0);
		}
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
FilenameListParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = FilenameListParameterClass::operator== ((const FilenameListParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
FilenameListParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_FILENAMELIST)) {
		FilenameListParameterClass real_src = (FilenameListParameterClass &)src;		
		(*m_FilenameList) = (*real_src.m_FilenameList);
	}

	ParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of ScriptListParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	ScriptListParameterClass
//
/////////////////////////////////////////////////////////////////////
ScriptListParameterClass::ScriptListParameterClass
(
	DynamicVectorClass<StringClass> *name_list,
	DynamicVectorClass<StringClass> *param_list
)
	:	m_NameList (name_list),
		m_ParamList (param_list)
{	
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	ScriptListParameterClass
//
/////////////////////////////////////////////////////////////////////
ScriptListParameterClass::ScriptListParameterClass (const ScriptListParameterClass &src)
	:	m_NameList (NULL),
		m_ParamList (NULL)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const ScriptListParameterClass &
ScriptListParameterClass::operator= (const ScriptListParameterClass &src)
{
	m_NameList	= src.m_NameList;
	m_ParamList	= src.m_ParamList;

	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ScriptListParameterClass::operator== (const ScriptListParameterClass &src)
{
	bool retval = false;

	//
	//	Data valid?
	//
	if (	(m_NameList != NULL) && (src.m_NameList != NULL) &&
			(m_ParamList != NULL) && (src.m_ParamList != NULL))
	{
		retval = Are_Lists_Identical (*m_NameList, *(src.m_NameList));
		retval &= Are_Lists_Identical (*m_ParamList, *(src.m_ParamList));
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Are_Lists_Identical
//
/////////////////////////////////////////////////////////////////////
bool
ScriptListParameterClass::Are_Lists_Identical
(
	DynamicVectorClass<StringClass> &list1,
	DynamicVectorClass<StringClass> &list2
)
{
	int count1 = list1.Count ();
	int count2 = list2.Count ();

	//
	//	Do a string compare on every entry
	//
	bool retval = (count1 == count2);
	for (int index = 0; (index < count1) && retval; index ++) {
		StringClass &string1 = list1[index];
		StringClass &string2 = list2[index];
		retval &= (::stricmp (string1, string2) == 0);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
ScriptListParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = ScriptListParameterClass::operator== ((const ScriptListParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
ScriptListParameterClass::Copy_Value (const ParameterClass &src)
{
	if (src.Is_Type (ParameterClass::TYPE_SCRIPTLIST)) {
		ScriptListParameterClass &real_src = (ScriptListParameterClass &)src;		
		(*m_NameList)	= (*real_src.m_NameList);
		(*m_ParamList)	= (*real_src.m_ParamList);
	}

	ParameterClass::Copy_Value (src);
	return ;
}


//*******************************************************************************************//
//
//	Start of SeparatorParameterClass
//
//*******************************************************************************************//


/////////////////////////////////////////////////////////////////////
//
//	SeparatorParameterClass
//
/////////////////////////////////////////////////////////////////////
SeparatorParameterClass::SeparatorParameterClass (const SeparatorParameterClass &src)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	operator=
//
/////////////////////////////////////////////////////////////////////
const SeparatorParameterClass &
SeparatorParameterClass::operator= (const SeparatorParameterClass &src)
{
	ParameterClass::operator= (src);
	return *this;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
SeparatorParameterClass::operator== (const SeparatorParameterClass &src)
{
	return true;
}


/////////////////////////////////////////////////////////////////////
//
//	operator==
//
/////////////////////////////////////////////////////////////////////
bool
SeparatorParameterClass::operator== (const ParameterClass &src)
{
	bool retval = false;

	if (src.Get_Type () == Get_Type ()) {
		retval = SeparatorParameterClass::operator== ((const SeparatorParameterClass &)src);
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Copy_Value
//
/////////////////////////////////////////////////////////////////////
void
SeparatorParameterClass::Copy_Value (const ParameterClass &src)
{
	ParameterClass::Copy_Value (src);
	return ;
}
