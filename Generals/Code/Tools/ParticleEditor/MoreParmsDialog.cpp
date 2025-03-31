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

// FILE: MoreParmsDialog.cpp 
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  MoreParmsDialog.cpp                                           */
/* Created:    John K. McDonald, Jr., 3/23/2002                              */
/* Desc:       Additional particle system parameters                         */
/* Revision History:                                                         */
/*		3/23/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "MoreParmsDialog.h"
#include "ParticleEditorDialog.h"

#define ARBITRARY_BUFF_SIZE 128

MoreParmsDialog::MoreParmsDialog(UINT nIDTemplate, CWnd* pParentWnd) : CDialog(nIDTemplate, pParentWnd)
{

}

MoreParmsDialog::~MoreParmsDialog()
{

}

void MoreParmsDialog::InitPanel( void )
{
	CComboBox* pCombo;

	pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_WindMotion);
	if (pCombo) {
	  for (int i = 1; WindMotionNames[i]; ++i) {
		  pCombo->AddString(WindMotionNames[i]);
		}
		pCombo->SetCurSel(0);
	}

}

void MoreParmsDialog::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}


	{	// Update all fields on this panel.
		CWnd *pWnd;

		{	// initial delay
			Real initialDelay;

			pWnd = GetDlgItem(IDC_PSEd_InitialDelayMin);
			if (pWnd) {
				if (toUI) {
					pParent->getInitialDelayFromSystem(0, initialDelay);
					sprintf(buff, FORMAT_STRING, initialDelay);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					initialDelay = atof(buff);
					pParent->updateInitialDelayToSystem(0, initialDelay);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_InitialDelayMax);
			if (pWnd) {
				if (toUI) {
					pParent->getInitialDelayFromSystem(1, initialDelay);
					sprintf(buff, FORMAT_STRING, initialDelay);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					initialDelay = atof(buff);
					pParent->updateInitialDelayToSystem(1, initialDelay);
				}
			}
		}

		{	// burst delay
			Real burstDelay;

			pWnd = GetDlgItem(IDC_PSEd_BurstDelayMin);
			if (pWnd) {
				if (toUI) {
					pParent->getBurstDelayFromSystem(0, burstDelay);
					sprintf(buff, FORMAT_STRING, burstDelay);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					burstDelay = atof(buff);
					pParent->updateBurstDelayToSystem(0, burstDelay);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_BurstDelayMax);
			if (pWnd) {
				if (toUI) {
					pParent->getBurstDelayFromSystem(1, burstDelay);
					sprintf(buff, FORMAT_STRING, burstDelay);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					burstDelay = atof(buff);
					pParent->updateBurstDelayToSystem(1, burstDelay);
				}
			}
		}

		{	// burst count
			Real burstCount;

			pWnd = GetDlgItem(IDC_PSEd_BurstCountMin);
			if (pWnd) {
				if (toUI) {
					pParent->getBurstCountFromSystem(0, burstCount);
					sprintf(buff, FORMAT_STRING, burstCount);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					burstCount = atof(buff);
					pParent->updateBurstCountToSystem(0, burstCount);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_BurstCountMax);
			if (pWnd) {
				if (toUI) {
					pParent->getBurstCountFromSystem(1, burstCount);
					sprintf(buff, FORMAT_STRING, burstCount);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					burstCount = atof(buff);
					pParent->updateBurstCountToSystem(1, burstCount);
				}
			}
		}

		{	// color scale
			Real colorScale;

			pWnd = GetDlgItem(IDC_PSEd_ColorScaleMin);
			if (pWnd) {
				if (toUI) {
					pParent->getColorScaleFromSystem(0, colorScale);
					sprintf(buff, FORMAT_STRING, colorScale);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					colorScale = atof(buff);
					pParent->updateColorScaleToSystem(0, colorScale);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_ColorScaleMax);
			if (pWnd) {
				if (toUI) {
					pParent->getColorScaleFromSystem(1, colorScale);
					sprintf(buff, FORMAT_STRING, colorScale);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					colorScale = atof(buff);
					pParent->updateColorScaleToSystem(1, colorScale);
				}
			}
		}

		{	// particle lifetime
			Real particleLifetime;

			pWnd = GetDlgItem(IDC_PSEd_ParticleLifetimeMin);
			if (pWnd) {
				if (toUI) {
					pParent->getParticleLifetimeFromSystem(0, particleLifetime);
					sprintf(buff, FORMAT_STRING, particleLifetime);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					particleLifetime = atof(buff);
					pParent->updateParticleLifetimeToSystem(0, particleLifetime);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_ParticleLifetimeMax);
			if (pWnd) {
				if (toUI) {
					pParent->getParticleLifetimeFromSystem(1, particleLifetime);
					sprintf(buff, FORMAT_STRING, particleLifetime);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					particleLifetime = atof(buff);
					pParent->updateParticleLifetimeToSystem(1, particleLifetime);
				}
			}
		}

		{	// particle size
			Real particleSize;

			pWnd = GetDlgItem(IDC_PSEd_SizeMin);
			if (pWnd) {
				if (toUI) {
					pParent->getParticleSizeFromSystem(0, particleSize);
					sprintf(buff, FORMAT_STRING, particleSize);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					particleSize = atof(buff);
					pParent->updateParticleSizeToSystem(0, particleSize);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_SizeMax);
			if (pWnd) {
				if (toUI) {
					pParent->getParticleSizeFromSystem(1, particleSize);
					sprintf(buff, FORMAT_STRING, particleSize);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					particleSize = atof(buff);
					pParent->updateParticleSizeToSystem(1, particleSize);
				}
			}
		}
	
		{	// start size rate
			Real startSizeRate;

			pWnd = GetDlgItem(IDC_PSEd_StartSizeRateMin);
			if (pWnd) {
				if (toUI) {
					pParent->getStartSizeRateFromSystem(0, startSizeRate);
					sprintf(buff, FORMAT_STRING, startSizeRate);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					startSizeRate = atof(buff);
					pParent->updateStartSizeRateToSystem(0, startSizeRate);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_StartSizeRateMax);
			if (pWnd) {
				if (toUI) {
					pParent->getStartSizeRateFromSystem(1, startSizeRate);
					sprintf(buff, FORMAT_STRING, startSizeRate);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					startSizeRate = atof(buff);
					pParent->updateStartSizeRateToSystem(1, startSizeRate);
				}
			}
		}
	
		{	// size rate
			Real sizeRate;

			pWnd = GetDlgItem(IDC_PSEd_SizeRateMin);
			if (pWnd) {
				if (toUI) {
					pParent->getSizeRateFromSystem(0, sizeRate);
					sprintf(buff, FORMAT_STRING, sizeRate);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					sizeRate = atof(buff);
					pParent->updateSizeRateToSystem(0, sizeRate);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_SizeRateMax);
			if (pWnd) {
				if (toUI) {
					pParent->getSizeRateFromSystem(1, sizeRate);
					sprintf(buff, FORMAT_STRING, sizeRate);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					sizeRate = atof(buff);
					pParent->updateSizeRateToSystem(1, sizeRate);
				}
			}
		}

		{	// size damping
			Real sizeDamping;

			pWnd = GetDlgItem(IDC_PSEd_SizeDampingMin);
			if (pWnd) {
				if (toUI) {
					pParent->getSizeDampingFromSystem(0, sizeDamping);
					sprintf(buff, FORMAT_STRING, sizeDamping);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					sizeDamping = atof(buff);
					pParent->updateSizeDampingToSystem(0, sizeDamping);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_SizeDampingMax);
			if (pWnd) {
				if (toUI) {
					pParent->getSizeDampingFromSystem(1, sizeDamping);
					sprintf(buff, FORMAT_STRING, sizeDamping);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					sizeDamping = atof(buff);
					pParent->updateSizeDampingToSystem(1, sizeDamping);
				}
			}
		}

		{	// system lifetime
			Real systemLifetime;

			pWnd = GetDlgItem(IDC_PSEd_SystemLifetime);
			if (pWnd) {
				if (toUI) {
					pParent->getSystemLifetimeFromSystem(systemLifetime);
					sprintf(buff, FORMAT_STRING, systemLifetime);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					systemLifetime = atof(buff);
					pParent->updateSystemLifetimeToSystem(systemLifetime);
				}
			}
		}

		{	// slave position offset
			Real slaveOffset;

			pWnd = GetDlgItem(IDC_PSEd_SlaveOffsetX);
			if (pWnd) {
				if (toUI) {
					pParent->getSlaveOffsetFromSystem(0, slaveOffset);
					sprintf(buff, FORMAT_STRING, slaveOffset);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					slaveOffset = atof(buff);
					pParent->updateSlaveOffsetToSystem(0, slaveOffset);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_SlaveOffsetY);
			if (pWnd) {
				if (toUI) {
					pParent->getSlaveOffsetFromSystem(1, slaveOffset);
					sprintf(buff, FORMAT_STRING, slaveOffset);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					slaveOffset = atof(buff);
					pParent->updateSlaveOffsetToSystem(1, slaveOffset);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_SlaveOffsetZ);
			if (pWnd) {
				if (toUI) {
					pParent->getSlaveOffsetFromSystem(2, slaveOffset);
					sprintf(buff, FORMAT_STRING, slaveOffset);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					slaveOffset = atof(buff);
					pParent->updateSlaveOffsetToSystem(2, slaveOffset);
				}
			}
		}

		{	// drift velocity
			Real driftVelocity;

			pWnd = GetDlgItem(IDC_PSEd_DriftVelocityX);
			if (pWnd) {
				if (toUI) {
					pParent->getDriftVelocityFromSystem(0, driftVelocity);
					sprintf(buff, FORMAT_STRING, driftVelocity);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					driftVelocity = atof(buff);
					pParent->updateDriftVelocityToSystem(0, driftVelocity);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_DriftVelocityY);
			if (pWnd) {
				if (toUI) {
					pParent->getDriftVelocityFromSystem(1, driftVelocity);
					sprintf(buff, FORMAT_STRING, driftVelocity);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					driftVelocity = atof(buff);
					pParent->updateDriftVelocityToSystem(1, driftVelocity);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_DriftVelocityZ);
			if (pWnd) {
				if (toUI) {
					pParent->getDriftVelocityFromSystem(2, driftVelocity);
					sprintf(buff, FORMAT_STRING, driftVelocity);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					driftVelocity = atof(buff);
					pParent->updateDriftVelocityToSystem(2, driftVelocity);
				}
			}
		}

		{	// slave system
			CComboBox *pCombo;
			pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_SlaveSystem);
			if (pCombo->GetCount() == 0) {
				// This is done here because InitPanel is called before Particle Systems have been sent over.
				pCombo->AddString(NONE_STRING);
				std::list<std::string>::const_iterator cit;
				const std::list<std::string> &r = pParent->getAllParticleSystems();
				for (cit = r.begin(); cit != r.end(); ++cit) {
					pCombo->AddString(cit->c_str());
				}
			}

			if (pCombo) {
				if (toUI) {
					pParent->getSlaveSystemFromSystem(buff, ARBITRARY_BUFF_SIZE - 1);
					if (buff[0] == 0) {
						pCombo->SelectString(-1, NONE_STRING);
					} else {
						pCombo->SelectString(-1, buff);
					}
				} else {
					int selndx = pCombo->GetCurSel();
					if (selndx >= 0) {
						pCombo->GetLBText(selndx, buff);
						if (strcmp(buff, NONE_STRING) == 0) {
							pParent->updateSlaveSystemToSystem("");
						} else {
							pParent->updateSlaveSystemToSystem(buff);
						}
					}
				}
			}
		}

		{	// slave system
			CComboBox *pCombo;
			pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_PerParticleSystem);
			if (pCombo->GetCount() == 0) {
				// This is done here because InitPanel is called before Particle Systems have been sent over.
				pCombo->AddString(NONE_STRING);
				std::list<std::string>::const_iterator cit;
				const std::list<std::string> &r = pParent->getAllParticleSystems();
				for (cit = r.begin(); cit != r.end(); ++cit) {
					pCombo->AddString(cit->c_str());
				}
			}

			if (pCombo) {
				if (toUI) {
					pParent->getPerParticleSystemFromSystem(buff, ARBITRARY_BUFF_SIZE - 1);
					if (buff[0] == 0) {
						pCombo->SelectString(-1, NONE_STRING);
					} else {
						pCombo->SelectString(-1, buff);
					}
				} else {
					int selndx = pCombo->GetCurSel();
					if (selndx >= 0) {
						pCombo->GetLBText(selndx, buff);
						if (strcmp(buff, NONE_STRING) == 0) {
							pParent->updatePerParticleSystemToSystem("");
						} else {
							pParent->updatePerParticleSystemToSystem(buff);
						}
					}
				}
			}
		}
	
		{	// ping pong wind start angle
			Real angle;

			pWnd = GetDlgItem(IDC_PSEd_WindPingPongStartAngleMin);
			if (pWnd) {
				if (toUI) {
					pParent->getPingPongStartAngleFromSystem(0, angle);
					sprintf(buff, FORMAT_STRING, angle);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					angle = atof(buff);
					pParent->updatePingPongStartAngleToSystem(0, angle);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_WindPingPongStartAngleMax);
			if (pWnd) {
				if (toUI) {
					pParent->getPingPongStartAngleFromSystem(1, angle);
					sprintf(buff, FORMAT_STRING, angle);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					angle = atof(buff);
					pParent->updatePingPongStartAngleToSystem(1, angle);
				}
			}
		}

		{	// ping pong wind end angle
			Real angle;

			pWnd = GetDlgItem(IDC_PSEd_WindPingPongEndAngleMin);
			if (pWnd) {
				if (toUI) {
					pParent->getPingPongEndAngleFromSystem(0, angle);
					sprintf(buff, FORMAT_STRING, angle);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					angle = atof(buff);
					pParent->updatePingPongEndAngleToSystem(0, angle);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_WindPingPongEndAngleMax);
			if (pWnd) {
				if (toUI) {
					pParent->getPingPongEndAngleFromSystem(1, angle);
					sprintf(buff, FORMAT_STRING, angle);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					angle = atof(buff);
					pParent->updatePingPongEndAngleToSystem(1, angle);
				}
			}
		}

		{	// wind angle change
			Real angle;

			pWnd = GetDlgItem(IDC_PSEd_WindAngleChangeMin);
			if (pWnd) {
				if (toUI) {
					pParent->getWindAngleChangeFromSystem(0, angle);
					sprintf(buff, FORMAT_STRING, angle);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					angle = atof(buff);
					pParent->updateWindAngleChangeToSystem(0, angle);
				}
			}

			pWnd = GetDlgItem(IDC_PSEd_WindAngleChangeMax);
			if (pWnd) {
				if (toUI) {
					pParent->getWindAngleChangeFromSystem(1, angle);
					sprintf(buff, FORMAT_STRING, angle);
					pWnd->SetWindowText(buff);
				} else {
					pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					angle = atof(buff);
					pParent->updateWindAngleChangeToSystem(1, angle);
				}
			}
		}

		{	// wind motion
			CComboBox *pCombo;
			pCombo = (CComboBox*) GetDlgItem(IDC_PSEd_WindMotion);

			if (pCombo) {
				int selndx;
				if (toUI) {
					ParticleSystemInfo::WindMotion windMotion;

					pParent->getWindMotionFromSystem( windMotion );
					selndx = pCombo->SelectString(-1, WindMotionNames[(long) windMotion]);
				} else {
					selndx = pCombo->GetCurSel();
					if (selndx >= 0) {
						pParent->updateWindMotionToSystem( (ParticleSystemInfo::WindMotion)(selndx + 1) );
					}
				}
			}
		}


	}
}

void MoreParmsDialog::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}
	
	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(MoreParmsDialog, CDialog)
	ON_EN_KILLFOCUS(IDC_PSEd_InitialDelayMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_InitialDelayMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_BurstDelayMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_BurstDelayMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_BurstCountMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_BurstCountMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_ColorScaleMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_ColorScaleMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_ParticleLifetimeMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_ParticleLifetimeMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SizeMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SizeMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_StartSizeRateMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_StartSizeRateMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SizeRateMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SizeRateMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SizeDampingMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SizeDampingMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SystemLifetime, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SlaveOffsetX, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SlaveOffsetY, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SlaveOffsetZ, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_DriftVelocityX, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_DriftVelocityY, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_DriftVelocityZ, OnParticleSystemEdit)
	ON_CBN_SELCHANGE(IDC_PSEd_SlaveSystem, OnParticleSystemEdit)
	ON_CBN_SELCHANGE(IDC_PSEd_PerParticleSystem, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_WindAngleChangeMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_WindAngleChangeMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_WindPingPongStartAngleMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_WindPingPongStartAngleMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_WindPingPongEndAngleMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_WindPingPongEndAngleMax, OnParticleSystemEdit)
	ON_CBN_SELCHANGE(IDC_PSEd_WindMotion, OnParticleSystemEdit)
END_MESSAGE_MAP()
