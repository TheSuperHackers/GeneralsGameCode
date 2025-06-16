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

// FILE: VelocityTypePanels.h 
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  VelocityTypePanels.h                                                      */
/* Created:    John K. McDonald, Jr., 3/21/2002                               */
/* Desc:       // @todo                                                      */
/* Revision History:                                                         */
/*		3/21/2002 : Initial creation                                          */
/*---------------------------------------------------------------------------*/

#pragma once
#ifndef _H_VELOCITYTYPEPANELS_
#define _H_VELOCITYTYPEPANELS_

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "Resource.h"
#include "ISwapablePanel.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

// VelocityPanelOrtho //////////////////////////////////////////////////////////
class VelocityPanelOrtho : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_VelocityPanelOrtho};
		virtual DWORD GetIDD( void ) { return IDD; }
		VelocityPanelOrtho(UINT nIDTemplate = VelocityPanelOrtho::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System. 
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );	
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// VelocityPanelSphere ////////////////////////////////////////////////////////
class VelocityPanelSphere : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_VelocityPanelSphere};
		virtual DWORD GetIDD( void ) { return IDD; }
		VelocityPanelSphere(UINT nIDTemplate = VelocityPanelSphere::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System. 
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );	
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// VelocityPanelHemisphere //////////////////////////////////////////////////////////
class VelocityPanelHemisphere : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_VelocityPanelHemisphere};
		virtual DWORD GetIDD( void ) { return IDD; }
		VelocityPanelHemisphere(UINT nIDTemplate = VelocityPanelHemisphere::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System. 
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );	
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// VelocityPanelCylinder //////////////////////////////////////////////////////
class VelocityPanelCylinder : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_VelocityPanelCylinder};
		virtual DWORD GetIDD( void ) { return IDD; }
		VelocityPanelCylinder(UINT nIDTemplate = VelocityPanelCylinder::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System. 
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );	
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// VelocityPanelOutward ///////////////////////////////////////////////////////////
class VelocityPanelOutward : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_VelocityPanelOutward};
		virtual DWORD GetIDD( void ) { return IDD; }
		VelocityPanelOutward(UINT nIDTemplate = VelocityPanelOutward::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System. 
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );	
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

#endif /* _H_VELOCITYTYPEPANELS_ */
