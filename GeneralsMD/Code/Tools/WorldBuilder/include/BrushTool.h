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

// BrushTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#include "Tool.h"
class WorldHeightMapEdit;
/*************************************************************************/
/**                             BrushTool
	 Does the Height Brush tool operation.
***************************************************************************/
///  Height brush tool.
class BrushTool : public Tool
{
public:
	enum {
		MIN_RAISE_LOWER = 1,
		MAX_RAISE_LOWER = 21,
		MIN_SMOOTH_RADIUS = 1,
		MAX_SMOOTH_RADIUS = 5,
		MIN_SMOOTH_RATE = 1,
		MAX_SMOOTH_RATE = 10
	};

	enum EBrushMode {
		BRUSH_MODE_RAISE,
		BRUSH_MODE_LOWER,
		BRUSH_MODE_SET,
		BRUSH_MODE_SMOOTH
	};

protected:
	enum {MIN_DELAY_TIME = 60};

	WorldHeightMapEdit *m_htMapEditCopy; ///< ref counted.
	WorldHeightMapEdit *m_htMapFeatherCopy; ///< ref counted.
	WorldHeightMapEdit *m_htMapRateCopy; ///< ref counted (smooth mode).

	static Int m_brushWidth;
	static Int m_brushFeather;
	static Bool m_brushSquare;
	static Int m_brushHeight;
	static Int m_raiseLowerAmount;
	static Int m_smoothRadius;
	static Int m_smoothRate;

	EBrushMode m_activeMode;
	Int m_lastMoveTime;

public:
	BrushTool(void);
	~BrushTool(void);

public:
	static Int getWidth(void) {return m_brushWidth;};  ///<Returns width.
	static Int getFeather(void) {return m_brushFeather;}; ///<Returns feather.
	static Int getHeight(void) {return m_brushHeight;}; ///<Returns height.
	static Int getRaiseLowerAmount(void) {return m_raiseLowerAmount;};
	static Int getSmoothRadius(void) {return m_smoothRadius;};
	static Int getSmoothRate(void) {return m_smoothRate;};
	static void setWidth(Int width);
	static void setFeather(Int feather);
	static void setHeight(Int height);
	static void setRaiseLowerAmount(Int amount);
	static void setSmoothRadius(Int radius);
	static void setSmoothRate(Int rate);
	static EBrushMode getModeFromModifiers(Bool shiftDown, Bool ctrlDown);
	static EBrushMode getPreviewModeFromKeys();
	static const char* getModeDisplayName(EBrushMode mode);
	static void getModeHintStrings(char *primaryBuf, Int primaryBufSize, char *secondaryBuf, Int secondaryBufSize);
	
	// Helper functions for brush hint drawing (shared between 2D and 3D views)
	struct BrushHintInfo {
		Bool shouldShow;
		Bool shouldClear;
		EBrushMode currentMode;
		Int modeInt;
		CPoint hintPos;
	};
	static Bool getBrushHintInfo(BrushHintInfo &info, char *hintTextBuf, Int hintTextBufSize, const CPoint &hintPos, Int lastBrushMode);
	
	// Shared smoothing algorithm function (processes a single cell)
	static void applySmoothingAlgorithm(
		WorldHeightMapEdit *editMap,
		WorldHeightMapEdit *featherMap,
		Int i, Int j,
		Int rate,
		Int smoothRadius,
		Int minRadius,
		Int maxRadius,
		CWorldBuilderDoc *pDoc);
	
	// Shared brush iteration with rate accumulation - returns true if rate buffer overflow occurred
	static Bool applyBrushWithRateAccumulation(
		WorldHeightMapEdit *editMap,
		WorldHeightMapEdit *featherMap,
		WorldHeightMapEdit *rateMap,
		const CPoint &ndx,
		Int brushWidth,
		Int rateMultiplier,
		Int smoothRadius,
		Int minRadius,
		Int maxRadius,
		CWorldBuilderDoc *pDoc);
	
	// Shared rate buffer reset
	static void resetSmoothingBuffers(
		WorldHeightMapEdit *editMap,
		WorldHeightMapEdit *featherMap,
		WorldHeightMapEdit *rateMap);
	
	// Helper to create a partial range for height map updates
	static IRegion2D makePartialRange(const CPoint &ndx, Int brushWidth);
	
	// Helper to calculate blend factor (square or round)
	static Real calcBlendFactor(const CPoint &ndx, Int i, Int j, Int width, Int feather, Bool square);
	
	// Helper to get effective brush width including feather
	static Int getEffectiveBrushWidth(Int baseWidth, Int feather);

public:
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual WorldHeightMapEdit *getHeightMap(void) {return m_htMapEditCopy;};
	virtual void activate(); ///< Become the current tool.
	virtual Bool followsTerrain(void) {return false;};

protected:
	EBrushMode determineBrushMode() const;
	void applySetHeightBrush(const CPoint &ndx, Int brushWidth, CWorldBuilderDoc *pDoc);
	void applyRaiseLowerBrush(const CPoint &ndx, Int brushWidth, Bool raising, CWorldBuilderDoc *pDoc);
	void applySmoothBrush(const CPoint &ndx, Int brushWidth, CWorldBuilderDoc *pDoc);
	void resetSmoothRateBuffer();
};
