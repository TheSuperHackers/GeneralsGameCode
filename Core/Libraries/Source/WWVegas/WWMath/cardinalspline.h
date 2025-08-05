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
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwmath/cardinalspline.h                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 6/13/01 2:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef CARDINALSPLINE_H
#define CARDINALSPLINE_H

#include "hermitespline.h"

/**
** CardinalSpline3DClass
** 3-Dimensional cardinal splines
*/
class CardinalSpline3DClass : public HermiteSpline3DClass
{
public:

	virtual int			Add_Key(const Vector3 & point,float t);
	virtual void		Remove_Key(int i);
	virtual void		Clear_Keys(void);

	virtual void		Set_Tightness(int i,float tightness);
	virtual float		Get_Tightness(int i);

	virtual void		Update_Tangents(void);

	// save-load support
	virtual const PersistFactoryClass &	Get_Factory(void) const;
	virtual bool								Save(ChunkSaveClass &csave);
	virtual bool								Load(ChunkLoadClass &cload);

protected:

	DynamicVectorClass<float>	Tightness;
};


/**
** CardinalSpline1DClass
** 1-Dimensional cardinal splines
*/
class CardinalSpline1DClass : public HermiteSpline1DClass
{
public:

	virtual int			Add_Key(float point,float t);
	virtual void		Remove_Key(int i);
	virtual void		Clear_Keys(void);

	virtual void		Set_Tightness(int i,float tightness);
	virtual float		Get_Tightness(int i);

	virtual void		Update_Tangents(void);

	// save-load support
	virtual const PersistFactoryClass &	Get_Factory(void) const;
	virtual bool								Save(ChunkSaveClass &csave);
	virtual bool								Load(ChunkLoadClass &cload);

protected:

	DynamicVectorClass<float>	Tightness;
};


#endif

