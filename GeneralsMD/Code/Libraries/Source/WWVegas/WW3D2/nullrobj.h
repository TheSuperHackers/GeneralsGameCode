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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/nullrobj.h                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef NULL_H
#define NULL_H

#ifndef RENDOBJ_H
#include "rendobj.h"
#endif

#include "proto.h"

class Null3DObjClass : public RenderObjClass
{
public:

	Null3DObjClass(const char * name = "NULL");
	Null3DObjClass(const Null3DObjClass & src);
	Null3DObjClass & operator = (const Null3DObjClass & that);

	virtual int						Class_ID(void) const;
	virtual RenderObjClass *	Clone(void) const;
	virtual const char *			Get_Name(void) const						{ return Name; }
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
	virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

protected:

	char								Name[2*W3D_NAME_LEN];
};


class NullPrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(NullPrototypeClass)
public:
	NullPrototypeClass(void);
	NullPrototypeClass(const W3dNullObjectStruct &null);

	virtual const char *			Get_Name(void)	const			{ return Definition.Name; }
	virtual int								Get_Class_ID(void) const	{ return RenderObjClass::CLASSID_NULL; }
	virtual RenderObjClass *	Create(void)					{ return NEW_REF(Null3DObjClass,(Definition.Name)); }
	virtual void							DeleteSelf()						{ delete this; }

protected:
	W3dNullObjectStruct			Definition;
};


class NullLoaderClass : public PrototypeLoaderClass
{
public:
	virtual int						Chunk_Type(void) { return W3D_CHUNK_NULL_OBJECT; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};


/*
** Instance of the default loader which the asset manager can
** automatically install at creation time
*/
extern NullLoaderClass _NullLoader;


#endif

