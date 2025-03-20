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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: GameSpyGP.h //////////////////////////////////////////////////////
// Generals GameSpy GP (Buddy)
// Author: Matthew D. Campbell, March 2002

#pragma once

#ifndef __GAMESPYGP_H__
#define __GAMESPYGP_H__

#include "gamespy/gp/gp.h"

void GPRecvBuddyRequestCallback(GPConnection * connection, GPRecvBuddyRequestArg * arg, void * param);
void GPRecvBuddyMessageCallback(GPConnection * pconnection, GPRecvBuddyMessageArg * arg, void * param);
void GPRecvBuddyStatusCallback(GPConnection * connection, GPRecvBuddyStatusArg * arg, void * param);
void GPErrorCallback(GPConnection * pconnection, GPErrorArg * arg, void * param);
void GPConnectCallback(GPConnection * pconnection, GPConnectResponseArg * arg, void * param);
void GameSpyUpdateBuddyOverlay(void);

extern GPConnection *TheGPConnection;

Bool IsGameSpyBuddy(GPProfile id);

#endif // __GAMESPYGP_H__
