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

/* $Header: /Commando/Code/Tools/max2w3d/namedsel.cpp 4     10/28/97 6:08p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - WWSkin                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/namedsel.cpp                   $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/26/97 1:29p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "namedsel.h"


NamedSelSetList::~NamedSelSetList()
{
	for (int i=0; i<Sets.Count(); i++) {
		delete Sets[i];
		Sets[i] = nullptr;
		delete Names[i];
		Names[i] = nullptr;
	}
}

void NamedSelSetList::Append_Set(BitArray & nset,TSTR & setname)
{
	BitArray *n = new BitArray(nset);
	Sets.Append(1,&n);

	TSTR * name = new TSTR(setname);
	Names.Append(1,&name);

	assert(Names.Count() == Sets.Count());
}

void NamedSelSetList::Delete_Set(int i)
{
	delete Sets[i];
	Sets.Delete(i,1);

	delete Names[i];
	Names.Delete(i,1);

	assert(Names.Count() == Sets.Count());
}

void NamedSelSetList::Delete_Set(TSTR & setname)
{
	int i = Find_Set(setname);
	if (i >= 0) Delete_Set(i);
}

void NamedSelSetList::Reset(void)
{
	while (Sets.Count() > 0) {
		Delete_Set(0);
	}
}

void NamedSelSetList::Set_Size(int size)
{
	for (int i=0; i<Sets.Count(); i++) {
		Sets[i]->SetSize(size,TRUE);
	}
}

NamedSelSetList& NamedSelSetList::operator=(NamedSelSetList& from)
{
	for (int i=0; i<Sets.Count(); i++) {
		Delete_Set(i);
	}
	Sets.SetCount(0);
	Names.SetCount(0);

	for (i=0; i<from.Count(); i++) {
		Append_Set(from[i],*(from.Names[i]));
	}

	return *this;
}


int NamedSelSetList::Find_Set(TSTR &setname)
{
	for (int i=0; i<Names.Count(); i++) {
		if (setname == *Names[i]) {
			return i;
		}
	}
	return -1;
}


IOResult NamedSelSetList::Save(ISave *isave)
{
	assert(Sets.Count() == Names.Count());

	for (int i=0; i<Sets.Count(); i++) {

		isave->BeginChunk(NAMED_SEL_SET_CHUNK);

			isave->BeginChunk(NAMED_SEL_NAME_CHUNK);
			isave->WriteWString(*Names[i]);
			isave->EndChunk();

			isave->BeginChunk(NAMED_SEL_BITS_CHUNK);
			Sets[i]->Save(isave);
			isave->EndChunk();

		isave->EndChunk();

	}
	return IO_OK;
}


IOResult NamedSelSetList::Load(ILoad *iload)
{
	IOResult res;

	while (IO_OK==(res=iload->OpenChunk())) {

		switch (iload->CurChunkID())  {

			case NAMED_SEL_SET_CHUNK:
				res = Load_Set(iload);
				break;

			default:
				assert(0);
				break;
		}

		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}
	return IO_OK;
}

IOResult NamedSelSetList::Load_Set(ILoad * iload)
{
	IOResult res;
	BitArray set;
	TCHAR * name;

	BOOL gotset = FALSE;
	BOOL gotname = FALSE;

	res = iload->OpenChunk();

	while (IO_OK==(res=iload->OpenChunk())) {

		switch (iload->CurChunkID())  {

			case NAMED_SEL_BITS_CHUNK:
			{
				res = set.Load(iload);
				gotset = TRUE;
				break;
			}

			case NAMED_SEL_NAME_CHUNK:
			{
				res = iload->ReadWStringChunk(&name);
				gotname = TRUE;
				break;
			}
		}
		iload->CloseChunk();
		if (res != IO_OK) return res;
	}

	assert(gotset && gotname);
	Append_Set(set,TSTR(name));

	return IO_OK;
}
