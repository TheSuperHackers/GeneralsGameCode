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

//
//	XLStuff.h
//
//

#ifndef XLSTUFF_H
#define XLSTUFF_H

#include "BabylonDlg.h"
typedef enum {
    xlAll = -4104,
    xlAutomatic = -4105,
    xlBoth = 1,
    xlCenter = -4108,
    xlChecker = 9,
    xlCircle = 8,
    xlCorner = 2,
    xlCrissCross = 16,
    xlCross = 4,
    xlDiamond = 2,
    xlDistributed = -4117,
    xlDoubleAccounting = 5,
    xlFixedValue = 1,
    xlFormats = -4122,
    xlGray16 = 17,
    xlGray8 = 18,
    xlGrid = 15,
    xlHigh = -4127,
    xlInside = 2,
    xlJustify = -4130,
    xlLightDown = 13,
    xlLightHorizontal = 11,
    xlLightUp = 14,
    xlLightVertical = 12,
    xlLow = -4134,
    xlManual = -4135,
    xlMinusValues = 3,
    xlModule = -4141,
    xlNextToAxis = 4,
    xlNone = -4142,
    xlNotes = -4144,
    xlOff = -4146,
    xlOn = 1,
    xlPercent = 2,
    xlPlus = 9,
    xlPlusValues = 2,
    xlSemiGray75 = 10,
    xlShowLabel = 4,
    xlShowLabelAndPercent = 5,
    xlShowPercent = 3,
    xlShowValue = 2,
    xlSimple = -4154,
    xlSingle = 2,
    xlSingleAccounting = 4,
    xlSolid = 1,
    xlSquare = 1,
    xlStar = 5,
    xlStError = 4,
    xlToolbarButton = 2,
    xlTriangle = 3,
    xlGray25 = -4124,
    xlGray50 = -4125,
    xlGray75 = -4126,
    xlBottom = -4107,
    xlLeft = -4131,
    xlRight = -4152,
    xlTop = -4160,
    xl3DBar = -4099,
    xl3DSurface = -4103,
    xlBar = 2,
    xlColumn = 3,
    xlCombination = -4111,
    xlCustom = -4114,
    xlDefaultAutoFormat = -1,
    xlMaximum = 2,
    xlMinimum = 4,
    xlOpaque = 3,
    xlTransparent = 2,
    xlBidi = -5000,
    xlLatin = -5001,
    xlContext = -5002,
    xlLTR = -5003,
    xlRTL = -5004,
    xlVisualCursor = 2,
    xlLogicalCursor = 1,
    xlSystem = 1,
    xlPartial = 3,
    xlHindiNumerals = 3,
    xlBidiCalendar = 3,
    xlGregorian = 2,
    xlComplete = 4,
    xlScale = 3,
    xlClosed = 3,
    xlColor1 = 7,
    xlColor2 = 8,
    xlColor3 = 9,
    xlConstants = 2,
    xlContents = 2,
    xlBelow = 1,
    xlCascade = 7,
    xlCenterAcrossSelection = 7,
    xlChart4 = 2,
    xlChartSeries = 17,
    xlChartShort = 6,
    xlChartTitles = 18,
    xlClassic1 = 1,
    xlClassic2 = 2,
    xlClassic3 = 3,
    xl3DEffects1 = 13,
    xl3DEffects2 = 14,
    xlAbove = 0,
    xlAccounting1 = 4,
    xlAccounting2 = 5,
    xlAccounting3 = 6,
    xlAccounting4 = 17,
    xlAdd = 2,
    xlDebugCodePane = 13,
    xlDesktop = 9,
    xlDirect = 1,
    xlDivide = 5,
    xlDoubleClosed = 5,
    xlDoubleOpen = 4,
    xlDoubleQuote = 1,
    xlEntireChart = 20,
    xlExcelMenus = 1,
    xlExtended = 3,
    xlFill = 5,
    xlFirst = 0,
    xlFloating = 5,
    xlFormula = 5,
    xlGeneral = 1,
    xlGridline = 22,
    xlIcons = 1,
    xlImmediatePane = 12,
    xlInteger = 2,
    xlLast = 1,
    xlLastCell = 11,
    xlList1 = 10,
    xlList2 = 11,
    xlList3 = 12,
    xlLocalFormat1 = 15,
    xlLocalFormat2 = 16,
    xlLong = 3,
    xlLotusHelp = 2,
    xlMacrosheetCell = 7,
    xlMixed = 2,
    xlMultiply = 4,
    xlNarrow = 1,
    xlNoDocuments = 3,
    xlOpen = 2,
    xlOutside = 3,
    xlReference = 4,
    xlSemiautomatic = 2,
    xlShort = 1,
    xlSingleQuote = 2,
    xlStrict = 2,
    xlSubtract = 3,
    xlTextBox = 16,
    xlTiled = 1,
    xlTitleBar = 8,
    xlToolbar = 1,
    xlVisible = 12,
    xlWatchPane = 11,
    xlWide = 3,
    xlWorkbookTab = 6,
    xlWorksheet4 = 1,
    xlWorksheetCell = 3,
    xlWorksheetShort = 5,
    xlAllExceptBorders = 6,
    xlLeftToRight = 2,
    xlTopToBottom = 1,
    xlVeryHidden = 2,
    xlDrawingObject = 14
} Constants;

typedef enum {
    xlHairline = 1,
    xlMedium = -4138,
    xlThick = 4,
    xlThin = 2
} XlBorderWeight;

typedef enum {
    xlContinuous = 1,
    xlDash = -4115,
    xlDashDot = 4,
    xlDashDotDot = 5,
    xlDot = -4118,
    xlDouble = -4119,
    xlSlantDashDot = 13,
    xlLineStyleNone = -4142
} XlLineStyle;

typedef enum {
    xlInsideHorizontal = 12,
    xlInsideVertical = 11,
    xlDiagonalDown = 5,
    xlDiagonalUp = 6,
    xlEdgeBottom = 9,
    xlEdgeLeft = 7,
    xlEdgeRight = 10,
    xlEdgeTop = 8
} XlBordersIndex;

enum {
		CELL_WAVEFILE = 1,
		CELL_LABEL,
		CELL_SPEAKER,
		CELL_LISTENER,
		CELL_STRINGID,
		CELL_ENGLISH,
		CELL_LOCALIZED,
		CELL_CONTEXT,
		CELL_COMMENT,
		CELL_MAXLEN,
		CELL_STRLEN,
		CELL_LENCHECK,
		CELL_REVISION,
		CELL_LAST
};

#define ROW_COUNT					1
#define COLUMN_COUNT			2
#define ROW_LANGUAGE			1
#define COLUMN_LANGUAGE		1



int OpenExcel ( void );
void CloseExcel ( void );
int NewWorkBook ( const char *path );
int SaveWorkBook ( const char *filename, int protect = FALSE );
int OpenWorkBook ( const char *filename );
void CloseWorkBook ( void );
int PutCell ( int row, int column, const OLECHAR *string, int val );
int PutSeparator ( int row );
int PutSection ( int row, const OLECHAR *title );
void SelectActiveSheet ( void );
int GetInt ( int row, int cell );
int GetString ( int row, int cell, OLECHAR *buffer );



#endif // XLSTUFF_H