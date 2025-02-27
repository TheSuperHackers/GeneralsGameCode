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

#pragma once

#include "CButtonShowColor.h"
#include "GameClient/ParticleSys.h"

class CColorAlphaDialog : public CDialog
{
	protected:
		DWORD m_customColors[16];
		CButtonShowColor m_colorButton[MAX_KEYFRAMES];

		void onColorPress( Int colorPressed );
	public:
		enum {IDD = IDD_PSEd_EditColorAndAlpha};
		CColorAlphaDialog(UINT nIDTemplate = CColorAlphaDialog::IDD, CWnd* pParentWnd = NULL);

		void InitPanel( void );

		// if true, updates the UI from the Particle System.
		// if false, updates the Particle System from the UI
		void performUpdate( IN Bool toUI );


	protected:
		virtual BOOL OnInitDialog();

		afx_msg void OnColor1();
		afx_msg void OnColor2();
		afx_msg void OnColor3();
		afx_msg void OnColor4();
		afx_msg void OnColor5();
		afx_msg void OnColor6();
		afx_msg void OnColor7();
		afx_msg void OnColor8();
		afx_msg void OnParticleSystemEdit();
		DECLARE_MESSAGE_MAP()
};

