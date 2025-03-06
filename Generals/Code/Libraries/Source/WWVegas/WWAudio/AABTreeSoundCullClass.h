/*
**	Command & Conquer Generals(tm)
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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/AABTreeSoundCullClass.h     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 4/12/99 11:07a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __AABTREESOUNDCULLCLASS_H
#define __AABTREESOUNDCULLCLASS_H

#include "aabtreecull.h"


/////////////////////////////////////////////////////////////////////////////////
//
//	AABTreeSoundCullClass
//
//	Simple derived class that implements 2 required methods from AABTreeCullClass.
//
class AABTreeSoundCullClass : public AABTreeCullClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		AABTreeSoundCullClass (void)
			:	AABTreeCullClass (NULL)		{ }

		virtual ~AABTreeSoundCullClass (void)	{ }

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////
		void				Load (ChunkLoadClass & cload)	{ }
		void				Save (ChunkSaveClass & csave)	{ }

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		virtual void	Load_Node_Contents (AABTreeNodeClass * node,ChunkLoadClass & cload)	{ };
		virtual void	Save_Node_Contents (AABTreeNodeClass * node,ChunkSaveClass & csave)	{ };

};

#endif //__AABTREESOUNDCULLCLASS_H

