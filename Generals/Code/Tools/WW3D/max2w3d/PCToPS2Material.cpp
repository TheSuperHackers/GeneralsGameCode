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
 *                 Project name : Buccaneer Bay                                                *
 *                                                                                             *
 *                    File name : PCToPS2Material.cpp                                          *
 *                                                                                             *
 *                   Programmer : Mike Lytle                                                   *
 *                                                                                             *
 *                   Start date : 10/22/1999                                                   *
 *                                                                                             *
 *                  Last update : 10/27/1999                                                   *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PTPMC::BeginEditParams -- Change all W3D materials in the mesh to be PS2 compatible.      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include <max.h>
#include <gport.h>
#include <hsv.h>
#include "dllmain.h"
#include "resource.h"
#include "util.h"
#include "utilapi.h"
#include "nodelist.h"
#include "gamemtl.h"


Class_ID PCToPS2MaterialClassID(0x40d11cee, 0x68881657);

class PCToPS2MaterialClass : public UtilityObj {

	public:
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu) {}
		void DeleteThis() {delete this;}

};


/***********************************************************************************************
 * PTPMC::BeginEditParams -- Change all W3D materials in the mesh to be PS2 compatible.        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/27/1999MLL: Created.                                                                   *
 *=============================================================================================*/
void PCToPS2MaterialClass::BeginEditParams(Interface *ip,IUtil *iu) 
{
	// This function is called when the utility is chosen.  
	// Since we don't need any window gadgets, we'll just go through all the materials right away.
	INode *root = ip->GetRootNode();

	INodeListClass *meshlist = NULL;

	// Change all materials associated with the mesh, starting with the root node.
	if (root) {
		meshlist = new INodeListClass(root, 0);

		if (meshlist) {
			int i;

			for (i = 0; i < meshlist->Num_Nodes(); i++) {

				Mtl	*nodemtl = ((*meshlist)[i])->GetMtl();

				if (nodemtl == NULL) {
					// No material on this node, go to the next.
					continue;
				}

				if (!nodemtl->IsMultiMtl()) {
					// Only change those that are W3D materials.
					if (nodemtl->ClassID() == GameMaterialClassID) {
						int pass;

						for (pass = 0; pass < ((GameMtl*)nodemtl)->Get_Pass_Count(); pass++) {
							// Change the material for each pass.
							((GameMtl*)nodemtl)->Compute_PS2_Shader_From_PC_Shader(pass);
						}

					}
				} else {

					// Loop through all sub materials of the multi-material.
					for (unsigned mi = 0; mi < nodemtl->NumSubMtls(); mi++) {

						// Only change those that are W3D materials.
						if (nodemtl->GetSubMtl(mi)->ClassID() == GameMaterialClassID) {
							int pass;

							for (pass = 0; pass < ((GameMtl*)nodemtl->GetSubMtl(mi))->Get_Pass_Count(); pass++) {
								// Change the material for each pass.
								((GameMtl*)nodemtl->GetSubMtl(mi))->Compute_PS2_Shader_From_PC_Shader(pass);
							}
						} else if (nodemtl->GetSubMtl(mi)->ClassID() == PS2GameMaterialClassID) {
						}
					}
				}
			}
		}

		delete meshlist;
	}
}

class PCToPS2MaterialClassDesc:public ClassDesc {

public:
	int				IsPublic()					{ return 1; }
	void *			Create(BOOL loading)		
	{ 
		return ((void*)new PCToPS2MaterialClass); 
	}
	const TCHAR *	ClassName()					{ return Get_String(IDS_PC_TO_PS2_MAT_CONVERTER); }
	SClass_ID		SuperClassID()				{ return UTILITY_CLASS_ID; }
	Class_ID 		ClassID()					{ return PCToPS2MaterialClassID; }
	const TCHAR* 	Category()					{ return _T("");  }
};

static PCToPS2MaterialClassDesc _PCToPS2MaterialCD;

ClassDesc * Get_PS2_Material_Conversion() { return &_PCToPS2MaterialCD;  }
