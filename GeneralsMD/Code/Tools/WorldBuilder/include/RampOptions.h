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

// FILE: RampOptions.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  RampOptions.h                                                 */
/* Created:    John K. McDonald, Jr., 4/23/2002                              */
/* Desc:       // Apply button for ramps.                                    */
/* Revision History:                                                         */
/*		4/23/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "OptionsPanel.h"
#include "resource.h"

// DEFINES ////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

class RampOptions : public COptionsPanel
{
	Bool m_shouldApplyTheRamp;
	Real m_rampWidth;
	public:
		enum { IDD = IDD_RAMP_OPTIONS };
		RampOptions(CWnd* pParent = NULL);
		virtual ~RampOptions();

		Bool shouldApplyTheRamp();
		Real getRampWidth() { return m_rampWidth; }

		afx_msg void OnApply();
		afx_msg void OnWidthChange();

	DECLARE_MESSAGE_MAP()
};

extern RampOptions* TheRampOptions;
