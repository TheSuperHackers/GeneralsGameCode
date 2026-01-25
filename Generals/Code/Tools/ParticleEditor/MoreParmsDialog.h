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

// FILE: MoreParmsDialog.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  MoreParmsDialog.h                                                      */
/* Created:    John K. McDonald, Jr., 3/23/2002                               */
/* Desc:       // @todo                                                      */
/* Revision History:                                                         */
/*		3/23/2002 : Initial creation                                          */
/*---------------------------------------------------------------------------*/

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "Resource.h"
#include "Lib/BaseType.h"

// DEFINES ////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

class MoreParmsDialog : public CDialog
{
	public:
		enum { IDD = IDD_PSEd_EditMoreParms };
		MoreParmsDialog(UINT nIDTemplate = MoreParmsDialog::IDD, CWnd* pParentWnd = nullptr);
		virtual ~MoreParmsDialog();

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );

	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};
