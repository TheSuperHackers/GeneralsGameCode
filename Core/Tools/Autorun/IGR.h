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

//
// IGR.h - A class used to access the IGR registry settings.
//
// JeffB 7/5/00
//

//
// Registry Path
//
#define WOLAPI_REG_KEY_TOP				"HKEY_LOCAL_MACHINE"
#define WOLAPI_REG_KEY_WOLAPI			"SOFTWARE\\Westwood\\WOLAPI"
#define WOLAPI_REG_KEY_BOTTOM			WOLAPI_REG_KEY_WOLAPI "\\" 
#define WOLAPI_REG_KEY_OPTIONS		"Options"
#define WOLAPI_REG_KEY					WOLAPI_REG_KEY_TOP "\\" WOLAPI_REG_KEY_BOTTOM
#define WOLAPI_KEY						"WOLAPI"

//
// Option Bits for Options key
//
#define IGR_NO_AUTO_LOGIN  			0x01
#define IGR_NEVER_STORE_NICKS 		0x02
#define IGR_NEVER_RUN_REG_APP			0x04
#define IGR_ALL							IGR_NO_AUTO_LOGIN | IGR_NEVER_STORE_NICKS |	IGR_NEVER_RUN_REG_APP
#define IGR_NONE							0x00

typedef unsigned int IGROptionsType;

class IGROptionsClass
{
	public:
		// Constructor
		IGROptionsClass( void ) : valid( false ), options( 0 ) {};

		// Destructor
		~IGROptionsClass( void ) {};

		// Initialize. Read value(s) from registry
		bool Init( void );

		// Check various options
		bool Is_Auto_Login_Allowed( void );
		bool Is_Storing_Nicks_Allowed( void );
		bool Is_Running_Reg_App_Allowed( void );

		// Set various options
		bool Set_Options( IGROptionsType options );

	private:

		// Private options
		IGROptionsType  options;

		// Is the data valid?
		bool	valid;
};

extern IGROptionsClass *OnlineOptions;