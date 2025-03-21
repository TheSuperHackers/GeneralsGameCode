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
 *                 Project Name : Commando Tools - WWSkin                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/skin.h                         $*
 *                                                                                             *
 *                      $Author:: Moumine_ballo                                               $*
 *                                                                                             *
 *                     $Modtime:: 4/18/01 11:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef SKIN_H
#define SKIN_H

#include <max.h>
#include "simpmod.h"
#include "simpobj.h"
#include "bpick.h"
#include "namedsel.h"
#include "w3d_file.h"


#define SKIN_OBJ_CLASS_ID		Class_ID(0x32b37e0c, 0x5a9612e4)		
#define SKIN_MOD_CLASS_ID 		Class_ID(0x6bad4898, 0x0d1d6ced)
extern ClassDesc * Get_Skin_Obj_Desc();
extern ClassDesc * Get_Skin_Mod_Desc();


/*

	Writing a space warp plug-in involves creating instances of two key classes.  
	One is derived from class WSMObject.  (WSMObject stands for Word Space Modifier Object,
	just another name for Space Warp Object).  The other class is subclassed off Modifier.
	These two classes work together.  The space warp object handles the display and management 
	of its user interface parameters, the display of the space warp node in the scene, and 
	provides a world space orientation.  The space warp modifier handles the actual deformation
	of the geometry of nodes bound to the space warp.  Each node bound to the space warp
	will have a ModContext which we will store data in.

	The following class is the WSMObject for the westwood skin modifier.

*/
class SkinWSMObjectClass : public SimpleWSMObject, BonePickerUserClass
{	
public:		

	SkinWSMObjectClass();		
	virtual ~SkinWSMObjectClass();

	/*
	** From Animatable		
	*/
	void DeleteThis() { delete this; }		
	void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
	TCHAR * GetObjectName() { return _T("WWSkin"); }		
	Class_ID ClassID() { return SKIN_OBJ_CLASS_ID; }		
				
	/*
	** From ReferenceTarget
	*/
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	/*
	** From Reference Maker.  These three functions give access to the "virtual array" of references.
	** For SkinWSMObjectClass, we have to remember that SimpleWSMObject already has a reference
	** so we are taking ours on after
	*/
	virtual int NumRefs() { return SimpleWSMObject::NumRefs() + Num_Bones();}
	virtual RefTargetHandle GetReference(int i);
	virtual void SetReference(int i, RefTargetHandle rtarg);
	RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);

	/*
	** From Object		
	*/
	int DoOwnSelectHilite() { return TRUE; }
	CreateMouseCallBack * GetCreateMouseCallBack();

	/*
	** From WSMObject
	*/
	Modifier *CreateWSMMod(INode *node);

	/*
	** From SimpleWSMObject		
	*/
	void BuildMesh(TimeValue t);

	/*
	** Setup a triangle
	*/
	void Build_Tri(Face * f, int a,  int b, int c);
	
	/*
	** Dialog box message processing
	*/
	BOOL SkinWSMObjectClass::Skeleton_Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
	
	/*
	** Bone picking.
	*/
	virtual void User_Picked_Bone(INode * node);
	virtual void User_Picked_Bones(INodeTab & nodetab);
	void Set_Bone_Selection_Mode(int mode);
	
	int  Add_Bone(INode * node);
	void Add_Bones(INodeTab & nodetab);
	void Remove_Bone(INode * node);
	void Remove_Bones(INodeTab & nodetab);
	void Update_Bone_List(void);

	/*
	** Converting between bone indexes and reference indexes
	** The bone references are a variable number of references which are
	** added at the end of the reference array.  
	*/
	int To_Bone_Index(int refidx) { return refidx - SimpleWSMObject::NumRefs(); }
	int To_Ref_Index(int boneidx) { return SimpleWSMObject::NumRefs() + boneidx; }
	
	/*
	** External access to the bones
	*/
	int Num_Bones(void) { return BoneTab.Count(); }
	INode * Get_Bone(int idx) { return BoneTab[idx]; }
	INodeTab & Get_Bone_List(void) { return BoneTab; }
	int Find_Bone(INode * node);
	int Get_Base_Pose_Frame(void) { return BasePoseFrame; }
	int Get_Base_Pose_Time(void) { return BasePoseFrame * GetTicksPerFrame(); }
	int Find_Closest_Bone(const Point3 & vertex);

	/*
	** Saving and loading.
	*/
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

	/*
	** Static UI variables.  These have to be static due to the strange way
	** that MAX behaves during creation of objects.  If you create an object
	** then delete it, EndEditParams is not called and its destructor isn't called...
	*/
	static HWND						SotHWND;
	static HWND						SkeletonHWND;
	static HWND						BoneListHWND;
	static IObjParam *			InterfacePtr;
	static ICustButton *			AddBonesButton;
	static ICustButton *			RemoveBonesButton;
	static ISpinnerControl *	BasePoseSpin;

	/*
	** flag for whether we need to build the bones mesh for this object
	*/
	BOOL				MeshBuilt;					

	/*
	** Bone Selection!
	*/
	enum {
		BONE_SEL_MODE_NONE = 0,
		BONE_SEL_MODE_ADD,
		BONE_SEL_MODE_REMOVE,
		BONE_SEL_MODE_ADD_MANY,
		BONE_SEL_MODE_REMOVE_MANY
	};

	int					BoneSelectionMode;
	INodeTab				BoneTab;	

	/*
	** Dialog controls
	*/
	int					BasePoseFrame;

	/*
	** Chunk ID's
	*/
	enum {
		NUM_BONES_CHUNK = 0x0001
	};
	
	/*
	** Friend functions
	*/
	friend BOOL CALLBACK _skeleton_dialog_thunk(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
};

/*
** SkinModifierClass
*/
class SkinModifierClass : public Modifier, BonePickerUserClass
{

public:		

	SkinModifierClass(void);
	SkinModifierClass(INode * node,SkinWSMObjectClass * skin_obj);
#if defined W3D_MAX4		//defined as in the project (.dsp)
	ISubObjType *GetSubObjType(int i);
	int NumSubObjTypes();
#endif
	void								Default_Init(void);

	/*
	** From Animatable
	*/
	void								DeleteThis() { delete this; }
	void								GetClassName(TSTR& s) { s = TSTR(_T("WWSkin")); }
	TCHAR *							GetObjectName() { return _T("WWSkin Binding"); }
	SClass_ID						SuperClassID() { return WSM_CLASS_ID; }		
	Class_ID							ClassID() { return SKIN_MOD_CLASS_ID; } 		
	RefTargetHandle				Clone(RemapDir& remap = NoRemap());
	RefResult						NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
	void								BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
	void								EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);
	CreateMouseCallBack *		GetCreateMouseCallBack() { return NULL; }

	/*
	** From Reference Maker.  These three functions give access to the "virtual array" of references.
	*/
	int								NumRefs() { return 2; }
	RefTargetHandle				GetReference(int i);
	void								SetReference(int i, RefTargetHandle rtarg);

	/*
	** Tell MAX what channels we use and what channels we change:
	** Note that if we do not tell max that we use a channel, that channel is not
	** guaranteed to be valid.
	*/
	virtual ChannelMask ChannelsUsed() { return SELECT_CHANNEL|SUBSEL_TYPE_CHANNEL|GEOM_CHANNEL; }
	virtual ChannelMask ChannelsChanged() { return SELECT_CHANNEL|SUBSEL_TYPE_CHANNEL|GEOM_CHANNEL; }

	/*
	** MAX tells us whenever an input changed.  If we cache anything, we can use this
	** function to dump the cached data and regenerate it.
	*/
	virtual void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {}               

	/*
	** This is where the modifier actually modifies the object!
	*/
	virtual void ModifyObject(TimeValue t, ModContext & mc, ObjectState * os, INode * node);

	/*
	** Since our modifier will be storing information based on the vertex indices, whenever
	** the topology of its input is changed things will no longer work correctly.  Therefore,
	** we tell max that we depend on the topology remaining the same.
	*/
	virtual BOOL DependOnTopology(ModContext &mc) { return TRUE; }

	/*
	** What types of objects can we modify:  The skin modifier will only work with TRIOBJ's
	*/
	virtual Class_ID InputType() { return Class_ID(TRIOBJ_CLASS_ID,0); }

	/*
	** Saving and loading.  Remember to call the base class's save and load functions as well.
	*/
	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);
	virtual IOResult LoadLocalData(ILoad *iload, LocalModData **pld);
	virtual IOResult SaveLocalData(ISave *isave, LocalModData *ld);
	
	/*
	** For SkinModifierClass, we allow vertex sub-object selection.
	** This function notifies an object being edited that the current sub object
	** selection level has changed. level==0 indicates object level selection.
	** level==1 or greater refer to the types registered by the object in the
	** order they appeared in the list when registered. If level >= 1, the object
	** should specify sub-object xform modes in the modes structure (defined in cmdmode.h).
	*/
	void ActivateSubobjSel(int level, XFormModes& modes);

	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
	void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
	void ClearSelection(int selLevel);//
	void SelectAll(int selLevel);
	void InvertSelection(int selLevel);

	/*
	** An object that supports sub-object selection can choose to
	** support named sub object selection sets. Methods in the the
	** interface passed to objects allow them to add items to the
	** sub-object selection set drop down.
	** The following methods implement named sub-obj selection sets 
	*/
	virtual BOOL SupportsNamedSubSels() { return TRUE; }
	virtual void ActivateSubSelSet(TSTR &setName);
	virtual void NewSetFromCurSel(TSTR &setName);
	virtual void RemoveSubSelSet(TSTR &setName);
	void Create_Named_Selection_Sets(void);
	void Install_Named_Selection_Sets(void);

	WSMObject * Get_WSMObject(void) { return (WSMObject*)GetReference(OBJ_REF); }
	Interval Get_Validity(TimeValue t);
	
	/*
	** Bone picking
	*/
	virtual void User_Picked_Bone(INode * node);
	virtual void User_Picked_Bones(INodeTab & nodetab);

	/*
	** Auto-Attach vertices to nearest bone
	*/
	void Auto_Attach_Verts(BOOL all = FALSE);					
	
	/*
	** Unlink selected verts (links them to the root or origin)
	*/
	void Unlink_Verts(void);

private:
	
	/*
	** Windows dialog management and communication functions
	*/	
	void Install_Bone_Influence_Dialog(void);
	void Remove_Bone_Influence_Dialog(void);

	BOOL Bone_Influence_Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
	
public:

	/*
	** References for SkinModifierClass
	*/
	enum {
		OBJ_REF = 0,
		NODE_REF = 1
	};

	SkinWSMObjectClass *		WSMObjectRef;
	INode *						WSMNodeRef;

	/*
	** Sub-Object Selection variables
	*/
	enum {
		OBJECT_SEL_LEVEL = 0,
		VERTEX_SEL_LEVEL = 1
	};

	int SubObjSelLevel;


	/*
	** Bone Influence Dialog panel variables
	*/
	HWND BoneInfluenceHWND;
	ICustButton *		LinkButton;
	ICustButton *		LinkByNameButton;
	ICustButton *		AutoLinkButton;
	ICustButton *		UnLinkButton;
	
	/*
	**  Cached pointers to some MAX objects
	*/
	IObjParam * InterfacePtr;
	SelectModBoxCMode * SelectMode;		

	/*
	** Load/Save Chunk ID's
	*/
	enum {
		SEL_LEVEL_CHUNK = 0xAA01,
	};

	/*
	** Friend "thunking" functions for the dialog handling.
	*/
	friend BOOL CALLBACK _bone_influence_dialog_thunk(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
};




#endif

