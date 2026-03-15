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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/exportlog.cpp                  $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/07/00 3:16p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ExportLog::Init -- Initialize the export logging system                                   *
 *   ExportLog::Shutdown -- Shutdown the export logging system                                 *
 *   ExportLog::printf -- Print a string to the log window                                     *
 *   ExportLog::rprintf -- Print a string over the last line printed                           *
 *   ExportLog::updatebar -- Set the position of the progress bar                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "exportlog.h"
#include "logdlg.h"
#include <assert.h>


/*
** Static variables
*/
LogDataDialogClass * _LogDialog = nullptr;


/*
**
** ExportLog implementation.  Note, this is a class which only contains static functions.
**
*/


/***********************************************************************************************
 * ExportLog::Init -- Initialize the export logging system                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/30/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void ExportLog::Init(HWND parent)
{
	assert(_LogDialog == nullptr);
	_LogDialog = new LogDataDialogClass(parent);
}


/***********************************************************************************************
 * ExportLog::Shutdown -- Shutdown the export logging system                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * wait_for_ok - should we wait for the user to press OK on the dialog?                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/30/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void ExportLog::Shutdown(bool wait_for_ok)
{
	if (_LogDialog != nullptr) {

		if (wait_for_ok) {
			_LogDialog->Wait_OK();
		}

		delete _LogDialog;
		_LogDialog = nullptr;
	}
}


/***********************************************************************************************
 * ExportLog::printf -- Print a string to the log window                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/30/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void ExportLog::printf(const char * format, ...)
{
	if (_LogDialog != nullptr) {
		va_list arguments;
		va_start(arguments, format);
		_LogDialog->printf(format,arguments);
	}
}


/***********************************************************************************************
 * ExportLog::rprintf -- Print a string over the last line printed                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/30/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void ExportLog::rprintf(const char * format, ...)
{
	if (_LogDialog != nullptr) {
		va_list arguments;
		va_start(arguments, format);
		_LogDialog->rprintf(format,arguments);
	}
}


/***********************************************************************************************
 * ExportLog::updatebar -- Set the position of the progress bar                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/30/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void ExportLog::updatebar(float position, float total)
{
	if (_LogDialog != nullptr) {
		_LogDialog->updatebar(position,total);
	}
}


