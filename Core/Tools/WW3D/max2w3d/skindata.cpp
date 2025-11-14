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

/* $Header: /Commando/Code/Tools/max2w3d/skindata.cpp 7     5/28/98 12:15p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - WWSkin                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/skindata.cpp                   $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/28/98 12:15p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SkinDataClass::Save -- save the skindata in the MAX file                                  *
 *   SkinDataClass::Load -- load the skindata from a MAX file                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "skindata.h"


/***********************************************************************************************
 * SkinDataClass::Save -- save the skindata in the MAX file                                    *
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
IOResult SkinDataClass::Save(ISave *isave)
{
	ULONG nb;

	/*
	** save the flags
	*/
	short flags = 0;
	if (Valid) flags |= 0x01;
	if (Held) flags |= 0x02;

	isave->BeginChunk(FLAGS_CHUNK);
	isave->Write(&flags,sizeof(flags),&nb);
	isave->EndChunk();

	/*
	** Save the bit array of currently selected vertices
	*/
	if (VertSel.NumberSet() > 0) {
		isave->BeginChunk(VERT_SEL_CHUNK);
		VertSel.Save(isave);
		isave->EndChunk();
	}

	/*
	** Save the named selection sets of vertices
	*/
#if 0
	if (VertSelSets.Count() > 0) {
		isave->BeginChunk(INFLUENCE_DATA_CHUNK);
		VertSelSets.Save(isave);
		isave->EndChunk();
	}
#endif

	/*
	** Save the vertex influence data
	*/
	if (VertData.Count() > 0) {
		isave->BeginChunk(INFLUENCE_DATA_CHUNK);
		isave->Write(VertData.Addr(0),VertData.Count() * sizeof(InfluenceStruct), &nb);
		isave->EndChunk();
	}

	return IO_OK;
}


/***********************************************************************************************
 * SkinDataClass::Load -- load the skindata from a MAX file                                    *
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
IOResult SkinDataClass::Load(ILoad *iload)
{
	ULONG nb;
	short flags;
	int n;
	IOResult res;

	while (IO_OK == (res=iload->OpenChunk())) {

		switch (iload->CurChunkID())  {

			case FLAGS_CHUNK:
				res = iload->Read(&flags,sizeof(flags),&nb);
				Valid = (flags & 0x01);
				Held = (flags & 0x02);
				break;

			case VERT_SEL_CHUNK:
				res = VertSel.Load(iload);
				break;

			case NAMED_SEL_SETS_CHUNK:
				res = VertSelSets.Load(iload);
				break;

			case INFLUENCE_DATA_CHUNK:
				n = iload->CurChunkLength() / sizeof(InfluenceStruct);
				VertData.SetCount(n);
				res = iload->Read(VertData.Addr(0),n*sizeof(InfluenceStruct),&nb);
				break;
		}

		iload->CloseChunk();

		if (res != IO_OK) {
			return res;
		}
	}

	/*
	** ensure that the arrays are sized correctly
	*/
	Invalidate();

	return IO_OK;
}

