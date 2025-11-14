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


// FILE: VelocityTypePanels.cpp
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  VelocityTypePanels.cpp                                        */
/* Created:    John K. McDonald, Jr., 3/21/2002                              */
/* Desc:       // @todo                                                      */
/* Revision History:                                                         */
/*		3/21/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "VelocityTypePanels.h"
#include "ParticleEditorDialog.h"

#define ARBITRARY_BUFF_SIZE 128

// VelocityPanelOrtho //////////////////////////////////////////////////////////
VelocityPanelOrtho::VelocityPanelOrtho(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void VelocityPanelOrtho::InitPanel( void )
{

}

void VelocityPanelOrtho::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update ortho parameters
		Real ortho;
		CWnd *pWnd;

		// first Xmin
		pWnd = GetDlgItem(IDC_PSEd_OrthoXMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOrthoFromSystem(0, ortho);

				sprintf(buff, FORMAT_STRING, ortho);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				ortho = atof(buff);
				pParent->updateVelOrthoToSystem(0, ortho);
			}
		}

		// now the Ymin
		pWnd = GetDlgItem(IDC_PSEd_OrthoYMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOrthoFromSystem(1, ortho);

				sprintf(buff, FORMAT_STRING, ortho);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				ortho = atof(buff);
				pParent->updateVelOrthoToSystem(1, ortho);
			}
		}

		// now the Zmin
		pWnd = GetDlgItem(IDC_PSEd_OrthoZMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOrthoFromSystem(2, ortho);

				sprintf(buff, FORMAT_STRING, ortho);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				ortho = atof(buff);
				pParent->updateVelOrthoToSystem(2, ortho);
			}
		}

		// first the Xmax
		pWnd = GetDlgItem(IDC_PSEd_OrthoXMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOrthoFromSystem(3, ortho);

				sprintf(buff, FORMAT_STRING, ortho);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				ortho = atof(buff);
				pParent->updateVelOrthoToSystem(3, ortho);
			}
		}

		// now the Ymax
		pWnd = GetDlgItem(IDC_PSEd_OrthoYMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOrthoFromSystem(4, ortho);

				sprintf(buff, FORMAT_STRING, ortho);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				ortho = atof(buff);
				pParent->updateVelOrthoToSystem(4, ortho);
			}
		}

		// the Zmax
		pWnd = GetDlgItem(IDC_PSEd_OrthoZMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOrthoFromSystem(5, ortho);

				sprintf(buff, FORMAT_STRING, ortho);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				ortho = atof(buff);
				pParent->updateVelOrthoToSystem(5, ortho);
			}
		}
	}
}

void VelocityPanelOrtho::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(VelocityPanelOrtho, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_OrthoXMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_OrthoYMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_OrthoZMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_OrthoXMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_OrthoYMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_OrthoZMax, OnParticleSystemEdit)
END_MESSAGE_MAP()

// VelocityPanelSphere ////////////////////////////////////////////////////////
VelocityPanelSphere::VelocityPanelSphere(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void VelocityPanelSphere::InitPanel( void )
{

}

void VelocityPanelSphere::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update Sphere Velocity Parameters
		Real radial;
		CWnd *pWnd;

		// radial min
		pWnd = GetDlgItem(IDC_PSEd_SphereRadialMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelSphereFromSystem(0, radial);

				sprintf(buff, FORMAT_STRING, radial);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				radial = atof(buff);
				pParent->updateVelSphereToSystem(0, radial);
			}
		}

		// radial max
		pWnd = GetDlgItem(IDC_PSEd_SphereRadialMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelSphereFromSystem(1, radial);

				sprintf(buff, FORMAT_STRING, radial);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				radial = atof(buff);
				pParent->updateVelSphereToSystem(1, radial);
			}
		}
	}
}

void VelocityPanelSphere::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(VelocityPanelSphere, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_SphereRadialMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_SphereRadialMax, OnParticleSystemEdit)
END_MESSAGE_MAP()


// VelocityPanelHemisphere //////////////////////////////////////////////////////////
VelocityPanelHemisphere::VelocityPanelHemisphere(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void VelocityPanelHemisphere::InitPanel( void )
{

}

void VelocityPanelHemisphere::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update Sphere Velocity Parameters
		Real radial;
		CWnd *pWnd;

		// radial min
		pWnd = GetDlgItem(IDC_PSEd_HemisphereRadialMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelHemisphereFromSystem(0, radial);

				sprintf(buff, FORMAT_STRING, radial);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				radial = atof(buff);
				pParent->updateVelHemisphereToSystem(0, radial);
			}
		}

		// radial max
		pWnd = GetDlgItem(IDC_PSEd_HemisphereRadialMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelHemisphereFromSystem(1, radial);

				sprintf(buff, FORMAT_STRING, radial);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				radial = atof(buff);
				pParent->updateVelHemisphereToSystem(1, radial);
			}
		}
	}
}

void VelocityPanelHemisphere::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(VelocityPanelHemisphere, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_HemisphereRadialMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_HemisphereRadialMax, OnParticleSystemEdit)
END_MESSAGE_MAP()

// VelocityPanelCylinder //////////////////////////////////////////////////////
VelocityPanelCylinder::VelocityPanelCylinder(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void VelocityPanelCylinder::InitPanel( void )
{

}

void VelocityPanelCylinder::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update cylinder parameters
		Real cylinder;
		CWnd *pWnd;

		// first radial min
		pWnd = GetDlgItem(IDC_PSEd_CylinderRadialMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelCylinderFromSystem(0, cylinder);

				sprintf(buff, FORMAT_STRING, cylinder);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				cylinder = atof(buff);
				pParent->updateVelCylinderToSystem(0, cylinder);
			}
		}

		// now the normal min
		pWnd = GetDlgItem(IDC_PSEd_CylinderNormalMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelCylinderFromSystem(1, cylinder);

				sprintf(buff, FORMAT_STRING, cylinder);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				cylinder = atof(buff);
				pParent->updateVelCylinderToSystem(1, cylinder);
			}
		}

		// now the radial max
		pWnd = GetDlgItem(IDC_PSEd_CylinderRadialMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelCylinderFromSystem(2, cylinder);

				sprintf(buff, FORMAT_STRING, cylinder);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				cylinder = atof(buff);
				pParent->updateVelCylinderToSystem(2, cylinder);
			}
		}

		// now normal max
		pWnd = GetDlgItem(IDC_PSEd_CylinderNormalMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelCylinderFromSystem(3, cylinder);

				sprintf(buff, FORMAT_STRING, cylinder);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				cylinder = atof(buff);
				pParent->updateVelCylinderToSystem(3, cylinder);
			}
		}
	}
}

void VelocityPanelCylinder::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(VelocityPanelCylinder, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_CylinderRadialMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CylinderNormalMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CylinderRadialMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CylinderNormalMax, OnParticleSystemEdit)
END_MESSAGE_MAP()

// VelocityPanelOutward ///////////////////////////////////////////////////////////
VelocityPanelOutward::VelocityPanelOutward(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void VelocityPanelOutward::InitPanel( void )
{

}

void VelocityPanelOutward::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update outward parameters
		Real outward;
		CWnd *pWnd;

		// first radial min
		pWnd = GetDlgItem(IDC_PSEd_OutwardSpeedMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOutwardFromSystem(0, outward);

				sprintf(buff, FORMAT_STRING, outward);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				outward = atof(buff);
				pParent->updateVelOutwardToSystem(0, outward);
			}
		}

		// now the normal min
		pWnd = GetDlgItem(IDC_PSEd_OutwardOtherMin);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOutwardFromSystem(1, outward);

				sprintf(buff, FORMAT_STRING, outward);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				outward = atof(buff);
				pParent->updateVelOutwardToSystem(1, outward);
			}
		}

		// now the radial max
		pWnd = GetDlgItem(IDC_PSEd_OutwardSpeedMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOutwardFromSystem(2, outward);

				sprintf(buff, FORMAT_STRING, outward);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				outward = atof(buff);
				pParent->updateVelOutwardToSystem(2, outward);
			}
		}

		// first the normal max
		pWnd = GetDlgItem(IDC_PSEd_OutwardOtherMax);
		if (pWnd) {
			if (toUI) {
				pParent->getVelOutwardFromSystem(3, outward);

				sprintf(buff, FORMAT_STRING, outward);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				outward = atof(buff);
				pParent->updateVelOutwardToSystem(3, outward);
			}
		}
	}
}

void VelocityPanelOutward::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(VelocityPanelOutward, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_OutwardSpeedMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_OutwardOtherMin, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_OutwardSpeedMax, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_OutwardOtherMax, OnParticleSystemEdit)
END_MESSAGE_MAP()
