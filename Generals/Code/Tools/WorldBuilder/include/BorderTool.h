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

#pragma once

#include "Tool.h"

class BorderTool : public Tool
{
	protected:
		enum ModificationType { MOD_TYPE_INVALID, MOD_TYPE_UP, MOD_TYPE_FREE, MOD_TYPE_RIGHT };
		Bool m_mouseDown;
		Bool m_addingNewBorder;
		Int m_modifyBorderNdx;
		ModificationType m_modificationType;


	public:
		BorderTool();
		~BorderTool();

		Int getToolID(void) {return m_toolID;}
		virtual void setCursor(void);

		virtual void activate();
		virtual void deactivate();

		virtual Bool followsTerrain(void) { return false;	}

		virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
		virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
		virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
};
