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

// WBPopupSlider.cpp implementation file
//

#include "StdAfx.h"

#include "Lib/BaseType.h"
#include "WBPopupSlider.h"
#include "resource.h"
#include "Common/Debug.h"

/////////////////////////////////////////////////////////////////////////////
// WBPopupSliderButton public member functions

void WBPopupSliderButton::SetupPopSliderButton
(
	CWnd *pParentWnd,
	long controlID,
	PopupSliderOwner *pOwner
)
{
	SubclassWindow(pParentWnd->GetDlgItem(controlID)->GetSafeHwnd());

	m_controlID = controlID;
	m_sliderStyle = SB_VERT;
	m_owner = pOwner;

	HBITMAP hBm = (HBITMAP) ::LoadImage((HINSTANCE) AfxGetResourceHandle(),
								 MAKEINTRESOURCE(IDB_DownArrow),
								 IMAGE_BITMAP, 0, 0,
								 LR_LOADMAP3DCOLORS);

	HBITMAP hbmOld = (HBITMAP) SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBm);

	if (hbmOld)
		::DeleteObject(hbmOld);
	hbmOld = NULL;

}


/////////////////////////////////////////////////////////////////////////////
// WBPopupSliderButton

WBPopupSliderButton::WBPopupSliderButton()
{
	m_owner = NULL;
}

WBPopupSliderButton::~WBPopupSliderButton()
{
}


BEGIN_MESSAGE_MAP(WBPopupSliderButton, CButton)
	//{{AFX_MSG_MAP(WBPopupSliderButton)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WBPopupSliderButton message handlers

void WBPopupSliderButton::OnLButtonDown(UINT nFlags, CPoint point)
{
nFlags;
point;

	// just create the slider; it will delete itself when the user is done scrolling
	PopupSlider::New(this, m_sliderStyle, m_owner, m_controlID);
}






/////////////////////////////////////////////////////////////////////////////
// defines and typedefs

#define POPUP_SLIDER_SMALLDIMENSION					16
#define POPUP_SLIDER_BIGDIMENSION					120
#define SLIDER_INSET_AMOUNT							1

// two guesses what these are
#define THUMB_ICON_WIDTH							5
#define THUMB_ICON_HEIGHT							8

// the width between the end of the channel and the edge of the window
#define CHANNEL_X_INSET_AMOUNT						5

// the height between the top of the thumb and the top of the window
#define PIXELS_ABOVE_SLIDER							3

// the height between the top of the thumb and the top of the channel
#define CHANNEL_BELOW_THUMB_TOP						1

// the height between the bottom of the channel and the bottom of the thumb
#define CHANNEL_ABOVE_THUMB_BOTTOM					3

/////////////////////////////////////////////////////////////////////////////
// PopupSlider static member variables

PopupSlider *PopupSlider::gPopupSlider = 0;


/////////////////////////////////////////////////////////////////////////////
// internal routines

/////////////////////////////////////////////////////////////////////////////
// PopupSlider private member functions

void PopupSlider::GetChannelRect(CRect* rect)
{
	GetClientRect(rect);
	if(SB_HORZ == m_kind) {
		rect->InflateRect(-CHANNEL_X_INSET_AMOUNT, 0);
		rect->top = PIXELS_ABOVE_SLIDER + CHANNEL_BELOW_THUMB_TOP;
		rect->bottom = THUMB_ICON_HEIGHT - (CHANNEL_BELOW_THUMB_TOP + CHANNEL_ABOVE_THUMB_BOTTOM);
		rect->bottom += rect->top;
	} else {
		rect->InflateRect(0, -CHANNEL_X_INSET_AMOUNT);
		rect->left = PIXELS_ABOVE_SLIDER + CHANNEL_BELOW_THUMB_TOP + 2;
		rect->right = THUMB_ICON_HEIGHT - (CHANNEL_BELOW_THUMB_TOP + CHANNEL_ABOVE_THUMB_BOTTOM);
		rect->right += rect->left;
	}
}

void PopupSlider::GetThumbIconRect(CRect* rect)
{
	CRect channelRect;
	GetChannelRect(&channelRect);

	int range = m_hi - m_lo;

	int along = m_curValue - m_lo;

	if(SB_HORZ == m_kind) {
		int thumbMiddle = along * channelRect.Width();
		thumbMiddle /= range;
		thumbMiddle += channelRect.left;

		if (thumbMiddle < channelRect.left) thumbMiddle = channelRect.left;
		if (thumbMiddle > channelRect.right) thumbMiddle = channelRect.right;

		rect->top = channelRect.top - CHANNEL_BELOW_THUMB_TOP;
		rect->bottom = channelRect.bottom + CHANNEL_ABOVE_THUMB_BOTTOM;
		rect->left = thumbMiddle - (THUMB_ICON_WIDTH / 2);
		rect->right = rect->left + THUMB_ICON_WIDTH;
	} else {
		int thumbMiddle = along * channelRect.Height();
		thumbMiddle /= range;
		thumbMiddle = channelRect.bottom - thumbMiddle;

		if (thumbMiddle < channelRect.top) thumbMiddle = channelRect.top;
		if (thumbMiddle > channelRect.bottom) thumbMiddle = channelRect.bottom;

		rect->top = thumbMiddle - (THUMB_ICON_WIDTH / 2);
		rect->bottom = thumbMiddle + (THUMB_ICON_WIDTH/2);
		rect->left = channelRect.left - CHANNEL_BELOW_THUMB_TOP;
		rect->right = channelRect.right + CHANNEL_ABOVE_THUMB_BOTTOM;
	}
}

void PopupSlider::MoveThumbUnderMouse(int xNew)
{
	CRect channelRect;
	GetChannelRect(&channelRect);

	int newVal = m_hi - m_lo;
	if(SB_HORZ == m_kind) {
		int along = xNew - channelRect.left;

		newVal *= along;
		newVal /= channelRect.Width();
		newVal += m_lo;
	} else {
		int along = channelRect.bottom - xNew;

		newVal *= along;
		newVal /= channelRect.Height();
		newVal += m_lo;
	}
	if (newVal < m_lo) newVal = m_lo;
	if (newVal >  m_hi) newVal =  m_hi;

	CRect iconRect;
	GetThumbIconRect(&iconRect);
	iconRect.InflateRect(3, 3);
	InvalidateRect(&iconRect, FALSE);

	m_curValue = newVal;

	GetThumbIconRect(&iconRect);
	iconRect.InflateRect(3, 3);
	InvalidateRect(&iconRect, FALSE);
}


/////////////////////////////////////////////////////////////////////////////
// PopupSlider static member functions

void PopupSlider::New(CWnd *pParentWnd, long kind,
						  PopupSliderOwner *pSliderOwner,
						  long sliderID)
{
	PopupSlider * pPopupSlider;

	DEBUG_ASSERTCRASH(((SB_HORZ == kind) || (SB_VERT == kind)),
					("PopupSlider - unexpected kind of slider!"));

	DEBUG_ASSERTCRASH(pSliderOwner, ("slider owner is NULL!"));

	try {
		CRect rect;

		pParentWnd->GetWindowRect(&rect);

		rect.left = rect.right;
		pPopupSlider = new PopupSlider();

		pPopupSlider->mSliderOwner = pSliderOwner;
		pPopupSlider->mSliderID = sliderID;
		pPopupSlider->m_kind = kind;

		pSliderOwner->GetPopSliderInfo(pPopupSlider->mSliderID,
								&(pPopupSlider->m_lo),
								&(pPopupSlider->m_hi),
								&(pPopupSlider->m_lineSize),
								&(pPopupSlider->m_curValue));

		DEBUG_ASSERTCRASH(pPopupSlider->m_hi != pPopupSlider->m_lo, ("PopupSlider: endpoint values are the same!"));
		DEBUG_ASSERTCRASH(pPopupSlider->m_lineSize != 0, ("PopupSlider: line size is zero!"));

		pPopupSlider->Create(rect, pParentWnd);

		/* if the slider is successfully created, it will
		be deleted automatically by its PostNcDestroy
		member function */
	} catch (...) {
		// don't rethrow
		if (pPopupSlider) {
			delete pPopupSlider;
			pPopupSlider = NULL;
		}

	}	// catch

	gPopupSlider = pPopupSlider;
	// gPopupSlider will be deleted when its PostNcDestroy method is called
}

/////////////////////////////////////////////////////////////////////////////
// PopupSlider

PopupSlider::PopupSlider()
{
	mSliderOwner = NULL;

	mDraggingThumb = false;
	mClickThrough = false;
	mSetOrigPt = false;
	mEverMoved = false;
	mIcon = NULL;
	m_lo = m_hi = m_curValue = 0;

	m_valOnLastFinished = 0;
}

PopupSlider::~PopupSlider()
{
	if (mIcon) {
		BOOL bRet = DestroyIcon(mIcon);
		(void)bRet;
		DEBUG_ASSERTCRASH(bRet != 0, ("Oops."));

		mIcon = NULL;
	}
}


BEGIN_MESSAGE_MAP(PopupSlider, CWnd)
	//{{AFX_MSG_MAP(PopupSlider)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PopupSlider message handlers

BOOL PopupSlider::Create(const RECT& rect, CWnd* pParentWnd)
{
	BOOL retVal;

	try {
		if (FALSE == m_brush3dFaceColor.CreateSolidBrush(GetSysColor(COLOR_3DFACE))) {
			throw(-1);
		}

		mIcon = (HICON) LoadImage(AfxGetResourceHandle(),
				MAKEINTRESOURCE(IDI_Thumb),
				IMAGE_ICON, 0, 0, LR_LOADMAP3DCOLORS);


		DWORD dwExStyle = WS_EX_TOPMOST;
		DWORD dwStyle = WS_POPUP;
		UINT nClassStyle = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT | CS_SAVEBITS;
		HCURSOR hCursor = ::LoadCursor(NULL, IDC_ARROW);
		CString className = AfxRegisterWndClass(nClassStyle, hCursor, (HBRUSH) m_brush3dFaceColor);

		long winWidth, winHeight;

		winWidth = ((SB_HORZ == m_kind) ? POPUP_SLIDER_BIGDIMENSION : POPUP_SLIDER_SMALLDIMENSION);
		winHeight = ((SB_HORZ == m_kind) ? POPUP_SLIDER_SMALLDIMENSION : POPUP_SLIDER_BIGDIMENSION);

		CRect winRect(rect.right - winWidth, rect.bottom, rect.right, rect.bottom + winHeight);

		// we'll just use "this" for the child ID
		if (FALSE == CWnd::CreateEx(dwExStyle, (LPCTSTR) className, "",
							  dwStyle, winRect.left, winRect.top,
							  winRect.Width(), winRect.Height(),
							  pParentWnd->GetSafeHwnd(),
							  NULL, NULL))
			throw(-1);


		// New code to center the slider's thumb under the parent window
		if(pParentWnd) {
			// Calculate the center of the parent window
			CPoint parentWindowCenter;
			GetCursorPos(&parentWindowCenter);

			// Calculate the center of the thumb
			CRect iconRect;
			GetThumbIconRect(&iconRect);
			CPoint iconCenter = iconRect.CenterPoint();
			ClientToScreen(&iconCenter);

			// from the centers, calculate how far to move the window
			int hAdjustToCenter = 0;
			int vAdjustToCenter = 0;
			if(SB_HORZ == m_kind)
				hAdjustToCenter = parentWindowCenter.x - iconCenter.x;
			// This may work for vertical sliders, but has not been tested
			if(SB_VERT == m_kind)
				vAdjustToCenter = parentWindowCenter.y - iconCenter.y;

			// Move the window
			CRect myWindowRect;
			GetWindowRect(&myWindowRect);
			myWindowRect.OffsetRect(hAdjustToCenter, vAdjustToCenter);
			SetWindowPos(NULL, myWindowRect.left, myWindowRect.top, myWindowRect.Width(), myWindowRect.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOREDRAW);
		}

		// finally, make sure the window appears on screen
		//MakeSureWindowIsVisible(GetSafeHwnd());
		ShowWindow(SW_SHOW);

		SetCapture();

		// success (we'll set the capture after all the failable operations)
		retVal = TRUE;
	} catch (...) {
		// don't rethrow
		retVal = FALSE;
	}	// catch

	return retVal;
}

void PopupSlider::PostNcDestroy()
{
	if (mSliderOwner && (m_valOnLastFinished != m_curValue)) {
		mSliderOwner->PopSliderFinished(mSliderID, m_curValue);
	}

	CWnd::PostNcDestroy();

	// now that the window has gone away, delete ourselves
	if (gPopupSlider == this) {
		delete gPopupSlider;
		gPopupSlider = NULL;
	}
}

void PopupSlider::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rc;
	CBrush cbr(GetSysColor(COLOR_BTNFACE));
	CBrush *pCbrOld;
	CPen cpen(PS_NULL, 0, RGB(0x00, 0x00, 0x00));
	CPen *pPenOld;
	COLORREF ulColor, brColor;

	pCbrOld = dc.SelectObject(&cbr);
	pPenOld = dc.SelectObject(&cpen);

	this->GetClientRect(&rc);

	dc.Rectangle(&rc);

	ulColor = GetSysColor(COLOR_3DHILIGHT);
	brColor = GetSysColor(COLOR_3DSHADOW);

	dc.Draw3dRect(&rc, ulColor, brColor);

	dc.SelectObject(pCbrOld);
	dc.SelectObject(pPenOld);

	CRect channelRect;
	GetChannelRect(&channelRect);
	dc.Draw3dRect(&channelRect, GetSysColor(COLOR_3DSHADOW), ulColor);
	channelRect.InflateRect(-1, -1);
	dc.Draw3dRect(&channelRect, GetSysColor(COLOR_3DDKSHADOW), GetSysColor(COLOR_3DLIGHT));

	if (mIcon) {
		CRect iconRect;
		GetThumbIconRect(&iconRect);
		::DrawIconEx(dc.GetSafeHdc(), iconRect.left, iconRect.top,
					 mIcon, 0, 0, 0, NULL, DI_NORMAL);
	}
	// Do not call CWnd::OnPaint() for painting messages
}

void PopupSlider::OnLButtonDown(UINT nFlags, CPoint point)
{
nFlags;
	CRect rc;
	GetClientRect(&rc);
	if (!mSetOrigPt) {
		mOrigPt = point;
	}
	mSetOrigPt = true;


	if (rc.PtInRect((POINT) point)) {
		CRect iconRect;
		GetThumbIconRect(&iconRect);

		// 990909: inflate the rect a little to make it easier to grab the thumb
		if(SB_HORZ == m_kind) {
			iconRect.InflateRect(3, 6);
		} else {
			iconRect.InflateRect(6, 3);
		}

		if (iconRect.PtInRect((POINT) point)) {
			mDraggingThumb = true;
		} else {
			CRect channelRect;
			GetChannelRect(&channelRect);

			if (channelRect.PtInRect((POINT) point)) {
				if(SB_HORZ == m_kind) {
					MoveThumbUnderMouse(point.x);
				} else {
					MoveThumbUnderMouse(point.y);
				}
				mDraggingThumb = true;
				mEverMoved = true;
			}
		}
	} else {
		try {
			if (mSliderOwner) {
				mSliderOwner->PopSliderFinished(mSliderID, m_curValue);
			}

			m_valOnLastFinished = m_curValue;
		} catch (...) {
		}

		// user clicked outside our area, close the windoid
		PostMessage(WM_CLOSE, 0, 0);
	}
}

void PopupSlider::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (mDraggingThumb) {
		mDraggingThumb = false;

		if (mSliderOwner) {
			try {
				mSliderOwner->PopSliderChanged(mSliderID, m_curValue);
				if (mClickThrough && mEverMoved) {
					mSliderOwner->PopSliderFinished(mSliderID, m_curValue);
				}

				m_valOnLastFinished = m_curValue;
			} catch (...) {
			}
		}

		if (mClickThrough && mEverMoved) {
			PostMessage(WM_CLOSE, 0, 0);
		}
	} else {
		CWnd::OnLButtonUp(nFlags, point);
	}
	mClickThrough = false;
}

void PopupSlider::OnDestroy()
{
	mEverMoved = false;
	ReleaseCapture();

	CWnd::OnDestroy();
}


void PopupSlider::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!mSetOrigPt) {
		mOrigPt = point;
	}
	mSetOrigPt = true;
	if (mDraggingThumb) {
		mEverMoved = true;
		if(SB_HORZ == m_kind) {
			MoveThumbUnderMouse(point.x);
		} else {
			MoveThumbUnderMouse(point.y);
		}

		if (mSliderOwner) {
			mSliderOwner->PopSliderChanged(mSliderID, m_curValue);
		}
	} else if (nFlags & MK_LBUTTON) {
		CRect rc;
		GetChannelRect(&rc);
		if(SB_HORZ == m_kind) {
			rc.InflateRect(0, 6);
		} else {
			rc.InflateRect(6, 0);
		}

		if (rc.PtInRect(point)) {
			// user just clicked thru, so mark the slider for click through
			mDraggingThumb = true;

			if(SB_HORZ == m_kind) {
				if (mOrigPt.x != point.x) {
					MoveThumbUnderMouse(point.x);
				}
				mClickThrough = true;
			} else {
				if (mOrigPt.y != point.y) {
					MoveThumbUnderMouse(point.y);
				}
				mClickThrough = true;
			}

			if (mSliderOwner) {
				mSliderOwner->PopSliderChanged(mSliderID, m_curValue);
			}
		}	// if (PtInRect)
	}
}

void PopupSlider::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

	if ((VK_RETURN == nChar) || (VK_ESCAPE == nChar)) {
		// close the window
		PostMessage(WM_CLOSE, 0, 0);
	}
}
