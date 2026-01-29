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
 *                 Project name : Buccaneer Bay                                                *
 *                                                                                             *
 *                    File name : AlphaModifier.cpp                                            *
 *                                                                                             *
 *                   Programmer : Mike Lytle                                                   *
 *                                                                                             *
 *                   Start date : 11/1/1999                                                    *
 *                                                                                             *
 *                  Last update : 11/1/1999                                                    *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "AlphaModifier.h"

enum Alpha_Messages
{
	AM_NOTHING,
	AM_UPDATE_DATA,
	AM_INITIALIZE,
	AM_BOX_CHECKED,
};

enum Dialog_Controls
{
	DL_EDIT_VALUE,
	DL_FIND_CHECK_BOX,
};

void AlphaModifierClass::ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node)
{
	if (!os->obj->IsSubClassOf(triObjectClassID))
	{
		return;
	}


	// Get a mesh from input object
	TriObject *object = (TriObject*)os->obj;

	Mesh *mesh = &object->mesh;

	assert(mesh);

	int numVert = mesh->getNumVerts();
	int i = 0;
	float *vdata = nullptr;

	// Get parameters from pblock
	float		sparam = 0.0f;
	Interval	valid = LocalValidity(t);
	int			pass = 1;

	pblock->GetValue(DL_EDIT_VALUE, t, sparam, valid);

	// If needed a control could be put into the dialog box to specify which
	// pass to apply the alpha values to.  At this time, it was decided to
	// not implement this because of the complexity to the artist and the
	// performance issues in game.
	//pblock->GetValue(DL_EDIT_PASS, t, pass, valid);


	// Start from 0.
	pass -= 1;
	assert(pass >= 0);

	// Use a channel for each pass.
	vdata = mesh->vertexFloat(ALPHA_VERTEX_CHANNEL + pass);

	if (!vdata)
	{
		// Turn on the channel for vertex alpha support.
		mesh->setVDataSupport(ALPHA_VERTEX_CHANNEL + pass);
		vdata = mesh->vertexFloat(ALPHA_VERTEX_CHANNEL + pass);

		assert(vdata);

		for (i = 0; i < numVert; i++)
		{
			if (mesh->VertSel()[i])
			{
				vdata[i] = 0.0f;
			}
		}

	}

	// Tracks the state of the FIND check box.
	int box_checked = 0;


	if (Message == AM_UPDATE_DATA)
	{
		// The user has updated the dialog box, so update the data.
		assert(vdata);

		pblock->GetValue(DL_FIND_CHECK_BOX, t, box_checked, valid);
		if (!box_checked)
		{
			for (i = 0; i < numVert; i++)
			{
				if (SelectedVertices[i])
				{
					vdata[i] = sparam;
				}
			}
		}
	}

	if (Message == AM_BOX_CHECKED)
	{
		pblock->GetValue(DL_FIND_CHECK_BOX, t, box_checked, valid);
	}


	// The user is trying to find vertices with certain values.
	if (box_checked)
	{
		assert(vdata);
		// Find the vertices that have the user entered value.
		for (i = 0; i < numVert; i++)
		{
			if (vdata[i] == sparam)
			{
				mesh->VertSel().Set(i);
				SelectedVertices.Set(i);
			}
			else
			{
				mesh->VertSel().Clear(i);
				SelectedVertices.Clear(i);
			}

		}

	}

	if (Message == AM_INITIALIZE)
	{
		assert(vdata);

		SelectedVertices = mesh->VertSel();

		for (i = 0; i < numVert; i++)
		{
			if (SelectedVertices[i])
			{
				// Set the value in the dialog box to the value of the
				// first selected vertex.
				pblock->SetValue(DL_EDIT_VALUE, t, vdata[i]);
				break;
			}
		}

	}


	// Always select the vertices that have been saved by the modifier.
	// This must be done because the mesh changes each time ModfiyObject is called.
	for (i = 0; i < numVert; i++)
	{
		if (SelectedVertices[i])
		{
			mesh->VertSel().Set(i);
		}
		else
		{
			mesh->VertSel().Clear(i);
		}

	}

	// Display the vertices.
	mesh->SetDispFlag(DISP_SELVERTS | DISP_VERTTICKS);
	mesh->selLevel = MESH_VERTEX;
	object->UpdateValidity(SELECT_CHAN_NUM, object->ChannelValidity (t, SELECT_CHAN_NUM));

	// Clear messages.
	Message = AM_NOTHING;
}




/*===========================================================================*\
 |	NotifyInputChanged is called each time the input object is changed in some way
 |	We can find out how it was changed by checking partID and message
\*===========================================================================*/

void AlphaModifierClass::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{
	if( (partID&PART_TOPO) || (partID&PART_GEOM) || (partID&PART_SELECT) )
	{
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	}
}


/*===========================================================================*\
 |	Class Descriptor OSM
\*===========================================================================*/

class AlphaClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic()					{ return TRUE; }
	void *			Create( BOOL loading )		{ return new AlphaModifierClass; }
	const TCHAR *	ClassName()					{ return Get_String(IDS_ALPHA_MODIFIER_CLASS); }
	SClass_ID		SuperClassID()				{ return OSM_CLASS_ID; }
	Class_ID 		ClassID()					{ return ALPHA_MODIFIER_CLASSID; }
	const TCHAR* 	Category()					{ return _T("");  }

	HINSTANCE		HInstance()					{ return AppInstance; }

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR*	InternalName()				{ return _T("AlphaMod"); }
};

static AlphaClassDesc AlphaCD;
ClassDesc* Get_Alpha_Desc() {return &AlphaCD;}

/*===========================================================================*\
 |	Paramblock2 Descriptor
\*===========================================================================*/
static ParamBlockDesc2 alpha_param_blk
(
	//rollout
		0, _T("AlphaModifierParams"),  0, &AlphaCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0,
		IDD_ALPHA_MODIFIER, IDS_PARAMETERS, 0, 0, nullptr,

	// params

		DL_EDIT_VALUE,	_T("Custom Data Value"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_ALPHA_MODIFIER_CLASS,
		p_default,		0.0f,
		p_range, 		0.0f, 100.0f,
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_ALPHA_EDIT, IDC_ALPHA_SPIN, 1.0f,
		end,

		DL_FIND_CHECK_BOX,	_T("1 Custom Data Value"),	TYPE_BOOL,	0,	IDS_ALPHA_MODIFIER_CLASS,
		p_default,		FALSE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_ALPHA_CHECKBOX,
		p_enabled,		TRUE,
		end,

/*
		DL_EDIT_PASS,	_T("2 Custom Data Value"),	TYPE_INT,	P_ANIMATABLE,	IDS_ALPHA_MODIFIER_CLASS,
		p_default,		1,
		p_range, 		1, 4,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_ALPHA_EDIT2, IDC_ALPHA_SPIN2, 1.0,
		end,
*/

	end
);


/*===========================================================================*\
 |	Basic implementation of a dialog handler
\*===========================================================================*/

BOOL AlphaModDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);

	switch (msg)
	{
		case WM_INITDIALOG:
			AlphaModifier->Message = AM_INITIALIZE;
			break;
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			switch (code)
			{
				case EN_UPDATE:
				break;
				case EN_SETFOCUS:
				break;
				case EN_KILLFOCUS:
				break;
				case EN_CHANGE:
				break;
			}
			if (id == IDC_ALPHA_EDIT)
			{
				AlphaModifier->Message = AM_UPDATE_DATA;
			}
			if (id == IDC_ALPHA_CHECKBOX)
			{
				AlphaModifier->Message = AM_BOX_CHECKED;
			}
			break;

		case WM_NOTIFY:
			if (id == IDC_ALPHA_EDIT)
			{
				AlphaModifier->Message = AM_UPDATE_DATA;
			}
			break;

		default:
			break;


	}
	return FALSE;
}


/*===========================================================================*\
 |	Constructor
 |  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

AlphaModifierClass::AlphaModifierClass()
{
	AlphaCD.MakeAutoParamBlocks(this);
	assert(pblock);
	Message = 0;
}


/*===========================================================================*\
 |	Invalidate our UI (or the recently changed parameter)
\*===========================================================================*/

void AlphaModifierClass::InvalidateUI()
{
	alpha_param_blk.InvalidateUI(pblock->LastNotifyParamID());
}



/*===========================================================================*\
 |	Open and Close dialog UIs
 |	We ask the ClassDesc2 to handle Beginning and Ending EditParams for us
\*===========================================================================*/

void AlphaModifierClass::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{

	AlphaCD.BeginEditParams(ip, this, flags, prev);

	alpha_param_blk.SetUserDlgProc(new AlphaModDlgProc(this));
}

void AlphaModifierClass::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	AlphaCD.EndEditParams(ip, this, flags, next);
}



/*===========================================================================*\
 |	Standard clone
\*===========================================================================*/


RefTargetHandle AlphaModifierClass::Clone(RemapDir& remap)
{
	AlphaModifierClass* newmod = new AlphaModifierClass();
	newmod->ReplaceReference(0,pblock->Clone(remap));
	return(newmod);
}




/*===========================================================================*\
 |	Subanim & References support
\*===========================================================================*/

Animatable* AlphaModifierClass::SubAnim(int i)
{
	switch (i)
	{
		case 0: return pblock;
		default: return nullptr;
	}
}

TSTR AlphaModifierClass::SubAnimName(int i)
{
	switch (i)
	{
		case 0: return Get_String(IDS_PARAMETERS);
		default: return _T("");
	}
}

RefTargetHandle AlphaModifierClass::GetReference(int i)
{
	switch (i)
	{
		case 0: return pblock;
		default:
			assert(TRUE);
			return nullptr;
	}
}

void AlphaModifierClass::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
		case 0: pblock = (IParamBlock2*)rtarg; break;
		default:
			assert(TRUE);
			break;
	}
}

RefResult AlphaModifierClass::NotifyRefChanged
(
		Interval changeInt,
		RefTargetHandle hTarget,
		PartID& partID,
		RefMessage message
)
{
	switch (message)
	{
		case REFMSG_CHANGE:
		{
 			alpha_param_blk.InvalidateUI();
		}
		break;
	}
	return REF_SUCCEED;
}




/*===========================================================================*\
 |	The validity of our parameters
 |	Start at FOREVER, and intersect with the validity of each item
\*===========================================================================*/

Interval AlphaModifierClass::GetValidity(TimeValue t)
{
	float f;
	Interval valid = FOREVER;
	pblock->GetValue(DL_EDIT_VALUE, t, f, valid);
	return valid;
}

Interval AlphaModifierClass::LocalValidity(TimeValue t)
{
	return GetValidity(t);
}



