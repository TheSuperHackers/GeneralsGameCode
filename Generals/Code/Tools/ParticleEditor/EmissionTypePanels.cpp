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

// FILE: EmissionTypePanels.cpp
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  EmissionTypePanels.cpp                                        */
/* Created:    John K. McDonald, Jr., 3/21/2002                              */
/* Desc:       // @todo                                                      */
/* Revision History:                                                         */
/*		3/21/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "EmissionTypePanels.h"
#include "ParticleEditorDialog.h"

// Defines ////////////////////////////////////////////////////////////////////
#define		ARBITRARY_BUFF_SIZE		128


// EmissionPanelLine //////////////////////////////////////////////////////////
EmissionPanelPoint::EmissionPanelPoint(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void EmissionPanelPoint::InitPanel( void )
{

}

void EmissionPanelPoint::performUpdate( IN Bool toUI )
{

}

void EmissionPanelPoint::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(EmissionPanelPoint, ISwapablePanel)
END_MESSAGE_MAP()


// Defines ////////////////////////////////////////////////////////////////////
#define		ARBITRARY_BUFF_SIZE		128

// EmissionPanelLine //////////////////////////////////////////////////////////
EmissionPanelLine::EmissionPanelLine(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void EmissionPanelLine::InitPanel( void )
{

}

void EmissionPanelLine::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update line parameters
		Real linePoint;
		CWnd *pWnd;

		// first X1
		pWnd = GetDlgItem(IDC_PSEd_LineStartX);
		if (pWnd) {
			if (toUI) {
				pParent->getLineFromSystem(0, linePoint);

				sprintf(buff, FORMAT_STRING, linePoint);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				linePoint = atof(buff);
				pParent->updateLineToSystem(0, linePoint);
			}
		}

		// now the Y1
		pWnd = GetDlgItem(IDC_PSEd_LineStartY);
		if (pWnd) {
			if (toUI) {
				pParent->getLineFromSystem(1, linePoint);

				sprintf(buff, FORMAT_STRING, linePoint);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				linePoint = atof(buff);
				pParent->updateLineToSystem(1, linePoint);
			}
		}

		// now the Z1
		pWnd = GetDlgItem(IDC_PSEd_LineStartZ);
		if (pWnd) {
			if (toUI) {
				pParent->getLineFromSystem(2, linePoint);

				sprintf(buff, FORMAT_STRING, linePoint);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				linePoint = atof(buff);
				pParent->updateLineToSystem(2, linePoint);
			}
		}

		// first the X2
		pWnd = GetDlgItem(IDC_PSEd_LineEndX);
		if (pWnd) {
			if (toUI) {
				pParent->getLineFromSystem(3, linePoint);

				sprintf(buff, FORMAT_STRING, linePoint);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				linePoint = atof(buff);
				pParent->updateLineToSystem(3, linePoint);
			}
		}

		// now the Y2
		pWnd = GetDlgItem(IDC_PSEd_LineEndY);
		if (pWnd) {
			if (toUI) {
				pParent->getLineFromSystem(4, linePoint);

				sprintf(buff, FORMAT_STRING, linePoint);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				linePoint = atof(buff);
				pParent->updateLineToSystem(4, linePoint);
			}
		}

		// the Z2
		pWnd = GetDlgItem(IDC_PSEd_LineEndZ);
		if (pWnd) {
			if (toUI) {
				pParent->getLineFromSystem(5, linePoint);

				sprintf(buff, FORMAT_STRING, linePoint);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				linePoint = atof(buff);
				pParent->updateLineToSystem(5, linePoint);
			}
		}
	}

}

void EmissionPanelLine::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(EmissionPanelLine, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_LineStartX, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_LineStartY, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_LineStartZ, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_LineEndX, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_LineEndY, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_LineEndZ, OnParticleSystemEdit)

END_MESSAGE_MAP()


// EmissionPanelBox ///////////////////////////////////////////////////////////
EmissionPanelBox::EmissionPanelBox(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void EmissionPanelBox::InitPanel( void )
{

}

void EmissionPanelBox::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update half size of box
		Real halfSize;
		CWnd *pWnd;

		// first the X
		pWnd = GetDlgItem(IDC_PSEd_BoxHalfSizeX);
		if (pWnd) {
			if (toUI) {
				pParent->getHalfSizeFromSystem(0, halfSize);

				sprintf(buff, FORMAT_STRING, halfSize);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				halfSize = atof(buff);
				pParent->updateHalfSizeToSystem(0, halfSize);
			}
		}

		// now the Y
		pWnd = GetDlgItem(IDC_PSEd_BoxHalfSizeY);
		if (pWnd) {
			if (toUI) {
				pParent->getHalfSizeFromSystem(1, halfSize);

				sprintf(buff, FORMAT_STRING, halfSize);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				halfSize = atof(buff);
				pParent->updateHalfSizeToSystem(1, halfSize);
			}
		}

		// finally, the Z
		pWnd = GetDlgItem(IDC_PSEd_BoxHalfSizeZ);
		if (pWnd) {
			if (toUI) {
				pParent->getHalfSizeFromSystem(2, halfSize);

				sprintf(buff, FORMAT_STRING, halfSize);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				halfSize = atof(buff);
				pParent->updateHalfSizeToSystem(2, halfSize);
			}
		}
	}
}

void EmissionPanelBox::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(EmissionPanelBox, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_BoxHalfSizeX, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_BoxHalfSizeY, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_BoxHalfSizeZ, OnParticleSystemEdit)
END_MESSAGE_MAP()


// EmissionPanelSphere ////////////////////////////////////////////////////////
EmissionPanelSphere::EmissionPanelSphere(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void EmissionPanelSphere::InitPanel( void )
{

}

void EmissionPanelSphere::performUpdate( IN Bool toUI )
{

	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update sphere radius
		Real radius;
		CWnd *pWnd;

		// first the X
		pWnd = GetDlgItem(IDC_PSEd_SphereRadius);
		if (pWnd) {
			if (toUI) {
				pParent->getHalfSizeFromSystem(0, radius);

				sprintf(buff, FORMAT_STRING, radius);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				radius = atof(buff);
				pParent->updateHalfSizeToSystem(0, radius);
			}
		}
	}
}

void EmissionPanelSphere::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(EmissionPanelSphere, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_SphereRadius, OnParticleSystemEdit)
END_MESSAGE_MAP()



// EmissionPanelCylinder //////////////////////////////////////////////////////
EmissionPanelCylinder::EmissionPanelCylinder(UINT nIDTemplate, CWnd* pParentWnd) : ISwapablePanel(nIDTemplate, pParentWnd)
{

}

void EmissionPanelCylinder::InitPanel( void )
{

}

void EmissionPanelCylinder::performUpdate( IN Bool toUI )
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{ // update cylinder parameters
		CWnd *pWnd;

		// first the Radius
		pWnd = GetDlgItem(IDC_PSEd_CylRadius);
		if (pWnd) {
			Real radius;
			if (toUI) {
				pParent->getCylinderRadiusFromSystem(radius);

				sprintf(buff, FORMAT_STRING, radius);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				radius = atof(buff);
				pParent->updateCylinderRadiusToSystem(radius);
			}
		}

		// now the Length
		pWnd = GetDlgItem(IDC_PSEd_CylLength);
		if (pWnd) {
			Real length;
			if (toUI) {

				pParent->getCylinderLengthFromSystem(length);

				sprintf(buff, FORMAT_STRING, length);
				pWnd->SetWindowText(buff);
			} else {
				pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
				length = atof(buff);
				pParent->updateCylinderLengthToSystem(length);
			}
		}
	}
}

void EmissionPanelCylinder::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BEGIN_MESSAGE_MAP(EmissionPanelCylinder, ISwapablePanel)
	ON_EN_KILLFOCUS(IDC_PSEd_CylRadius, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CylLength, OnParticleSystemEdit)
END_MESSAGE_MAP()
