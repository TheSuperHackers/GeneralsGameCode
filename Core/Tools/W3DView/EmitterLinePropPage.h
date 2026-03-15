/*
**	Command & Conquer Renegade(tm)
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

// EmitterLinePropPage.h : header file
//


// Forward delcarations
class EmitterInstanceListClass;


/////////////////////////////////////////////////////////////////////////////
// EmitterLinePropPageClass dialog

class EmitterLinePropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterLinePropPageClass)

// Construction
public:
	EmitterLinePropPageClass();
	~EmitterLinePropPageClass();

// Dialog Data
	//{{AFX_DATA(EmitterLinePropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_LINEPROPS };
	CComboBox	m_MapModeCombo;
	CSpinButtonCtrl	m_MergeAbortFactorSpin;
	CSpinButtonCtrl	m_VPerSecSpin;
	CSpinButtonCtrl	m_UVTilingSpin;
	CSpinButtonCtrl	m_UPerSecSpin;
	CSpinButtonCtrl	m_NoiseAmplitudeSpin;
	CSpinButtonCtrl	m_SubdivisionLevelSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterLinePropPageClass)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(EmitterLinePropPageClass)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()



	public:

		/////////////////////////////////////////////////////////
		//
		//	Public methods
		//
		EmitterInstanceListClass *	Get_Emitter (void) const { return m_pEmitterList; }
		void								Set_Emitter (EmitterInstanceListClass *pemitter_list) { m_pEmitterList = pemitter_list; Initialize (); }
		bool								Is_Data_Valid (void) const { return m_bValid; }

		int								Get_Mapping_Mode (void) const { return m_MappingMode; }
		bool								Get_Merge_Intersections (void) const { return m_MergeIntersections; }
		bool								Get_End_Caps (void) const { return m_EndCaps; }
		bool								Get_Disable_Sorting (void) const { return m_DisableSorting; }

		int								Get_Subdivision_Level (void) const { return m_SubdivisionLevel; }
		float								Get_Noise_Amplitude (void) const { return m_NoiseAmplitude; }
		float								Get_Merge_Abort_Factor (void) const { return m_MergeAbortFactor; }
		float								Get_Texture_Tile_Factor (void) const { return m_TextureTileFactor; }
		float								Get_U_Per_Sec(void) const { return m_UPerSec; }
		float								Get_V_Per_Sed(void) const { return m_VPerSec; }

	protected:

		/////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void								Initialize (void);

	private:

		/////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		EmitterInstanceListClass *	m_pEmitterList;
		bool								m_bValid;

		int								m_MappingMode;
		bool								m_MergeIntersections;
		bool								m_EndCaps;
		bool								m_DisableSorting;

		int								m_SubdivisionLevel;
		float								m_NoiseAmplitude;
		float								m_MergeAbortFactor;
		float								m_TextureTileFactor;
		float								m_UPerSec;
		float								m_VPerSec;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
