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
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/gridsnapmodifier.cpp           $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/01/01 8:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#include "max.h"
#include "resource.h"
#include "simpmod.h"
#include "dllmain.h"
#include "iparamb2.h"


/*

  WARNING WARNING WARNING PLEASE READ - This modifier was an experiment to see if we could
  solve cracks in adjacent meshes by snapping to a world-space grid.  It didn't work for a
  few reasons:
  - I couldn't implement the world space snapping; the SimpleMod code seems to always force
  you to work relative to each object.
  - Snapping to a grid won't always snap vertices to the same grid.  The probability that
  it will work is a function of the distance between the points and the grid spacing

*/



#define GRIDSNAPMOD_CLASSID Class_ID(0x7a2d399b, 0x1e3d2004)


/**
** GridSnapModifierClass
** This modifier will snap all vertices in the geometry being modified to a grid.  Its motivation is to 
** try to help solve the problem of cracks between adjacent meshes in Renegade levels.  This will work
** a lot better if the objects have reset-transforms prior to being processed by this modifier.
*/
class GridSnapModifierClass : public SimpleMod2 
{	
public:

	GridSnapModifierClass();

	// From Animatable
	void DeleteThis()																			{ delete this; }
	void GetClassName(TSTR& s)																{ s = Get_String(IDS_GRIDSNAPMODIFIER); }  
	virtual Class_ID ClassID()																{ return GRIDSNAPMOD_CLASSID; }		
	void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	TCHAR *GetObjectName()																	{ return Get_String(IDS_GRIDSNAPMODIFIER);}
	IOResult Load(ILoad *iload);

	// Direct paramblock access
	int	NumParamBlocks()																	{ return 1; }	
	IParamBlock2* GetParamBlock(int i)													{ return pblock2; }
	IParamBlock2* GetParamBlockByID(BlockID id)										{ return (pblock2->ID() == id) ? pblock2 : NULL; }

	// From simple mod
	Deformer& GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);		
	Interval GetValidity(TimeValue t);

	//RefTargetHandle GetReference(int i)
	//void SetReference(int i,RefTargetHandle rtar)
	//Animatable * SubAnim(int i)
};

/**
** GridSnapDeformerClass
** This is the callback object used by GridSnapModifierClass to implement the actual geometry changes.  This
** architecture is required by the SimpleMod base-class.  This Deformer simply snaps vertex positions
** to the grid defined by its parameters.
*/
class GridSnapDeformerClass : public Deformer
{
public:
	GridSnapDeformerClass(void) : GridDimension(0.001f) {}
		
	void		Set_Grid_Dimension(float grid_dim)		{ GridDimension = grid_dim; }
	float		Get_Grid_Dimension(void)					{ return GridDimension; }
	
	void		Set_Matrices(const Matrix3 & tm,const Matrix3 & invtm)	{ Transform = tm; InvTransform = invtm; }

	virtual Point3 Map(int i,Point3 p)
	{
		p = p*Transform;
		p.x = floor(p.x / GridDimension) * GridDimension;
		p.y = floor(p.y / GridDimension) * GridDimension;
		p.z = floor(p.z / GridDimension) * GridDimension;
		p = p*InvTransform;

		return p;
	}

private:
	float		GridDimension;
	Matrix3	Transform;
	Matrix3	InvTransform;
};


/** 
** GridSnapModifier Class Descriptor
** This object "links" the plugin into Max's plugin system.  It links the Class-ID to a virtual construction
** method.  The function Get_Grid_Snap_Modifier_Desc is the only hook to external code.
*/
class GridSnapModifierClassDesc:public ClassDesc2 
{
public:
	int 				IsPublic()											{ return 1; }
	void *			Create(BOOL loading = FALSE)					{ return new GridSnapModifierClass; }
	const TCHAR *	ClassName()											{ return _T("Grid Snap Modifier"); }
	SClass_ID		SuperClassID()										{ return OSM_CLASS_ID; }
	Class_ID			ClassID()											{ return GRIDSNAPMOD_CLASSID; }
	const TCHAR* 	Category()											{ return _T("Westwood Modifiers");}
	HINSTANCE		HInstance()											{ return AppInstance; }
	const TCHAR *	InternalName()										{ return _T("Westwood GridSnap"); }
};

static GridSnapModifierClassDesc _GridSnapModifierDesc;

ClassDesc* Get_Grid_Snap_Modifier_Desc(void) 
{	
	return &_GridSnapModifierDesc; 
}


/*
** ParamBlock2 Setup
*/
enum 
{
	GSM_PARAMS = 0,
};

enum 
{
	GSM_PARAM_GRIDDIMENSION = 0,	
};

static ParamBlockDesc2 _GridSnapParamBlockDesc
(
	// parameter block settings
	GSM_PARAMS,_T("GridSnap Parameters"), 0, &_GridSnapModifierDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, SIMPMOD_PBLOCKREF,
	
	// dialog box
	IDD_GRIDSNAP_PARAMS, IDS_GRIDSNAP_TITLE, 0, 0, NULL,
	
	// parameters
	GSM_PARAM_GRIDDIMENSION, _T("Grid Dimension"), TYPE_FLOAT, P_RESET_DEFAULT, IDS_GRID_DIMENSION,
		p_default,		0.001f,
		p_range,			0.0001f, 10.0f,
		p_ui,				TYPE_SPINNER, EDITTYPE_FLOAT, IDC_GRIDDIM_EDIT, IDC_GRIDDIM_SPIN, 0.0001f,
		end,

	end
);


/********************************************************************************************
**
** GridSnapModifierClass Implementation
**
********************************************************************************************/

GridSnapModifierClass::GridSnapModifierClass()
{
	// create the parameter block, storing in the base-class's pblock variable
	_GridSnapModifierDesc.MakeAutoParamBlocks(this);
	assert(pblock2);
}

void GridSnapModifierClass::BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev)
{
	this->ip = ip;

	SimpleMod2::BeginEditParams(ip,flags,prev);
	_GridSnapModifierDesc.BeginEditParams(ip, this, flags, prev);
}

void GridSnapModifierClass::EndEditParams( IObjParam *ip,ULONG flags,Animatable *next)
{
	SimpleMod2::EndEditParams(ip,flags,next);
	_GridSnapModifierDesc.EndEditParams(ip, this, flags, next);

	this->ip = NULL;
}

RefTargetHandle GridSnapModifierClass::Clone(RemapDir& remap)
{
	GridSnapModifierClass * newmod = new GridSnapModifierClass();	
	newmod->ReplaceReference(SIMPMOD_PBLOCKREF,pblock2->Clone(remap));
	newmod->SimpleModClone(this);
	return(newmod);
}

IOResult GridSnapModifierClass::Load(ILoad *iload)
{
	Modifier::Load(iload);
	return IO_OK;
}

Deformer& GridSnapModifierClass::GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat)
{
	float dimension = 0.0f; Interval valid = FOREVER;
	pblock2->GetValue(GSM_PARAM_GRIDDIMENSION, t, dimension, FOREVER);

	static GridSnapDeformerClass deformer;
	deformer.Set_Grid_Dimension(dimension);
	deformer.Set_Matrices(mat,invmat);
	return deformer;
}

Interval GridSnapModifierClass::GetValidity(TimeValue t)
{
	float f;	
	Interval valid = FOREVER;
	pblock2->GetValue(GSM_PARAM_GRIDDIMENSION, t, f, valid);
	return valid;
}

RefTargetHandle SimpleMod2::GetReference(int i)
{
	switch (i) {
		case 0: return tmControl;
		case 1: return posControl;
		case 2: return pblock2;
		default: return NULL;
	}
}

void SimpleMod2::SetReference(int i,RefTargetHandle rtarg)
{
	switch (i) {
		case 0: tmControl = (Control*)rtarg; break;
		case 1: posControl = (Control*)rtarg; break;
		case 2: pblock2 = (IParamBlock2*)rtarg; break;
	}
}

Animatable * SimpleMod2::SubAnim(int i)
{
	switch (i) {
		case 0: return posControl;
		case 1: return tmControl;
		case 2: return pblock2;		
		default: return NULL;
	}
}