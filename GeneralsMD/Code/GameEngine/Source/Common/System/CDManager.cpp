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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   Generals
//
// Module:    Game Engine Common
//
// File name: CDManager.cpp
//
// Created:   11/26/01 TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/CDManager.h"
#include "GameLogic/GameLogic.h"

//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------

CDManagerInterface* TheCDManager = NULL;

//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------


//============================================================================
// CDDrive::CDDrive
//============================================================================

CDDrive::CDDrive()
: m_disk(CD::UNKNOWN_DISK)
{
	m_diskName.clear();
	m_drivePath.clear();

}

//============================================================================
// CDDrive::~CDDrive
//============================================================================

CDDrive::~CDDrive()
{

}

//============================================================================
// CDDrive::getPath
//============================================================================

AsciiString CDDrive::getPath( void )
{
	return m_drivePath;
}

//============================================================================
// CDDrive::getDiskName
//============================================================================

AsciiString CDDrive::getDiskName( void )
{
	return m_diskName;
}

void CDDrive::refreshInfo( void )
{
		// map disk names to disk ID
		m_disk = CD::UNKNOWN_DISK;
}

//============================================================================
// CDDrive::getDisk
//============================================================================

CD::Disk CDDrive::getDisk( void )
{
	return m_disk;
}

//============================================================================
// CDDrive::setPath
//============================================================================

void CDDrive::setPath( const Char *path )
{
	m_drivePath = path;
}

//============================================================================
// CDManager::CDManager
//============================================================================

CDManager::CDManager()
{

}

//============================================================================
// CDManager::~CDManager
//============================================================================

CDManager::~CDManager()
{
	destroyAllDrives();
}

//============================================================================
// CDManager::init
//============================================================================

void CDManager::init( void )
{

}

//============================================================================
// CDManager::update
//============================================================================

void CDManager::update( void )
{
	// Every so often, check to make sure the CD is still in the drive
	if ((TheGameLogic->getFrame() % 300) == 299) {
		refreshDrives();
	}
}

//============================================================================
// CDManager::reset
//============================================================================

void CDManager::reset( void )
{

}

//============================================================================
// CDManager::driveCount
//============================================================================

Int CDManager::driveCount( void )
{
	return m_drives.nodeCount();
}

//============================================================================
// CDManager::getDrive
//============================================================================

CDDriveInterface* CDManager::getDrive( Int index )
{
	CDDriveInterface *cd = NULL;
	LListNode *node = m_drives.getNode( index );

	if ( node )
	{
		cd = (CDDriveInterface*) node->item();
	}

	return cd;
}

//============================================================================
// CDManager::newDrive
//============================================================================

CDDriveInterface* CDManager::newDrive( const Char *path )
{
	CDDrive *drive= (CDDrive*) createDrive();

	if ( drive )
	{
		drive->setPath( path );

		drive->m_node.setItem( drive );
		m_drives.add( &drive->m_node );
	}
	return drive;
}

//============================================================================
// CDManager::refreshDrives
//============================================================================

void CDManager::refreshDrives( void )
{
	LListNode *node = m_drives.firstNode();

	while ( node )
	{
		CDDriveInterface *drive = (CDDriveInterface *) node->item();
		if ( drive )
		{
			drive->refreshInfo();
		}

		node = node->next();
	}
}


//============================================================================
// CDManager::destroyAllDrives
//============================================================================

void CDManager::destroyAllDrives( void )
{
	LListNode *node;

	while ( (node = m_drives.firstNode() ) != NULL )
	{
		node->remove();
		CDDriveInterface *drive = (CDDriveInterface *) node->item();
		delete drive;
	}
}





