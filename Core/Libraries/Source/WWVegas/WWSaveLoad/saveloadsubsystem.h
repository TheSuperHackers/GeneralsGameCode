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
 *                     $Archive:: /Commando/Code/wwsaveload/saveloadsubsystem.h               $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 6/12/00 4:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "bittype.h"
#include "postloadable.h"

class ChunkLoadClass;
class ChunkSaveClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	SaveLoadSubSystemClass
//
// Each SaveLoadSubSystem will automatically be registered with the SaveLoadSystem
// at construction time.  The plan is to have a single static instance of each
// sub-system so that it automatically registers when the global constructors
// are executed.
//
// When an application wants to create a file it does so by asking the
// SaveLoadSystem to save the particular set of SaveLoadSubSystems that contain
// that data.
//
//////////////////////////////////////////////////////////////////////////////////
class SaveLoadSubSystemClass : public PostLoadableClass
{
public:

	SaveLoadSubSystemClass (void);
	virtual ~SaveLoadSubSystemClass (void);

	virtual uint32				Chunk_ID (void) const = 0;

protected:

	virtual bool				Contains_Data(void) const						{ return true; }
	virtual bool				Save (ChunkSaveClass &csave) = 0;
	virtual bool				Load (ChunkLoadClass &cload) = 0;

	virtual const char*		Name() const = 0;

private:

	SaveLoadSubSystemClass *	NextSubSystem;			// managed by SaveLoadSystem

	friend class SaveLoadSystemClass;
};
