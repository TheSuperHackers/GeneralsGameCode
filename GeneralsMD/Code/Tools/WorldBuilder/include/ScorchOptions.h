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

// ScorchOptions.h : header file
//

#include "WBPopupSlider.h"
#include "OptionsPanel.h"
#include "Common/Dict.h"
#include "Common/GameType.h"

class MapObject;
/////////////////////////////////////////////////////////////////////////////
// ScorchOptions dialog

class ScorchOptions : public COptionsPanel, public PopupSliderOwner
{
// Construction
public:
	ScorchOptions(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ScorchOptions)
	enum { IDD = IDD_SCORCH_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ScorchOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(){return;};  //!< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; //!< Modeless dialogs don't close on ESC, so eat this for modeless.
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ScorchOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeScorchtype();
	afx_msg void OnChangeSizeEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static MapObject *getSingleSelectedScorch(void);
	void updateTheUI(void);
	WBPopupSliderButton m_radiusPopup;
	std::vector<Dict*> m_allSelectedDicts;
	Bool		m_updating; ///<true if the ui is updating itself.

	static Scorches	m_scorchtype;
	static Real		m_scorchsize;
	static ScorchOptions* m_staticThis;
	void changeSize(void);
	void changeScorch(void);
	void getAllSelectedDicts(void);
	Dict** getAllSelectedDictsData();

public:
	static void update(void);
	static Scorches getScorchType(void) {return m_scorchtype;}
	static Real getScorchSize(void) {return m_scorchsize;}

	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial);
	virtual void PopSliderChanged(const long sliderID, long theVal);
	virtual void PopSliderFinished(const long sliderID, long theVal);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
