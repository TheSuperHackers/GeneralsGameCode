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

// FILE: ParticleTypePanels.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  ParticleTypePanels.h                                                      */
/* Created:    John K. McDonald, Jr., 3/21/2002                               */
/* Desc:       // @todo                                                      */
/* Revision History:                                                         */
/*		3/21/2002 : Initial creation                                          */
/*---------------------------------------------------------------------------*/

#pragma once

#include "Resource.h"
#include "ISwapablePanel.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

// ParticlePanelParticle //////////////////////////////////////////////////////////
class ParticlePanelParticle : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_ParticlePanelParticle};
		virtual DWORD GetIDD( void ) { return IDD; }
		ParticlePanelParticle(UINT nIDTemplate = ParticlePanelParticle::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// ParticlePanelDrawable //////////////////////////////////////////////////////////
class ParticlePanelDrawable : public ISwapablePanel
{
	public:
		enum {IDD = IDD_PSEd_ParticlePanelDrawable};
		virtual DWORD GetIDD( void ) { return IDD; }
		ParticlePanelDrawable(UINT nIDTemplate = ParticlePanelDrawable::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );
		void clearAllThingTemplates( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

// ParticlePanelStreak //////////////////////////////////////////////////////////
class ParticlePanelStreak : public ParticlePanelParticle
{
	public:
		enum {IDD = IDD_PSEd_ParticlePanelStreak};
		virtual DWORD GetIDD( void ) { return IDD; }
		ParticlePanelStreak(UINT nIDTemplate = ParticlePanelStreak::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );
	protected:
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};
