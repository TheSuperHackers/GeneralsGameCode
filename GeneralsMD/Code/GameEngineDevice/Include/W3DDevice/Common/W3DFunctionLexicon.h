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

// FILE: W3DFunctionLexicon.h /////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Function lexicon for w3d specific funtion pointers
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DFUNCTIONLEXICON_H_
#define __W3DFUNCTIONLEXICON_H_

#include "Common/FunctionLexicon.h"

//-------------------------------------------------------------------------------------------------
/** The function lexicon that can also contain w3d device methods */
//-------------------------------------------------------------------------------------------------
class W3DFunctionLexicon : public FunctionLexicon
{

public:

	W3DFunctionLexicon( void );
	virtual ~W3DFunctionLexicon( void );

	virtual void init( void );
	virtual void reset( void );
	virtual void update( void );

protected:

};

#endif // __W3DFUNCTIONLEXICON_H_

