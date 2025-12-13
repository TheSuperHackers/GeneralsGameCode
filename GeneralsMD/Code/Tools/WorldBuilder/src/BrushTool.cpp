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
#include "WorldBuilder.h"
#include "Lib/BaseType.h"
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
	amount = clamp((Int)MIN_RAISE_LOWER, amount, (Int)MAX_RAISE_LOWER);
	if (m_raiseLowerAmount != amount) {
		m_raiseLowerAmount = amount;
		BrushOptions::setRaiseLowerAmount(amount);
	}
}

void BrushTool::setSmoothRadius(Int radius)
{
	radius = clamp((Int)MIN_SMOOTH_RADIUS, radius, (Int)MAX_SMOOTH_RADIUS);
	if (m_smoothRadius != radius) {
		m_smoothRadius = radius;
		BrushOptions::setSmoothRadius(radius);
	}
}

void BrushTool::setSmoothRate(Int rate)
{
	rate = clamp((Int)MIN_SMOOTH_RATE, rate, (Int)MAX_SMOOTH_RATE);
	if (m_smoothRate != rate) {
		m_smoothRate = rate;
		BrushOptions::setSmoothRate(rate);
	}
}

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

	// just in case, release it.
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapFeatherCopy);
	REF_PTR_RELEASE(m_htMapRateCopy);

	m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();
	m_htMapFeatherCopy = m_htMapEditCopy->duplicate();

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
			Int brushWidth = getEffectiveBrushWidth(m_brushWidth, m_brushFeather);
			applyRaiseLowerBrush(ndx, brushWidth, (m_activeMode == BRUSH_MODE_RAISE), pDoc);
			break;
		}
		case BRUSH_MODE_SET: {
			Int brushWidth = getEffectiveBrushWidth(m_brushWidth, m_brushFeather);
			applySetHeightBrush(ndx, brushWidth, pDoc);
			break;
		}
		case BRUSH_MODE_SMOOTH: {
			Int brushWidth = max(1, m_brushWidth);
			applySmoothBrush(ndx, brushWidth, pDoc);
			break;
		}
	}
}

BrushTool::EBrushMode BrushTool::determineBrushMode() const
{
	Bool shift = (0x8000 & ::GetAsyncKeyState(VK_SHIFT)) != 0;
	Bool ctrl = (0x8000 & ::GetAsyncKeyState(VK_CONTROL)) != 0;
	return getModeFromModifiers(shift, ctrl);
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
			Real blendFactor = calcBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather, m_brushSquare);
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

	pDoc->updateHeightMap(m_htMapEditCopy, true, makePartialRange(ndx, brushWidth));
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
			Real blendFactor = calcBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather, m_brushSquare);
			Int curHeight = m_htMapEditCopy->getHeight(i,j);
			float fNewHeight = (blendFactor*(htDelta+curHeight))+((1.0f-blendFactor)*curHeight);
			Int newHeight = floor(fNewHeight+0.5f);
			newHeight = clamp(m_htMapFeatherCopy->getMinHeightValue(), newHeight, m_htMapFeatherCopy->getMaxHeightValue());

			m_htMapEditCopy->setHeight(i, j, newHeight);
			pDoc->invalCell(i, j);
		}
	}

	pDoc->updateHeightMap(m_htMapEditCopy, true, makePartialRange(ndx, brushWidth));
}

void BrushTool::applySmoothBrush(const CPoint &ndx, Int brushWidth, CWorldBuilderDoc *pDoc)
{
	Bool redoRate = applyBrushWithRateAccumulation(
		m_htMapEditCopy, m_htMapFeatherCopy, m_htMapRateCopy,
		ndx, brushWidth, m_smoothRate * 5,
		m_smoothRadius, MIN_SMOOTH_RADIUS, MAX_SMOOTH_RADIUS, pDoc);

	pDoc->updateHeightMap(m_htMapEditCopy, true, makePartialRange(ndx, brushWidth));

	if (redoRate) {
		resetSmoothRateBuffer();
	}
}

void BrushTool::resetSmoothRateBuffer()
{
	resetSmoothingBuffers(m_htMapEditCopy, m_htMapFeatherCopy, m_htMapRateCopy);
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

Bool BrushTool::getBrushHintInfo(BrushHintInfo &info, char *hintTextBuf, Int hintTextBufSize, const CPoint &hintPos, Int lastBrushMode)
{
	// Static cache for hint text - regenerated only when mode changes
	static char s_cachedHintText[512] = "";
	static Int s_cachedMode = -1;
	
	info.shouldShow = false;
	info.shouldClear = false;
	info.hintPos = hintPos;
	
	if (!hintTextBuf || hintTextBufSize <= 0) {
		return false;
	}
	hintTextBuf[0] = '\0';
	
	Tool *pCurTool = WbApp()->getCurTool();
	if (!pCurTool || pCurTool->getToolID() != ID_BRUSH_TOOL) {
		info.shouldClear = true;
		s_cachedMode = -1;  // Invalidate cache when not brush tool
		return false;
	}
	
	if (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
		info.shouldClear = true;
		return false;
	}
	
	info.currentMode = getPreviewModeFromKeys();
	info.modeInt = (Int)info.currentMode;
	
	// Use cached text if mode hasn't changed
	if (info.modeInt == s_cachedMode && s_cachedHintText[0] != '\0') {
		strlcpy(hintTextBuf, s_cachedHintText, hintTextBufSize);
		info.shouldShow = true;
		return true;
	}
	
	// Regenerate hint text
	char secondaryBuf[512];
	getModeHintStrings(NULL, 0, secondaryBuf, sizeof(secondaryBuf));
	
	if (secondaryBuf[0] == '\0') {
		info.shouldClear = true;
		s_cachedMode = -1;
		return false;
	}
	
	// Update cache
	strlcpy(s_cachedHintText, secondaryBuf, sizeof(s_cachedHintText));
	s_cachedMode = info.modeInt;
	
	strlcpy(hintTextBuf, s_cachedHintText, hintTextBufSize);
	info.shouldShow = true;
	return true;
}

void BrushTool::applySmoothingAlgorithm(
	WorldHeightMapEdit *editMap,
	WorldHeightMapEdit *featherMap,
	Int i, Int j,
	Int rate,
	Int smoothRadius,
	Int minRadius,
	Int maxRadius,
	CWorldBuilderDoc *pDoc)
{
	if (i < 0 || i >= editMap->getXExtent() || j < 0 || j >= editMap->getYExtent()) {
		return;
	}
	
	Int radius = clamp(minRadius, smoothRadius, maxRadius);
	
	Int total = 0;
	Real numSamples = 0;
	
	for (Int ii = i - radius; ii < i + radius + 1; ii++) {
		for (Int jj = j - radius; jj < j + radius + 1; jj++) {
			Real factor;
			if (i == ii && j == jj) {
				factor = 1.0f;
			} else {
				Real dist = sqrt((Real)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
				dist = max(1.0f, dist);
				factor = (dist > radius) ? 0.0f : 1.0f - (dist - 1) / radius;
			}
			
			int iNdx = clamp(0, ii, editMap->getXExtent() - 1);
			int jNdx = clamp(0, jj, editMap->getYExtent() - 1);
			
			total += featherMap->getHeight(iNdx, jNdx);
			numSamples += 1;
		}
	}
	
	total = (Int)floor(total / numSamples);
	UnsignedByte origHeight = featherMap->getHeight(i, j);
	float rateF = rate / 255.0f;
	total = (Int)floor(origHeight * (1.0f - rateF) + total * rateF + 0.5f);
	editMap->setHeight(i, j, total);
	pDoc->invalCell(i, j);
}

Bool BrushTool::applyBrushWithRateAccumulation(
	WorldHeightMapEdit *editMap,
	WorldHeightMapEdit *featherMap,
	WorldHeightMapEdit *rateMap,
	const CPoint &ndx,
	Int brushWidth,
	Int rateMultiplier,
	Int smoothRadius,
	Int minRadius,
	Int maxRadius,
	CWorldBuilderDoc *pDoc)
{
	if (!editMap || !featherMap || !rateMap) return false;

	Int sub = brushWidth / 2;
	Int add = brushWidth - sub;
	Bool rateOverflow = false;
	Int xExtent = editMap->getXExtent();
	Int yExtent = editMap->getYExtent();

	for (Int i = ndx.x - sub; i < ndx.x + add; i++) {
		if (i < 0 || i >= xExtent) continue;
		
		for (Int j = ndx.y - sub; j < ndx.y + add; j++) {
			if (j < 0 || j >= yExtent) continue;
			
			Real blendFactor = calcRoundBlendFactor(ndx, i, j, brushWidth, 0);
			if (blendFactor <= 0.0f) continue;
			
			Int rate = rateMap->getHeight(i, j);
			rate += (Int)(blendFactor * rateMultiplier);
			if (rate > 255) {
				rate = 255;
				rateOverflow = true;
			}
			rateMap->setHeight(i, j, rate);
			
			applySmoothingAlgorithm(editMap, featherMap, i, j, rate,
				smoothRadius, minRadius, maxRadius, pDoc);
		}
	}
	
	return rateOverflow;
}

void BrushTool::resetSmoothingBuffers(
	WorldHeightMapEdit *editMap,
	WorldHeightMapEdit *featherMap,
	WorldHeightMapEdit *rateMap)
{
	if (!editMap || !featherMap || !rateMap) return;

	Int size = rateMap->getXExtent() * rateMap->getYExtent();
	UnsignedByte *pRate = rateMap->getDataPtr();
	UnsignedByte *pFeather = featherMap->getDataPtr();
	UnsignedByte *pEdit = editMap->getDataPtr();
	
	for (Int i = 0; i < size; i++) {
		*pRate++ = 0;
		*pFeather++ = *pEdit++;
	}
}

IRegion2D BrushTool::makePartialRange(const CPoint &ndx, Int brushWidth)
{
	IRegion2D range;
	range.lo.x = ndx.x - brushWidth;
	range.hi.x = ndx.x + brushWidth;
	range.lo.y = ndx.y - brushWidth;
	range.hi.y = ndx.y + brushWidth;
	return range;
}

Real BrushTool::calcBlendFactor(const CPoint &ndx, Int i, Int j, Int width, Int feather, Bool square)
{
	if (square) {
		return calcSquareBlendFactor(ndx, i, j, width, feather);
	}
	return calcRoundBlendFactor(ndx, i, j, width, feather);
}

Int BrushTool::getEffectiveBrushWidth(Int baseWidth, Int feather)
{
	Int width = baseWidth;
	if (feather > 0) {
		width += 2 * feather;
	}
	return width + 2;
}
