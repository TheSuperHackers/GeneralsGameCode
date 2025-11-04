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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: INIAnimation.cpp /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   Parsing animation INI entries for 2D image animations
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "GameClient/Anim2D.h"

//-------------------------------------------------------------------------------------------------
/** Parse animation entry */
//-------------------------------------------------------------------------------------------------
void INI::parseAnim2DDefinition( INI* ini )
{
	AsciiString name;
	Anim2DTemplate *animTemplate;

	// read the name
	const char* c = ini->getNextToken();
	name.set( c );

	//
	// find existing item if present, note that we do not support overrides
	// in the animations like we do in systems that are more "design" oriented, images
	// are assets as they are
	//
	if( !TheAnim2DCollection )
	{

		//We don't need it if we're in the builder... which doesn't have this.
		return;

	}

	// find existing animation template if present
	animTemplate = TheAnim2DCollection->findTemplate( name );
	if( animTemplate == NULL )
	{

		// item not found, create a new one
		animTemplate = TheAnim2DCollection->newTemplate( name );
		DEBUG_ASSERTCRASH( animTemplate, ("INI""parseAnim2DDefinition -  unable to allocate animation template for '%s'",
											 name.str()) );

	}
	else
	{

		// we're loading over an existing animation template ... something is probably wrong
		DEBUG_CRASH(( "INI::parseAnim2DDefinition - Animation template '%s' already exists",
									animTemplate->getName().str() ));
		return;

	}

	// parse the ini definition
	ini->initFromINI( animTemplate, animTemplate->getFieldParse() );

}



