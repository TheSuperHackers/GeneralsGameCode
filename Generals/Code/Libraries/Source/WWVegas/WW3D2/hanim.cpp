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

/* $Header: /Commando/Code/ww3d2/hanim.cpp 3     12/13/01 7:01p Patrick $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D Library                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/ww3d2/hanim.cpp                              $* 
 *                                                                                             * 
 *                       Author:: Greg_h                                                       * 
 *                                                                                             * 
 *                     $Modtime:: 12/13/01 6:54p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "hanim.h"
#include "assetmgr.h"
#include "htree.h"
#include "motchan.h"
#include "chunkio.h"
#include "w3d_file.h"
#include "wwdebug.h"
#include <string.h>
#include <nstrdup.h>



/*
**
**	HAnimComboClass
**
**
*/


NamedPivotMapClass::~NamedPivotMapClass(void)
{
}

NamedPivotMapClass::WeightInfoStruct & NamedPivotMapClass::WeightInfoStruct::operator = (WeightInfoStruct const &that)
{	
	if(Name) delete [] Name;
	assert(that.Name != 0);
	Name = nstrdup(that.Name); 
	Weight = that.Weight; 
	return *this; 
}

// add a name & weight to the arrays
void NamedPivotMapClass::Add(const char *Name, float Weight)
{
	WeightInfoStruct info;
	info.Name = (char *) Name;
	info.Weight = Weight;
	WeightInfo.Add(info);	
	info.Name = 0;
}

// configure the base pivot map using the specified tree
void NamedPivotMapClass::Update_Pivot_Map(const HTreeClass *Tree)
{
	// first, resize the base pivot map to fit the tree
	int numPivots = Tree->Num_Pivots();

	Resize(numPivots);
	ActiveCount = numPivots;

	// first, reset all weights to 1 (the default)
	while(numPivots--) {
		(*this)[numPivots] = 1.0f;
	}

	// for each named pivot, find the correct index and set the weight if the indicated bone is present
	// note: there is no check for redundant names
	int count = WeightInfo.Count();
	while(count--) {
		int actualPivot = Tree->Get_Bone_Index(WeightInfo[count].Name);
		if(actualPivot != -1) {
			(*this)[actualPivot] = WeightInfo[count].Weight;
		} 
	}
}

/*
**
*/
DEFINE_AUTO_POOL(HAnimComboDataClass,256);

HAnimComboDataClass::HAnimComboDataClass(bool shared) 
: Shared(shared), HAnim(0), PivotMap(0), Frame(0), PrevFrame(0), Weight(1) 
{}


HAnimComboDataClass::HAnimComboDataClass(const HAnimComboDataClass &src)
:	PivotMap(src.Get_Pivot_Map()),
	HAnim(src.Get_HAnim())
{
	Shared = src.Is_Shared();
	Frame = src.Get_Frame();
	PrevFrame = src.Get_Prev_Frame();
	Weight = src.Get_Weight();
}

void HAnimComboDataClass::Copy(const HAnimComboDataClass *src)
{
	if(src) {
		HAnim = src->Get_HAnim();
		PivotMap = src->Get_Pivot_Map();
		Frame = src->Get_Frame();
		PrevFrame = src->Get_Prev_Frame();
		Weight = src->Get_Weight();
	} else {
		HAnim = 0;
		PivotMap = 0;
		Frame = 0;
		PrevFrame = 0;
		Weight = 1;
	}
}

HAnimComboDataClass::~HAnimComboDataClass(void)
{
	if(HAnim) 
		HAnim->Release_Ref();
	if(PivotMap)
		PivotMap->Release_Ref();
}

void HAnimComboDataClass::Clear(void) 
{
	if ( HAnim != NULL ) {
		HAnim->Release_Ref();
		HAnim = NULL;
	}

	// not sure if the pivot map should be deleted or just have everything set to one.
	// removing it effectively sets it to one, so that's what I'm doing for now.
	if(PivotMap) {
		PivotMap->Release_Ref();
		PivotMap = NULL;
	}

	Frame = 0.0f;
	PrevFrame = 0.0f;
	Weight = 1.0;
	PivotMap = NULL;
}

void HAnimComboDataClass::Set_HAnim(HAnimClass *motion)
{
	if ( motion != NULL ) {
		motion->Add_Ref();
	}
	if ( HAnim != NULL ) {
		HAnim->Release_Ref();
	}
	HAnim = motion;
}


void HAnimComboDataClass::Set_Pivot_Map(PivotMapClass *map)
{
	if ( map != NULL ) {
		map->Add_Ref();
	}
	if ( PivotMap != NULL ) {
		PivotMap->Release_Ref();
	}
	PivotMap = map;
}

/*
**	This function will replace the current pivot map (if any) with another pivot map that is
** set to 1 for only those pivot indices that actually have data.
*/
void HAnimComboDataClass::Build_Active_Pivot_Map(void) 
{
	if ( PivotMap != NULL ) {
		PivotMap->Release_Ref();
	}
	if(HAnim == NULL) {
		PivotMap = 0;
		return;
	}

	int numpivots = HAnim->Get_Num_Pivots();
	PivotMap = NEW_REF( PivotMapClass, ());
	PivotMap->Resize(numpivots);

	int count = 0;
	while(count < numpivots) {
		if(HAnim->Is_Node_Motion_Present(count)) {
			PivotMap->Add(1);

		} else {
			PivotMap->Add(0);
		}
		count++;
	}
}

/*	
**	HAnimComboClass
**
**
*/

HAnimComboClass::HAnimComboClass(void)
{}

HAnimComboClass::HAnimComboClass( int num_animations )
{
	HAnimComboData.Resize(num_animations);

	while(num_animations--) {
		HAnimComboData.Add(new HAnimComboDataClass());
	}
}


HAnimComboClass::~HAnimComboClass(void)
{
	Reset();
}


void	HAnimComboClass::Clear( void )
{
	int numAnimations = HAnimComboData.Count();
	while ( numAnimations-- ) {
		HAnimComboDataClass *data = HAnimComboData[numAnimations];
		if(data && (! data->Is_Shared()))
			data->Clear();
	}
}

void	HAnimComboClass::Reset( void )
{
	int numAnimations = HAnimComboData.Count();
	while ( numAnimations-- ) {
		HAnimComboDataClass *data = HAnimComboData[numAnimations];
		if(data && (! data->Is_Shared())) {
			delete data; 
		}
	}
	HAnimComboData.Reset_Active();
}

bool	HAnimComboClass::Normalize_Weights(void)
{
	// NOTE: This can only work if either no anims have pivot weight maps (in which case we will
	// adjust the anim weights to ensure normalization), or else if all do (in which case we will
	// adjust the pivot maps). Otherwise we do nothing and return false.
	int anim_count = Get_Num_Anims();
	if (!anim_count) return true;	// Trivially succeeded
	
	// Loop over all anims. Check if all or none have pivot maps, and also calculate the minimum
	// number of pivots.
	int anim_idx = 0;
	bool all_pivot_maps = true;
	bool none_pivot_maps = true;
	int num_anim_pivots = 100000;
	for (anim_idx = 0; anim_idx < anim_count; anim_idx++ ) {
		num_anim_pivots = MIN(num_anim_pivots, Peek_Motion(anim_idx)->Get_Num_Pivots());
		bool has_pivot_map = Peek_Pivot_Weight_Map(anim_idx) != NULL;
		all_pivot_maps &= has_pivot_map;
		none_pivot_maps &= !has_pivot_map;
	}
	if ( num_anim_pivots == 100000 ) {
		num_anim_pivots = 0;
	}

	if (none_pivot_maps) {

		// Calculate total weight of all active anims, ensure it is very close to 1.
		float weight_total = 0.0f;
		for (anim_idx = 0; anim_idx < anim_count; anim_idx++ ) {
			if (Peek_Motion(anim_idx) != NULL ) {
				float	weight = Get_Weight(anim_idx);
				weight_total += weight;
			}
		}

		// weight_total should be very close to 1. If not, normalize this pivot's weights
		if (weight_total != 0.0 && WWMath::Fabs( weight_total - 1.0 ) > WWMATH_EPSILON) {
			float oo_total = 1.0f / weight_total;
			for (anim_idx = 0; anim_idx < anim_count; anim_idx++ ) {
				if (Peek_Motion(anim_idx) != NULL ) {
					Set_Weight(anim_idx, Get_Weight(anim_idx) * oo_total);
				}
			}
		}

	} else {

		if (all_pivot_maps) {

			// For each pivot, calculate total weight of all active anims, ensure close to 1.
			for (int piv_idx = 1; piv_idx < num_anim_pivots; piv_idx++) {

				float weight_total = 0.0f;
				for (anim_idx = 0; anim_idx < anim_count; anim_idx++ ) {
					if (Peek_Motion(anim_idx) != NULL ) {
						float	weight = Get_Weight(anim_idx) * (*Peek_Pivot_Weight_Map(anim_idx))[piv_idx];
						weight_total += weight;
					}
				}

				// weight_total should be very close to 1. If not, normalize this pivot's weights
				if (weight_total != 0.0 && WWMath::Fabs( weight_total - 1.0 ) > WWMATH_EPSILON) {
					float oo_total = 1.0f / weight_total;
					for (anim_idx = 0; anim_idx < anim_count; anim_idx++ ) {
						if (Peek_Motion(anim_idx) != NULL ) {
							PivotMapClass *pivot_map = Get_Pivot_Weight_Map(anim_idx);
							float new_weight = (*pivot_map)[piv_idx] * oo_total;
							(*pivot_map)[piv_idx] = new_weight;
							pivot_map->Release_Ref();
						}
					}
				}

			}

		} else {
			return false;
		}
	}

	return true;

}

void	HAnimComboClass::Set_Motion( int index, HAnimClass *motion )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	data->Set_HAnim(motion);
}

HAnimClass *HAnimComboClass::Get_Motion( int index )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	HAnimClass *anim = data->Peek_HAnim();

	if ( anim != NULL ) {
		anim->Add_Ref();
	}
	return anim;
}

HAnimClass *HAnimComboClass::Peek_Motion( int index )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	HAnimClass *anim = data->Peek_HAnim();
	return anim;
}

void	HAnimComboClass::Set_Frame( int index, float frame )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	data->Set_Frame(frame);
}

float	HAnimComboClass::Get_Frame( int index )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	return data->Get_Frame();
}

void	HAnimComboClass::Set_Prev_Frame( int index, float frame )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	data->Set_Prev_Frame(frame);
}

float	HAnimComboClass::Get_Prev_Frame( int index )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	return data->Get_Prev_Frame();
}

void	HAnimComboClass::Set_Weight( int index, float weight )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	data->Set_Weight(weight);
}

float	HAnimComboClass::Get_Weight( int index )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	return data->Get_Weight();
}

void	HAnimComboClass::Set_Pivot_Weight_Map( int index, PivotMapClass *map )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	data->Set_Pivot_Map(map);
}

PivotMapClass	*HAnimComboClass::Get_Pivot_Weight_Map( int index )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	return data->Get_Pivot_Map();
}

PivotMapClass	*HAnimComboClass::Peek_Pivot_Weight_Map( int index )
{
	HAnimComboDataClass *data = HAnimComboData[index];
	WWASSERT(data);

	return data->Peek_Pivot_Map();
}

void HAnimComboClass::Append_Anim_Combo_Data(HAnimComboDataClass * Data)
{
	HAnimComboData.Add(Data);
}

void HAnimComboClass::Remove_Anim_Combo_Data(HAnimComboDataClass * Data)
{
	HAnimComboData.Delete(Data);
}


