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

/* $Header: /Commando/Code/Tools/max2w3d/util.cpp 28    10/27/00 4:12p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/util.cpp                       $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/27/00 1:13p                                              $*
 *                                                                                             *
 *                    $Revision:: 28                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Cleanup_Orthogonal_Matrix -- removes very small numbers from the matrix                   *
 *   Set_W3D_Name -- set a W3D name                                                            *
 *   Split_Node_Name -- break a node name into the base and extension                          *
 *   Is_Max_Tri_Mesh -- Is this node a triangle mesh?                                          *
 *    -- checks if the node is the origin of a model                                           *
 *    -- Checks if the node is the origin for the base obect (non-LOD'd).                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "util.h"
#include "w3dutil.h"
#include "skin.h"
#include "skindata.h"
#include "modstack.h"


#define MAX_NODE_NAME_LEN 256		// max name size we can handle

const float EPSILON = 0.00001f;
static char _string[256];



static int get_geometry_type(INode * node)
{
	assert(node != nullptr);
	return W3DAppData2Struct::Get_App_Data(node)->Get_Geometry_Type();

	//return (get_w3d_bits(node) & GEO_TYPE_MASK);
}


/***********************************************************************************************
 * Cleanup_Orthogonal_Matrix -- removes very small numbers from the matrix                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Matrix3	Cleanup_Orthogonal_Matrix(Matrix3 & mat)
{
	Matrix3 newmat = mat;

	for (int j=0; j<3; j++) {
		Point3 row = newmat.GetRow(j);

		if (fabs(row.x) < EPSILON) row.x = 0.0f;
		if (fabs(row.y) < EPSILON) row.y = 0.0f;
		if (fabs(row.z) < EPSILON) row.z = 0.0f;

		row = Normalize(row);

		newmat.SetRow(j,row);
	}

	return newmat;
}

/***********************************************************************************************
 * Set_W3D_Name -- set a W3D name                                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *   9/13/1999  AJA : Strip off the trailing ".digits" since this is a convention we've set in *
 *                    MAX to help artists manage LODs.                                         *
 *=============================================================================================*/
void Set_W3D_Name(char * set_name,const char * src)
{
	memset(set_name,0,W3D_NAME_LEN);
	strncpy(set_name,src,W3D_NAME_LEN-1);
	char *dot = strrchr(set_name, '.');
	if (dot)
	{
		// If a number comes after the dot, strip it off
		int value;
		if (sscanf(dot+1, "%d", &value) == 1)
			*dot = 0;
		// If nothing comes after the dot, strip it off
		else if (*(dot+1) == 0)
			*dot = 0;
	}
	strupr(set_name);
}

/***********************************************************************************************
 * Split_Node_Name -- break a node name into the base and extension                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void Split_Node_Name(const char * name,char * set_base,char * set_exten,int * set_exten_index)
{
	// Nodes are assumed to be named in the following way:
	// <name>.<exten character><exten digits>
	// for example: mesh.d1

	char buf[MAX_NODE_NAME_LEN];
	char * ptr;

	assert(strlen(name) < MAX_NODE_NAME_LEN);

	// Initialize
	if (set_base != nullptr) set_base[0] = 0;
	if (set_exten != nullptr) set_exten[0] = 0;
	if (set_exten_index != nullptr) *set_exten_index = 0;

	// Get the base name
	strncpy(buf,name,MAX_NODE_NAME_LEN);
	ptr = buf;
	while ((*ptr != 0) && (*ptr != '.')) {
		ptr++;
	}

	if (*ptr == '.') {

		// copy what we have so far into set_base
		*ptr = 0;
		if (set_base != nullptr) strncpy(set_base,buf,MAX_NODE_NAME_LEN);

		// copy the rest back into the extension
		ptr++;
		if (set_exten != nullptr) strncpy(set_exten,ptr,MAX_NODE_NAME_LEN);

		// now get the extension index
		ptr++;
		if (set_exten_index != nullptr) *set_exten_index = atoi(ptr);

	} else {

		// no extension, just copy the base name
		if (set_base != nullptr) strncpy(set_base,buf,MAX_NODE_NAME_LEN);
		return;
	}
}


bool Append_Lod_Character (char *meshname, int lod_level, INodeListClass *origin_list)
{
	if (meshname == nullptr || lod_level < 0)
		return false;

	if (!origin_list)
		return false;

	int num_lods = origin_list->Num_Nodes();

	/*
	** Search the other LODs to see if there is a mesh with the same name.
	** If there is, we will append the current LOD level digit to the name.
	** If there is not, the name will not be modified.
	*/
	INode *conflict = nullptr, *cur_origin = nullptr;
	int i, lod;
	for (i = 0; i < num_lods; i++)
	{
		// Don't bother searching the current LOD.
		lod = Get_Lod_Level((*origin_list)[i]);
		if (lod == lod_level)
			continue;

		// Search this lod for a node of the same name.
		conflict = Find_Named_Node(meshname, (*origin_list)[i]);
		if (conflict)
		{
			// Name length is a worry here, because the name plus the number
			// must be less than W3D_NAME_LEN (which is fairly small!).
			int length = strlen(meshname);
			if ( (lod_level < 10)  && (length < W3D_NAME_LEN - 1) )
			{
				// Append a number corresponding to the LOD level to the mesh name.
				// Highest-detail LOD is '0' (convention).
				char *insert = meshname + length;
				*insert++ = '0' + lod_level;
				*insert = '\0';
			}
			else if ( (lod_level < 100) && (length < W3D_NAME_LEN - 2) )
			{
				// Append a number corresponding to the LOD level to the mesh name.
				// Highest-detail LOD is '0' (convention).
				char buf[3];
				sprintf(buf, "%d", lod_level);
				strcat(meshname, buf);
			}
			else
			{
				// Replace the last character of the mesh name with the lod character (as above).
				meshname[W3D_NAME_LEN-2] = '0' + lod_level;
			}

			// Name mangling finished (conflict with other LODs is solved on their pass).
			break;
		}
	}

	return true;
}



void Create_Full_Path(char *full_path, const char *curr, const char *rel_path)
{
	// Copy current dir to full path. If it doesn't end with a slash, add one.
	strcpy(full_path, curr);
	int curr_len = strlen(curr);
	char *full_p = full_path + curr_len;	// Point at the terminating nullptr
	if (curr_len == 0 ||(*(full_p - 1) != '/' && *(full_p - 1) != '\\')) {
		*full_p = '\\';
		*(++full_p) = '\000';	// Point at the terminating nullptr
	}

	// Scan "..\"s at the beginning of the rel path, scan backwards on the
	// full path (current dir):
	const char *rel_p;
	for (	rel_p = rel_path;
			*rel_p == '.' && *(rel_p+1) == '.' && ( *(rel_p+2) == '/' || *(rel_p+2) == '\\' );
			rel_p += 3)
	{
		full_p--;
		for (; full_p > full_path && *(full_p-1) != '/' && *(full_p-1) != '\\'; full_p--);
		*full_p = '\000';
	}

	// Copy the remainder of the relative path to the full path:
	strcpy(full_p, rel_p);
}

// This enum is used inside Create_Relative_Path:
enum PathCharType {
	NULL_CHAR,
	SLASH_CHAR,
	PLAIN_CHAR
};

void Create_Relative_Path(char *rel_path,	const char *curr, const char *full_path)
{
	// Copy both constant strings and convert them to uppercase:
	int curr_len = strlen(curr);
	char *up_curr = (char *)malloc(curr_len + 1);
	strcpy(up_curr, curr);
	_strupr(up_curr);

	int full_len = strlen(full_path);
	char *up_full = (char *)malloc(full_len + 1);
	strcpy(up_full, full_path);
	_strupr(up_full);

	char *rel_p = rel_path;

	// Find shared prefix of curr and full path
	const char *full_p = up_full;
	const char *curr_p = up_curr;
	for (	;
			*full_p && *full_p == *curr_p || (*full_p == '/' && *curr_p == '\\') || (*full_p == '\\' && *curr_p == '/');
			full_p++, curr_p++
	);

	// If no shared prefix at this point set the relative path to 0
	// This will force the code to use the absolute path.
	if (full_p == up_full) {
		rel_path[0] = 0;
		goto end;
	}

	// The first different character for each string can be: a nullptr, a slash,
	// or an ordinary character.
	PathCharType full_type, curr_type;
	if (*full_p == '\000') {
		full_type = NULL_CHAR;
	} else {
		if (*full_p == '/' || *full_p == '\\') {
			full_type = SLASH_CHAR;
		} else {
			full_type = PLAIN_CHAR;
		}
	}
	if (*curr_p == '\000') {
		curr_type = NULL_CHAR;
	} else {
		if (*curr_p == '/' || *curr_p == '\\') {
			curr_type = SLASH_CHAR;
		} else {
			curr_type = PLAIN_CHAR;
		}
	}
	// If the last fullpath char is a nullptr or both are slashes, we have an
	// error - return full path
	if (full_type == NULL_CHAR || (full_type == SLASH_CHAR && curr_type == SLASH_CHAR)) {
		strcpy(rel_path, up_full);
		goto end;
	}

	// If the current path has ended (last char is a nullptr) and the full path's
	// last char is a slash, then just copy the remainder of the full path
	// (w/o the slash) to the relative path, and exit.
	if (curr_type == NULL_CHAR && full_type == SLASH_CHAR) {
		full_p++;	// skip slash
		strcpy(rel_path, full_p);
		goto end;
	}

	// If one of following holds:
	// 1) One of the last chars is a slash and the other is a plain char
	// 2) The current path has ended (last char is nullptr) and the last char
	//    of the full path is a plain char
	// 3) The last char of both are plain chars and the previous char is not a
	//    slash
	// Then we must backtrack both pointers until the characters before them
	// are slashes. If there are no previous slashes, we have an error.
	if (	(full_type == SLASH_CHAR && curr_type == PLAIN_CHAR) ||
			(curr_type == SLASH_CHAR && full_type == PLAIN_CHAR) ||
			(curr_type == NULL_CHAR && full_type == PLAIN_CHAR) ||
			(curr_type == PLAIN_CHAR && full_type == PLAIN_CHAR &&
				*(full_p-1) != '/' && *(full_p-1) != '\\')  )
	{
		for (;
			full_p > up_full &&
				(	(*(full_p - 1) != '/' && *(full_p - 1) != '\\') ||
					(*(curr_p - 1) != '/' && *(curr_p - 1) != '\\') );
			full_p--, curr_p--);
	}

	// If no shared prefix at this point (not even a drive letter) return the
	// full path
	if (full_p == up_full) {
		strcpy(rel_path, up_full);
		goto end;
	}

	// Scan all directories levels in current path from shared point to end -
	// for each one add a "../" to the relative path. Note that at this point
	// we know we have to add at least one.
	*rel_p++ = '.';
	*rel_p++ = '.';
	*rel_p++ = '\\';
	// Go over remaining current path, for each slash we find add one "../" to
	// the relative path
	for (; *curr_p; curr_p++) {
		if (*curr_p == '/' || *curr_p == '\\') {
			*rel_p++ = '.';
			*rel_p++ = '.';
			*rel_p++ = '\\';
		}
	}

	// If the last char of the current path is a slash remove a "../" from the
	// relative path.
	if (*(curr_p - 1) == '/' || *(curr_p - 1) == '\\') {
		rel_p -= 3;
	}

	// Copy remaining full path (from shared point to end) to relative path
	strcpy(rel_p, full_p);

end:
	free(up_curr);
	free(up_full);
}

bool Is_Full_Path(char * path)
{
	// first scan for a drive letter (scan for a colon)
	if (strchr(path,':') != nullptr) {
		return true;
	}

	// now scan for a "network" path (starts with "//")
	if ((path[0] == '/') && (path[1] == '/')) {
		return true;
	}
	if ((path[0] == '\\') && (path[1] == '\\')) {
		return true;
	}

	return false;
}


/***********************************************************************************************
 * Is_Max_Tri_Mesh -- Is this node a triangle mesh?                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/2/98     GTH : Created.                                                                 *
 *=============================================================================================*/
bool Is_Max_Tri_Mesh(INode * node)
{
	Object *obj = node->EvalWorldState(0).obj;

	if (obj && obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) {
		return true;
	}
	return false;
}



bool Is_Damage_Root(INode *node)
{
	if (node == nullptr)
		return false;

	// Is the node's parent the scene root?
	INode *parent = node->GetParentNode();
	if (!parent || !parent->IsRootNode())
		return false;

	// Is the node's name in the form "damage.*"?
	char *name = node->GetName();
	if (strnicmp(name, "damage.", strlen("damage.")) != 0)
		return false;

/* This won't pick up references to a dummy object for some reason.
	// Does the node point to a dummy object?
	Object *obj = node->GetObjectRef();
	if (!obj || !obj->CanConvertToType(Class_ID(DUMMY_CLASS_ID, 0)))
		return false;
*/

	return true;
}


/***********************************************************************************************
 * Is_Origin -- checks if the node is the origin of a model                                    *
 *                                                                                             *
 *  A node is an origin if its parent is the scene root, it is a dummy object, and its name    *
 * is of the form "origin.*" (case insensitive). All descendants of the origin will be        *
 * expressed in coordinates relative to the origin object.                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/13/99    AJA : Created.
 *=============================================================================================*/
bool Is_Origin(INode * node)
{
	if (!node) return false;
	if (node->IsRootNode()) return true;
	if (node->IsHidden()) return false;

	// Is the node's parent the scene root?
	INode *parent = node->GetParentNode();
	if (!parent || !parent->IsRootNode())
		return false;

	// Is the node's name in the form "origin.*"?
	char *name = node->GetName();
	if (strnicmp(name, "origin.", strlen("origin.")) != 0)
		return false;

/* This won't pick up references to a dummy object for some reason.
	// Does the node point to a dummy object?
	Object *obj = node->GetObjectRef();
	if (!obj || !obj->CanConvertToType(Class_ID(DUMMY_CLASS_ID, 0)))
		return false;
*/

	// This is an origin.
	return true;
}


/***********************************************************************************************
 * Is_Base_Origin -- Checks if the node is the origin for the base obect (non-LOD'd).          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/13/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
bool Is_Base_Origin(INode * node)
{
	if (!node) return false;

	if (node->IsRootNode()) return true;

	if (!Is_Origin(node)) return false;

	// An origin is the base object origin if it's name is "origin." or
	// "origin.0" (a numeric value evaluating to zero as scanned by sscanf
	// which would include "origin.00" "origin.000", etc.).
	bool is_base_origin = false;
	char *name = node->GetName();
	if (stricmp(name, "origin.") == 0)
		is_base_origin = true;
	else if (strlen(name) > strlen("origin."))
	{
		// We know the first 7 characters are "origin." because that
		// was tested in Is_Origin(). Is it "origin.0"?
		int idx;
		if ((sscanf(name+strlen("origin."), "%d", &idx) == 1) && (idx == 0))
			is_base_origin = true;
	}

	return is_base_origin;
}


int Get_Lod_Level(INode *node)
{
	if (node->IsRootNode()) return 0;
	if (!Is_Origin(node)) return -1;

	char *name = node->GetName();
	char *dot = strrchr(name, '.');
	assert(dot);
	return atoi(dot+1);
}

int Get_Damage_State(INode *node)
{
	if (!Is_Damage_Root(node)) return -1;

	char *name = node->GetName();
	char *dot = strrchr(name, '.');
	assert(dot);
	return atoi(dot+1);
}

INode *Find_Named_Node(char *nodename, INode *root)
{
	if (!root || !nodename)
		return nullptr;

	// Perform a breadth-first search of the tree for a node
	// of the given name.
	INode	*child = nullptr;
	int	i;
	char	cur_name[W3D_NAME_LEN];

	// Is this the node we're looking for?
	Set_W3D_Name(cur_name, root->GetName());
	if (strcmp(cur_name, nodename) == 0)
		return root;

	// Check the children against the given name.
	for (i = 0; i < root->NumChildren(); i++)
	{
		// Is it this child?
		child = root->GetChildNode(i);
		Set_W3D_Name(cur_name, child->GetName());
		if (strcmp(nodename, cur_name) == 0)
			return child;
	}

	// Wasn't any children. Check each child's descendants.
	for (i = 0; i < root->NumChildren(); i++)
	{
		child = root->GetChildNode(i);
		INode *found = Find_Named_Node(nodename, child);
		if (found)
			return found;
	}

	// Didn't find the node anywhere.
	return nullptr;
}

