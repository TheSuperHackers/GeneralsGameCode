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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/lookuptable.cpp                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/23/01 6:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "lookuptable.h"
#include "curve.h"


/*
** Static members
*/
RefMultiListClass<LookupTableClass>	LookupTableMgrClass::Tables;


/***********************************************************************************************
**
** LookupTableClass Implementation
**
***********************************************************************************************/
LookupTableClass::LookupTableClass(int sample_count) :
	MinInputValue(0.0f),
	MaxInputValue(0.0f),
	OutputSamples(sample_count)
{
}

LookupTableClass::~LookupTableClass()
{
}

void LookupTableClass::Init(const char * name,Curve1DClass * curve)
{
	// copy the name
	Name = name;

	// Store the min and max input values for the table
	curve->Get_Key(0,nullptr,&MinInputValue,nullptr);
	curve->Get_Key(curve->Key_Count()-1,nullptr,&MaxInputValue,nullptr);
	OOMaxMinusMin = 1.0f / (MaxInputValue - MinInputValue);

	// Sample the curve and store the output values
	for (int i=0; i<OutputSamples.Length(); i++) {
		float x = MinInputValue + (MaxInputValue - MinInputValue) * (float)i / (float)(OutputSamples.Length() - 1);
		float y;
		curve->Evaluate(x,&y);
		OutputSamples[i] = y;
	}
}



/***********************************************************************************************
**
** LookupTableManager Implementation
**
***********************************************************************************************/
void LookupTableMgrClass::Init()
{
	// create a default table that the user can use in an emergency
	LookupTableClass * default_table = NEW_REF(LookupTableClass,(2));
	LinearCurve1DClass * default_curve = W3DNEW LinearCurve1DClass;

	default_curve->Add_Key(0.5f,0.0f);
	default_curve->Add_Key(0.5f,1.0f);
	default_table->Init("DefaultTable",default_curve);
	Add_Table(default_table);

	delete default_curve;
	default_table->Release_Ref();
}

void LookupTableMgrClass::Shutdown()
{
	Reset();
}

void LookupTableMgrClass::Reset()
{
	while (Tables.Peek_Head() != nullptr) {
		Tables.Release_Head();
	}
}

bool LookupTableMgrClass::Add_Table(LookupTableClass * table)
{
	return Tables.Add(table);
}

bool LookupTableMgrClass::Remove_Table(LookupTableClass * table)
{
	return Tables.Remove(table);
}

LookupTableClass * LookupTableMgrClass::Get_Table(const char * name)
{
	// find the table with this name...
	RefMultiListIterator<LookupTableClass> it(&Tables);
	for (it.First(); !it.Is_Done(); it.Next()) {
		if (stricmp(it.Peek_Obj()->Get_Name(),name) == 0) {
			return it.Get_Obj(); // add a reference for the user...
		}
	}

	return nullptr;
}



