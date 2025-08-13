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

/* $Header: /Commando/Code/Tools/max2w3d/PROGRESS.H 5     10/28/97 6:08p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D engine                                       *
 *                                                                                             *
 *                    File Name : PROGRESS.H                                                   *
 *                                                                                             *
 *                   Programmer : James McNeil                                                 *
 *                                                                                             *
 *                   Start Date : 06/19/97                                                     *
 *                                                                                             *
 *                  Last Update : June 19, 1997 [GH]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef PROGRESS_H
#define PROGRESS_H

#include "always.h"

class Progress_Meter_Class
{
public:

	Progress_Meter_Class
	(
		Interface * gi,
		float base,
		float range
	):
		Max ( gi ),
		Base ( base ),
		Range ( range ),
		Amount_Done ( 0.0f ),
		Increment ( 0.0f ),
		Accum( 0.0f ),
		Cancel_Requested ( FALSE )
	{
	}

	Progress_Meter_Class ( Progress_Meter_Class & other, float sub_amount ):
		Max ( other.Max ),
		Base ( other.Base + other.Amount_Done * other.Range ),
		Range ( other.Range * sub_amount ),
		Amount_Done ( 0.0f ),
		Increment ( 0.0f ),
		Accum ( other.Accum ),
		Cancel_Requested ( other.Cancel_Requested )
	{
	}

	void Finish_In_Steps ( int number_of_steps )
	{
		Increment = (1.0f - Amount_Done) / number_of_steps;
	}

	void Add_Increment ()
	{
		Set_Amount_Done ( Amount_Done + Increment );
	}

	void Set_Amount_Done ( float percentage )
	{
		Accum += percentage;
		Amount_Done = percentage;
		if (Accum > 0.01f) {
			Max->ProgressUpdate ( (int) (Amount_Done * Range + Base) );
			Accum = 0.0f;
		}

		if (Max->GetCancel())
		{
			int choice = MessageBox
			(
				Max->GetMAXHWnd(),
				_T("Do you really want to cancel the export?"),
				_T("Cancel Export?"), MB_ICONQUESTION | MB_YESNO
			);

			if ( choice == IDYES )
				Cancel_Requested = TRUE;
			else
				Max->SetCancel ( FALSE );
		}
	}

	BOOL Cancelled () { return Cancel_Requested; }

	float Increment;

private:

	Interface * Max;

	float Base;
	float Range;
	float Amount_Done;
	float Accum;

	BOOL Cancel_Requested;
};

#endif /* PROGRESS_H */
