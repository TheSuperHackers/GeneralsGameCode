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

// FILE: EmissionTypePanels.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  EmissionTypePanels.h                                          */
/* Created:    John K. McDonald, Jr., 3/21/2002                              */
/* Desc:       Emission panels are pretty similar, they all go here.         */
/* Revision History:                                                         */
/*		3/21/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#pragma once
#ifndef _H_EMISSIONTYPEPANELS_
#define _H_EMISSIONTYPEPANELS_

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "Resource.h"
#include "ISwapablePanel.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

// EmissionPanelPoint //////////////////////////////////////////////////////////
class EmissionPanelPoint : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_EmissionPanelPoint};
		virtual DWORD GetIDD( void ) { return IDD; }
		EmissionPanelPoint(UINT nIDTemplate = EmissionPanelPoint::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// EmissionPanelLine //////////////////////////////////////////////////////////
class EmissionPanelLine : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_EmissionPanelLine};
		virtual DWORD GetIDD( void ) { return IDD; }
		EmissionPanelLine(UINT nIDTemplate = EmissionPanelLine::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// EmissionPanelBox ///////////////////////////////////////////////////////////
class EmissionPanelBox : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_EmissionPanelBox};
		virtual DWORD GetIDD( void ) { return IDD; }
		EmissionPanelBox(UINT nIDTemplate = EmissionPanelBox::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// EmissionPanelSphere ////////////////////////////////////////////////////////
class EmissionPanelSphere : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_EmissionPanelSphere};
		virtual DWORD GetIDD( void ) { return IDD; }
		EmissionPanelSphere(UINT nIDTemplate = EmissionPanelSphere::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// EmissionPanelCylinder //////////////////////////////////////////////////////
class EmissionPanelCylinder : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_EmissionPanelCylinder};
		virtual DWORD GetIDD( void ) { return IDD; }
		EmissionPanelCylinder(UINT nIDTemplate = EmissionPanelCylinder::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

#endif /* _H_EMISSIONTYPEPANELS_ */
