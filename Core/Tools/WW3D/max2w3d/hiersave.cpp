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

/* $Header: /Commando/Code/Tools/max2w3d/hiersave.cpp 56    10/30/00 6:58p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Engine                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/hiersave.cpp                   $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/30/00 6:14p                                              $*
 *                                                                                             *
 *                    $Revision:: 56                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HierarchySaveClass::HierarchySaveClass -- constructor                                     *
 *   HierarchySaveClass::HierarchySaveClass -- constructor                                     *
 *   HierarchySaveClass::HierarchySaveClass -- constructor                                     *
 *   HierarchySaveClass::~HierarchySaveClass -- destructor                                     *
 *   HierarchySaveClass::Free -- releases all allocated memory                                 *
 *   HierarchySaveClass::Get_Node_Transform -- returns the transformation matrix of specified n*
 *   HierarchySaveClass::get_relative_transform -- retruns tm between this node and its parent *
 *   HierarchySaveClass::Get_Name -- returns the name of this hierarchy                        *
 *   HierarchySaveClass::Get_Node -- Get the Max INode                                         *
 *   HierarchySaveClass::Get_Node_Name -- returns name of this hierarchy node                  *
 *   HierarchySaveClass::Find_Named_Node -- returns index of a named node                      *
 *   HierarchySaveClass::Get_Export_Coordinate_System - find the bone and coordinate system    *
 *   HierarchySaveClass::Save -- write the hierarchy into a W3D file                           *
 *   HierarchySaveClass::Load -- read the hierarchy from a W3D file                            *
 *   HierarchySaveClass::add_tree -- adds a node and all of its children                       *
 *   HierarchySaveClass::add_node -- adds a single node to the tree                            *
 *   HierarchySaveClass::Get_Fixup_Transform -- gets the "fixup" transform for a node          *
 *   HierarchySaveClass::fixup_matrix -- conditions a matrix                                   *
 *   HierarchySaveClass::save_header -- writes the header into a W3D file                      *
 *   HierarchySaveClass::save_pivots -- writes the pivots into a W3D file                      *
 *   HierarchySaveClass::save_fixups -- writes the fixup transforms into a W3D file            *
 *   HierarchySaveClass::load_header -- reads the header from a W3D file                       *
 *   HierarchySaveClass::load_pivots -- reads the pivots from a W3D file                       *
 *   HierarchySaveClass::load_fixups -- reads the fixup transforms from a W3D file             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "hiersave.h"
#include "w3d_file.h"
#include "nodefilt.h"
#include "EULER.h"
#include "util.h"
#include "w3dappdata.h"
#include "errclass.h"
#include "exportlog.h"


bool HierarchySaveClass::TerrainModeEnabled = false;


/***********************************************************************************************
 * HierarchySaveClass::HierarchySaveClass -- constructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 * root - root INode to construct the HTree from                                               *
 * time - current time in Max, transforms at this time will be used                            *
 * treemeter - progress meter                                                                  *
 * hname - name for the hierarchy tree                                                         *
 * fixup_type - can be used to force all transforms to be translation only                     *
 * fixuptree - htree loaded from a previous export                                             *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HierarchySaveClass::HierarchySaveClass
(
	INode * root,
	TimeValue time,
	Progress_Meter_Class & treemeter,
	char * hname,
	int fixuptype,
	HierarchySaveClass * fixuptree
) :
	Node(DEFAULT_NODE_ARRAY_SIZE),
	CurNode(0),
	FixupType(fixuptype),
	FixupTree(fixuptree)
{
	CurNode = 0;
	CurTime = time;

	/*
	** This code-path is activated when the user has created a custom origin.  In this case, we
	** need to compute the transform which will make all bones relative to this origin.
	*/
	OriginOffsetTransform = Inverse(root->GetNodeTM(CurTime));

	/*
	** Build our tree from the given tree of nodes
	*/
	int rootidx = add_node(NULL,-1);
 	assert(rootidx == 0);
	add_tree(root,rootidx);

	HierarchyHeader.Version = W3D_CURRENT_HTREE_VERSION;
	Set_W3D_Name(HierarchyHeader.Name,hname);
	HierarchyHeader.NumPivots = CurNode;
	HierarchyHeader.Center.X = 0.0f;
	HierarchyHeader.Center.Y = 0.0f;
	HierarchyHeader.Center.Z = 0.0f;
}

/***********************************************************************************************
 * HierarchySaveClass::HierarchySaveClass -- constructor                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * rootlist - list of root nodes to add to the htree                                           *
 * time - current time in Max, transforms at this time will be used                            *
 * treemeter - progress meter                                                                  *
 * hname - name for the hierarchy tree                                                         *
 * fixup_type - can be used to force all transforms to be translation only                     *
 * fixuptree - htree loaded from a previous export                                             *
 * origin_offset - origin offset transform                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HierarchySaveClass::HierarchySaveClass
(
	INodeListClass * rootlist,
	TimeValue time,
	Progress_Meter_Class & treemeter,
	char * hname,
	int fixuptype,
	HierarchySaveClass * fixuptree,
	const Matrix3 & origin_offset
) :
	Node(DEFAULT_NODE_ARRAY_SIZE),
	CurNode(0),
	FixupType(fixuptype),
	FixupTree(fixuptree),
	OriginOffsetTransform(origin_offset)
{
	CurNode = 0;
	CurTime = time;

	/*
	** Build the tree with all leaves of all of the nodes given
	*/
	int rootidx = add_node(NULL,-1);
 	assert(rootidx == 0);

	for (unsigned int i = 0; i < rootlist->Num_Nodes(); i++) {
		add_tree((*rootlist)[i],rootidx);
	}

	HierarchyHeader.Version = W3D_CURRENT_HTREE_VERSION;
	Set_W3D_Name(HierarchyHeader.Name,hname);
	HierarchyHeader.NumPivots = CurNode;
	HierarchyHeader.Center.X = 0.0f;
	HierarchyHeader.Center.Y = 0.0f;
	HierarchyHeader.Center.Z = 0.0f;

}

/***********************************************************************************************
 * HierarchySaveClass::HierarchySaveClass -- constructor                                       *
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
HierarchySaveClass::HierarchySaveClass():
	Node(NULL),
	CurNode(0),
	CurTime(0)
{
}

/***********************************************************************************************
 * HierarchySaveClass::~HierarchySaveClass -- destructor                                       *
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
HierarchySaveClass::~HierarchySaveClass(void)
{
	Free();
}

/***********************************************************************************************
 * HierarchySaveClass::Free -- releases all allocated memory                                   *
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
void HierarchySaveClass::Free(void)
{
	Node.Clear();
}


/***********************************************************************************************
 * HierarchySaveClass::Get_Node_Transform -- returns the transformation matrix of specified no *
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
Matrix3 HierarchySaveClass::Get_Node_Transform(int nodeidx) const
{
	Matrix3 tm(1);

	int idx = nodeidx;

	while (idx != -1) {
		tm = tm * get_relative_transform(idx);
		idx = Node[idx].Pivot.ParentIdx;
	}

	return tm;
}


/***********************************************************************************************
 * HierarchySaveClass::get_relative_transform -- retruns tm between this node and its parent   *
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
Matrix3 HierarchySaveClass::get_relative_transform(int nodeidx) const
{
	assert(nodeidx >= 0);
	assert(nodeidx < CurNode);

	Point3	trans;
	Quat		rot;
	Matrix3	tm(true);
	Matrix3	rtm(true);

	trans.x = Node[nodeidx].Pivot.Translation.X;
	trans.y = Node[nodeidx].Pivot.Translation.Y;
	trans.z = Node[nodeidx].Pivot.Translation.Z;

	//	WARNING! I had to fudge the orientation
	// quaternion (Max's representation seems to
	// rotate in the opposite sense as mine...)
	rot[0] = -Node[nodeidx].Pivot.Rotation.Q[0];
	rot[1] = -Node[nodeidx].Pivot.Rotation.Q[1];
	rot[2] = -Node[nodeidx].Pivot.Rotation.Q[2];
	rot[3] = Node[nodeidx].Pivot.Rotation.Q[3];

	tm.Translate(trans);
	rot.MakeMatrix(rtm);
	tm = rtm * tm;

	return tm;
}


/***********************************************************************************************
 * HierarchySaveClass::Get_Name -- returns the name of this hierarchy                          *
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
const char * HierarchySaveClass::Get_Name(void) const
{
	return HierarchyHeader.Name;
}


/***********************************************************************************************
 * HierarchySaveClass::Get_Node -- Get the Max INode                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
INode * HierarchySaveClass::Get_Node(int node) const
{
	assert(node >= 0);
	assert(node < CurNode);

	return Node[node].MaxNode;
}

/***********************************************************************************************
 * HierarchySaveClass::Get_Node_Name -- returns name of this hierarchy node                    *
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
const char * HierarchySaveClass::Get_Node_Name(int node) const
{
	assert(node >= 0);
	assert(node < CurNode);

	return Node[node].Pivot.Name;
}


/***********************************************************************************************
 * HierarchySaveClass::Find_Named_Node -- returns index of a named node                        *
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
int HierarchySaveClass::Find_Named_Node(const char * name) const
{
	int match = -1;
	for (int index=0; index<CurNode; index++) {
		if (strcmp(Node[index].Pivot.Name,name) == 0) {
			match = index;
		}
	}

	return match;
}

/***********************************************************************************************
 * HierarchySaveClass::Get_Export_Coordinate_System - find the bone and coordinate system      *
 * for the given object                                                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void HierarchySaveClass::Get_Export_Coordinate_System
(
	INode * node,
	int * set_bone_index,
	INode ** set_bone_node,
	Matrix3 * set_transform
)
{
	/*
	** find the first parent of this node which
	** is in the base pose.  Note, we're finding the parent bone
	** in our hierarchy with the same name as a bone in the "main"
	** hierarchy.  When we're exporting LOD models, there are multiple
	** hierarchies...
	*/
	bool	done = false;
	int	boneidx = -1;
	INode * pbone = node;

	while (!done) {

		char name[W3D_NAME_LEN];
		Set_W3D_Name(name,pbone->GetName());

		boneidx = Find_Named_Node(name);

		if (boneidx != -1) {

			/*
			** We found the parent bone!
			*/
			done = true;

		} else if (Is_Origin(pbone)) {

			/*
			** Don't go up past our origin, use this as our bone.
			*/
			boneidx = 0;
			done = true;

		} else {

			/*
			** Nope, try the next parent
			*/
			pbone = pbone->GetParentNode();
			assert(pbone != NULL);

#if 0
			if (pbone == NULL) {

				/*
				** mesh isn't connected to a bone, use the root
				*/
				boneidx = 0;
				pbone = node;
				done = true;
			}
#endif
		}
	}

	if (set_bone_index != NULL) {
		*set_bone_index = boneidx;
	}
	if (set_bone_node != NULL) {
		*set_bone_node = pbone;
	}
	if (set_transform != NULL) {
		*set_transform = Get_Fixup_Transform(boneidx) * pbone->GetNodeTM(CurTime);
	}
}

/***********************************************************************************************
 * HierarchySaveClass::Save -- write the hierarchy into a W3D file                             *
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
bool HierarchySaveClass::Save(ChunkSaveClass & csave)
{
	ExportLog::printf("\nSaving Hierarchy Tree %s.\n",HierarchyHeader.Name);
	ExportLog::printf("Node Count: %d\n",CurNode);
	ExportLog::printf("Nodes: \n");
	for (int inode = 0; inode < CurNode; inode++) {
		ExportLog::printf("  %s\n",Node[inode].Pivot.Name);
	}

	if (!csave.Begin_Chunk(W3D_CHUNK_HIERARCHY)) {
		return false;
	}

	if (!save_header(csave)) {
		return false;
	}

	if (!save_pivots(csave)) {
		return false;
	}

	if (!save_fixups(csave)) {
		return false;
	}

	if (!csave.End_Chunk()) {
		return false;
	}

	return true;
}


/***********************************************************************************************
 * HierarchySaveClass::Load -- read the hierarchy from a W3D file                              *
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
bool HierarchySaveClass::Load(ChunkLoadClass & cload)
{
	Free();
	bool error = false;

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {
			case W3D_CHUNK_HIERARCHY_HEADER:
				if (!load_header(cload)) error = true;
				break;
			case W3D_CHUNK_PIVOTS:
				if (!load_pivots(cload)) error = true;
				break;
			case W3D_CHUNK_PIVOT_FIXUPS:
				if (!load_fixups(cload)) error = true;
				break;
			default:
				break;
		}

		if (!cload.Close_Chunk() || error) {
			return false;
		}
	}

	CurNode = HierarchyHeader.NumPivots;

	return true;
}


/***********************************************************************************************
 * HierarchySaveClass::add_tree -- adds a node and all of its children                         *
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
void HierarchySaveClass::add_tree(INode * node,int pidx)
{
	int nextparent;

	if (node->IsHidden ()) {

		// if the node is hidden, do not add it but add its children to the current parent.
		nextparent = pidx;

	} else if (TerrainModeEnabled && (Is_Normal_Mesh(node) || Is_Null_Object(node))) {

		// terrain optimization, normal meshes are not allowed to have transforms
		nextparent = pidx;

	} else if (!Is_Bone(node)) {

		// This node isn't a bone, don't add it
		nextparent = pidx;

	} else {

		// Add new pivot!	it will be parent of all below it.
		nextparent = add_node(node,pidx);

	}

	// Add all of this nodes children
	for (int i=0; i < node->NumberOfChildren(); i++) {
		add_tree(node->GetChildNode(i),nextparent);
	}
}


/***********************************************************************************************
 * HierarchySaveClass::add_node -- adds a single node to the tree                              *
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
int HierarchySaveClass::add_node(INode * node,int pidx)
{
	/*
	** 'grow' the node array if necessary
	*/
	if (CurNode >= Node.Length ()) {
		Node.Resize (Node.Length () + NODE_ARRAY_GROWTH_SIZE);
	}

	/*
	** setup the pivot
	*/
	Node[CurNode].MaxNode = node;
	Node[CurNode].Pivot.ParentIdx = pidx;

	if (node) {
		Set_W3D_Name(Node[CurNode].Pivot.Name,node->GetName());
	} else {
		Set_W3D_Name(Node[CurNode].Pivot.Name,"RootTransform");
	}

	/*
	** Now, check if there is a bone with this W3D name already
	** if there is, scold the user and bail out
	*/
	if (Find_Named_Node(Node[CurNode].Pivot.Name) != -1) {
		char buf[128];
		sprintf(buf,"Bones with duplicate names found!\nDuplicated Name: %s\n",Node[CurNode].Pivot.Name);
		throw ErrorClass(buf);
	}

	/*
	** Compute the transformation for this node
	*/
	Matrix3		maxnodeTM(1);
	Matrix3		ournodeTM(1);
	Matrix3		fixupTM(1);
	Point3		trans(0,0,0);
	Quat			rot(1);
	Point3		scale(1,1,1);

	if (node) {
		maxnodeTM = node->GetNodeTM(CurTime) * OriginOffsetTransform;
	} else {
		maxnodeTM = Matrix3(1);
	}

	/*
	** If this tree is being "fixed up" the first thing we do
	** is to transform Max's nodeTM by the fixup transform.
	** This is done when a base pose was created using our own
	** types of transforms and we want to apply the same
	** changes to this tree.
	**
	** Note that if FixupType is not "NONE", FixupTree must be NULL,
	*/
	assert(!((FixupTree != NULL) && (FixupType != MATRIX_FIXUP_NONE)));

	if (FixupTree != NULL) {
		int fi = FixupTree->Find_Named_Node(Node[CurNode].Pivot.Name);
		if (fi == -1) {
			char buf[128];
			sprintf(buf,"Incompatible Base Pose!\nMissing Bone: %s\n",Node[CurNode].Pivot.Name);
			throw ErrorClass(buf);
		}

		Matrix3 fixup = FixupTree->Get_Fixup_Transform(fi);

		maxnodeTM = fixup * maxnodeTM;
	}


	ournodeTM = fixup_matrix(maxnodeTM);
	fixupTM = ournodeTM * Inverse(maxnodeTM);

	/*
	** Now, make ournodeTM relative to its parent transform.  We
	** will always store relative transformations.  (Also, note
	** that it is relative to our version of the parent transform
	** which is not necessarily the same as the MAX version...)
	*/
	if (pidx != -1) {
		Matrix3 parentTM = Get_Node_Transform(pidx);
		Matrix3 pinv = Inverse(parentTM);
		ournodeTM = ournodeTM * pinv;
	}


	/*
	** Break the matrix down into a rotation and translation.
	*/
	DecomposeMatrix(ournodeTM,trans,rot,scale);

	/*
	** Save the "fixup" matrix
	*/
	for (int j=0;j<4;j++) {
		Point3 row = fixupTM.GetRow(j);
		Node[CurNode].Fixup.TM[j][0] = row.x;
		Node[CurNode].Fixup.TM[j][1] = row.y;
		Node[CurNode].Fixup.TM[j][2] = row.z;
	}

	/*
	** Set the translation and rotation for this pivot.
	*/
	Node[CurNode].Pivot.Translation.X	= trans.x;
	Node[CurNode].Pivot.Translation.Y	= trans.y;
	Node[CurNode].Pivot.Translation.Z	= trans.z;

	Node[CurNode].Pivot.Rotation.Q[0] = -rot[0];
	Node[CurNode].Pivot.Rotation.Q[1] = -rot[1];
	Node[CurNode].Pivot.Rotation.Q[2] = -rot[2];
	Node[CurNode].Pivot.Rotation.Q[3] = rot[3];

	/*
	** Compute the Euler angles and set them.
	*/
	Matrix3 rotmat;
	rot.MakeMatrix(rotmat);
	EulerAnglesClass eangs(rotmat,EulerOrderXYZr);

	Node[CurNode].Pivot.EulerAngles.X = eangs.Get_Angle(0);
	Node[CurNode].Pivot.EulerAngles.Y = eangs.Get_Angle(1);
	Node[CurNode].Pivot.EulerAngles.Z = eangs.Get_Angle(2);

	return CurNode++;
}

/***********************************************************************************************
 * HierarchySaveClass::Get_Fixup_Transform -- gets the "fixup" transform for a node            *
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
Matrix3 HierarchySaveClass::Get_Fixup_Transform(int node) const
{
	assert(node >= 0);
	assert(node < CurNode);

	Matrix3 m;

	for (int j=0;j<4;j++) {
		m.SetRow(j,Point3(Node[node].Fixup.TM[j][0],Node[node].Fixup.TM[j][1],Node[node].Fixup.TM[j][2]));
	}

	return m;
}

/***********************************************************************************************
 * HierarchySaveClass::fixup_matrix -- conditions a matrix                                     *
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
Matrix3 HierarchySaveClass::fixup_matrix(const Matrix3 & csrc) const
{
	Matrix3	src = csrc;	// the GetTrans function is not const correct...
	Matrix3	newtm(1);
	Point3	trans;
	Quat		rot;
	Point3	scale;

	switch (FixupType) {
		case MATRIX_FIXUP_NONE:
			newtm = src;
			break;

		case MATRIX_FIXUP_TRANS:
			newtm.SetTrans(src.GetTrans());
			newtm = Cleanup_Orthogonal_Matrix(newtm);
			break;

		case MATRIX_FIXUP_TRANS_ROT:
			DecomposeMatrix(src,trans,rot,scale);
			rot.MakeMatrix(newtm);
			newtm.SetTrans(trans);
			newtm = Cleanup_Orthogonal_Matrix(newtm);
			break;
	};

	return newtm;
}


/***********************************************************************************************
 * HierarchySaveClass::save_header -- writes the header into a W3D file                        *
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
bool HierarchySaveClass::save_header(ChunkSaveClass & csave)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_HIERARCHY_HEADER)) {
		return false;
	}

	if (csave.Write(&HierarchyHeader,sizeof(HierarchyHeader)) != sizeof(HierarchyHeader)) {
		return false;
	}

	if (!csave.End_Chunk()) {
		return false;
	}

	return true;
}


/***********************************************************************************************
 * HierarchySaveClass::save_pivots -- writes the pivots into a W3D file                        *
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
bool HierarchySaveClass::save_pivots(ChunkSaveClass & csave)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_PIVOTS)) {
		return false;
	}

	for (uint32 i=0; i<HierarchyHeader.NumPivots; i++) {
		if (csave.Write(&Node[i].Pivot,sizeof(W3dPivotStruct)) != sizeof(W3dPivotStruct)) {
			return false;
		}
	}

	if (!csave.End_Chunk()) {
		return false;
	}

	return true;
}

/***********************************************************************************************
 * HierarchySaveClass::save_fixups -- writes the fixup transforms into a W3D file              *
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
bool HierarchySaveClass::save_fixups(ChunkSaveClass & csave)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_PIVOT_FIXUPS)) {
		return false;
	}

	for (uint32 i=0; i<HierarchyHeader.NumPivots; i++) {
		if (csave.Write(&Node[i].Fixup,sizeof(W3dPivotFixupStruct)) != sizeof(W3dPivotFixupStruct)) {
			return false;
		}
	}

	if (!csave.End_Chunk()) {
		return false;
	}

	return true;
}


/***********************************************************************************************
 * HierarchySaveClass::load_header -- reads the header from a W3D file                         *
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
bool HierarchySaveClass::load_header(ChunkLoadClass & cload)
{
	/*
	** Load the header
	*/
	if (cload.Read(&HierarchyHeader,sizeof(HierarchyHeader)) != sizeof(HierarchyHeader)) {
		return false;
	}

	/*
	** Reset the current node count
	*/
	CurNode = 0;
	Node.Resize(HierarchyHeader.NumPivots);

	/*
	** Initialize everything to a default state (particularly the
	** fixup matrices to identity...)
	*/
	for (unsigned i=0; i < HierarchyHeader.NumPivots; i++) {
		memset(&(Node[i]),0,sizeof(HierarchyNodeStruct));

		Matrix3 ident(1);
		for (int j=0; j<3; j++) {
			Point3 row = ident.GetRow(j);
			Node[i].Fixup.TM[j][0] = row.x;
			Node[i].Fixup.TM[j][1] = row.y;
			Node[i].Fixup.TM[j][2] = row.z;
		}
	}

	return true;
}


/***********************************************************************************************
 * HierarchySaveClass::load_pivots -- reads the pivots from a W3D file                         *
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
bool HierarchySaveClass::load_pivots(ChunkLoadClass & cload)
{
	for (uint32 i=0; i<HierarchyHeader.NumPivots; i++) {
		Node[i].MaxNode = NULL;
		if (cload.Read(&Node[i].Pivot,sizeof(W3dPivotStruct)) != sizeof(W3dPivotStruct)) {
			return false;
		}
	}
	return true;
}

/***********************************************************************************************
 * HierarchySaveClass::load_fixups -- reads the fixup transforms from a W3D file               *
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
bool HierarchySaveClass::load_fixups(ChunkLoadClass & cload)
{
	for (uint32 i=0; i<HierarchyHeader.NumPivots; i++) {
		if (cload.Read(&Node[i].Fixup,sizeof(W3dPivotFixupStruct)) != sizeof(W3dPivotFixupStruct)) {
			return false;
		}
	}
	return true;
}


