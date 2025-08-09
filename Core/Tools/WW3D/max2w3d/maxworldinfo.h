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
 *                 Project Name : Max2W3D                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/maxworldinfo.h                 $*
 *                                                                                             *
 *              Original Author:: Patrick Smith                                                *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/27/00 6:55p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef MAXWORLDINFO_H
#define MAXWORLDINFO_H


#include <max.h>
#include "meshbuild.h"
#include "nodelist.h"
#include "Vector.H"


class GeometryExportTaskClass;


/**
** MaxWorldInfoClass - Provides information about the max 'world' (or scene)
** This class is used by the plugin to cause the MeshBuilder to smooth normals
** across adjacent meshes.
*/
class MaxWorldInfoClass : public WorldInfoClass
{
	public:
		MaxWorldInfoClass(DynamicVectorClass<GeometryExportTaskClass *> & mesh_list)
			:	MeshList (mesh_list),
				SmoothBetweenMeshes (true),
				CurrentTask(NULL),
				CurrentTime(0)					{ }
		virtual ~MaxWorldInfoClass(void)	{ }

		// Public methods
		virtual Vector3	Get_Shared_Vertex_Normal(Vector3 pos, int smgroup);

		virtual GeometryExportTaskClass *	Get_Current_Task(void) const								{ return CurrentTask; }
		virtual void								Set_Current_Task(GeometryExportTaskClass * task)	{ CurrentTask = task; }

		virtual TimeValue	Get_Current_Time(void) const	{ return CurrentTime; }
		virtual void		Set_Current_Time(TimeValue &time) { CurrentTime = time; }

		virtual Matrix3	Get_Export_Transform(void) const	{ return ExportTrans; }
		virtual void		Set_Export_Transform(const Matrix3 &matrix) { ExportTrans = matrix; }

		virtual void		Allow_Mesh_Smoothing (bool onoff)	{ SmoothBetweenMeshes = onoff; }
		virtual bool		Are_Meshes_Smoothed (void) const		{ return SmoothBetweenMeshes; }

	private:

		DynamicVectorClass<GeometryExportTaskClass *> &	MeshList;
		GeometryExportTaskClass *								CurrentTask;
		TimeValue			CurrentTime;
		Matrix3				ExportTrans;
		bool					SmoothBetweenMeshes;
};



#endif
