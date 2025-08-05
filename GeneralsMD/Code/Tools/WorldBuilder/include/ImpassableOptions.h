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

#ifndef __IMPASSABLEOPTIONS_H__
#define __IMPASSABLEOPTIONS_H__

class ImpassableOptions : public CDialog
{
	public:
		enum { IDD = IDD_IMPASSABLEOPTIONS };

	public:
		ImpassableOptions(CWnd* pParent = NULL, Real defaultSlope = 45.0f);
		virtual ~ImpassableOptions();
		Real GetSlopeToShow() const { return m_slopeToShow; }
		Real GetDefaultSlope() const { return m_defaultSlopeToShow; }
		void SetDefaultSlopeToShow(Real slopeToShow) { m_slopeToShow = slopeToShow; }

	protected:
		Real m_slopeToShow;	// Clamped in the range of [0,90]
		Real m_defaultSlopeToShow;
		Bool m_showImpassableAreas;

		Bool ValidateSlope();	// Returns TRUE if it was valid, FALSE if it had to adjust it.

	protected:
		virtual BOOL OnInitDialog();
		afx_msg void OnAngleChange();
		afx_msg void OnPreview();
		DECLARE_MESSAGE_MAP()
};

#endif /* __IMPASSABLEOPTIONS_H__ */
