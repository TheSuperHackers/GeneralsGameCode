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

#if !defined(AFX_EDITPARAMETER_H__465E4002_6405_47E3_97BA_D46A8C108600__INCLUDED_)
#define AFX_EDITPARAMETER_H__465E4002_6405_47E3_97BA_D46A8C108600__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditParameter.h : header file
//
#include "GameLogic/Scripts.h"
#include "Common/SubsystemInterface.h"

class SidesList;
/////////////////////////////////////////////////////////////////////////////
// EditParameter dialog

class EditParameter : public CDialog
{
// Construction
public:
	EditParameter(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(EditParameter)
	enum { IDD = IDD_EDIT_PARAMETER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditParameter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:
	static Int edit( Parameter *pParm, Int keyPressed, AsciiString unitName = AsciiString::TheEmptyString );
	static AsciiString getWarningText(Parameter *pParm, Bool isAction);
	static AsciiString getInfoText(Parameter *pParm);
	static void setCurSidesList(SidesList *sidesListP) {m_sidesListP = sidesListP;};
	static Bool loadScripts(CComboBox *pCombo, Bool subr, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadWaypoints(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadTransports(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadObjectTypeList(CComboBox *pCombo, std::vector<AsciiString> *strings = NULL, AsciiString match = AsciiString::TheEmptyString);

protected:
	static Bool loadSides(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadTriggerAreas(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadCommandButtons(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadFontNames(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static void readFontFile( const char *filename );
	static Bool loadTeams(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadTeamOrUnit(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadUnits(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadBridges(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadObjectType(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadAudioType(Parameter::ParameterType comboType, CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadMovies(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool getMovieComment(AsciiString match, AsciiString& outCommentFromINI);
	static Bool loadSpecialPowers(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadSciences(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadScienceAvailabilities(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadUpgrades(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadAbilities( CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString );
	static Bool loadAllAbilities( CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString );
	static Bool loadWaypointPaths(CComboBox *pCombo, AsciiString match= AsciiString::TheEmptyString);
	static Bool loadObjectFlags(CComboBox *pCombo, AsciiString match= AsciiString::TheEmptyString);

	static Bool loadAttackPrioritySets(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static AsciiString loadLocalizedText(CComboBox *pCombo, AsciiString isStringInTable = AsciiString::TheEmptyString);
	static Bool loadAttackSetParameter(Script *pScr, CComboBox *pCombo, AsciiString match);
	static Bool loadCreateUnitParameter(Script *pScr, CComboBox *pCombo, AsciiString match);
	static Bool loadCreateObjectListsParameter(Script *pScr, CComboBox *pCombo, std::vector<AsciiString> *strings, AsciiString match);
	static Bool loadRevealNames(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static Bool loadRevealNamesParameter(Script *pScr, CComboBox *pCombo, AsciiString match);

	static Bool loadEmoticons( CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString );
	static AsciiString getCreatedUnitTemplateName(AsciiString unitName);

	static Bool loadCounters(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);
	static void loadConditionParameter(Script *pScr, Parameter::ParameterType type, CComboBox *pCombo);
	static Bool loadActionParameter(Script *pScr, Parameter::ParameterType type, CComboBox *pCombo, AsciiString match);
	static Bool loadFlags(CComboBox *pCombo, AsciiString match = AsciiString::TheEmptyString);


protected:
	Parameter		*m_parameter;
	static AsciiString m_unitName; //This is the name of the unit that this script command is dedicated to (if applicable).
	AsciiString m_string;
	Int					m_int;
	Real				m_real;

	Int					m_key; // key pressed to invoke the edit parameter dialog.

	static SidesList *m_sidesListP;
	static AsciiString m_selectedLocalizedString;

protected:
	Bool scanReal(CEdit *pEdit, Real scale);

protected:
	// Generated message map functions
	//{{AFX_MSG(EditParameter)
	afx_msg void OnChangeEdit();
	afx_msg void OnEditchangeCombo();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPreviewSound();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITPARAMETER_H__465E4002_6405_47E3_97BA_D46A8C108600__INCLUDED_)
