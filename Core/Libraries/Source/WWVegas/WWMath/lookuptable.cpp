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
#include "WWFILE.h"
#include "ffactory.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "vector2.h"


/*
** Static members
*/
RefMultiListClass<LookupTableClass>	LookupTableMgrClass::Tables;

/*
** Save-Load stuff.  Lookup tables are basically saved 1D curve classes.  They are turned
** into tables at load time.  For future expansion, the curve is wrapped in a chunk.
*/
enum
{
	LOOKUPTABLE_CHUNK_CURVE		= 03071200,
	LOOKUPTABLE_CHUNK_EXTENTS,
};


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

LookupTableClass::~LookupTableClass(void)
{
}

void LookupTableClass::Init(const char * name,Curve1DClass * curve)
{
	// copy the name
	Name = name;

	// Store the min and max input values for the table
	curve->Get_Key(0,NULL,&MinInputValue,NULL);
	curve->Get_Key(curve->Key_Count()-1,NULL,&MaxInputValue,NULL);
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
void LookupTableMgrClass::Init(void)
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

void LookupTableMgrClass::Shutdown(void)
{
	Reset();
}

void LookupTableMgrClass::Reset(void)
{
	while (Tables.Peek_Head() != NULL) {
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

LookupTableClass * LookupTableMgrClass::Get_Table(const char * name,bool try_to_load)
{
	// check if we already have this table loaded...
	RefMultiListIterator<LookupTableClass> it(&Tables);
	for (it.First(); !it.Is_Done(); it.Next()) {
		if (stricmp(it.Peek_Obj()->Get_Name(),name) == 0) {
			return it.Get_Obj(); // add a reference for the user...
		}
	}

	// otherwise we can try to load it.
	LookupTableClass * new_table = NULL;
	if (try_to_load) {

		FileClass * file = _TheFileFactory->Get_File(name);
		if (file && file->Open()) {

			ChunkLoadClass cload(file);

			Curve1DClass * curve = NULL;
			Load_Table_Desc(cload,&curve);
			if (curve != NULL) {
				new_table = NEW_REF(LookupTableClass,());
				new_table->Init(name,curve);
				Add_Table(new_table);
				delete curve;
			}
		}
		_TheFileFactory->Return_File(file);
	}

	return new_table;  // constructor ref is returned to user.
}

void LookupTableMgrClass::Save_Table_Desc
(
	ChunkSaveClass &	csave,
	Curve1DClass *		curve,
	const Vector2 &	min_corner,
	const Vector2 &	max_corner
)
{
	// save the curve
	csave.Begin_Chunk(LOOKUPTABLE_CHUNK_CURVE);
	csave.Begin_Chunk(curve->Get_Factory().Chunk_ID());
	curve->Get_Factory().Save(csave,curve);
	csave.End_Chunk();
	csave.End_Chunk();

	// save the extents
	csave.Begin_Chunk(LOOKUPTABLE_CHUNK_EXTENTS);
	csave.Write(&(min_corner.X),sizeof(float));
	csave.Write(&(min_corner.Y),sizeof(float));
	csave.Write(&(max_corner.X),sizeof(float));
	csave.Write(&(max_corner.Y),sizeof(float));
	csave.End_Chunk();
}

void LookupTableMgrClass::Load_Table_Desc
(
	ChunkLoadClass &	cload,
	Curve1DClass **	curve_ptr,
	Vector2 *			set_min_corner,
	Vector2 *			set_max_corner
)
{
	*curve_ptr = NULL;
	PersistFactoryClass * factory;

	float xmin,xmax;
	float ymin,ymax;
	xmin = xmax = ymin = ymax = 0.0f;

	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID()) {
			case LOOKUPTABLE_CHUNK_CURVE:
				cload.Open_Chunk();
				factory = SaveLoadSystemClass::Find_Persist_Factory(cload.Cur_Chunk_ID());
				WWASSERT(factory != NULL);
				if (factory != NULL) {
					*curve_ptr = (Curve1DClass *)factory->Load(cload);
				}
				cload.Close_Chunk();
				break;
			case LOOKUPTABLE_CHUNK_EXTENTS:
				cload.Read(&xmin,sizeof(xmin));
				cload.Read(&ymin,sizeof(ymin));
				cload.Read(&xmax,sizeof(xmax));
				cload.Read(&ymax,sizeof(ymax));
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d",__FILE__,__LINE__));
				break;
		}
		cload.Close_Chunk();
	}

	if (set_min_corner != NULL) {
		set_min_corner->Set(xmin,ymin);
	}
	if (set_max_corner != NULL) {
		set_max_corner->Set(xmax,ymax);
	}
}



