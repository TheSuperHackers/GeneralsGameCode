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
 *                 Project Name : Westwood Library                                             *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/registry.cpp                           $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/27/01 2:03p                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "registry.h"
#include "RAWFILE.h"
#include "INI.h"
#include "inisup.h"
#include <assert.h>
#include <windows.h>

//#include "wwdebug.h"

bool RegistryClass::IsLocked = false;


bool RegistryClass::Exists(const char* sub_key)
{
	HKEY hKey;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sub_key, 0, KEY_READ, &hKey);

	if (ERROR_SUCCESS == result) {
		RegCloseKey(hKey);
		return true;
	}

	return false;
}

/*
**
*/
RegistryClass::RegistryClass( const char * sub_key, bool create ) :
	IsValid( false )
{
	HKEY key;
	assert( sizeof(HKEY) == sizeof(int) );

	LONG result = -1;

	if (create && !IsLocked) {
		DWORD disposition;
		result = RegCreateKeyEx(HKEY_LOCAL_MACHINE, sub_key, 0, NULL, 0,
			KEY_ALL_ACCESS, NULL, &key, &disposition);
	} else {
		result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sub_key, 0, IsLocked ? KEY_READ : KEY_ALL_ACCESS, &key);
	}

	if (ERROR_SUCCESS == result) {
		IsValid = true;
		Key = (int)key;
	}
}

RegistryClass::~RegistryClass( void )
{
	if ( IsValid ) {
		if (::RegCloseKey( (HKEY)Key ) != ERROR_SUCCESS) {		// Close the reg key
		}
		IsValid = false;
	}
}

int	RegistryClass::Get_Int( const char * name, int def_value )
{
	assert( IsValid );
	DWORD type, data = 0, data_len = sizeof( data );
	if (( ::RegQueryValueEx( (HKEY)Key, name, NULL, &type, (LPBYTE)&data, &data_len ) ==
		ERROR_SUCCESS ) && ( type == REG_DWORD )) {
	} else {
		data = def_value;
	}
	return data;
}

void	RegistryClass::Set_Int( const char * name, int value )
{
	assert( IsValid );
	if (IsLocked) {
		return;
	}
	if (::RegSetValueEx( (HKEY)Key, name, 0, REG_DWORD, (LPBYTE)&value, sizeof( DWORD ) ) !=
			ERROR_SUCCESS) {
	}
}


bool	RegistryClass::Get_Bool( const char * name, bool def_value )
{
	return (Get_Int( name, def_value ) != 0);
}

void	RegistryClass::Set_Bool( const char * name, bool value )
{
	Set_Int( name, value ? 1 : 0 );
}


float	RegistryClass::Get_Float( const char * name, float def_value )
{
	assert( IsValid );
	float data = 0;
	DWORD type, data_len = sizeof( data );
	if (( ::RegQueryValueEx( (HKEY)Key, name, NULL, &type, (LPBYTE)&data, &data_len ) ==
		ERROR_SUCCESS ) && ( type == REG_DWORD )) {
	} else {
		data = def_value;
	}
	return data;
}

void	RegistryClass::Set_Float( const char * name, float value )
{
	assert( IsValid );
	if (IsLocked) {
		return;
	}
	if (::RegSetValueEx( (HKEY)Key, name, 0, REG_DWORD, (LPBYTE)&value, sizeof( DWORD ) ) !=
			ERROR_SUCCESS) {
	}
}

int RegistryClass::Get_Bin_Size( const char * name )
{
	assert( IsValid );

	unsigned long size = 0;
	::RegQueryValueEx( (HKEY)Key, name, NULL, NULL, NULL, &size );
	return size;
}


void RegistryClass::Get_Bin( const char * name, void *buffer, int buffer_size )
{
	assert( IsValid );
	assert( buffer != NULL );
	assert( buffer_size > 0 );

	unsigned long size = buffer_size;
	::RegQueryValueEx( (HKEY)Key, name, NULL, NULL, (LPBYTE)buffer, &size );
	return ;
}

void	RegistryClass::Set_Bin( const char * name, const void *buffer, int buffer_size )
{
	assert( IsValid );
	assert( buffer != NULL );
	assert( buffer_size > 0 );

	if (IsLocked) {
		return;
	}
	::RegSetValueEx( (HKEY)Key, name, 0, REG_BINARY, (LPBYTE)buffer, buffer_size );
	return ;
}

void	RegistryClass::Get_String( const char * name, StringClass &string, const char *default_string )
{
	assert( IsValid );
	string = (default_string == NULL) ? "" : default_string;

	//
	//	Get the size of the entry
	//
	DWORD data_size = 0;
	DWORD type = 0;
	LONG result = ::RegQueryValueEx ((HKEY)Key, name, NULL, &type, NULL, &data_size);
	if (result == ERROR_SUCCESS && type == REG_SZ) {

		//
		//	Read the entry from the registry
		//
		::RegQueryValueEx ((HKEY)Key, name, NULL, &type,
			(LPBYTE)string.Get_Buffer (data_size), &data_size);
	}

	return ;
}


char *RegistryClass::Get_String( const char * name, char *value, int value_size,
   const char * default_string )
{
	assert( IsValid );
	DWORD type = 0;
	if (( ::RegQueryValueEx( (HKEY)Key, name, NULL, &type, (LPBYTE)value, (DWORD*)&value_size ) ==
			ERROR_SUCCESS ) && ( type == REG_SZ )) {
	} else {
		//*value = 0;
		//value = (char *) default_string;
      if (default_string == NULL) {
		   *value = 0;
      } else {
         assert(strlen(default_string) < (unsigned int) value_size);
         strcpy(value, default_string);
      }
	}
	return value;
}

void	RegistryClass::Set_String( const char * name, const char *value )
{
	assert( IsValid );
   int size = strlen( value ) + 1; // must include NULL
	if (IsLocked) {
		return;
	}
	if (::RegSetValueEx( (HKEY)Key, name, 0, REG_SZ, (LPBYTE)value, size ) !=
		ERROR_SUCCESS ) {
	}
}

void	RegistryClass::Get_Value_List( DynamicVectorClass<StringClass> &list )
{
	char value_name[128];

	//
	//	Simply enumerate all the values in this key
	//
	int index = 0;
	unsigned long sizeof_name = sizeof (value_name);
	while (::RegEnumValue ((HKEY)Key, index ++,
					value_name, &sizeof_name, 0, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		sizeof_name = sizeof (value_name);

		//
		//	Add this value name to the list
		//
		list.Add( value_name );
	}

	return ;
}

void	RegistryClass::Delete_Value( const char * name)
{
	if (IsLocked) {
		return;
	}
	::RegDeleteValue( (HKEY)Key, name );
	return ;
}

void	RegistryClass::Deleta_All_Values( void )
{
	if (IsLocked) {
		return;
	}
	//
	//	Build a list of the values in this key
	//
	DynamicVectorClass<StringClass> value_list;
	Get_Value_List (value_list);

	//
	//	Loop over and delete each value
	//
	for (int index = 0; index < value_list.Count (); index ++) {
		Delete_Value( value_list[index] );
	}

	return ;
}


void	RegistryClass::Get_String( const WCHAR * name, WideStringClass &string, const WCHAR *default_string )
{
	assert( IsValid );
	string = (default_string == NULL) ? L"" : default_string;

	//
	//	Get the size of the entry
	//
	DWORD data_size = 0;
	DWORD type = 0;
	LONG result = ::RegQueryValueExW ((HKEY)Key, name, NULL, &type, NULL, &data_size);
	if (result == ERROR_SUCCESS && type == REG_SZ) {

		//
		//	Read the entry from the registry
		//
		::RegQueryValueExW ((HKEY)Key, name, NULL, &type,
			(LPBYTE)string.Get_Buffer ((data_size / 2) + 1), &data_size);
	}

	return ;
}


void	RegistryClass::Set_String( const WCHAR * name, const WCHAR *value )
{
	assert( IsValid );

   //
	//	Determine the size
	//
	int size = wcslen( value ) + 1;
	size		= size * 2;

	//
	//	Set the registry key
	//
	if (IsLocked) {
		return;
	}
	::RegSetValueExW ( (HKEY)Key, name, 0, REG_SZ, (LPBYTE)value, size );
	return ;
}









/***********************************************************************************************
 * RegistryClass::Save_Registry_Values -- Save values in a key to an .ini file                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Handle to key                                                                     *
 *           Path to key                                                                       *
 *           INI                                                                               *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:32PM ST : Created                                                            *
 *=============================================================================================*/
void RegistryClass::Save_Registry_Values(HKEY key, char *path, INIClass *ini)
{
	int index = 0;
	long result = ERROR_SUCCESS;
	char save_name[512];

	while (result == ERROR_SUCCESS) {
		unsigned long type = 0;
		unsigned char data[8192];
		unsigned long data_size = sizeof(data);
		char value_name[256];
		unsigned long value_name_size = sizeof(value_name);

		result = RegEnumValue(key, index, value_name, &value_name_size, 0, &type, data, &data_size);

		if (result == ERROR_SUCCESS) {
			switch (type) {

				/*
				** Handle dword values.
				*/
				case REG_DWORD:
					strcpy(save_name, "DWORD_");
					strlcat(save_name, value_name, ARRAY_SIZE(save_name));
					ini->Put_Int(path, save_name, *((unsigned long*)data));
					break;

				/*
				** Handle string values.
				*/
				case REG_SZ:
					strcpy(save_name, "STRING_");
					strlcat(save_name, value_name, ARRAY_SIZE(save_name));
					ini->Put_String(path, save_name, (char*)data);
					break;

				/*
				** Handle binary values.
				*/
				case REG_BINARY:
					strcpy(save_name, "BIN_");
					strlcat(save_name, value_name, ARRAY_SIZE(save_name));
					ini->Put_UUBlock(path, save_name, (char*)data, data_size);
					break;

				/*
				** Anything else isn't handled yet.
				*/
				default:
					WWASSERT(type == REG_DWORD || type == REG_SZ || type == REG_BINARY);
					break;
			}
		}
		index++;
	}
}





/***********************************************************************************************
 * RegistryClass::Save_Registry_Tree -- Save out a whole chunk or registry as an .INI          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Registry path                                                                     *
 *           INI to write to                                                                   *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:33PM ST : Created                                                            *
 *=============================================================================================*/
void RegistryClass::Save_Registry_Tree(char *path, INIClass *ini)
{
	HKEY base_key;
	HKEY sub_key;
	int index = 0;
	char name[256];
	unsigned long name_size = sizeof(name);
	char class_name[256];
	unsigned long class_name_size = sizeof(class_name);
	FILETIME file_time;
	memset(&file_time, 0, sizeof(file_time));


	long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_ALL_ACCESS, &base_key);

	WWASSERT(result == ERROR_SUCCESS);

	if (result == ERROR_SUCCESS) {

		Save_Registry_Values(base_key, path, ini);

		while (result == ERROR_SUCCESS) {
			class_name_size = sizeof(class_name);
			name_size = sizeof(name);
			result = RegEnumKeyEx(base_key, index, name, &name_size, 0, class_name, &class_name_size, &file_time);
			if (result == ERROR_SUCCESS) {

				/*
				** See if there are sub keys.
				*/
				char new_key_path[512];
				strcpy(new_key_path, path);
				strlcat(new_key_path, "\\", ARRAY_SIZE(new_key_path));
				strlcat(new_key_path, name, ARRAY_SIZE(new_key_path));

				unsigned long num_subs = 0;
				unsigned long num_values = 0;

				long new_result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, new_key_path, 0, KEY_ALL_ACCESS, &sub_key);
				if (new_result == ERROR_SUCCESS) {
					new_result = RegQueryInfoKey(sub_key, NULL, NULL, 0, &num_subs, NULL, NULL, &num_values, NULL, NULL, NULL, NULL);

					/*
					** If there are sun keys then enumerate those.
					*/
					if (num_subs > 0) {
						Save_Registry_Tree(new_key_path, ini);
					}

					if (num_values > 0) {
						Save_Registry_Values(sub_key, new_key_path, ini);
					}
					RegCloseKey(sub_key);
				}
			}
			index++;
		}
		RegCloseKey(base_key);
	}
}





/***********************************************************************************************
 * RegistryClass::Save_Registry -- Save a chunk of registry to an .ini file.                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    File name                                                                         *
 *           Registry path                                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:36PM ST : Created                                                            *
 *=============================================================================================*/
void RegistryClass::Save_Registry(const char *filename, char *path)
{
	RawFileClass file(filename);
	INIClass ini;
	Save_Registry_Tree(path, &ini);
	ini.Save(file);
}



/***********************************************************************************************
 * RegistryClass::Load_Registry -- Load a chunk of registry from an .INI file                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:35PM ST : Created                                                            *
 *=============================================================================================*/
void RegistryClass::Load_Registry(const char *filename, char *old_path, char *new_path)
{
	if (!IsLocked) {
		RawFileClass file(filename);
		INIClass ini;
		ini.Load(file);

		int old_path_len = strlen(old_path);
		char path[1024];
		char string[1024];
		unsigned char buffer[8192];


		List<INISection *> &section_list = ini.Get_Section_List();

		for (INISection *section = section_list.First() ; section != NULL ; section = section->Next_Valid()) {

			/*
			** Build the new path to use in the registry.
			*/
			char *section_name = section->Section;
			strcpy(path, new_path);
			char *cut = strstr(section_name, old_path);
			if (cut) {
				strlcat(path, cut + old_path_len, ARRAY_SIZE(path));
			}

			/*
			** Create the registry key.
			*/
			RegistryClass reg(path);
			if (reg.Is_Valid()) {

				char *entry = (char*)1;
				int index = 0;

				while (entry) {
					entry = (char*)ini.Get_Entry(section_name, index++);
					if (entry) {

						if (strncmp(entry, "BIN_", 4) == 0) {
							int len = ini.Get_UUBlock(section_name, entry, buffer, sizeof(buffer));
							reg.Set_Bin(entry+4, buffer, len);
						} else {
							if (strncmp(entry, "DWORD_", 6) == 0) {
								int temp = ini.Get_Int(section_name, entry, 0);
								reg.Set_Int(entry+6, temp);
							} else {
					 			if (strncmp(entry, "STRING_", 7) == 0) {
									ini.Get_String(section_name, entry, "", string, sizeof(string));
									reg.Set_String(entry+7, string);
								} else {
									WWASSERT(false);
								}
							}
						}
					}
				}
			}
		}
	}
}






/***********************************************************************************************
 * RegistryClass::Delete_Registry_Values -- Delete all values under the given key              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Key handle                                                                        *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:37PM ST : Created                                                            *
 *=============================================================================================*/
void RegistryClass::Delete_Registry_Values(HKEY key)
{
	int index = 0;
	long result = ERROR_SUCCESS;

	while (result == ERROR_SUCCESS) {
		unsigned long type = 0;
		unsigned char data[8192];
		unsigned long data_size = sizeof(data);
		char value_name[256];
		unsigned long value_name_size = sizeof(value_name);

		result = RegEnumValue(key, index, value_name, &value_name_size, 0, &type, data, &data_size);

		if (result == ERROR_SUCCESS) {
			result = RegDeleteValue(key, value_name);
		}
	}
}




/***********************************************************************************************
 * RegistryClass::Delete_Registry_Tree -- Delete all values and sub keys of a registry key     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Registry path to delete                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: !!!!! DANGER DANGER !!!!!                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/21/2001 3:38PM ST : Created                                                            *
 *=============================================================================================*/
void RegistryClass::Delete_Registry_Tree(char *path)
{
	if (!IsLocked) {
		HKEY base_key;
		HKEY sub_key;
		int index = 0;
		char name[256];
		unsigned long name_size = sizeof(name);
		char class_name[256];
		unsigned long class_name_size = sizeof(class_name);
		FILETIME file_time;
		memset(&file_time, 0, sizeof(file_time));
		int max_times = 1000;


		long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_ALL_ACCESS, &base_key);

		if (result == ERROR_SUCCESS) {
			Delete_Registry_Values(base_key);

			index = 0;
			while (result == ERROR_SUCCESS) {
				class_name_size = sizeof(class_name);
				name_size = sizeof(name);
				result = RegEnumKeyEx(base_key, index, name, &name_size, 0, class_name, &class_name_size, &file_time);
				if (result == ERROR_SUCCESS) {

					/*
					** See if there are sub keys.
					*/
					char new_key_path[512];
					strcpy(new_key_path, path);
					strlcat(new_key_path, "\\", ARRAY_SIZE(new_key_path));
					strlcat(new_key_path, name, ARRAY_SIZE(new_key_path));

					unsigned long num_subs = 0;
					unsigned long num_values = 0;

					long new_result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, new_key_path, 0, KEY_ALL_ACCESS, &sub_key);
					if (new_result == ERROR_SUCCESS) {
						new_result = RegQueryInfoKey(sub_key, NULL, NULL, 0, &num_subs, NULL, NULL, &num_values, NULL, NULL, NULL, NULL);

						/*
						** If there are sub keys then enumerate those.
						*/
						if (num_subs > 0) {
							Delete_Registry_Tree(new_key_path);
						}

						if (num_values > 0) {
							Delete_Registry_Values(sub_key);
						}

						RegCloseKey(sub_key);

						RegDeleteKey(base_key, name);
					}
				}
				max_times--;
				if (max_times <= 0) {
					break;
				}
			}
			RegCloseKey(base_key);

			RegDeleteKey(HKEY_LOCAL_MACHINE, path);
		}
	}
}













