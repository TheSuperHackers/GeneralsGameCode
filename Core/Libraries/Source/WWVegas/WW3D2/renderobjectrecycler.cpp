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
 *                     $Archive:: /Commando/Code/ww3d2/renderobjectrecycler.cpp               $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/08/01 3:54p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RenderObjectRecyclerClass::Reset -- reset the cache                                       *
 *   RenderObjectRecyclerClass::Get_Render_Object -- Create/Re-use a render object             *
 *   RenderObjectRecyclerClass::Return_Render_Object -- Return a render object to the cache    *
 *   RenderObjectRecyclerClass::Insert_Inactive_Model -- Internally add a render object to the *
 *   RenderObjectRecyclerClass::Reset_Model -- resets a render object so it can be re-used     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "renderobjectrecycler.h"
#include "rendobj.h"
#include "assetmgr.h"
#include "part_emt.h"
#include "matrix3d.h"


/***********************************************************************************************
 * RenderObjectRecyclerClass::Reset -- reset the pool                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void RenderObjectRecyclerClass::Reset(void)
{
	InactiveModels.Reset_List();
}


/***********************************************************************************************
 * RenderObjectRecyclerClass::Get_Render_Object -- Create/Re-use a render object               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * RenderObjectRecyclerClass::Get_Render_Object(const char * name,const Matrix3D & tm)
{
	RefRenderObjListIterator it(&InactiveModels);

	RenderObjClass * found = nullptr;
	while (!it.Is_Done()) {
		if (stricmp(it.Peek_Obj()->Get_Name(),name) == 0) {
			found = it.Peek_Obj();
			break;
		}
		it.Next();
	}

	if (found != nullptr) {
		found->Add_Ref();
		InactiveModels.Remove(found);
		found->Set_Transform(tm);
		Reset_Model(found);
		return found;

	} else {

		RenderObjClass * new_model = WW3DAssetManager::Get_Instance()->Create_Render_Obj(name);
		if (new_model != nullptr) {
			new_model->Set_Transform(tm);
			return new_model;
		}
	}

	return nullptr;
}


/***********************************************************************************************
 * RenderObjectRecyclerClass::Return_Render_Object -- Return a render object to the pool       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void RenderObjectRecyclerClass::Return_Render_Object(RenderObjClass * obj)
{
	Insert_Inactive_Model(obj);
}


/***********************************************************************************************
 * RenderObjectRecyclerClass::Insert_Inactive_Model -- Internally add a render object to the po*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void RenderObjectRecyclerClass::Insert_Inactive_Model(RenderObjClass * obj)
{
	WWASSERT(obj != nullptr);
	InactiveModels.Add(obj);
}

/***********************************************************************************************
 * RenderObjectRecyclerClass::Reset_Model -- resets a render object so it can be re-used       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/8/2001   gth : Created.                                                                 *
 *=============================================================================================*/
void RenderObjectRecyclerClass::Reset_Model(RenderObjClass * model)
{
	/*
	** recursively reset all sub-objects (if any)
	*/
	for (int i=0; i<model->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = model->Get_Sub_Object(i);
		Reset_Model(robj);
		robj->Release_Ref();
	}

	/*
	** particle emitters must be reset (this should release their buffers)
	*/
	if (model->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
		ParticleEmitterClass * emitter = (ParticleEmitterClass *)model;
		emitter->Reset();
	}

	/*
	** animated models must have their animation reset (if present)
	*/
	if (model->Peek_Animation() != nullptr) {
		model->Set_Animation(model->Peek_Animation(),0.0f,RenderObjClass::ANIM_MODE_ONCE);
	}
}
