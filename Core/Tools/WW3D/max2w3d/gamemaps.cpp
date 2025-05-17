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

/* $Header: /Commando/Code/Tools/max2w3d/gamemaps.cpp 5     10/28/97 6:08p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D engine                                       * 
 *                                                                                             * 
 *                    File Name : GAMEMAPS.CPP                                                 * 
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Start Date : 06/26/97                                                     * 
 *                                                                                             * 
 *                  Last Update : June 26, 1997 [GH]                                           * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   GameMapsClass::ClassID -- Returns the ClassID for GameMapsClass                           * 
 *   GameMapsClass::AssignController -- Assigns a controller to one of the Sub-Anims           * 
 *   GameMapsClass::NotifyRefChanged -- Max is notifying GameMapsClass that a reference has cha* 
 *   GameMapsClass::Clone -- Create a clone of the GameMapsClass                               * 
 *   GameMapsClass::Save -- Saves the GameMapsClass data into a MAX file                       * 
 *   GameMapsClass::Load -- Loads GameMapsClass data from a MAX file                           * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "gamemaps.h"


/*****************************************************************
*
*		Chunk ID's for saving data in the MAX file.
*
*****************************************************************/
#define GAMEMAPS_ONOFF_CHUNK	0x0000
#define GAMEMAPS_AMT0_CHUNK	0x0001
#define GAMEMAPS_AMT1_CHUNK	0x0002
#define GAMEMAPS_AMT2_CHUNK	0x0003
#define GAMEMAPS_AMT3_CHUNK	0x0004
#define GAMEMAPS_AMT4_CHUNK	0x0005
#define GAMEMAPS_AMT5_CHUNK	0x0006
#define GAMEMAPS_AMT6_CHUNK	0x0007
#define GAMEMAPS_AMT7_CHUNK	0x0008
#define GAMEMAPS_AMT8_CHUNK	0x0009
#define GAMEMAPS_AMT9_CHUNK	0x000A
#define GAMEMAPS_AMTA_CHUNK	0x000B


/*****************************************************************
*
*		A PostLoadCallback which does nothing...
*
*****************************************************************/
class GameMapsPostLoad : public PostLoadCallback 
{
public:
	GameMapsClass *tm;
	GameMapsPostLoad(GameMapsClass *b) {tm=b;}
	void proc(ILoad *iload) { delete this; } 
};


/*****************************************************************
*
*		GameMapsClass Class Desriptor
*
*****************************************************************/
static Class_ID _GameMapsClassID(0x36d23f7b, 0x79ce63e1);

class GameMapsClassDesc : public ClassDesc 
{
	public:
	int 				IsPublic()			  		{ return 0; }
	void *			Create(BOOL loading)		{ return new GameMapsClass(NULL); }
	const TCHAR *	ClassName()					{ return _T("GameMaps"); }
	SClass_ID		SuperClassID()				{ return REF_MAKER_CLASS_ID; }
	Class_ID 		ClassID()			  		{ return _GameMapsClassID; }
	const TCHAR* 	Category()					{ return _T(""); }
};

static GameMapsClassDesc _GameMapsCD;

ClassDesc * Get_Game_Maps_Desc() { return &_GameMapsCD; }


/*********************************************************************************************** 
 * GameMapsClass::ClassID -- Returns the ClassID for GameMapsClass                             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
Class_ID GameMapsClass::ClassID() 
{
	return _GameMapsClassID; 
}

/*********************************************************************************************** 
 * GameMapsClass::AssignController -- Assigns a controller to one of the Sub-Anims             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
BOOL GameMapsClass::AssignController(Animatable *control,int subAnim)
{
	ReplaceReference(SubNumToRefNum(subAnim),(ReferenceTarget *)control);
	return TRUE;
}

/*********************************************************************************************** 
 * GameMapsClass::NotifyRefChanged -- Max is notifying GameMapsClass that a reference has chan * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
RefResult GameMapsClass::NotifyRefChanged
(
	Interval				changeInt, 
	RefTargetHandle	hTarget,
	PartID &				partID, 
	RefMessage			message 		
) 
{
	switch (message) {
		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			break;
		}
		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			return REF_STOP; 
		}
	}
	return(REF_SUCCEED);
}

/*********************************************************************************************** 
 * GameMapsClass::Clone -- Create a clone of the GameMapsClass                                 * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
RefTargetHandle GameMapsClass::Clone(RemapDir &remap) 
{
	GameMapsClass *tm = new GameMapsClass(NULL);

	for (int i=0; i<NTEXMAPS; i++) {
		tm->TextureSlot[i].MapOn = TextureSlot[i].MapOn;
		tm->TextureSlot[i].Map = NULL;

		if (TextureSlot[i].Map) {
			tm->ReplaceReference(i,remap.CloneRef(TextureSlot[i].Map));
		}
	}

	return tm;
}


/*********************************************************************************************** 
 * GameMapsClass::Save -- Saves the GameMapsClass data into a MAX file                         * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
IOResult GameMapsClass::Save(ISave * isave) 
{
	ULONG nb,f=0;

	isave->BeginChunk(GAMEMAPS_ONOFF_CHUNK);
	for (int i=0; i<NTEXMAPS; i++) {
		if (TextureSlot[i].MapOn) f|= (1<<i);
	}
	isave->Write(&f,sizeof(f),&nb);			
	isave->EndChunk();

	for (i=0; i<NTEXMAPS; i++) {
		if (TextureSlot[i].Amount != 1.0f) {
			isave->BeginChunk(GAMEMAPS_AMT0_CHUNK + i);
			isave->Write(&(TextureSlot[i].Amount),sizeof(float),&nb);
			isave->EndChunk();
		}
	}
	return IO_OK;
}


/*********************************************************************************************** 
 * GameMapsClass::Load -- Loads GameMapsClass data from a MAX file                             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
IOResult GameMapsClass::Load(ILoad * iload) 
{ 
	ULONG nb;
	int id;
	IOResult res;

	while (IO_OK==(res=iload->OpenChunk())) {

		switch (id = iload->CurChunkID())  {
			case GAMEMAPS_ONOFF_CHUNK:
				{
					ULONG f;
					res = iload->Read(&f,sizeof(f), &nb);
					for (int i=0; i<NTEXMAPS; i++) 
						 (*this)[i].MapOn = (f&(1<<i))?1:0;
				}
				break;
			
			case GAMEMAPS_AMT0_CHUNK:
			case GAMEMAPS_AMT1_CHUNK:
			case GAMEMAPS_AMT2_CHUNK:
			case GAMEMAPS_AMT3_CHUNK:
			case GAMEMAPS_AMT4_CHUNK:
			case GAMEMAPS_AMT5_CHUNK:
			case GAMEMAPS_AMT7_CHUNK:
			case GAMEMAPS_AMT8_CHUNK:
			case GAMEMAPS_AMT9_CHUNK:
			case GAMEMAPS_AMTA_CHUNK:
				int index = id - GAMEMAPS_AMT0_CHUNK;
				res = iload->Read(&(TextureSlot[index].Amount),sizeof(float),&nb);
				break;
		}

		iload->CloseChunk();
		if (res!=IO_OK) {
			return res;
		}
	}
	return IO_OK;
}

