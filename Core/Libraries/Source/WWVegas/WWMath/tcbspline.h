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
 *                     $Archive:: /VSS_Sync/wwmath/tcbspline.h                                $*
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

#ifndef TCBSPLINE_H
#define TCBSPLINE_H

#include "hermitespline.h"

/*
** TCBSpline3DClass
** Tension-Continuity-Bias splines.  Otherwise known as Kochanek-Bartels cubic splines
*/
class TCBSpline3DClass : public HermiteSpline3DClass
{
public:

	virtual int			Add_Key(const Vector3 & point,float t);
	virtual void		Remove_Key(int i);
	virtual void		Clear_Keys(void);

	virtual void		Set_TCB_Params(int i,float tension,float continuity,float bias);
	virtual void		Get_TCB_Params(int i,float *tension,float *continuity,float *bias);

	void					Update_Tangents(void);

	// save-load support
	virtual const PersistFactoryClass &	Get_Factory(void) const;
	virtual bool								Save(ChunkSaveClass &csave);
	virtual bool								Load(ChunkLoadClass &cload);

protected:	

	class TCBClass
	{
	public:
		float				Tension;
		float				Continuity;
		float				Bias;
		bool				operator == (const TCBClass & that) { return ((Tension == that.Tension) && (Continuity == that.Continuity) && (Bias == that.Bias)); }
		bool				operator != (const TCBClass & that) { return !TCBClass::operator == (that); }
	};

	DynamicVectorClass<TCBClass> Params;
};


#endif

