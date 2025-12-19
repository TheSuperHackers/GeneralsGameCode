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

/* $Header: /Commando/Code/Tools/max2w3d/MeshDeform.cpp 7     5/01/01 8:56p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : MeshDeform.cpp                                               *
 *                                                                                             *
 *                   Programmer : Patrick Smith                                                *
 *                                                                                             *
 *                   Start Date : 04/19/99                                                     *
 *                                                                                             *
 *                  Last Update :
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "MeshDeform.h"
#include "MeshDeformData.h"
#include "MeshDeformPanel.h"
#include "MeshDeformUndo.h"
#include "dllmain.h"
#include "resource.h"
#include "util.h"

#if defined W3D_MAX4		//defined as in the project (.dsp)
static GenSubObjType _SubObjectTypeVertex(1);
#endif

///////////////////////////////////////////////////////////////////////////
//
//	MeshDeformClass Class ID
//
///////////////////////////////////////////////////////////////////////////
Class_ID _MeshDeformClassID(0x51981f5b, 0x1db2bf3);


///////////////////////////////////////////////////////////////////////////
//
//	MeshDeformClassDesc
//
///////////////////////////////////////////////////////////////////////////
class MeshDeformClassDesc : public ClassDesc
{
	public:
	int 				IsPublic (void)			{ return 1; }
	void *			Create (BOOL loading)	{ return new MeshDeformClass (); }
	const TCHAR *	ClassName ()				{ return _T("WWDeform"); }
	SClass_ID		SuperClassID ()			{ return OSM_CLASS_ID; }
	Class_ID 		ClassID ()			  		{ return _MeshDeformClassID; }
	const TCHAR* 	Category ()					{ return _T("Westwood Modifiers"); }
};


///////////////////////////////////////////////////////////////////////////
//
//	Static class desc instance
//
///////////////////////////////////////////////////////////////////////////
#if 0 // (gth) MeshDeform is obsolete! making sure nobody uses it...
static MeshDeformClassDesc _MeshDeformCD;
ClassDesc * Get_Mesh_Deform_Desc (void) { return &_MeshDeformCD; }
#else
ClassDesc * Get_Mesh_Deform_Desc (void) { return nullptr; }
#endif


///////////////////////////////////////////////////////////////////////////
//
//	ChannelsUsed
//
///////////////////////////////////////////////////////////////////////////
ChannelMask
MeshDeformClass::ChannelsUsed (void)
{
	return GEOM_CHANNEL | SELECT_CHANNEL | SUBSEL_TYPE_CHANNEL | VERTCOLOR_CHANNEL;
}


///////////////////////////////////////////////////////////////////////////
//
//	ChannelsChanged
//
///////////////////////////////////////////////////////////////////////////
ChannelMask
MeshDeformClass::ChannelsChanged (void)
{
	return GEOM_CHANNEL | SELECT_CHANNEL | SUBSEL_TYPE_CHANNEL | VERTCOLOR_CHANNEL;
}


///////////////////////////////////////////////////////////////////////////
//
//	ModifyObject
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::ModifyObject
(
	TimeValue		time,
	ModContext &	mod_context,
	ObjectState *	object_state,
	INode *			/*node*/
)
{
	assert(object_state->obj->IsSubClassOf(triObjectClassID));


	MeshDeformModData *mod_data = nullptr;
	if (mod_context.localData == nullptr) {
		mod_data = new MeshDeformModData;
		mod_context.localData = mod_data;
	} else {
		mod_data = static_cast <MeshDeformModData *> (mod_context.localData);
	}

	// Display the verts
	TriObject *tri_obj = (TriObject *)object_state->obj;
	tri_obj->mesh.SetDispFlag (DISP_SELVERTS | DISP_VERTTICKS);

	// Record the initial state of the mesh
	bool lock_sets = false;
	if (m_pPanel != nullptr) {
		lock_sets = (m_pPanel->Are_Sets_Tied () == TRUE);
	}
	mod_data->Record_Mesh_State (*tri_obj, m_DeformState, lock_sets);

	tri_obj->PointsWereChanged();

	// Kind of a waste when there's no animation...
	tri_obj->UpdateValidity (GEOM_CHAN_NUM, Interval (time, time + 1));
	tri_obj->UpdateValidity (SELECT_CHAN_NUM, Interval (time, time + 1));
	tri_obj->UpdateValidity (SUBSEL_TYPE_CHAN_NUM, Interval (time, time + 1));
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	InputType
//
///////////////////////////////////////////////////////////////////////////
Class_ID
MeshDeformClass::InputType (void)
{
	return triObjectClassID;
}


///////////////////////////////////////////////////////////////////////////
//
//	NotifyRefChanged
//
///////////////////////////////////////////////////////////////////////////
RefResult
MeshDeformClass::NotifyRefChanged
(
	Interval time,
	RefTargetHandle htarget,
	PartID &part_id,
	RefMessage mesage
)
{
	return REF_SUCCEED;
}


///////////////////////////////////////////////////////////////////////////
//
//	GetCreateMouseCallBack
//
///////////////////////////////////////////////////////////////////////////
CreateMouseCallBack *
MeshDeformClass::GetCreateMouseCallBack (void)
{
	return nullptr;
}


///////////////////////////////////////////////////////////////////////////
//
//	BeginEditParams
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::BeginEditParams
(
	IObjParam *max_interface,
	ULONG flags,
	Animatable *prev
)
{
	m_MaxInterface = max_interface;
	Update_Set_Count ();
	Set_Max_Deform_Sets (m_MaxSets);

	// Add our rollup to the command panel
	m_hRollupWnd = m_MaxInterface->AddRollupPage (AppInstance,
																MAKEINTRESOURCE (IDD_MESH_DEFORM_PANEL),
																MeshDeformPanelClass::Message_Proc,
																"Westwood Mesh Deform",
																0,
																0);

	//
	//	Update the UI
	//
	m_pPanel = MeshDeformPanelClass::Get_Object (m_hRollupWnd);
	m_pPanel->Set_Deformer (this);
	m_pPanel->Set_Max_Sets (m_MaxSets);
	m_pPanel->Set_Current_Set (m_CurrentSet);
	Set_Current_Set (m_CurrentSet, false);

	//
	// Register the desired sub-object selection types.
	//
	const TCHAR * ptype[] = { "Vertices" };
#if defined W3D_MAX4		//defined as in the project (.dsp)
 	max_interface->SetSubObjectLevel(1);
#else
	//---This call is obsolete from max4.
	max_interface->RegisterSubObjectTypes( ptype, 1);
#endif

	//
	//	Create the mode handlers
	//
	m_ModeSelect	= new SelectModBoxCMode (this, max_interface);
	m_ModeMove		= new MoveModBoxCMode (this, max_interface);
	m_ModeRotate	= new RotateModBoxCMode (this, max_interface);
	m_ModeUScale	= new UScaleModBoxCMode (this, max_interface);
	m_ModeNUScale	= new NUScaleModBoxCMode (this, max_interface);
	m_ModeSquash	= new SquashModBoxCMode (this, max_interface);

	//
	// Restore the selection level.
	///
	max_interface->SetSubObjectLevel (1);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	EndEditParams
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::EndEditParams
(
	IObjParam *max_interface,
	ULONG flags,
	Animatable *next
)
{
	// Remove our deform rollup
	if (m_hRollupWnd != nullptr) {
		max_interface->DeleteRollupPage (m_hRollupWnd);
		m_hRollupWnd = nullptr;
	}

	//
	// Free the mode handlers
	//
	max_interface->DeleteMode (m_ModeMove);
	max_interface->DeleteMode (m_ModeSelect);
	max_interface->DeleteMode (m_ModeRotate);
	max_interface->DeleteMode (m_ModeNUScale);
	max_interface->DeleteMode (m_ModeUScale);
	max_interface->DeleteMode (m_ModeSquash);
	SAFE_DELETE (m_ModeMove);
	SAFE_DELETE (m_ModeSelect);
	SAFE_DELETE (m_ModeRotate);
	SAFE_DELETE (m_ModeNUScale);
	SAFE_DELETE (m_ModeUScale);
	SAFE_DELETE (m_ModeSquash);

	// Release our hold on the max interface pointer
	m_MaxInterface = nullptr;
	m_pPanel = nullptr;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	ActivateSubobjSel
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::ActivateSubobjSel
(
	int level,
	XFormModes &modes
)
{
	switch (level)
	{
		// Vertex manipulation
		case 1:
			modes = XFormModes (m_ModeMove, m_ModeRotate, m_ModeNUScale, m_ModeUScale, m_ModeSquash, m_ModeSelect);
			break;
	}

	/*
	** Notify our dependents that the subselection type,
	** and the display have changed
	*/
	NotifyDependents(FOREVER, PART_SUBSEL_TYPE|PART_DISPLAY, REFMSG_CHANGE);

	/*
	** Notify the pipeline that the selection level has changed.
	*/
	m_MaxInterface->PipeSelLevelChanged();

	/*
	** Notify our dependents that the selection channel,
	** display attributes, and subselection type channels have changed
	*/
	NotifyDependents(FOREVER, VERTCOLOR_CHANNEL|SELECT_CHANNEL|DISP_ATTRIB_CHANNEL|SUBSEL_TYPE_CHANNEL, REFMSG_CHANGE);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	HitTest
//
///////////////////////////////////////////////////////////////////////////
int
MeshDeformClass::HitTest
(
	TimeValue		time_value,
	INode *			node,
	int				type,
	int				crossing,
	int				flags,
	IPoint2 *		point,
	ViewExp *		viewport,
	ModContext *	mod_context
)
{
	// Initialize the HitRegion
	HitRegion hit_rgn;
	MakeHitRegion (hit_rgn, type, crossing, 4, point);

	Matrix3 transform = node->GetObjectTM (time_value);
	Object *       obj = node->EvalWorldState(time_value).obj;
	TriObject *    tri = (TriObject *)obj->ConvertToType(time_value, triObjectClassID);

	//
	//	Set up the graphics window to do the hit-test
	//
	GraphicsWindow *graphics_wnd = viewport->getGW ();
	graphics_wnd->setHitRegion (&hit_rgn);
	graphics_wnd->setTransform (transform);

	int saved_limits = graphics_wnd->getRndLimits ();
	graphics_wnd->setRndLimits ((saved_limits | GW_PICK) & ~(GW_ILLUM | GW_BACKCULL));
	graphics_wnd->clearHitCode ();

	//
	// Perform the hit test
	//
	SubObjHitList hitlist;
	MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context->localData);
	Mesh &mesh = tri->mesh;//mod_data->Peek_Mesh ();
	int result = mesh.SubObjectHitTest (graphics_wnd,
													 graphics_wnd->getMaterial (),
													 &hit_rgn,
													 flags | SUBHIT_VERTS,
													 hitlist);

	//
	// Record all of the hits
	//
	for (MeshSubHitRec *hit_record = hitlist.First ();
		  hit_record != nullptr;
		  hit_record = hit_record->Next ()) {

		// rec->index is the index of vertex which was hit!
		viewport->LogHit (node, mod_context, hit_record->dist, hit_record->index, nullptr);
	}

	// Cleanup
	graphics_wnd->setRndLimits (saved_limits);

	// Return the integer result code
	return result;
}

///////////////////////////////////////////////////////////////////////////
//
//	HitTest
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::SelectSubComponent
(
	HitRecord *	hit_record,
	BOOL			selected,
	BOOL			all,
	BOOL			invert
)
{
	// Loop through all the hit records
	for (; hit_record != nullptr; hit_record = hit_record->Next ()) {

		// Peek at the vertex selection array for this hit record
		MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (hit_record->modContext->localData);
		Mesh *mesh = mod_data->Peek_Mesh ();
		BitArray &array = (mesh->vertSel);

		if (all & invert) {

			/*
			** hitRec->hitInfo is the MeshSubHitRec::index that was stored in the
			** HitTest method through LogHit
			*/
			if (array[hit_record->hitInfo]) {
				array.Clear (hit_record->hitInfo);
			} else {
				array.Set (hit_record->hitInfo, selected);
			}
		} else {
			array.Set (hit_record->hitInfo, selected);
		}

		if (!all) break;
	}

	m_pPanel->Update_Vertex_Color ();
	NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
	m_bSetDirty = true;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	GetSubObjectTMs
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::GetSubObjectTMs
(
	SubObjAxisCallback *cb,
	TimeValue t,
	INode *node,
	ModContext *mc
)
{
	int test = 0;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	HitTest
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::GetSubObjectCenters
(
	SubObjAxisCallback *	callback,
	TimeValue				time_val,
	INode *					node,
	ModContext *			mod_context
)
{
	// Peek at the vertex selection array for this hit record
	MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context->localData);
	const Point3 *vertex_array = mod_data->Peek_Orig_Vertex_Array ();
	Mesh *mesh = mod_data->Peek_Mesh ();

	BitArray sel_array = mesh->vertSel;
	Matrix3 transform = node->GetObjectTM (time_val);
	Box3 box;

	// Loop through all the selected verticies and create a bounding
	// box which we can use to determine the selection center.
	for (int index = 0; index < mesh->getNumVerts (); index++ ) {
		if (sel_array[index]) {
			box += mesh->getVert (index) * transform;
		}
	}

	// Pass the 'selection' center onto MAX
	callback->Center (box.Center (), 0);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	ClearSelection
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::ClearSelection (int selLevel)
{
	ModContextList mod_context_list;
	INodeTab nodes;
	m_MaxInterface->GetModContexts (mod_context_list, nodes);

	for (int i = 0; i < mod_context_list.Count (); i++) {

		MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[i]->localData);

		if (mod_data != nullptr) {
			mod_data->Peek_Mesh ()->vertSel.ClearAll ();
		}
	}

	/*
	** Get rid of the temporary copies of the INodes.
	*/
	nodes.DisposeTemporary ();

	/*
	** Tell our dependents that the selection set has changed
	*/
	NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
	m_bSetDirty = true;
	return ;
}

static Point3 last_delta;
static Point3 last_scale;
static Matrix3 last_rot;

///////////////////////////////////////////////////////////////////////////
//
//	Move
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Move
(
	TimeValue	time_val,
	Matrix3 &	parent_tm,
	Matrix3 &	tm_axis,
	Point3 &		displacement,
	BOOL			local_origin
)
{
	if (m_pPanel->Is_Edit_Mode ()) {

		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			// Get the data we've cached for this modifier context
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				Mesh *mesh = mod_data->Peek_Mesh ();
				const Point3 *vertex_array = mod_data->Peek_Orig_Vertex_Array ();
				Point3 *opstart_array = mod_data->Peek_Vertex_OPStart_Array ();

				// Loop through all the selected verts
				for (int vert = 0; vert < mesh->numVerts; vert ++) {
					if (mesh->vertSel[vert]) {

						// Do the 'displacment' in axis-space
						Point3 vert_ws = parent_tm * mesh->verts[vert];
						Point3 vert_as = Inverse (tm_axis) * vert_ws;
						vert_as += displacement - last_delta;

						// Convert back to obj-space
						vert_ws = tm_axis * vert_as;
						mesh->verts[vert] = Inverse (parent_tm) * vert_ws;

						// Record the delta
						//delta_array[vert] = mesh->verts[vert] - vertex_array[vert];
					}
				}

				//
				//	Record these changes in the current set
				//
				mod_data->Update_Set (m_CurrentSet, VERT_POSITION);
			}
		}

		// Remember what our last displacement was because we
		// want to perform our calculations relative to the current
		// position.
		last_delta = displacement;

		// Repaint the view
		nodes.DisposeTemporary ();
		NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

	m_OperationName = "Move";
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Move
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Scale
(
	TimeValue	time_val,
	Matrix3 &	parent_tm,
	Matrix3 &	tm_axis,
	Point3 &		value,
	BOOL			local_origin
)
{
	Point3 test = value - last_scale;
	bool bok = (test.x != 0) && (test.y != 0) && (test.z != 0);
	if (m_pPanel->Is_Edit_Mode () && bok) {

		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			// Get the data we've cached for this modifier context
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				Mesh *mesh = mod_data->Peek_Mesh ();
				const Point3 *vertex_array = mod_data->Peek_Orig_Vertex_Array ();
				Point3 *opstart_array = mod_data->Peek_Vertex_OPStart_Array ();

				// Loop through all the selected verts
				for (int vert = 0; vert < mesh->numVerts; vert ++) {
					if (mesh->vertSel[vert]) {

						// Do the 'scale' in axis-space
						Point3 vert_ws = parent_tm * opstart_array[vert];//mesh->verts[vert];
						Point3 vert_as = Inverse (tm_axis) * vert_ws;
						vert_as = ScaleMatrix (value) * vert_as;

						// Convert back to obj-space
						vert_ws = tm_axis * vert_as;
						mesh->verts[vert] = Inverse (parent_tm) * vert_ws;

						// Record the delta
						//delta_array[vert] = mesh->verts[vert] - vertex_array[vert];
					}
				}

				//
				//	Record these changes in the current set
				//
				mod_data->Update_Set (m_CurrentSet, VERT_POSITION);
			}
		}

		// Remember what our last displacement was because we
		// want to perform our calculations relative to the current
		// position.
		//last_scale = value - Point3 (1,1,1);
		//last_delta = displacement;

		// Repaint the view
		nodes.DisposeTemporary ();
		NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
	}

	m_OperationName = "Scale";
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Rotate
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Rotate
(
	TimeValue	time_val,
	Matrix3 &	parent_tm,
	Matrix3 &	tm_axis,
	Quat &		rotation,
	BOOL			local_origin
)
{
	if (m_pPanel->Is_Edit_Mode ()) {
		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		Matrix3 matrix_rot;
		rotation.MakeMatrix (matrix_rot);

		Matrix3 rel_rot;
		rel_rot = Inverse (last_rot) * matrix_rot;

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			// Get the data we've cached for this modifier context
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				Mesh *mesh = mod_data->Peek_Mesh ();
				const Point3 *vertex_array = mod_data->Peek_Orig_Vertex_Array ();
				Point3 *opstart_array = mod_data->Peek_Vertex_OPStart_Array ();

				// Loop through all the selected verts
				for (int vert = 0; vert < mesh->numVerts; vert ++) {
					if (mesh->vertSel[vert]) {

						// Do the 'displacment' in axis-space
						Point3 vert_ws = parent_tm * mesh->verts[vert];
						Point3 vert_as = Inverse (tm_axis) * vert_ws;
						vert_as = (rel_rot * vert_as);

						// Convert back to obj-space
						vert_ws = tm_axis * vert_as;
						mesh->verts[vert] = Inverse (parent_tm) * vert_ws;

						// Record the delta
						//delta_array[vert] = mesh->verts[vert] - vertex_array[vert];
					}
				}

				//
				//	Record these changes in the current set
				//
				mod_data->Update_Set (m_CurrentSet, VERT_POSITION);
			}
		}

		// Remember what our last displacement was because we
		// want to perform our calculations relative to the current
		// position.
		//last_delta = displacement;
		last_rot = matrix_rot;

		// Repaint the view
		nodes.DisposeTemporary ();
		NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);

		// Make sure our current 'set' knows what verts have changed
		//Update_Current_Set ();
	}

	m_OperationName = "Rotate";
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	TransformStart
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::TransformStart (TimeValue time_val)
{
	if (m_MaxInterface != nullptr) {
		m_MaxInterface->LockAxisTripods (TRUE);
	}

	// Reset our last-delta value
	last_delta.x = 0;
	last_delta.y = 0;
	last_delta.z = 0;
	last_rot.IdentityMatrix ();
	last_scale.x = 0;
	last_scale.y = 0;
	last_scale.z = 0;

	INodeTab nodes;
	ModContextList mod_context_list;
	m_MaxInterface->GetModContexts (mod_context_list, nodes);

	// Begin the undo operation
	theHold.Begin ();

	// Loop through all the modifier contexts
	for (int index = 0; index < mod_context_list.Count (); index ++) {

		// Get the data we've cached for this modifier context
		MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
		if (mod_data != nullptr) {
			Mesh *mesh = mod_data->Peek_Mesh ();
			Point3 *opstart_array = mod_data->Peek_Vertex_OPStart_Array ();

			// Copy the current state of the mesh
			for (int vert = 0; vert < mesh->numVerts; vert ++) {
				opstart_array[vert] = mesh->verts[vert];
			}

			// Add the 'position restore' object to the undo stack
			theHold.Put (new VertexPositionRestoreClass (mesh, this, mod_data));
		}
	}

	// Repaint the view
	nodes.DisposeTemporary ();
	NotifyDependents (FOREVER, PART_GEOM, REFMSG_CHANGE);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	TransformFinish
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::TransformFinish (TimeValue time_val)
{
	if (m_MaxInterface != nullptr) {
		m_MaxInterface->LockAxisTripods (FALSE);
	}

	// Accept the undo operation
	theHold.Accept (m_OperationName);
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	TransformCancel
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::TransformCancel (TimeValue time_val)
{
	if (m_MaxInterface != nullptr) {
		m_MaxInterface->LockAxisTripods (FALSE);
	}

	// Cancel the undo operation
	theHold.Cancel ();
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Deform_State
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Set_Deform_State (float state)
{
	if ((m_MaxInterface != nullptr) && (state != m_DeformState)) {
		m_DeformState = state;
		NotifyDependents (FOREVER, PART_GEOM | PART_VERTCOLOR, REFMSG_CHANGE);
		m_MaxInterface->RedrawViews (m_MaxInterface->GetTime ());
		if (m_pPanel != nullptr) {
			m_pPanel->Update_Vertex_Color ();
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Vertex_Color
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Set_Vertex_Color (const Point3 &color, bool button_up)
{
	if (m_MaxInterface != nullptr) {

		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		bool save_undo = false;
		if ((button_up == false) && (m_VertColorChanging == false)) {
			theHold.Begin ();
			m_VertColorChanging = true;
			save_undo = true;
		}

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			// Get the data we've cached for this modifier context
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				Mesh *mesh = mod_data->Peek_Mesh ();

				//
				//	Record the original color in the undo stack
				//
				if (save_undo) {
					theHold.Put (new VertexColorRestoreClass (mesh, this, mod_data));
				}

				// Only do this if the mesh is using vertex coloring
				if (mesh->numCVerts >= mesh->numVerts) {

					//
					//	Loop through all the per-face verts
					//
					for (int face = 0; face < mesh->numFaces; face ++) {
						for (int vert = 0; vert < 3; vert ++) {

							//
							//	If the vertex is selected, then change its color
							//
							if (mesh->vertSel[mesh->faces[face].v[vert]]) {
								int color_index = mesh->vcFace[face].t[vert];
								mesh->vertCol[color_index] = color;
							}
						}
					}
				}

				//
				//	Record these changes in the current set
				//
				mod_data->Update_Set (m_CurrentSet, VERT_COLORS);
			}
		}

		if (button_up && m_VertColorChanging) {
			theHold.Accept ("Vertex Color");
		}

		// Repaint the model
		nodes.DisposeTemporary ();
		NotifyDependents (FOREVER, PART_GEOM | PART_VERTCOLOR, REFMSG_CHANGE);
		m_MaxInterface->RedrawViews (m_MaxInterface->GetTime ());
	}

	m_VertColorChanging = !button_up;
	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Get_Vertex_Color
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Get_Vertex_Color (Point3 &color)
{
	// Assume black
	color.x = 0;
	color.y = 0;
	color.z = 0;

	if (m_MaxInterface != nullptr) {

		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		//
		// Loop through all the modifier contexts
		//
		int sel_count = 0;
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			//
			// Get the data we've cached for this modifier context
			//
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				Mesh *mesh = mod_data->Peek_Mesh ();

				// Only do this if the mesh is using vertex coloring
				if (mesh->numCVerts >= mesh->numVerts) {

					//
					//	Loop through all the per-face verts
					//
					for (int face = 0; face < mesh->numFaces; face ++) {
						for (int vert = 0; vert < 3; vert ++) {

							//
							//	If this vert is selected, then add its color to the total
							//
							if (mesh->vertSel[mesh->faces[face].v[vert]]) {
								int color_index	= mesh->vcFace[face].t[vert];
								Point3 vert_color = mesh->vertCol[color_index];
								color	+= vert_color;
								sel_count ++;
							}
						}
					}
				}
			}
		}

		//
		// Normalize the selected color
		//
		if (sel_count > 0) {
			color = color / sel_count;
		}

		nodes.DisposeTemporary ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_UI
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Update_UI (MeshDeformModData *mod_data)
{
	assert (mod_data != nullptr);

	if (m_pPanel != nullptr) {
		Update_Set_Count ();

		m_CurrentSet = mod_data->Get_Current_Set ();
		int keyframe = mod_data->Peek_Set (m_CurrentSet).Get_Current_Key_Frame ();
		Set_Deform_State ((float(keyframe + 1) + 0.5F) / 10.0F);

		m_pPanel->Update_Vertex_Color ();
		m_pPanel->Set_Max_Sets (m_MaxSets);
		m_pPanel->Set_Current_Set (m_CurrentSet);
		m_pPanel->Set_Current_State (m_DeformState);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Auto_Apply
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Auto_Apply (bool auto_apply)
{
	if (m_MaxInterface != nullptr) {

		// Get a list of contexts that we are part of.
		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			//
			//	Let the mod context know what it's auto apply state is
			//
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				mod_data->Auto_Apply (auto_apply);
			}
		}

		// Cleanup
		nodes.DisposeTemporary ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Max_Deform_Sets
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Set_Max_Deform_Sets (int max)
{
	//
	//	Make sure the current set doesn't exceed the total sets
	//
	if (m_CurrentSet >= max) {
		Set_Current_Set (max - 1, true);
	}

	m_MaxSets = max;
	if (m_MaxInterface != nullptr) {

		// Get a list of contexts that we are part of.
		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			//
			//	Let the mod context know the max sets have changed
			//
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				mod_data->Set_Max_Deform_Sets (max);
			}
		}

		// Cleanup
		nodes.DisposeTemporary ();
		NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
		m_MaxInterface->RedrawViews (m_MaxInterface->GetTime ());
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Set_Count
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Update_Set_Count (void)
{
	m_MaxSets = 1;
	if (m_MaxInterface != nullptr) {

		// Get a list of contexts that we are part of.
		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			//
			//	Get the count of sets for this context
			//
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if ((mod_data != nullptr) && (mod_data->Get_Set_Count () > m_MaxSets)) {
				m_MaxSets = mod_data->Get_Set_Count ();
			}
		}

		// Cleanup
		nodes.DisposeTemporary ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Set_Current_Set
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Set_Current_Set
(
	int index,
	bool update_selection
)
{
	last_delta.x = 0;
	last_delta.y = 0;
	last_delta.z = 0;

	m_CurrentSet = index;
	if (m_MaxInterface != nullptr) {
		if (update_selection) {
			ClearSelection (1);
		}

		// Get a list of contexts that we are part of.
		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			//
			//	Have the mod context select the verts in its set
			//
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				mod_data->Set_Current_Set (m_CurrentSet);
				if (update_selection) {
					mod_data->Select_Set (m_CurrentSet);
				}
				m_pPanel->Set_Auto_Apply_Check (mod_data->Is_Auto_Apply ());
			}
		}

		// Repaint the model
		nodes.DisposeTemporary ();
		if (update_selection) {
			NotifyDependents (FOREVER, PART_SELECT, REFMSG_CHANGE);
			m_MaxInterface->RedrawViews (m_MaxInterface->GetTime ());
		}

		// Update the current 'vertex color' on the UI panel
		m_pPanel->Update_Vertex_Color ();
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	Update_Current_Set
//
///////////////////////////////////////////////////////////////////////////
void
MeshDeformClass::Update_Current_Set (void)
{
	if (m_MaxInterface != nullptr) {

		// Get a list of contexts that we are part of.
		INodeTab nodes;
		ModContextList mod_context_list;
		m_MaxInterface->GetModContexts (mod_context_list, nodes);

		// Loop through all the modifier contexts
		for (int index = 0; index < mod_context_list.Count (); index ++) {

			//
			//	Notify the mod context so it can update its list of verts
			// in the current set.
			//
			MeshDeformModData *mod_data = static_cast <MeshDeformModData *> (mod_context_list[index]->localData);
			if (mod_data != nullptr) {
				//mod_data->Update_Set (m_CurrentSet);
			}
		}

		// Cleanup
		nodes.DisposeTemporary ();
		m_bSetDirty = false;
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////
//
//	SaveLocalData
//
///////////////////////////////////////////////////////////////////////////
IOResult
MeshDeformClass::SaveLocalData (ISave *save_obj, LocalModData *mod_context)
{
	assert (mod_context != nullptr);
	return ((MeshDeformModData *)mod_context)->Save (save_obj);
}


///////////////////////////////////////////////////////////////////////////
//
//	LoadLocalData
//
///////////////////////////////////////////////////////////////////////////
IOResult
MeshDeformClass::LoadLocalData (ILoad *load_obj, LocalModData **mod_context)
{
	assert (mod_context != nullptr);
	MeshDeformModData *mod_data = new MeshDeformModData;
	(*mod_context) = mod_data;
	return mod_data->Load (load_obj);
}






#if 0

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

		if (skindata==nullptr) continue;

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

		if (skindata==nullptr) continue;

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

#endif // 0


#if defined W3D_MAX4		//defined as in the project (.dsp)
////////////////////////////////////////////////////////////////////////////////////////
int MeshDeformClass::NumSubObjTypes()
{
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////
ISubObjType *MeshDeformClass::GetSubObjType(int i)
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
