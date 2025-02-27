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

#include "StdAfx.h"
#include "Resource.h"
#include "CSwitchesDialog.h"

#include "ParticleEditorDialog.h"

CSwitchesDialog::CSwitchesDialog(UINT nIDTemplate, CWnd* pParentWnd) : CDialog(nIDTemplate, pParentWnd)
{

}

void CSwitchesDialog::InitPanel( void )
{

}

// if true, updates the UI from the Particle System.
// if false, updates the Particle System from the UI
void CSwitchesDialog::performUpdate( IN Bool toUI )
{
	DebugWindowDialog *parent = GetDWDParent();
	if (!parent) {
		return;
	}

	{ // update hollowness
		CButton *pWnd;
		pWnd = (CButton*)GetDlgItem(IDC_PSEd_Hollow);
		if (pWnd) {
			Bool hollow;
			if (toUI) {
				parent->getSwitchFromSystem(ST_HOLLOW, hollow);
				pWnd->SetCheck(hollow);
			} else {
				hollow = pWnd->GetCheck();
				parent->updateSwitchToSystem(ST_HOLLOW, hollow);
			}
		}
	}

	{ // update one shot
		CButton *pWnd;
		pWnd = (CButton*)GetDlgItem(IDC_PSEd_OneShot);
		if (pWnd) {
			Bool oneShot;
			if (toUI) {
				parent->getSwitchFromSystem(ST_ONESHOT, oneShot);
				pWnd->SetCheck(oneShot);
			} else {
				oneShot = pWnd->GetCheck();
				parent->updateSwitchToSystem(ST_ONESHOT, oneShot);
			}
		}
	}

	{ // update Ground Aligned
		CButton *pWnd;
		pWnd = (CButton*)GetDlgItem(IDC_PSEd_GroundAligned);
		if (pWnd) {
			Bool groundAlign;
			if (toUI) {
				parent->getSwitchFromSystem(ST_ALIGNXY, groundAlign);
				pWnd->SetCheck(groundAlign);
			} else {
				groundAlign = pWnd->GetCheck();
				parent->updateSwitchToSystem(ST_ALIGNXY, groundAlign);
			}
		}
	}

	{ // update Emit above ground only
		CButton *pWnd;
		pWnd = (CButton*)GetDlgItem(IDC_PSEd_EmitAboveGroundOnly);
		if (pWnd) {
			Bool aboveGroundOnly;
			if (toUI) {
				parent->getSwitchFromSystem(ST_EMITABOVEGROUNDONLY, aboveGroundOnly);
				pWnd->SetCheck(aboveGroundOnly);
			} else {
				aboveGroundOnly = pWnd->GetCheck();
				parent->updateSwitchToSystem(ST_EMITABOVEGROUNDONLY, aboveGroundOnly);
			}
		}
	}

	{ // update Particle Up towards emitter
		CButton *pWnd;
		pWnd = (CButton*)GetDlgItem(IDC_PSEd_ParticleUpTowardsEmitter);
		if (pWnd) {
			Bool upTowardsEmitter;
			if (toUI) {
				parent->getSwitchFromSystem(ST_PARTICLEUPTOWARDSEMITTER, upTowardsEmitter);
				pWnd->SetCheck(upTowardsEmitter);
			} else {
				upTowardsEmitter = pWnd->GetCheck();
				parent->updateSwitchToSystem(ST_PARTICLEUPTOWARDSEMITTER, upTowardsEmitter);
			}
		}
	}
}

void CSwitchesDialog::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = GetDWDParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(CSwitchesDialog, CDialog)
	ON_BN_CLICKED(IDC_PSEd_OneShot, OnParticleSystemEdit)
	ON_BN_CLICKED(IDC_PSEd_Hollow, OnParticleSystemEdit)
	ON_BN_CLICKED(IDC_PSEd_GroundAligned, OnParticleSystemEdit)
	ON_BN_CLICKED(IDC_PSEd_EmitAboveGroundOnly, OnParticleSystemEdit)
	ON_BN_CLICKED(IDC_PSEd_ParticleUpTowardsEmitter, OnParticleSystemEdit)
END_MESSAGE_MAP()
