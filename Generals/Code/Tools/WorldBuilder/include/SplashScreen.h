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

#ifndef __SPLASHSCREEN_H__
#define __SPLASHSCREEN_H__

class SplashScreen : public CDialog
{
	protected:
		CRect m_rect;
		CString m_loadString;
		CFont m_font;

	public:
		SplashScreen();

	public:
		void setTextOutputLocation(const CRect& rect);
		void outputText(UINT nIDString);

	protected:
		afx_msg void OnPaint();

		DECLARE_MESSAGE_MAP()
};

#endif /* __SPLASHSCREEN_H__ */
