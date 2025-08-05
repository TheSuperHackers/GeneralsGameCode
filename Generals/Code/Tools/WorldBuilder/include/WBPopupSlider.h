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

#if !defined(AFX_WBPopupSliderButton_H__8B217F41_937B_11D1_8CA3_00A024CDC039__INCLUDED_)
#define AFX_WBPopupSliderButton_H__8B217F41_937B_11D1_8CA3_00A024CDC039__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WBPopupSliderButton.h : header file
//

class PopupSliderOwner
{
public:
	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)=0;
	virtual void PopSliderChanged(const long sliderID, long theVal) = 0;
	virtual void PopSliderFinished(const long sliderID, long theVal) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// WBPopupSliderButton window

// note: as of 990315, these things produce horizontal sliders by default!

class WBPopupSliderButton : public CButton
{
// Construction
public:
	WBPopupSliderButton();

// Attributes
public:

// Operations
public:
	void SetupPopSliderButton(CWnd *pParentWnd, long controlID,
							   PopupSliderOwner *pOwner);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WBPopupSliderButton)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~WBPopupSliderButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(WBPopupSliderButton)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	Int m_controlID;
	Int m_sliderStyle;
	PopupSliderOwner *m_owner;
};

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// PopupSlider window

/*
	note: PopupSlider is a self-deleting window. It cannot
	be instantiated except through its New function, and once
	its Create() function has been called successfully it should
	not be deleted. Its PostNcDestroy method will handle that
	for you.
*/

class PopupSlider : public CWnd
{
// Construction
public:
	PopupSlider();

public:
	/*
		the New function takes care of EVERYTHING for you! It
		constructs the object, calls its create method,
		and starts it running
	*/
	static void New(CWnd* pParentWnd, long kind,
					PopupSliderOwner *pSliderOwner, long sliderID);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PopupSlider)
	public:
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~PopupSlider();

	// Generated message map functions
protected:
	//{{AFX_MSG(PopupSlider)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void MoveThumbUnderMouse(int xNew);

	// our owner
	PopupSliderOwner *mSliderOwner;

	// the slider value
	long m_curValue;

	// setup values for the slider
	long m_lineSize;
	long m_lo;
	long m_hi;
	long m_kind;
	long mSliderID;

	/* the value when the slider last called "Finished"; we keep this around to
	see if we need to call Finished again on NcDestroy */
	long m_valOnLastFinished;

private:
	Bool mDraggingThumb;
	Bool mClickThrough;
	CPoint mOrigPt;
	Bool mSetOrigPt;
	Bool mEverMoved;

	// for the thumb icon
	HICON mIcon;

private:
	// the one and only slider windoid that should ever be open
	static PopupSlider *gPopupSlider;

	void GetChannelRect(CRect* rect);
	void GetThumbIconRect(CRect* rect);

	// the background color brush
	CBrush	m_brush3dFaceColor;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WBPopupSliderButton_H__8B217F41_937B_11D1_8CA3_00A024CDC039__INCLUDED_)
