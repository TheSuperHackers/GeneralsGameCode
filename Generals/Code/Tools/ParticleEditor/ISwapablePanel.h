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

// FILE: ISwapablePanel.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  ISwapablePanel.h                                              */
/* Created:    John K. McDonald, Jr., 3/21/2002                              */
/* Desc:       Swapable panels derive from this so that we can easily call   */
/*						 the update function                                           */
/* Revision History:                                                         */
/*		3/21/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#pragma once

#include "Lib/BaseType.h"

// INCLUDES ///////////////////////////////////////////////////////////////////
// DEFINES ////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

interface ISwapablePanel : public CDialog
{
	ISwapablePanel(UINT nIDTemplate = 0, CWnd* pParentWnd = NULL) : CDialog(nIDTemplate, pParentWnd) {}
	virtual DWORD GetIDD( void ) = 0;
	virtual void performUpdate( IN Bool toUI ) = 0;
	virtual void InitPanel( void ) = 0;
};
