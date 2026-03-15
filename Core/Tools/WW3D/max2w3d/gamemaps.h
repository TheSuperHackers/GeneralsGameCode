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

/* $Header: /Commando/Code/Tools/max2w3d/gamemaps.h 7     10/28/97 6:08p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : GAMEMAPS.h                                                   *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/26/97                                                     *
 *                                                                                             *
 *                  Last Update : June 26, 1997 [GH]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <max.h>
#include "stdmat.h"


ClassDesc * Get_Game_Maps_Desc();


///////////////////////////////////////////////////////////////////////////
//
//		TexmapSlotClass
//
///////////////////////////////////////////////////////////////////////////
class TexmapSlotClass
{
public:

	BOOL			MapOn;
	float			Amount;
	Texmap *		Map;

	TexmapSlotClass() : MapOn(FALSE), Amount(1.0f), Map(nullptr) {};

	RGBA		Eval(ShadeContext& sc)						{ return Map->EvalColor(sc); 	}
	float		EvalMono(ShadeContext& sc) 				{ return Map->EvalMono(sc); }
	Point3	EvalNormalPerturb(ShadeContext &sc) 	{ return Map->EvalNormalPerturb(sc); }
	BOOL		IsActive() 										{ return (Map && MapOn); }
	void		Update(TimeValue t, Interval &ivalid)	{ if (IsActive()) Map->Update(t,ivalid); };
	float		GetAmount(TimeValue t) 						{ return Amount; }
};


///////////////////////////////////////////////////////////////////////////
//
//		Texture Maps for In-Game material
//
//		This class can contain a collection of all of the maps which
//		MAX uses but the GameMtl plugin will only give the user access
//		to the ones we can actually use in the game.
//
///////////////////////////////////////////////////////////////////////////
class GameMapsClass: public ReferenceTarget
{
public:

	MtlBase *			Client;
	TexmapSlotClass	TextureSlot[NTEXMAPS];

	GameMapsClass()				 												{ Client = nullptr; }
	GameMapsClass(MtlBase *mb)	 												{ Client = mb;	}

	void					DeleteThis()											{ delete this;	}
	void					SetClientPtr(MtlBase *mb)							{ Client = mb; }
	TexmapSlotClass &	operator[](int i) 									{ return TextureSlot[i]; }
	Class_ID				ClassID();
	SClass_ID			SuperClassID() 										{ return REF_MAKER_CLASS_ID; }
	int					NumSubs()												{ return NTEXMAPS; }
	Animatable * 		SubAnim(int i)											{ return TextureSlot[i].Map; }
	TSTR					SubAnimName(int i)									{ return Client->GetSubTexmapTVName(i); }
	int					NumRefs()												{ return NTEXMAPS; }
	RefTargetHandle	GetReference(int i)									{ return TextureSlot[i].Map; }
	void					SetReference(int i, RefTargetHandle rtarg)	{ TextureSlot[i].Map = (Texmap*)rtarg; }
	int					SubNumToRefNum(int subNum) 						{ return subNum; }


	BOOL					AssignController(Animatable *control,int subAnim);
	RefTargetHandle	Clone(RemapDir &remap);
	RefResult			NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	IOResult				Save(ISave * isave);
	IOResult				Load(ILoad * iload);
};
