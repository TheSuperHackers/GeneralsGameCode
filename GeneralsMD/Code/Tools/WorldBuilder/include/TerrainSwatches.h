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

#if !defined(AFX_TERRAINSWATCHES_H__2EC47AA6_06CA_43D1_9003_15472AE76CE7__INCLUDED_)
#define AFX_TERRAINSWATCHES_H__2EC47AA6_06CA_43D1_9003_15472AE76CE7__INCLUDED_

#pragma once
// TerrainSwatches.h : header file
//

#include "Lib/BaseType.h"
/////////////////////////////////////////////////////////////////////////////
// TerrainSwatches window

class TerrainSwatches : public CWnd
{
// Construction
public:
	TerrainSwatches();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TerrainSwatches)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TerrainSwatches();

	// Generated message map functions
protected:
	//{{AFX_MSG(TerrainSwatches)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void DrawMyTexture(CDC *pDc, int top, int left, Int width, UnsignedByte *rgbData);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINSWATCHES_H__2EC47AA6_06CA_43D1_9003_15472AE76CE7__INCLUDED_)
