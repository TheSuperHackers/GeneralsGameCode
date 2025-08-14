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

// FILE: ParticleTypePanels.cpp
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  ParticleTypePanels.cpp                                        */
/* Created:    John K. McDonald, Jr., 3/21/2002                              */
/* Desc:       // @todo                                                      */
/* Revision History:                                                         */
/*		3/21/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "ParticleTypePanels.h"
#include "ParticleEditorDialog.h"
#include <direct.h>

#define ARBITRARY_BUFF_SIZE 128
static const char *PATH = "Art\\Textures\\";
//static const char *PATH = "..\\FinalArt\\Textures\\";
static const char *PREFIX = "EX";
static const char *POSTFIX = "*.*";

// ParticlePanelParticle //////////////////////////////////////////////////////////
ParticlePanelParticle::ParticlePanelParticle(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void ParticlePanelParticle::InitPanel( void )
{
	CFileFind finder;

	CComboBox *pWnd = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleTypeParticle);
	if (!pWnd) {
		return;
	}

	// first, clear out any items.
	pWnd->ResetContent();

	std::string findString;
	findString = PATH;
	findString += PREFIX;
	findString += POSTFIX;
//	DEBUG_LOG(("ParticlePanedParticle::InitPanel - looking for textures, search string is '%s'", findString.begin()));
	BOOL bWorkin = finder.FindFile(findString.c_str());
	while (bWorkin) {
		bWorkin = finder.FindNextFile();
		pWnd->AddString(finder.GetFileName());
	}
}

void ParticlePanelParticle::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update Particle parameters
		CComboBox *pWnd;

		// first Xmin
		pWnd = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleTypeParticle);
		if (pWnd) {
			if (toUI) {
				pParent->getParticleNameFromSystem(buff, ARBITRARY_BUFF_SIZE - 1);
				pWnd->SelectString(-1, buff);
			} else {
				int curSel = pWnd->GetCurSel();
				if (curSel >= 0) {
					pWnd->GetLBText(curSel, buff);
					pParent->updateParticleNameToSystem(buff);
				}
			}
		}
	}
}

void ParticlePanelParticle::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(ParticlePanelParticle, ISwapablePanel)
	ON_CBN_SELCHANGE(IDC_PSEd_ParticleTypeParticle, OnParticleSystemEdit)
END_MESSAGE_MAP()

// ParticlePanelDrawable //////////////////////////////////////////////////////////
ParticlePanelDrawable::ParticlePanelDrawable(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void ParticlePanelDrawable::InitPanel( void )
{

}

void ParticlePanelDrawable::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update Drawable parameters
		CComboBox *pWnd = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleTypeDrawable);
		if (pWnd) {
			if (pWnd->GetCount() == 0) {
				// This is done here because InitPanel is called before ThingTemplates have been sent over.
				std::list<std::string>::const_iterator cit;
				pWnd->AddString(NONE_STRING);
				const std::list<std::string> &r = pParent->getAllThingTemplates();
				for (cit = r.begin(); cit != r.end(); ++cit) {
					pWnd->AddString(cit->c_str());
				}
			}


			if (toUI) {
				pParent->getDrawableNameFromSystem(buff, ARBITRARY_BUFF_SIZE - 1);
				pWnd->SelectString(-1, buff);
			} else {
				int curSel = pWnd->GetCurSel();
				if (curSel >= 0) {
					pWnd->GetLBText(curSel, buff);
					pParent->updateDrawableNameToSystem(buff);
				}
			}
		}
	}
}

void ParticlePanelDrawable::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

void ParticlePanelDrawable::clearAllThingTemplates( void )
{
	CComboBox *pWnd = (CComboBox*) GetDlgItem(IDC_PSEd_ParticleTypeDrawable);
	if (!pWnd) {
		return;
	}

	pWnd->Clear();
}

BEGIN_MESSAGE_MAP(ParticlePanelDrawable, ISwapablePanel)
	ON_CBN_SELCHANGE(IDC_PSEd_ParticleTypeDrawable, OnParticleSystemEdit)
END_MESSAGE_MAP()

// ParticlePanelStreak //////////////////////////////////////////////////////////
ParticlePanelStreak::ParticlePanelStreak(UINT nIDTemplate, CWnd* pParentWnd) : ParticlePanelParticle(nIDTemplate, pParentWnd)
{

}

void ParticlePanelStreak::InitPanel( void )
{
	ParticlePanelParticle::InitPanel();
}

void ParticlePanelStreak::performUpdate( IN Bool toUI )
{
	ParticlePanelParticle::performUpdate(toUI);
}

void ParticlePanelStreak::OnParticleSystemEdit()
{
	ParticlePanelParticle::OnParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(ParticlePanelStreak, ParticlePanelParticle)
	ON_CBN_SELCHANGE(IDC_PSEd_ParticleTypeParticle, OnParticleSystemEdit)
END_MESSAGE_MAP()
