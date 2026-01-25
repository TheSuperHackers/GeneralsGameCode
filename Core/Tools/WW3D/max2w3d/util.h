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

/* $Header: /Commando/Code/Tools/max2w3d/util.h 25    10/27/00 4:11p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/util.h                         $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/27/00 10:24a                                             $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef ALWAYS_H
#include "always.h"
#endif

#include <max.h>

#include "skin.h"
#include "nodelist.h"


/*
** Gets rid of very small numbers in the matrix
** (sets them to zero).
*/
Matrix3	Cleanup_Orthogonal_Matrix(Matrix3 & mat);

/*
** Naming utility functions
*/
void Set_W3D_Name(char * set_name,const char * src);
void Split_Node_Name(const char * name,char * set_base,char * set_exten,int * set_exten_index);
bool Append_Lod_Character(char *meshname, int lod_level, INodeListClass *origin_list);

/*
** File path utility functions
*/
void Create_Full_Path(char *full_path, const char *cwd, const char *rel_path);
void Create_Relative_Path(char *rel_path,	const char *cwd, const char *full_path);
bool Is_Full_Path(char * path);

/*
** Check if this is some kind of triangle mesh inside of MAX.  We will assume
** different default export behavior depending on whether the object is a
** triangle mesh or not.
*/
bool Is_Max_Tri_Mesh(INode * node);


/*
** Origin support.
*/
bool	Is_Origin(INode *node);
bool	Is_Base_Origin(INode *node);
INode *Find_Origin(INode *node);

bool Is_Damage_Root(INode *node);

/*
** Lod-Level and Damage State settings for an INode
*/
int Get_Lod_Level(INode *node);
int Get_Damage_State(INode *node);

/*
**  Utility function to find a named node in a hierarchy.
*/
INode *Find_Named_Node (char *nodename, INode *root);


/*
** Macros
*/
#define SAFE_DELETE(pobject) \
			if (pobject) { \
				delete pobject; \
				pobject = nullptr; \
			}

#define SAFE_DELETE_ARRAY(pobject) \
			if (pobject) { \
				delete [] pobject; \
				pobject = nullptr; \
			}
