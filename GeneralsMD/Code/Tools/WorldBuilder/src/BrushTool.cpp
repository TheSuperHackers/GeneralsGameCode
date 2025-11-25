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

// BrushTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "BrushTool.h"
#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "brushoptions.h"
#include "DrawObject.h"
//
// BrushTool class.
//

Int BrushTool::m_brushWidth;
Int BrushTool::m_brushFeather;
Bool BrushTool::m_brushSquare;
Int BrushTool::m_brushHeight;
Int BrushTool::m_raiseLowerAmount = BrushTool::MIN_RAISE_LOWER;
Int BrushTool::m_smoothRadius = BrushTool::MIN_SMOOTH_RADIUS;
Int BrushTool::m_smoothRate = BrushTool::MIN_SMOOTH_RATE;



/// Constructor
BrushTool::BrushTool(void) :
	Tool(ID_BRUSH_TOOL, IDC_BRUSH_CROSS)
{
	m_htMapEditCopy = NULL;
	m_htMapFeatherCopy = NULL;
	m_htMapRateCopy = NULL;

	m_brushWidth = 0;
	m_brushFeather = 0;
	m_brushHeight = 0;
	m_brushSquare = false;
	m_activeMode = BRUSH_MODE_RAISE;
	m_lastMoveTime = 0;

	// Sensible defaults for the combined brush workflow.
	setRaiseLowerAmount(4);
	setSmoothRadius(2);
	setSmoothRate(5);
}

/// Destructor
BrushTool::~BrushTool(void)
{
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapFeatherCopy);
	REF_PTR_RELEASE(m_htMapRateCopy);
}

/// Set the brush height and notify the height options panel of the change.
void BrushTool::setHeight(Int height)
{
	if (m_brushHeight != height) {
		m_brushHeight = height;
		// notify height palette options panel
		BrushOptions::setHeight(height);
	}
};

void BrushTool::setRaiseLowerAmount(Int amount)
{
	if (amount < MIN_RAISE_LOWER) amount = MIN_RAISE_LOWER;
	if (amount > MAX_RAISE_LOWER) amount = MAX_RAISE_LOWER;
	if (m_raiseLowerAmount != amount) {
		m_raiseLowerAmount = amount;
		BrushOptions::setRaiseLowerAmount(amount);
	}
};

void BrushTool::setSmoothRadius(Int radius)
{
	if (radius < MIN_SMOOTH_RADIUS) radius = MIN_SMOOTH_RADIUS;
	if (radius > MAX_SMOOTH_RADIUS) radius = MAX_SMOOTH_RADIUS;
	if (m_smoothRadius != radius) {
		m_smoothRadius = radius;
		BrushOptions::setSmoothRadius(radius);
	}
};

void BrushTool::setSmoothRate(Int rate)
{
	if (rate < MIN_SMOOTH_RATE) rate = MIN_SMOOTH_RATE;
	if (rate > MAX_SMOOTH_RATE) rate = MAX_SMOOTH_RATE;
	if (m_smoothRate != rate) {
		m_smoothRate = rate;
		BrushOptions::setSmoothRate(rate);
	}
};

/// Set the brush width and notify the height options panel of the change.
void BrushTool::setWidth(Int width)
{
	if (m_brushWidth != width) {
		m_brushWidth = width;
		// notify brush palette options panel
		BrushOptions::setWidth(width);
		DrawObject::setBrushFeedbackParms(m_brushSquare, m_brushWidth, m_brushFeather);
	}
};

/// Set the brush feather and notify the height options panel of the change.
void BrushTool::setFeather(Int feather)
{
	if (m_brushFeather != feather) {
		m_brushFeather = feather;
		// notify height palette options panel
		BrushOptions::setFeather(feather);
		DrawObject::setBrushFeedbackParms(m_brushSquare, m_brushWidth, m_brushFeather);
	}
};

/// Shows the brush options panel.
void BrushTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_BRUSH_OPTIONS);
	DrawObject::setDoBrushFeedback(true);
	DrawObject::setBrushFeedbackParms(m_brushSquare, m_brushWidth, m_brushFeather);
}

/// Start tool.
/** Setup the tool to start brushing - make a copy of the height map
to edit, another copy because we need it :), and call mouseMovedDown. */
void BrushTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	m_activeMode = determineBrushMode();

	REF_PTR_RELEASE(m_htMapEditCopy);
	m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();
	REF_PTR_RELEASE(m_htMapFeatherCopy);
	m_htMapFeatherCopy = m_htMapEditCopy->duplicate();
	REF_PTR_RELEASE(m_htMapRateCopy);

	if (m_activeMode == BRUSH_MODE_SMOOTH) {
		m_htMapRateCopy = m_htMapEditCopy->duplicate();
		resetSmoothRateBuffer();
	}

	m_prevXIndex = -1;
	m_prevYIndex = -1;

	m_lastMoveTime = ::GetTickCount();
	if (m_activeMode == BRUSH_MODE_RAISE || m_activeMode == BRUSH_MODE_LOWER) {
		m_lastMoveTime -= MIN_DELAY_TIME + 1; // Fire immediately for raise/lower.
	}

	mouseMoved(m, viewPt, pView, pDoc);
}

/// End tool.
/** Finish the tool operation - create a command, pass it to the
doc to execute, and cleanup ref'd objects. */
void BrushTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	WBDocUndoable *pUndo = new WBDocUndoable(pDoc, m_htMapEditCopy);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapFeatherCopy);
	REF_PTR_RELEASE(m_htMapRateCopy);
}

/// Execute the tool.
/** Apply the height brush at the current point. */
void BrushTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	DrawObject::setFeedbackPos(cpt);

	if (m != TRACK_L) return;

	pView->viewToDocCoords(viewPt, &cpt);

	CPoint ndx;
	getCenterIndex(&cpt, m_brushWidth, &ndx, pDoc);

	Bool skipDuplicateCell = (m_activeMode == BRUSH_MODE_SET);
	if (skipDuplicateCell && m_prevXIndex == ndx.x && m_prevYIndex == ndx.y) return;

	m_prevXIndex = ndx.x;
	m_prevYIndex = ndx.y;

	switch (m_activeMode) {
		case BRUSH_MODE_RAISE:
		case BRUSH_MODE_LOWER: {
			Int brushWidth = m_brushWidth;
			if (m_brushFeather>0) {
				brushWidth += 2*m_brushFeather;
			}
			brushWidth += 2;
			applyRaiseLowerBrush(ndx, brushWidth, (m_activeMode == BRUSH_MODE_RAISE), pDoc);
			break;
		}
		case BRUSH_MODE_SET: {
			Int brushWidth = m_brushWidth;
			if (m_brushFeather>0) {
				brushWidth += 2*m_brushFeather;
			}
			brushWidth += 2;
			applySetHeightBrush(ndx, brushWidth, pDoc);
			break;
		}
		case BRUSH_MODE_SMOOTH: {
			Int brushWidth = m_brushWidth;
			if (brushWidth < 1) brushWidth = 1;
			applySmoothBrush(ndx, brushWidth, pDoc);
			break;
		}
	}
}

BrushTool::EBrushMode BrushTool::determineBrushMode() const
{
	Bool ctrl = (0x8000 & ::GetAsyncKeyState(VK_CONTROL)) != 0;
	Bool shift = (0x8000 & ::GetAsyncKeyState(VK_SHIFT)) != 0;

	if (ctrl) {
		if (shift) {
			return BRUSH_MODE_SMOOTH;
		}
		return BRUSH_MODE_SET;
	}
	if (shift) {
		return BRUSH_MODE_LOWER;
	}
	return BRUSH_MODE_RAISE;
}

void BrushTool::applySetHeightBrush(const CPoint &ndx, Int brushWidth, CWorldBuilderDoc *pDoc)
{
	if (!m_htMapEditCopy || !m_htMapFeatherCopy) return;

	int sub = brushWidth/2;
	int add = brushWidth-sub;

	Int i, j;
	for (i=ndx.x-sub; i<ndx.x+add; i++) {
		if (i<0 || i>=m_htMapEditCopy->getXExtent()) {
			continue;
		}
		for (j=ndx.y-sub; j<ndx.y+add; j++) {
			if (j<0 || j>=m_htMapEditCopy->getYExtent()) {
				continue;
			}
			Real blendFactor;
			if (m_brushSquare) {
				blendFactor = calcSquareBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather);
			} else {
				blendFactor = calcRoundBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather);
			}
			Int curHeight = m_htMapFeatherCopy->getHeight(i,j);
			float fNewHeight = blendFactor*m_brushHeight+((1.0f-blendFactor)*curHeight);
			Int newHeight = floor(fNewHeight+0.5f);
			if (m_brushHeight > curHeight) {
				if (m_htMapEditCopy->getHeight(i,j)>newHeight) {
					newHeight = m_htMapEditCopy->getHeight(i,j);
				}
			} else {
				if (m_htMapEditCopy->getHeight(i,j)<newHeight) {
					newHeight = m_htMapEditCopy->getHeight(i,j);
				}
			}
			m_htMapEditCopy->setHeight(i, j, newHeight);
		}
	}

	IRegion2D partialRange;
	partialRange.lo.x = ndx.x - brushWidth;
	partialRange.hi.x = ndx.x + brushWidth;
	partialRange.lo.y = ndx.y - brushWidth;
	partialRange.hi.y = ndx.y + brushWidth;
	pDoc->updateHeightMap(m_htMapEditCopy, true, partialRange);
}

void BrushTool::applyRaiseLowerBrush(const CPoint &ndx, Int brushWidth, Bool raising, CWorldBuilderDoc *pDoc)
{
	if (!m_htMapEditCopy || !m_htMapFeatherCopy) return;

	Int curTime = ::GetTickCount();
	Int deltaTime = curTime - m_lastMoveTime;
	if (deltaTime < MIN_DELAY_TIME) return;
	m_lastMoveTime = curTime;

	int sub = brushWidth/2;
	int add = brushWidth-sub;

	Int htDelta = raising?m_raiseLowerAmount:-m_raiseLowerAmount;
	Int i, j;
	for (i=ndx.x-sub; i<ndx.x+add; i++) {
		if (i<0 || i>=m_htMapEditCopy->getXExtent()) {
			continue;
		}
		for (j=ndx.y-sub; j<ndx.y+add; j++) {
			if (j<0 || j>=m_htMapEditCopy->getYExtent()) {
				continue;
			}
			Real blendFactor;
			if (m_brushSquare) {
				blendFactor = calcSquareBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather);
			} else {
				blendFactor = calcRoundBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather);
			}
			Int curHeight = m_htMapEditCopy->getHeight(i,j);
			float fNewHeight = (blendFactor*(htDelta+curHeight))+((1.0f-blendFactor)*curHeight);
			Int newHeight = floor(fNewHeight+0.5f);

			if (newHeight < m_htMapFeatherCopy->getMinHeightValue()) newHeight = m_htMapFeatherCopy->getMinHeightValue();
			if (newHeight > m_htMapFeatherCopy->getMaxHeightValue()) newHeight = m_htMapFeatherCopy->getMaxHeightValue();

			m_htMapEditCopy->setHeight(i, j, newHeight);
			pDoc->invalCell(i, j);
		}
	}

	IRegion2D partialRange;
	partialRange.lo.x = ndx.x - brushWidth;
	partialRange.hi.x = ndx.x + brushWidth;
	partialRange.lo.y = ndx.y - brushWidth;
	partialRange.hi.y = ndx.y + brushWidth;
	pDoc->updateHeightMap(m_htMapEditCopy, true, partialRange);
}

void BrushTool::applySmoothBrush(const CPoint &ndx, Int brushWidth, CWorldBuilderDoc *pDoc)
{
	if (!m_htMapEditCopy || !m_htMapFeatherCopy || !m_htMapRateCopy) return;

	int sub = brushWidth/2;
	int add = brushWidth-sub;

	Int i, j;
	Bool redoRate = false;

	for (i= ndx.x-sub; i< ndx.x+add; i++) {
		if (i<0 || i>=m_htMapEditCopy->getXExtent()) {
			continue;
		}
		for (j=ndx.y-sub; j<ndx.y+add; j++) {
			if (j<0 || j>=m_htMapEditCopy->getYExtent()) {
				continue;
			}
			Real blendFactor = calcRoundBlendFactor(ndx, i, j, brushWidth, 0);

			if (blendFactor > 0.0f) {
				Int rate = m_htMapRateCopy->getHeight(i, j);
				rate += blendFactor * m_smoothRate*5;
				if (rate>255) {
					rate = 255;
					redoRate = true;
				}
				m_htMapRateCopy->setHeight(i,j,rate);

				Int total=0;
				Real numSamples=0;
				Int ii, jj;
				Int radius = m_smoothRadius;
				if (radius<MIN_SMOOTH_RADIUS) radius=MIN_SMOOTH_RADIUS;
				if (radius>MAX_SMOOTH_RADIUS) radius = MAX_SMOOTH_RADIUS;
				for (ii = i-radius; ii < i+radius+1; ii++) {
					for (jj = j-radius; jj<j+radius+1; jj++) {
						Real factor;
						if (i==ii && j==jj) {
							factor = 1.0f;
						} else {
							Real dist = sqrt((ii-i)*(ii-i)+(jj-j)*(jj-j));
							if (dist<1.0) dist = 1.0f;
							if (dist>radius) {
								factor = 0;
							} else {
								factor = 1.0f - (dist-1)/radius;
							}
						}
						int iNdx = ii;
						if (iNdx<0) iNdx = 1;
						if (iNdx >=m_htMapEditCopy->getXExtent()) {
							iNdx = m_htMapEditCopy->getXExtent()-1;
						}
						int jNdx = jj;
						if (jNdx<0) jNdx = 1;
						if (jNdx >=m_htMapEditCopy->getYExtent()) {
							jNdx = m_htMapEditCopy->getYExtent()-1;
						}
						total += m_htMapFeatherCopy->getHeight(iNdx, jNdx);
						numSamples+=1;
					}
				}
				total = floor((total/numSamples));
				UnsignedByte origHeight =  m_htMapFeatherCopy->getHeight(i, j);
				float rateF = rate/255.0f;
				total = floor(origHeight*(1.0f-rateF) + total*rateF + 0.5f);
				m_htMapEditCopy->setHeight(i, j, total);
				pDoc->invalCell(i, j);
			}
		}
	}

	IRegion2D partialRange;
	partialRange.lo.x = ndx.x - brushWidth;
	partialRange.hi.x = ndx.x + brushWidth;
	partialRange.lo.y = ndx.y - brushWidth;
	partialRange.hi.y = ndx.y + brushWidth;
	pDoc->updateHeightMap(m_htMapEditCopy, true, partialRange);

	if (redoRate) {
		resetSmoothRateBuffer();
	}
}

void BrushTool::resetSmoothRateBuffer()
{
	if (!m_htMapRateCopy || !m_htMapFeatherCopy || !m_htMapEditCopy) return;

	Int size = m_htMapRateCopy->getXExtent() * m_htMapRateCopy->getYExtent();
	UnsignedByte *pRate = m_htMapRateCopy->getDataPtr();
	UnsignedByte *pFeather = m_htMapFeatherCopy->getDataPtr();
	UnsignedByte *pEdit = m_htMapEditCopy->getDataPtr();
	for (Int idx=0; idx<size; idx++) {
		*pRate++ = 0;
		*pFeather++ = *pEdit++;
	}
}

BrushTool::EBrushMode BrushTool::getModeFromModifiers(Bool shiftDown, Bool ctrlDown)
{
	if (ctrlDown) {
		return shiftDown ? BRUSH_MODE_SMOOTH : BRUSH_MODE_SET;
	}
	if (shiftDown) {
		return BRUSH_MODE_LOWER;
	}
	return BRUSH_MODE_RAISE;
}

BrushTool::EBrushMode BrushTool::getPreviewModeFromKeys()
{
	Bool shiftDown = (0x8000 & ::GetAsyncKeyState(VK_SHIFT)) != 0;
	Bool ctrlDown = (0x8000 & ::GetAsyncKeyState(VK_CONTROL)) != 0;
	return getModeFromModifiers(shiftDown, ctrlDown);
}

const char* BrushTool::getModeDisplayName(EBrushMode mode)
{
	switch (mode) {
		case BRUSH_MODE_LOWER:
			return "Lower Height (Shift + LMB)";
		case BRUSH_MODE_SET:
			return "Set Height (Ctrl + LMB)";
		case BRUSH_MODE_SMOOTH:
			return "Smooth Height (Ctrl + Shift + LMB)";
		case BRUSH_MODE_RAISE:
		default:
			return "Raise Height (LMB)";
	}
}

void BrushTool::getModeHintStrings(char *primaryBuf, Int primaryBufSize, char *secondaryBuf, Int secondaryBufSize)
{
	if (primaryBuf && primaryBufSize > 0) {
		primaryBuf[0] = '\0';
	}
	if (secondaryBuf && secondaryBufSize > 0) {
		secondaryBuf[0] = '\0';
	}

	Bool shiftDown = (0x8000 & ::GetAsyncKeyState(VK_SHIFT)) != 0;
	Bool ctrlDown = (0x8000 & ::GetAsyncKeyState(VK_CONTROL)) != 0;

	EBrushMode mode = getModeFromModifiers(shiftDown, ctrlDown);
	const char *modeLabel = getModeDisplayName(mode);

	if (primaryBuf && primaryBufSize > 0) {
		snprintf(primaryBuf, primaryBufSize, "Brush Mode: %s", modeLabel);
	}

	if (secondaryBuf && secondaryBufSize > 0) {
		const char *defaultPrefix = (!shiftDown && !ctrlDown) ? "[" : "";
		const char *defaultSuffix = (!shiftDown && !ctrlDown) ? "]" : "";
		const char *shiftPrefix = (shiftDown && !ctrlDown) ? "[" : "";
		const char *shiftSuffix = (shiftDown && !ctrlDown) ? "]" : "";
		const char *ctrlPrefix = (!shiftDown && ctrlDown) ? "[" : "";
		const char *ctrlSuffix = (!shiftDown && ctrlDown) ? "]" : "";
		const char *ctrlShiftPrefix = (shiftDown && ctrlDown) ? "[" : "";
		const char *ctrlShiftSuffix = (shiftDown && ctrlDown) ? "]" : "";

		snprintf(secondaryBuf, secondaryBufSize,
						 "%sDefault: Raise%s   %sShift: Lower%s   %sCtrl: Set%s   %sCtrl+Shift: Smooth%s",
						 defaultPrefix, defaultSuffix,
						 shiftPrefix, shiftSuffix,
						 ctrlPrefix, ctrlSuffix,
						 ctrlShiftPrefix, ctrlShiftSuffix);
	}
}
