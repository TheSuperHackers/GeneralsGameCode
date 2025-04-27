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
 *                 Project Name : Commando Tools - WWSkin                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/skin.cpp                       $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/24/01 5:15p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "skin.h"
#include "dllmain.h"
#include "max.h"
#include "simpmod.h"
#include "simpobj.h"
#include "resource.h"

#include "skindata.h"
#include "bpick.h"
#include "namedsel.h"
#include "boneicon.h"

#if defined W3D_MAX4		//defined as in the project (.dsp)
static GenSubObjType _SubObjectTypeVertex(1);
#endif

/*
** Static functions
*/
static BOOL CALLBACK _sot_dialog_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK _skeleton_dialog_thunk(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
static BOOL CALLBACK _bone_influence_dialog_thunk(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
static TriObject * Get_Tri_Object(TimeValue t,ObjectState & os,Interval & valid,BOOL & needsdel);
static float Bone_Distance(INode * bone,TimeValue time,const Point3 & vertex);

/*
** Static variables
*/
HWND					SkinWSMObjectClass::SotHWND = NULL;
HWND					SkinWSMObjectClass::SkeletonHWND = NULL;
HWND					SkinWSMObjectClass::BoneListHWND = NULL;
IObjParam *			SkinWSMObjectClass::InterfacePtr = NULL;
ICustButton *		SkinWSMObjectClass::AddBonesButton = NULL;
ICustButton *		SkinWSMObjectClass::RemoveBonesButton = NULL;
ISpinnerControl *	SkinWSMObjectClass::BasePoseSpin = NULL;


/*******************************************************************************
**
** Class Descriptor for SkinWSMObjectClass
**
*******************************************************************************/
class SkinWSMObjectClassDesc:public ClassDesc 
{
public:

	int 				IsPublic()								{ return 1; }
	void *			Create(BOOL loading = FALSE)		{ return new SkinWSMObjectClass; }
	const TCHAR *	ClassName()								{ return _T("WWSkin"); }
	SClass_ID		SuperClassID()							{ return WSM_OBJECT_CLASS_ID; }
	Class_ID			ClassID()								{ return SKIN_OBJ_CLASS_ID; }
	const TCHAR* 	Category()								{ return _T("Westwood Space Warps");  }
};

static SkinWSMObjectClassDesc _SkinWSMObjectDesc;
ClassDesc * Get_Skin_Obj_Desc() { return &_SkinWSMObjectDesc; }


/*******************************************************************************
**
** Class Descriptor for the SkinModifier
**
*******************************************************************************/
class SkinModClassDesc:public ClassDesc 
{
public:

	int 				IsPublic()								{ return 0; }
	void *			Create(BOOL loading = FALSE)		{ return new SkinModifierClass; }
	const TCHAR *	ClassName()								{ return _T("WWSkin"); }
	SClass_ID		SuperClassID()							{ return WSM_CLASS_ID; }
	Class_ID			ClassID()								{ return SKIN_MOD_CLASS_ID; }
	const TCHAR * 	Category()								{ return _T("Westwood Space Warps");  }
};

static SkinModClassDesc _SkinModDesc;
ClassDesc * Get_Skin_Mod_Desc() { return &_SkinModDesc; }



/*******************************************************************************
**
**		SkinWSMObjectCreateCallback
**		A class derived from CreateMouseCallBack to handle
**		the user input during the creation phase of the SkinWSMObject.
** 
*******************************************************************************/
class SkinWSMObjectCreateCallBack : public CreateMouseCallBack 
{	
public:
	int proc( ViewExp * vpt,int msg, int point, int flags, IPoint2 m, Matrix3 & mat)
	{
		if (msg == MOUSE_POINT) {
			Point3 pos = vpt->GetPointOnCP(m);
			mat.IdentityMatrix();
			mat.SetTrans(pos);
			return CREATE_STOP;	
		}
		return TRUE;
	}
};

static SkinWSMObjectCreateCallBack _SkinCreateCB;

/*******************************************************************************
**
**		SkinWSMObjectClass
**
*******************************************************************************/
SkinWSMObjectClass::SkinWSMObjectClass() 
{
	/*
	** Initialize class variables to default state!
	*/
	MeshBuilt = FALSE;
	BoneSelectionMode = BONE_SEL_MODE_NONE;
	BoneTab.SetCount(0);
	BasePoseFrame = 0;

	pblock = NULL;
}

SkinWSMObjectClass::~SkinWSMObjectClass(void) 
{
	assert(!((InterfacePtr == NULL) && (SotHWND != NULL)));
	if (SotHWND != NULL) {
		InterfacePtr->UnRegisterDlgWnd(SotHWND);		
		InterfacePtr->DeleteRollupPage(SotHWND);
	  	SotHWND = NULL;				
	}

	assert(!((InterfacePtr == NULL) && (SkeletonHWND != NULL)));
	if (SkeletonHWND != NULL) {
		InterfacePtr->UnRegisterDlgWnd(SkeletonHWND);		
		InterfacePtr->DeleteRollupPage(SkeletonHWND);
	  	SkeletonHWND = NULL;				
	}
}

void SkinWSMObjectClass::BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev)
{
	SimpleWSMObject::BeginEditParams(ip,flags,prev);

	/*
	** save off a copy of the interface pointer
	*/
	InterfacePtr = ip;

	/*
	** Install the "supports objects of type" rollup
	*/
	if (SotHWND == NULL) {
		SotHWND = ip->AddRollupPage( 
					AppInstance, 
					MAKEINTRESOURCE(IDD_SKIN_SOT),
					_sot_dialog_proc,
					Get_String(IDS_SOT), 
					(LPARAM)InterfacePtr,
					APPENDROLL_CLOSED);
	} else {
		SetWindowLong(SotHWND,GWL_USERDATA,(LPARAM)ip);
	}

	/*
	** Install the skeleton rollup
	*/
	if (SkeletonHWND == NULL) {
		SkeletonHWND = InterfacePtr->AddRollupPage( 
					AppInstance, 
					MAKEINTRESOURCE(IDD_SKELETON_PARAMETERS),
					_skeleton_dialog_thunk,
					Get_String(IDS_SKELETON_PARAMETERS), 
					(LPARAM)this,
					0);
	} else {
		SetWindowLong(SkeletonHWND,GWL_USERDATA,(LPARAM)this);
	}

	Update_Bone_List();
}

void SkinWSMObjectClass::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
{
	SimpleWSMObject::EndEditParams(ip,flags,next);

	if (flags & END_EDIT_REMOVEUI) {
		/*
		** Remove the Sot rollup
		*/
		if (SotHWND != NULL) {
			InterfacePtr->UnRegisterDlgWnd(SotHWND);		
			InterfacePtr->DeleteRollupPage(SotHWND);
	  		SotHWND = NULL;				
		}
	
		/*
		** Remove the info rollup
		*/
		if (SkeletonHWND != NULL) {
			InterfacePtr->UnRegisterDlgWnd(SkeletonHWND);		
			InterfacePtr->DeleteRollupPage(SkeletonHWND);
	  		SkeletonHWND = NULL;				
		}
	}

	/*
	** get rid of our copy of the interface pointer
	*/
	InterfacePtr = NULL;
}

RefTargetHandle SkinWSMObjectClass::Clone(RemapDir & remap)
{
	/*
	** create another SkinWSMObject and return it.
	*/
	SkinWSMObjectClass * sobj = new SkinWSMObjectClass();	
	return(sobj);
}

RefTargetHandle SkinWSMObjectClass::GetReference(int i)
{
	/*
	** return reference "i".  If i==0, the reference belongs
	** to SimpleWSMObject so thunk down to it.
	*/
	if (i < SimpleWSMObject::NumRefs()) {
		return SimpleWSMObject::GetReference(i);
	}

	/*
	** The rest of the references are ours.
	*/
	int boneidx = To_Bone_Index(i);
	return BoneTab[boneidx];
}

void SkinWSMObjectClass::SetReference(int i, RefTargetHandle rtarg) 
{
	if (i < SimpleWSMObject::NumRefs()) {
		SimpleWSMObject::SetReference(i,rtarg);
	} else {
		int boneidx = To_Bone_Index(i);
		assert(boneidx >= 0);
		assert(boneidx < BoneTab.Count());
		BoneTab[boneidx] = (INode *)rtarg;
	}
}		

RefResult SkinWSMObjectClass::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	int i;
	switch (message) {

		case REFMSG_TARGET_DELETED:
			for (i=0; i<BoneTab.Count(); i++) {
				if (BoneTab[i] == hTarget) {
					break;
				}
			}
			if (i < BoneTab.Count()) {
				BoneTab.Delete(i,1);
				// TODO: cause all Modifier objects to re-index to accomodate
				// the deletion of this bone!!
			}
			break;

	}

	return(REF_SUCCEED);
}

CreateMouseCallBack * SkinWSMObjectClass::GetCreateMouseCallBack(void)
{
	/*
	** The "CreateMouseCallback" is used when creating the 
	** object.  Since our object doesn't need an interactive
	** creation phase, we return NULL.
	*/
	return &_SkinCreateCB;
}

Modifier * SkinWSMObjectClass::CreateWSMMod(INode *node)
{
	/*
	** create an instance of a SkinModifierClass...
	*/
	return new SkinModifierClass(node,this);
}

void SkinWSMObjectClass::BuildMesh(TimeValue t)
{
	int i;

	/*
	** We only need to build the object mesh once since it
	** doesn't change or animate...
	*/
	if (MeshBuilt) return;

	/*
	** Set our validity interval, since this mesh doesn't animate
	** we just use FOREVER.
	*/
	ivalid = FOREVER;
	
	/*
	** Ok, this is the first time BuildMesh has been called.
	** Create a mesh which will represent this space warp object in 
	** in the max viewports.  Note that we are using the mesh
	** member which is inherited from SimpleWSMObject.
	*/
	mesh.setNumVerts(NumBoneIconVerts);
	mesh.setNumFaces(NumBoneIconFaces);
	for (i=0; i<NumBoneIconVerts; i++) {
		mesh.setVert(i, Point3(BoneIconVerts[i].X, BoneIconVerts[i].Y, BoneIconVerts[i].Z));
	}

	for (i=0; i<NumBoneIconFaces; i++) {
		Build_Tri(&(mesh.faces[i]),BoneIconFaces[i].V0,BoneIconFaces[i].V1,BoneIconFaces[i].V2);
	}

	mesh.InvalidateGeomCache();

	MeshBuilt = TRUE;
}

void SkinWSMObjectClass::Build_Tri(Face * f, int a,  int b, int c) 
{
	f->setVerts(a, b, c);
	f->setSmGroup(0);
	f->setEdgeVisFlags(1,1,1);
}

void SkinWSMObjectClass::User_Picked_Bone(INode * node)
{
	// TODO: Undo/Redo!
	switch (BoneSelectionMode) {
		case BONE_SEL_MODE_ADD:
			Add_Bone(node);
			break;

		case BONE_SEL_MODE_REMOVE:
			Remove_Bone(node);
			break;

		default:
			assert(0);
	}
	
	Set_Bone_Selection_Mode(BONE_SEL_MODE_NONE);
	Update_Bone_List();
}

void SkinWSMObjectClass::User_Picked_Bones(INodeTab & nodetab)
{
	// TODO: Undo/Redo!
	switch (BoneSelectionMode) {
		case BONE_SEL_MODE_ADD_MANY:
			Add_Bones(nodetab);
			break;

		case BONE_SEL_MODE_REMOVE_MANY:
			Remove_Bones(nodetab);
			break;

		default:
			assert(0);
	}

	Set_Bone_Selection_Mode(BONE_SEL_MODE_NONE);
	Update_Bone_List();
}

void SkinWSMObjectClass::Set_Bone_Selection_Mode(int mode)
{
	assert(mode >= BONE_SEL_MODE_NONE);
	assert(mode <= BONE_SEL_MODE_REMOVE_MANY);
	
	/*
	** store the selection mode
	*/
	BoneSelectionMode = mode;

	/*
	** update the dialog box buttons
	*/
	AddBonesButton->SetCheck(mode == BONE_SEL_MODE_ADD_MANY);
	RemoveBonesButton->SetCheck(mode == BONE_SEL_MODE_REMOVE_MANY);
}

int SkinWSMObjectClass::Add_Bone(INode * node)
{
	int refidx;
	int boneidx;

	/*
	** If we already have this bone, just return
	*/
	boneidx = Find_Bone(node);
	if (boneidx != -1) {
		return boneidx;
	}

	/*
	** Otherwise, look for a NULL bone and we'll re-use
	** its slot.  This happens when a user removes a bone or 
	** a bone in the scene is deleted.
	*/
	boneidx = Find_Bone(NULL);
	if (boneidx != -1) {
		refidx = To_Ref_Index(boneidx);
		MakeRefByID(FOREVER,refidx,node);
		return boneidx;
	}
	
	/*
	** If we made it here, add the bone to the end of the 
	** reference array.
	*/
	BoneTab.Append(1,&node);		
	boneidx = BoneTab.Count() - 1;
	refidx = To_Ref_Index(boneidx);
	MakeRefByID(FOREVER,refidx,node);
	return boneidx;
}

void SkinWSMObjectClass::Add_Bones(INodeTab & nodetab)
{
	/*
	** Add each bone individually
	*/
	for (int i=0; i<nodetab.Count(); i++) {
		Add_Bone(nodetab[i]);
	}
}

void SkinWSMObjectClass::Remove_Bone(INode * node)
{
	int boneidx = Find_Bone(node);
	if (boneidx != -1) {
		BoneTab[boneidx] = NULL;
		DeleteReference(To_Ref_Index(boneidx));
	}
}

void SkinWSMObjectClass::Remove_Bones(INodeTab & nodetab)
{
	/*
	** remove each bone
	*/
	for (int i=0; i<nodetab.Count(); i++) {
		Remove_Bone(nodetab[i]);
	}
}

void SkinWSMObjectClass::Update_Bone_List(void)
{
	assert(BoneListHWND != NULL);

	/*
	** remove all strings in the bone listbox
	*/
	SendMessage(BoneListHWND,LB_RESETCONTENT,0,0);

	/*
	** loop through the bone tab and add the name of each
	*/
	for (int i=0; i<BoneTab.Count(); i++) {
		if (BoneTab[i] != NULL) {
			SendMessage(BoneListHWND,LB_ADDSTRING,0,(LPARAM)BoneTab[i]->GetName());
		}
	}
}

int SkinWSMObjectClass::Find_Bone(INode * node)
{
	for (int i=0; i<BoneTab.Count(); i++) {
		if (BoneTab[i] == node) return i;
	}
	return -1;
}

IOResult SkinWSMObjectClass::Save(ISave * isave)
{
	ULONG nb;
	SimpleWSMObject::Save(isave);

	/*
	** Save the number of bones
	*/
	ULONG numbones = BoneTab.Count();
	if (numbones > 0) {
		isave->BeginChunk(NUM_BONES_CHUNK);
		isave->Write(&numbones,sizeof(ULONG),&nb);
		isave->EndChunk();
	}
	return IO_OK;
}

IOResult SkinWSMObjectClass::Load(ILoad * iload)
{
	SimpleWSMObject::Load(iload);

	IOResult res;
	ULONG nb;
	int level = -1;

	while (IO_OK==(res=iload->OpenChunk())) {

		switch (iload->CurChunkID())  {

			case NUM_BONES_CHUNK: {
				ULONG numbones;
				res = iload->Read(&numbones,sizeof(numbones),&nb);
				BoneTab.SetCount(numbones);
				for (int i=0; i<BoneTab.Count(); i++) {
					BoneTab[i] = NULL;
				}
			}
			break;
		}

		iload->CloseChunk();

		if (res!=IO_OK) {
			return res;
		}
	}
	return IO_OK;
}


int SkinWSMObjectClass::Find_Closest_Bone(const Point3 & vertex)
{
	float mindist = 10000.0f;
	int minindex = -1;
	TimeValue basetime = Get_Base_Pose_Time();

	for (int boneidx = 0; boneidx < BoneTab.Count(); boneidx++) {
		
		if (BoneTab[boneidx] == NULL) continue;

		float bonedist = Bone_Distance(BoneTab[boneidx],basetime,vertex);
		if (bonedist < mindist) {
			mindist = bonedist;
			minindex = boneidx;
		}
	}

	return minindex;
}



/*******************************************************************************
**
**		SkinModifierClass
**
*******************************************************************************/

SkinModifierClass::SkinModifierClass(void)
{
	Default_Init();
}

SkinModifierClass::SkinModifierClass(INode * node,SkinWSMObjectClass * skin_obj) 
{
	Default_Init();

	/*
	** Make the reference to the space warp node.
	*/
	MakeRefByID(FOREVER,NODE_REF,node);	

	/*
	** Make reference to the WSMObject
	*/
	MakeRefByID(FOREVER,OBJ_REF,skin_obj);

}

void SkinModifierClass::Default_Init(void)
{
	SubObjSelLevel = VERTEX_SEL_LEVEL;
	WSMObjectRef = NULL;
	WSMNodeRef = NULL;	
	InterfacePtr = NULL;

	BoneInfluenceHWND = NULL;
	LinkButton = NULL;
	LinkByNameButton = NULL;
	AutoLinkButton = NULL;
	UnLinkButton = NULL;
}


RefTargetHandle SkinModifierClass::Clone(RemapDir & remap)
{
	SkinModifierClass * newmod = new SkinModifierClass(WSMNodeRef,(SkinWSMObjectClass *)WSMObjectRef);
	return newmod;
}


void SkinModifierClass::BeginEditParams(IObjParam * ip, ULONG flags,Animatable * prev)
{
	static int i=0;

	i++;

	/*
	** Grab a copy of the interface pointer
	*/
	InterfacePtr = ip;
	
	/*
	** allocate the selection command mode for use in vertex selection
	*/
	SelectMode = new SelectModBoxCMode(this,InterfacePtr);

	/*
	** register the desired sub-object selection types.
	*/
	const TCHAR * ptype[] = { "Vertices" };
#if defined W3D_MAX4		//defined as in the project (.dsp)
	InterfacePtr->SetSubObjectLevel(1);
#else
	//---This call is obsolete from version 4.   
	InterfacePtr->RegisterSubObjectTypes( ptype, 1);
#endif
	/*
	** Restore the selection level.
	*/
	ip->SetSubObjectLevel(SubObjSelLevel);

}


void SkinModifierClass::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
{
	/*
	** just checking...
	*/
	assert(ip == InterfacePtr);

	/*
	** Make sure we clear out the pick mode
	*/
	InterfacePtr->ClearPickMode();

	/*
	** remove and deallocate the selection command mode
	*/
	InterfacePtr->DeleteMode(SelectMode);
	if (SelectMode ) delete SelectMode;
	SelectMode = NULL;
		
	/*
	** Remove the rollup window(s) if needed
	*/
	if (flags & END_EDIT_REMOVEUI) {		
		Remove_Bone_Influence_Dialog();
	} 

	/*
	** Make sure we don't hang onto an invalid interface
	*/
	InterfacePtr = NULL;
}

Interval SkinModifierClass::Get_Validity(TimeValue t)
{
	/*
	** Start with an infinite interval and chop it down
	** using the validity intervals of each of the controlling bones
	*/
	Interval valid = FOREVER;

	/*
	** Now intersect the validity with the validities of all of
	** the controlling bones.
	*/
	SkinWSMObjectClass * obj = (SkinWSMObjectClass *)Get_WSMObject();

//	for (int i=0; i<obj->Num_Bones(); i++) {
//		valid &= obj->Get_Bone(i)->tmValid();	//TODO: is this right?
//	}

//	return valid;
	return Interval(t,t+1); //KLUDGE - only valid for this frame
}

RefTargetHandle SkinModifierClass::GetReference(int i)
{
	switch (i) {
		case OBJ_REF:			return WSMObjectRef;
		case NODE_REF:			return WSMNodeRef;
		default:					return NULL;
	}
}

void SkinModifierClass::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
		case OBJ_REF:			WSMObjectRef = (SkinWSMObjectClass *)rtarg; break;
		case NODE_REF:			WSMNodeRef = (INode *)rtarg; break;
	}
}

RefResult SkinModifierClass::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
{
	switch (message) {

		case REFMSG_TARGET_DELETED:
	
			/*
			** This means the WSM node is being deleted. As a result,
			** we must delete ourselves. 
			*/
			DeleteMe();		// also deletes all refs and 
								// sends REFMSG_TARGET_DELETED to all Dependents
			return REF_STOP;
	}

	return(REF_SUCCEED);

}

void SkinModifierClass::ModifyObject(TimeValue t, ModContext & mc, ObjectState * os, INode * node)
{
	/*
	** Get a TriObject from the object state
	*/
	assert(os->obj->IsSubClassOf(triObjectClassID));
	TriObject *triobj = (TriObject *)os->obj;

	/*
	** Get the skin data from the ModContext.
	*/
	SkinDataClass * skindata = (SkinDataClass *)mc.localData;
	
	/*
	** If there is no skin data, allocate it
	** Also, do an initial auto attach.
	*/
	if (skindata == NULL) {
		mc.localData = skindata = new SkinDataClass(&triobj->mesh);
	}

	if (!skindata->IsValid()) {
		skindata->Validate(&triobj->mesh);
	}
	
	/*
	** If in vertex selection mode, tell the mesh to display the 
	** selected vertices and turn on vertex tick marks.  Otherwise
	** make sure vertex tick marks are off.
	*/
	if (SubObjSelLevel == VERTEX_SEL_LEVEL) {
		triobj->mesh.vertSel = skindata->VertSel;
		triobj->mesh.SetDispFlag(DISP_VERTTICKS|DISP_SELVERTS);

		if (triobj->mesh.selLevel != MESH_VERTEX) {	
			triobj->mesh.selLevel = MESH_VERTEX;
		}
	} else {
		triobj->mesh.selLevel = MESH_OBJECT;
		triobj->mesh.ClearDispFlag(DISP_VERTTICKS|DISP_SELVERTS);
	}

	/*
	** Loop through the points in the deformable object
	*/
	for (int vidx = 0; vidx < triobj->NumPoints(); vidx++) {


		// TODO: Allow multiple bone influences here...
		// issues - UI to set the weights, rebalance weights whenever
		// a bone is deleted, should also then never get NULL bones
		// and remove the need to check for NULL bones in this routine...
 
		/*
		** Get a pointer to the bone that this vertex is attached to
		*/
		InfluenceStruct * inf = &(skindata->VertData[vidx]);

		int boneidx = inf->BoneIdx[0];
		if ((boneidx != -1) && (boneidx < WSMObjectRef->Num_Bones())) {
				
			INode * bone = WSMObjectRef->Get_Bone(inf->BoneIdx[0]);

			if (bone == NULL) {
				/*
				** this bone has gone away for some reason so
				** clear this vert's bone influence index
				*/
				inf->BoneIdx[0] = -1;
	
			} else {
					
				/*
				** Ok, got the bone, now transform the point and
				** give it back to the mesh
				*/
				Point3 pnew;
				Matrix3 tm;

				pnew = triobj->GetPoint(vidx);
				
				if (os->GetTM()) {
					tm = *(os->GetTM());
				} else {
					tm.IdentityMatrix();
				}

				pnew = tm * pnew;
								
				TimeValue basetime = WSMObjectRef->Get_Base_Pose_Time();
				Matrix3 basetm = bone->GetObjectTM(basetime);
				Matrix3 curtm = bone->GetObjectTM(t);
				pnew = (pnew * Inverse(basetm)) * curtm;
				
				pnew = Inverse(tm) * pnew;

				triobj->SetPoint(vidx,pnew);
			}
		}
	}

	/*
	** Tell the object that points were changed
	*/
	triobj->PointsWereChanged();		

	/*
	** Set the validity of the updated geometry data
	*/
	triobj->UpdateValidity(GEOM_CHAN_NUM,Get_Validity(t));	
}

IOResult SkinModifierClass::Save(ISave * isave)
{
	ULONG nb;
	Modifier::Save(isave);

	/*
	** Save the sub object selection level
	*/
	short sl = SubObjSelLevel;
	isave->BeginChunk(SEL_LEVEL_CHUNK);
	isave->Write(&sl,sizeof(short),&nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult SkinModifierClass::Load(ILoad * iload)
{
	Modifier::Load(iload);

	IOResult res;
	ULONG nb;
	int level = -1;

	while (IO_OK==(res=iload->OpenChunk())) {

		switch (iload->CurChunkID())  {

			case SEL_LEVEL_CHUNK: {
				short sl;
				res = iload->Read(&sl,sizeof(short),&nb);
				SubObjSelLevel = sl;
			}
			break;
		}

		iload->CloseChunk();

		if (res!=IO_OK) {
			return res;
		}
	}
	return IO_OK;
}

IOResult SkinModifierClass::SaveLocalData(ISave *isave, LocalModData *ld)
{
	SkinDataClass * skindata = (SkinDataClass *)ld;

	return skindata->Save(isave);
}

IOResult SkinModifierClass::LoadLocalData(ILoad *iload, LocalModData **pld)
{
	/*
	** Create a new SkinDataClass
	*/
	if (*pld==NULL) {
		*pld = (SkinDataClass *) new SkinDataClass();
	}
	SkinDataClass * newskin = (SkinDataClass *)*pld;

	/*
	** Initialize it from ILoad...
	*/
	return newskin->Load(iload);
}


void SkinModifierClass::ActivateSubobjSel(int level, XFormModes & modes)
{
	/*
	** Storing the current sub-object selection level
	*/
	SubObjSelLevel = level;

	/*
	** Set the appropriate command mode.  We only want selection.
	*/
	switch (SubObjSelLevel) 
	{
		case OBJECT_SEL_LEVEL:
			Remove_Bone_Influence_Dialog();
			break;

		case VERTEX_SEL_LEVEL: // Modifying Vertices
			modes = XFormModes(NULL,NULL,NULL,NULL,NULL,SelectMode);
			Install_Bone_Influence_Dialog();
			break;
	}

	/*
	** Put our named subobject selection sets into the drop down list
	*/
	Create_Named_Selection_Sets();
	
	/*
	** Notify our dependents that the subselection type, 
	** and the display have changed
	*/
	NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);

	/*
	** Notify the pipeline that the selection level has changed.
	*/
	InterfacePtr->PipeSelLevelChanged();

	/*
	** Notify our dependents that the selection channel, 
	** display attributes, and subselection type channels have changed
	*/
	NotifyDependents(FOREVER, SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
}

int SkinModifierClass::HitTest
(
	TimeValue		t, 
	INode *			inode,
	int				type, 
	int				crossing, 
	int				flags, 
	IPoint2 *		p, 
	ViewExp *		vpt, 
	ModContext *	mc
)
{
	Interval valid = FOREVER;
	int needsdel;
	int savedLimits;
	int res = 0;
	HitRegion hr;
	Matrix3 mat;

	MakeHitRegion(hr,type, crossing,4,p);
	mat = inode->GetObjectTM(t);

	/*
	** Set up the graphics window to do the hit test
	*/
	GraphicsWindow *gw = vpt->getGW();
	gw->setHitRegion(&hr);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	
	if (1 /*IgnoreBackfaces*/) {
		gw->setRndLimits(gw->getRndLimits() |  GW_BACKCULL);
	} else {
		gw->setRndLimits(gw->getRndLimits() & ~GW_BACKCULL);
	}

	gw->clearHitCode();

	/*
	** Do the hit test!
	*/
	SubObjHitList hitlist;
	MeshSubHitRec * rec;	
	
	ObjectState os = inode->EvalWorldState(InterfacePtr->GetTime());
	TriObject * tobj = Get_Tri_Object(InterfacePtr->GetTime(),os,valid,needsdel);
	res = tobj->mesh.SubObjectHitTest(gw,gw->getMaterial(),&hr,flags | SUBHIT_VERTS,hitlist);
	
	/*
	** Record all of the hits
	*/
	rec = hitlist.First();
	while (rec) 
	{
		/*
		** rec->index is the index of vertex which was hit!
		** Remember that we are always turning on vertex hit testing;
		** if we were testing for edges, index would be the edge index.
		*/
		vpt->LogHit(inode,mc,rec->dist,rec->index,NULL);
		rec = rec->Next();
	}

	/*
	** Cleanup
	*/
	gw->setRndLimits(savedLimits);	
	
	if (needsdel) {
		tobj->DeleteThis();
	}

	return res;	
}

void SkinModifierClass::SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert)
{
	SkinDataClass * skindata = NULL;
	int count = 0;

	switch (SubObjSelLevel) {
		
		case VERTEX_SEL_LEVEL:

			while (hitRec) {				

				skindata = (SkinDataClass *)hitRec->modContext->localData;				

				/*
				** Undo/Redo functionality
				*/
#if 0
				if (theHold.Holding() && !SelData->held) {
					theHold.Put(new SubSelRestore(this,SelData));
				}
				theHold.Accept(_T("Select Vertex"));
#endif
				
				BitArray * array = &(skindata->VertSel);

				if (all & invert) {

					/*
					** hitRec->hitInfo is the MeshSubHitRec::index that was stored in the
					** HitTest method through LogHit
					*/
					if ((*array)[hitRec->hitInfo]) {
						array->Clear(hitRec->hitInfo);
					} else {
						array->Set(hitRec->hitInfo,selected);
					}
				} else {
					array->Set(hitRec->hitInfo,selected);
				}
				
				if (!all) break;
				hitRec = hitRec->Next();
			}
			break;
	}

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

void SkinModifierClass::ClearSelection(int selLevel)
{
	int needsdel = 0;
	Interval valid = FOREVER;
	ModContextList mcList;
	INodeTab nodes;

	if (!InterfacePtr ) return;	
	
	InterfacePtr->GetModContexts(mcList,nodes);
	InterfacePtr->ClearCurNamedSelSet();

	for (int i = 0; i < mcList.Count(); i++) {

		SkinDataClass * skindata = (SkinDataClass *)mcList[i]->localData;
		
		if (skindata==NULL) continue;		
		
		ObjectState os = nodes[i]->EvalWorldState(InterfacePtr->GetTime());
		TriObject * tobj = Get_Tri_Object(InterfacePtr->GetTime(),os,valid,needsdel);

		switch (SubObjSelLevel) {
#if 0
			case OBJECT_SEL_LEVEL:
				assert(0);
				return;
#endif

			case VERTEX_SEL_LEVEL:	
#if 0 // undo/redo
				if (theHold.Holding()) {
					theHold.Put(new VertexSelRestore(meshData,this));
				}
#endif
				tobj->mesh.vertSel.ClearAll();
				skindata->VertSel.ClearAll();
				break;
		}

		if (needsdel) {
			tobj->DeleteThis();
		}
	}
	
	/*
	** Get rid of the temporary copies of the INodes.
	*/
	nodes.DisposeTemporary();

	/*
	** Tell our dependents that the selection set has changed
	*/
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

}

void SkinModifierClass::SelectAll(int selLevel)
{
	int needsdel = 0;
	Interval valid = FOREVER;
	ModContextList mclist;
	INodeTab nodes;

	if (!InterfacePtr) return;	
	
	InterfacePtr->GetModContexts(mclist,nodes);
	InterfacePtr->ClearCurNamedSelSet();

	for (int i = 0; i < mclist.Count(); i++) {

		SkinDataClass * skindata = (SkinDataClass *)mclist[i]->localData;
		
		if (skindata==NULL) continue;		
		
		ObjectState os = nodes[i]->EvalWorldState(InterfacePtr->GetTime());
		TriObject * tobj = Get_Tri_Object(InterfacePtr->GetTime(),os,valid,needsdel);

		switch (SubObjSelLevel) {

			case OBJECT_SEL_LEVEL:
				assert(0);
				return;

			case VERTEX_SEL_LEVEL:	
#if 0 // undo/redo
				if (theHold.Holding()) {
					theHold.Put(new VertexSelRestore(meshData,this));
				}
#endif
				tobj->mesh.vertSel.SetAll();
				skindata->VertSel.SetAll();
				break;
		}

		if (needsdel) {
			tobj->DeleteThis();
		}
	}
	
	/*
	** Get rid of the temporary copies of the INodes.
	*/
	nodes.DisposeTemporary();

	/*
	** Tell our dependents that the selection set has changed
	*/
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
}

void SkinModifierClass::InvertSelection(int selLevel)
{
	int needsdel = 0;
	Interval valid = FOREVER;
	ModContextList mclist;
	INodeTab nodes;

	if (!InterfacePtr) return;	
	
	InterfacePtr->GetModContexts(mclist,nodes);
	InterfacePtr->ClearCurNamedSelSet();

	for (int i = 0; i < mclist.Count(); i++) {

		SkinDataClass * skindata = (SkinDataClass *)mclist[i]->localData;
		
		if (skindata==NULL) continue;		
		
		ObjectState os = nodes[i]->EvalWorldState(InterfacePtr->GetTime());
		TriObject * tobj = Get_Tri_Object(InterfacePtr->GetTime(),os,valid,needsdel);

		switch (SubObjSelLevel) {

			case OBJECT_SEL_LEVEL:
				assert(0);
				return;

			case VERTEX_SEL_LEVEL:	
#if 0 // undo/redo
				if (theHold.Holding()) {
					theHold.Put(new VertexSelRestore(meshData,this));
				}
#endif
				for (int j=0; j<tobj->mesh.vertSel.GetSize(); j++) {
					if (tobj->mesh.vertSel[j]) tobj->mesh.vertSel.Clear(j);
					else tobj->mesh.vertSel.Set(j);
				}				
				skindata->VertSel = tobj->mesh.vertSel;
				break;
		}

		if (needsdel) {
			tobj->DeleteThis();
		}
	}
	
	/*
	** Get rid of the temporary copies of the INodes.
	*/
	nodes.DisposeTemporary();

	/*
	** Tell our dependents that the selection set has changed
	*/
	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);

}

void SkinModifierClass::User_Picked_Bone(INode * node)
{
	assert(InterfacePtr != NULL);

	/*
	** Get a pointer to the ModContext and SkinData for
	** the mesh currently being messed with.
	*/
	ModContext * mc = NULL;
	ModContextList mclist;
	INodeTab nodelist;

	InterfacePtr->GetModContexts(mclist,nodelist);
	
	/*
	** This seems wrong... But I always get only one ModContext and
	** it is the one that I want so I'll just use it...  
	** I believe that OS Modifiers can get multiple ones but WS modifiers
	** don't
	*/
	mc = mclist[0];
	assert(mc != NULL);
	SkinDataClass * skindata = (SkinDataClass *)(mc->localData);

	/*
	** Add this bone to the influences of all selected vertices
	*/
	int boneidx = WSMObjectRef->Find_Bone(node);
	assert(boneidx != -1);
	skindata->Add_Influence(boneidx);

	/*
	** Recreate all of the named selection sets!
	*/
	Create_Named_Selection_Sets();

	/*
	** Update dependents and redraw the views.
	*/
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	InterfacePtr->RedrawViews(InterfacePtr->GetTime());
}
	
void SkinModifierClass::User_Picked_Bones(INodeTab & nodetab)
{
	/*
	** One by one, add the selected bones to the influences of
	** all selected vertices.  
	*/
	for (int i=0; i<nodetab.Count() && i<2; i++) {
		User_Picked_Bone(nodetab[i]);
	}
}

void SkinModifierClass::ActivateSubSelSet(TSTR & setname)
{
	ModContextList mclist;
	INodeTab nodes;
	
	if (InterfacePtr == NULL) return;

	InterfacePtr->GetModContexts(mclist,nodes);

	for (int i = 0; i < mclist.Count(); i++) {

		SkinDataClass * skindata = (SkinDataClass *)mclist[i]->localData;
		if (!skindata) continue;		
	
		int index = skindata->VertSelSets.Find_Set(setname);
		if (index < 0) continue;

		int needsdel;
		Interval valid;
		ObjectState os = nodes[i]->EvalWorldState(InterfacePtr->GetTime());
		TriObject * tobj = Get_Tri_Object(InterfacePtr->GetTime(),os,valid,needsdel);
		Mesh * mesh = &(tobj->mesh);
		
		// TODO: undo redo
#if 0
		if (theHold.Holding()) {
			theHold.Put(new VertexSelRestore(meshData,this));
		}
#endif
				
		if (skindata->VertSelSets[index].GetSize() != mesh->getNumVerts()) {
			skindata->VertSelSets[index].SetSize(mesh->getNumVerts(),TRUE);
		}
		mesh->vertSel  = skindata->VertSelSets[index];
		skindata->VertSel = mesh->vertSel;
	
		if (needsdel) {
			tobj->DeleteThis();
		}
	}
	
	nodes.DisposeTemporary();	

	NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE);
	InterfacePtr->RedrawViews(InterfacePtr->GetTime());
}

void SkinModifierClass::NewSetFromCurSel(TSTR &setname)
{
	Install_Named_Selection_Sets();
}

void SkinModifierClass::RemoveSubSelSet(TSTR &setname)
{
	Install_Named_Selection_Sets();
}

void SkinModifierClass::Create_Named_Selection_Sets(void)
{
	/*
	** This function creates a named selection set of vertices
	** for each bone in the skeleton.  
	*/
	if (InterfacePtr == NULL) return;

	SkinWSMObjectClass * skinobj = WSMObjectRef;
	if (skinobj == NULL) return;

	ModContextList mclist;
	INodeTab nodes;
	InterfacePtr->GetModContexts(mclist,nodes);
	SkinDataClass * skindata = (SkinDataClass *)mclist[0]->localData;
	if (skindata == NULL) return;

	/*
	** Clear out the old selection sets
	*/
	skindata->VertSelSets.Reset();

	/*
	** Create and add a set for each bone
	*/
	for (int boneidx = 0; boneidx < skinobj->Num_Bones(); boneidx++) {

		if (skinobj->Get_Bone(boneidx) != NULL) {
			BitArray boneverts;
			boneverts.SetSize(skindata->VertData.Count());
			
			for (int vertidx = 0; vertidx < skindata->VertData.Count(); vertidx++) {
				if (skindata->VertData[vertidx].BoneIdx[0] == boneidx) boneverts.Set(vertidx);
				else boneverts.Clear(vertidx);
			}

			TSTR bonename = skinobj->Get_Bone(boneidx)->GetName();

			skindata->VertSelSets.Append_Set(boneverts,bonename);
		}
	}

	Install_Named_Selection_Sets();

	nodes.DisposeTemporary();	
}

void SkinModifierClass::Install_Named_Selection_Sets(void)
{
	/*
	** If we are in sub-object selection mode add the sets
	** to the drop down box.
	*/
	if ((SubObjSelLevel == VERTEX_SEL_LEVEL) && (InterfacePtr != NULL)) {

		ModContextList mclist;
		INodeTab nodes;
		InterfacePtr->GetModContexts(mclist,nodes);
		SkinDataClass * skindata = (SkinDataClass *)mclist[0]->localData;
		if (skindata == NULL) return;
		
		InterfacePtr->ClearSubObjectNamedSelSets();
		for (int i=0; i < skindata->VertSelSets.Count(); i++) {
			InterfacePtr->AppendSubObjectNamedSelSet(*skindata->VertSelSets.Names[i]);
		}

		nodes.DisposeTemporary();
	}	
}

void SkinModifierClass::Auto_Attach_Verts(BOOL all)
{
	assert(InterfacePtr);

	/*
	** Get the skin data. 
	*/
	ModContextList mclist;
	INodeTab nodes;
	InterfacePtr->GetModContexts(mclist,nodes);
	SkinDataClass * skindata = (SkinDataClass *)mclist[0]->localData;
	if (skindata == NULL) return;
	
	/*
	** get the skin WSM object.
	*/
	SkinWSMObjectClass * skinobj = WSMObjectRef;
	if (skinobj == NULL) return;

	/*
	** Get a triobject representing the object state in the base pose.
	*/
	Interval valid;
	BOOL needsdel;

	TimeValue basetime = WSMObjectRef->Get_Base_Pose_Time();
	ObjectState os = nodes[0]->EvalWorldState(basetime);
	TriObject * triobj = Get_Tri_Object(basetime,os,valid,needsdel);
		
	/*
	** Attach each selected vertex (or all of them) to their closest bone.
	*/
	for (int vertidx = 0; vertidx < skindata->VertData.Count(); vertidx++){
		if (skindata->VertSel[vertidx] || all) {
			
			Point3 vert = triobj->GetPoint(vertidx);
			if (os.GetTM()) vert = vert * (*os.GetTM());
			int boneidx = skinobj->Find_Closest_Bone(vert);
			skindata->VertData[vertidx].Set_Influence(boneidx);
		}
	}

	/*
	** Re-create the named selection sets
	*/
	Create_Named_Selection_Sets();

	/*
	** Update dependents and redraw the views.
	*/
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	InterfacePtr->RedrawViews(InterfacePtr->GetTime());

	/*
	** Cleanup...
	*/
	nodes.DisposeTemporary();
	
	if (needsdel) {
		triobj->DeleteThis();
	}
}

void SkinModifierClass::Unlink_Verts(void)
{
	assert(InterfacePtr);

	/*
	** Get the skin data. 
	*/
	ModContextList mclist;
	INodeTab nodes;
	InterfacePtr->GetModContexts(mclist,nodes);
	SkinDataClass * skindata = (SkinDataClass *)mclist[0]->localData;
	if (skindata == NULL) return;
	
	/*
	** Unlink each selected vertex (give them bone index -1)
	*/
	for (int vertidx = 0; vertidx < skindata->VertData.Count(); vertidx++){
		if (skindata->VertSel[vertidx]) {
			skindata->VertData[vertidx].Set_Influence(-1);
		}
	}

	/*
	** Re-create the named selection sets
	*/
	Create_Named_Selection_Sets();

	/*
	** Update dependents and redraw the views.
	*/
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	InterfacePtr->RedrawViews(InterfacePtr->GetTime());

	/*
	** Cleanup...
	*/
	nodes.DisposeTemporary();
}


/****************************************************************************
**
**    DIALOG BOX JUNK
**
****************************************************************************/

void SkinModifierClass::Install_Bone_Influence_Dialog(void)
{
	if (BoneInfluenceHWND != NULL) return;

	/*
	** loading resource string for the name of the dialog
	*/
	static int loaded = 0;
	static TCHAR string[MAX_STRING_LENGTH];

	if (!loaded) {
		LoadString(AppInstance,IDS_BONE_INFLUENCE_PARAMS,string,MAX_STRING_LENGTH);
		loaded = 1;
	}

	/*
	** Put up the UI that is used to assign vertices to bones
	*/
	BoneInfluenceHWND = InterfacePtr->AddRollupPage( 
				AppInstance,
				MAKEINTRESOURCE(IDD_BONE_INFLUENCE_PARAMS),
				_bone_influence_dialog_thunk,
				string, 
				(LPARAM)this,
				0);
}

void SkinModifierClass::Remove_Bone_Influence_Dialog(void)
{
	/*
	** If it is currently up, remove the bone influences dialog
	*/
	if (BoneInfluenceHWND != NULL) {
		InterfacePtr->UnRegisterDlgWnd(BoneInfluenceHWND);
		InterfacePtr->DeleteRollupPage(BoneInfluenceHWND);
		BoneInfluenceHWND = NULL;
	}
}

/*********************************************************************************
*
* _sot_dialog_proc
*
*********************************************************************************/
static BOOL CALLBACK _sot_dialog_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	IObjParam *ip = (IObjParam*)GetWindowLong(hWnd,GWL_USERDATA);

	switch (message) {
		case WM_INITDIALOG:
			SetWindowLong(hWnd,GWL_USERDATA,lParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			if (ip) ip->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;

		default:
			return FALSE;
	}
	return TRUE;
}


/*********************************************************************************
*
* _skeleton_dialog_proc
*
*********************************************************************************/
static BOOL CALLBACK _skeleton_dialog_thunk(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SkinWSMObjectClass * skinobj = (SkinWSMObjectClass *)GetWindowLong(hWnd,GWL_USERDATA);
	if (!skinobj && message != WM_INITDIALOG) return FALSE;
	
	if (message == WM_INITDIALOG) {
		skinobj = (SkinWSMObjectClass *)lParam;
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)skinobj);			
	}

	return skinobj->Skeleton_Dialog_Proc(hWnd,message,wParam,lParam);
}

BOOL SkinWSMObjectClass::Skeleton_Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) {

		case WM_INITDIALOG:

			BoneListHWND = GetDlgItem(hWnd,IDC_BONE_LIST);

			/*
			** Intitialize the add bone and remove bone check buttons
			*/
			AddBonesButton = GetICustButton(GetDlgItem(hWnd, IDC_ADD_BONES_BUTTON));
			RemoveBonesButton = GetICustButton(GetDlgItem(hWnd, IDC_REMOVE_BONES_BUTTON));
			
			AddBonesButton->SetType(CBT_CHECK);
			AddBonesButton->SetHighlightColor(GREEN_WASH);
			AddBonesButton->SetTooltip(TRUE, _T("Add bones by name"));

			RemoveBonesButton->SetType(CBT_CHECK);
			RemoveBonesButton->SetHighlightColor(GREEN_WASH);
			RemoveBonesButton->SetTooltip(TRUE, _T("Remove bones by name"));
	
			/*
			** Initialize the "Base Pose Frame" spinner
			*/
			BasePoseSpin = GetISpinner(GetDlgItem(hWnd, IDC_BASE_POSE_SPIN));
			BasePoseSpin->SetLimits(0,9999, FALSE);
			BasePoseSpin->SetValue(0,FALSE);
			BasePoseSpin->SetResetValue(0);
			BasePoseSpin->LinkToEdit(GetDlgItem(hWnd,IDC_BASE_POSE_EDIT),EDITTYPE_INT);
			return TRUE;

		case WM_DESTROY:
			ReleaseICustButton(AddBonesButton);
			ReleaseICustButton(RemoveBonesButton);
			ReleaseISpinner(BasePoseSpin);
			
			AddBonesButton = NULL;
			RemoveBonesButton = NULL;
			BasePoseSpin = NULL;
			BoneListHWND = NULL;
			
			return FALSE;
	
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam)) 
			{
				case IDC_BASE_POSE_SPIN:
					BasePoseFrame = BasePoseSpin->GetIVal();						
					break;
			}
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			InterfacePtr->RedrawViews(InterfacePtr->GetTime(),REDRAW_INTERACTIVE);
			return TRUE;

		case CC_SPINNER_BUTTONUP:
			NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			InterfacePtr->RedrawViews(InterfacePtr->GetTime(),REDRAW_END);
			return TRUE;

		
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			InterfacePtr->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{

				case IDC_ADD_BONES_BUTTON:
					TheBonePicker.Set_User(this);
					Set_Bone_Selection_Mode(BONE_SEL_MODE_ADD_MANY);
					InterfacePtr->DoHitByNameDialog(&TheBonePicker);
					Set_Bone_Selection_Mode(BONE_SEL_MODE_NONE);
					break;

				case IDC_REMOVE_BONES_BUTTON:
					TheBonePicker.Set_User(this,FALSE,&(BoneTab));
					Set_Bone_Selection_Mode(BONE_SEL_MODE_REMOVE_MANY);
					InterfacePtr->DoHitByNameDialog(&TheBonePicker);
					Set_Bone_Selection_Mode(BONE_SEL_MODE_NONE);
					break;
			}

		default:
			return FALSE;
	}
}

/*********************************************************************************
*
* Bone_Influence_Dialog_Proc
*
*********************************************************************************/
static BOOL CALLBACK _bone_influence_dialog_thunk(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SkinModifierClass * skinmod = (SkinModifierClass *)GetWindowLong(hWnd,GWL_USERDATA);
	if (!skinmod && message != WM_INITDIALOG) return FALSE;
	
	if (message == WM_INITDIALOG) {
			skinmod = (SkinModifierClass *)lParam;
			SetWindowLong(hWnd,GWL_USERDATA,(LONG)skinmod);			
	}

	return skinmod->Bone_Influence_Dialog_Proc(hWnd,message,wParam,lParam);
}


BOOL SkinModifierClass::Bone_Influence_Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message) {

		case WM_INITDIALOG:
			/*
			** Intitialize the bone influence buttons
			*/
			LinkButton = GetICustButton(GetDlgItem(hWnd, IDC_LINK_BUTTON));
			LinkByNameButton = GetICustButton(GetDlgItem(hWnd, IDC_LINK_BY_NAME_BUTTON));
			AutoLinkButton = GetICustButton(GetDlgItem(hWnd, IDC_AUTO_LINK_BUTTON));
			UnLinkButton = GetICustButton(GetDlgItem(hWnd, IDC_UNLINK_BUTTON));
			
			LinkButton->SetType(CBT_PUSH);
			LinkButton->SetTooltip(TRUE, _T("Link Vertices to a bone by selecting the bone"));

			LinkByNameButton->SetType(CBT_PUSH);
			LinkByNameButton->SetTooltip(TRUE, _T("Link Vertices to a bone by name"));
			
			AutoLinkButton->SetType(CBT_PUSH);
			AutoLinkButton->SetTooltip(TRUE, _T("Link Vertices to nearest bone"));
			
			UnLinkButton->SetType(CBT_PUSH);
			UnLinkButton->SetTooltip(TRUE, _T("Unlink selected vertices"));
			
			return TRUE;

		case WM_DESTROY:
			ReleaseICustButton(LinkButton);
			ReleaseICustButton(LinkByNameButton);
			ReleaseICustButton(AutoLinkButton);
			ReleaseICustButton(UnLinkButton);

			LinkButton = NULL;
			LinkByNameButton = NULL;
			AutoLinkButton = NULL;
			UnLinkButton = NULL;
			return FALSE;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			InterfacePtr->RollupMouseMessage(hWnd,message,wParam,lParam);
			return FALSE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_LINK_BUTTON:
				{
					/*
					** user picks a bone out of the scene to link to.
					*/
					assert(WSMObjectRef != NULL);
					INodeTab * bonetab = &(WSMObjectRef->Get_Bone_List());
					TheBonePicker.Set_User(this,TRUE,bonetab);
					InterfacePtr->SetPickMode(&TheBonePicker); 
					break;
				}

				case IDC_LINK_BY_NAME_BUTTON:
				{
					/*
					** pop up a bone selection dialog
					*/
					assert(WSMObjectRef != NULL);
					INodeTab * bonetab = &(WSMObjectRef->Get_Bone_List());
					TheBonePicker.Set_User(this,TRUE,bonetab);
					InterfacePtr->DoHitByNameDialog(&TheBonePicker);
					break;
				}

				case IDC_AUTO_LINK_BUTTON:
				{
					Auto_Attach_Verts();					
					break;
				}
				
				case IDC_UNLINK_BUTTON:
				{
					Unlink_Verts();
					break;
				}
			}

		default:
			return FALSE;
	}
}

static TriObject * Get_Tri_Object(TimeValue t,ObjectState & os,Interval & valid,BOOL & needsdel)
{	
	needsdel = FALSE;
	valid &= os.Validity(t);
	
	if (os.obj->IsSubClassOf(triObjectClassID)) {
		return (TriObject *)os.obj;
	} else {
		if (os.obj->CanConvertToType(triObjectClassID)) {
			Object * oldObj = os.obj;
			TriObject * tobj = (TriObject *)os.obj->ConvertToType(t,triObjectClassID);			
			needsdel = (tobj != oldObj);			
			return tobj;
		}
	}
	return NULL;
}


float Bone_Distance(INode * bone,TimeValue time,const Point3 & vertex)
{
	/*
	** Average the pivot point of this bone with the pivot points of
	** all of its children.
	*/
	Point3 icenter = bone->GetObjectTM(time).GetTrans();

	for (int ci=0; ci<bone->NumberOfChildren(); ci++) {
		icenter += bone->GetChildNode(ci)->GetObjectTM(time).GetTrans();
	}

	icenter = icenter / (float)(bone->NumberOfChildren() + 1);

	return Length(icenter - vertex);
}

#if defined W3D_MAX4		//defined as in the project (.dsp)

int SkinModifierClass::NumSubObjTypes()
{
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////
ISubObjType *SkinModifierClass::GetSubObjType(int i) 
{

	static bool _initialized = false;
	if(!_initialized){
		_initialized = true;
		_SubObjectTypeVertex.SetName("Vertices");
	}
	if(i == -1){
		if(GetSubObjectLevel() > 0){
			return GetSubObjType(GetSubObjectLevel()-1);
		}
	}
	return &_SubObjectTypeVertex;
}
#endif
