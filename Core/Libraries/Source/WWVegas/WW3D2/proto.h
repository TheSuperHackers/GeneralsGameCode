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
 *                     $Archive:: /Commando/Code/ww3d2/proto.h                                $*
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

#include "always.h"
#include <stdlib.h>
#include "w3d_file.h"

class RenderObjClass;
class ChunkLoadClass;

/*
** W3D Prototype system
**
** Prototypes are basically abstract factories for render objects.
** Prototype Loaders are abstract factories for prototypes.
** Here is an example of the sequence of events that happen when
** a mesh is loaded into the asset manager and then used by the
** user:
**
** - At initialization time, a mesh prototype loader is installed automatically
** - User asks the asset manager to load "mesh.w3d"
** - asset manager encounters a W3D_CHUNK_MESH
** - asset manager looks through its loaders to find one that claims to handle this chunk
** - the meshloader object is found and its Load method called
** - the meshloader creates a mesh prototype object which the asset manager adds to its list
** - User asks for the render object named "Mesh"
** - asset manager searches through its prototypes to find the one named "Mesh"
** - the mesh prototype object is found and the asset manager calls its "Create" method
** - the mesh prototype creates a mesh (clones the one it contains) which is returned to the user.
*/

/*
** PrototypeClass
** This class is a generic interface to a render object prototype.
** The asset manager will store a these and use them whenever the
** user wants to create an instance of a named render object.
** Some simple render objects will be created through cloning.  In
** that case, their associated prototype simply stores an object and
** clones it whenever the Create method is called.  More complex
** composite render objects will be created from a "blueprint" object.
** Basically this class simply associates a name with a render object
** creation function.
*/
class PrototypeClass
{

public:

	PrototypeClass(void) : NextHash(NULL) {}

	virtual const char *			Get_Name(void)	const = 0;
	virtual int								Get_Class_ID(void) const = 0;
	virtual RenderObjClass *	Create(void) = 0;
	virtual void							DeleteSelf() = 0;

	inline void friend_setNextHash(PrototypeClass* n) { NextHash = n; }
	inline PrototypeClass* friend_getNextHash() { return NextHash; }

protected:
	virtual ~PrototypeClass(void) {};

private:
	PrototypeClass *				NextHash;

	// Not Implemented
	PrototypeClass(const PrototypeClass & that);
	PrototypeClass & operator = (const PrototypeClass & that);
};

class PrimitivePrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(PrimitivePrototypeClass)
public:
	PrimitivePrototypeClass(RenderObjClass * proto);

	virtual const char *			Get_Name(void) const;
	virtual int						Get_Class_ID(void) const;
	virtual RenderObjClass *	Create(void);
	virtual void							DeleteSelf()										{ delete this; }

	RenderObjClass *				Proto;

protected:
	virtual ~PrimitivePrototypeClass(void);
};

/*
** PrototypeLoaderClass
** This is the interface for an object which recognizes a certain
** chunk type in a W3D file and can load it and create a PrototypeClass
** for it.
*/
class PrototypeLoaderClass
{

public:

	PrototypeLoaderClass(void) {}
	~PrototypeLoaderClass(void) {}

	virtual int						Chunk_Type(void) = 0;
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload) = 0;

private:

	// Not Implemented:
	PrototypeLoaderClass(const PrototypeLoaderClass & that);
	PrototypeLoaderClass & operator = (const PrototypeLoaderClass & that);

};


/*
** Default Prototype Loaders for Meshes and HModels
*/
class MeshLoaderClass : public PrototypeLoaderClass
{
public:

	virtual int						Chunk_Type(void) { return W3D_CHUNK_MESH; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};

class HModelLoaderClass : public PrototypeLoaderClass
{
public:

	virtual int						Chunk_Type(void) { return W3D_CHUNK_HMODEL; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};


/*
** Instances of the default loaders which the asset manager can
** automatically install at creation time
*/
extern MeshLoaderClass			_MeshLoader;
extern HModelLoaderClass		_HModelLoader;
