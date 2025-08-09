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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/renderobjectrecycler.h                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/08/01 3:52p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef RENDEROBJECTRECYCLER_H
#define RENDEROBJECTRECYCLER_H

#include "always.h"
#include "robjlist.h"

class RenderObjClass;
class Matrix3D;

/**
** RenderObjectRecyclerClass
** This class can be used to eliminate dynamic render object allocation.  An example usage would be
** the case where you have a projectile system in a game.  Projectiles could be set up to use a
** variety of render objects and would be rapidly created and destroyed as the players and AI's fire
** their weapons.  If all of your bullet objects request their models from a RenderObjectCache, and
** return their models to the cache when the bullet is destroyed; the model will get re-used by
** the next bullet that requests that same model type (highly likely in a game situation).
**
** Public member functions:
** Reset - release all of the models in the cache.
** Get_Render_Object - returns either recycles a model or creates a new one through the asset manager
** Return_Render_Object - give your model back to the recycler for re-use later.
*/
class RenderObjectRecyclerClass
{
public:

	void					Reset(void);
	RenderObjClass*	Get_Render_Object(const char * name,const Matrix3D & tm);
	void					Return_Render_Object(RenderObjClass * obj);

private:

	void					Insert_Inactive_Model(RenderObjClass * obj);
	void					Reset_Model(RenderObjClass * model);


	RefRenderObjListClass	InactiveModels;

};




#endif //RENDEROBJECTRECYCLER_H
