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

// FILE: RampOptions.cpp
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  RampOptions.cpp                                               */
/* Created:    John K. McDonald, Jr., 4/23/2002                              */
/* Desc:       // Ramp options. Contains the Apply button                    */
/* Revision History:                                                         */
/*		4/23/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "RampOptions.h"

RampOptions::RampOptions(CWnd* pParent) : COptionsPanel(RampOptions::IDD, pParent)
{
	if (TheRampOptions) {
		// oh shit.
		return;
	}

	TheRampOptions = this;
	m_rampWidth = 20;
	m_shouldApplyTheRamp = false;
}

RampOptions::~RampOptions()
{
	TheRampOptions = NULL;
}

Bool RampOptions::shouldApplyTheRamp()
{
	if (m_shouldApplyTheRamp) {
		m_shouldApplyTheRamp = false;
		return true;
	}

	return false;
}

void RampOptions::OnApply()
{
	// Set m_shouldApplyTheRamp true. The call to shouldApplyRamp will set it false
	m_shouldApplyTheRamp = true;
}

void RampOptions::OnWidthChange()
{
	CString str;
	CWnd* pWnd = GetDlgItem(IDC_RO_WIDTH);
	if (!pWnd) {
		return;
	}

	pWnd->GetWindowText(str);
	m_rampWidth = atof(str.GetBuffer(0));
}

extern RampOptions* TheRampOptions = NULL;

BEGIN_MESSAGE_MAP(RampOptions, COptionsPanel)
	ON_BN_CLICKED(IDC_RO_APPLY, OnApply)
	ON_EN_CHANGE(IDC_RO_WIDTH, OnWidthChange)
END_MESSAGE_MAP()
