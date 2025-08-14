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
 *                     $Archive:: /Commando/Code/wwmath/lookuptable.h                         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/18/00 1:20p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include "always.h"
#include "simplevec.h"
#include "wwstring.h"
#include "refcount.h"
#include "multilist.h"
#include "wwmath.h"


class Vector2;
class Curve1DClass;
class ChunkSaveClass;
class ChunkLoadClass;


/**
** LookupTableClass
** This class contains the tabulated values for a function.
*/
class LookupTableClass : public RefCountClass, public MultiListObjectClass
{
public:

	LookupTableClass(int sample_count = 256);
	virtual ~LookupTableClass(void);

	void								Init(const char * name,Curve1DClass * curve);
	float								Get_Value(float input);
	float								Get_Value_Quick(float input);
	const char *					Get_Name(void)							{ return Name; }
protected:

	StringClass						Name;				// name of this table, if it came from a file, this is also the filename
	float								MinInputValue;
	float								MaxInputValue;
	float								OOMaxMinusMin;
	SimpleVecClass<float>		OutputSamples;

};

inline float LookupTableClass::Get_Value(float input)
{
	if (input <= MinInputValue) {
		return OutputSamples[0];
	}
	if (input >= MaxInputValue) {
		return OutputSamples[OutputSamples.Length()-1];
	}

	float normalized_input = (float)(OutputSamples.Length()-1) * (input - MinInputValue) * OOMaxMinusMin;
	float input0 = WWMath::Floor(normalized_input);

	int index0 = WWMath::Float_To_Long(input0);
	int index1 = index0+1;
	float lerp = normalized_input - input0;

	return OutputSamples[index0] + lerp * (OutputSamples[index1] - OutputSamples[index0]);
}

inline float LookupTableClass::Get_Value_Quick(float input)
{
	if (input <= MinInputValue) {
		return OutputSamples[0];
	}
	if (input >= MaxInputValue) {
		return OutputSamples[OutputSamples.Length()-1];
	}

	int index = (OutputSamples.Length()-1) * WWMath::Float_To_Long((input - MinInputValue) * OOMaxMinusMin);
	return OutputSamples[index];
}


/**
** LookupTableMgrClass
** This class tracks all of the LookupTableClass's that have been loaded or installed.
** LookupTables can be created using the "SimpleGraph" tool.  It basically allows you
** to edit a curve which will be used to generate the table.  These curves are stored
** in .TBL files.
**
** NOTE: Use the SimpleGraph program to create lookup tables!  Then just ask for them
** by filename and it will load the table for you (unless it is already loaded).
**
** NOTE: I add a table called "DefaultTable" so that you can revert to that if
** your table isn't found.
*/
class LookupTableMgrClass
{
public:
	LookupTableMgrClass(void);
	~LookupTableMgrClass(void);

	// init and shutdown are automatically called from WWMath::Init, WWMath::Shutdown...
	static void					Init(void);
	static void					Shutdown(void);

	static bool					Add_Table(LookupTableClass * table);
	static bool					Remove_Table(LookupTableClass * table);
	static LookupTableClass *	Get_Table(const char * name,bool try_to_load = true);

	static void					Save_Table_Desc(		ChunkSaveClass &	csave,
																Curve1DClass *		curve,
																const Vector2 &	min,
																const Vector2 &	max			);

	static void					Load_Table_Desc(		ChunkLoadClass &	cload,
																Curve1DClass **	curve_ptr,
																Vector2 *			set_min = NULL,
																Vector2 *			set_max = NULL	);

	static void					Reset(void);

protected:

	static RefMultiListClass<LookupTableClass>	Tables;

};


#endif // LOOKUPTABLE_H
