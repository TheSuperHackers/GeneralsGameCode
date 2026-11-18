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


void ExportLog::Init(HWND parent)
{
	assert(_LogDialog == nullptr);
	_LogDialog = new LogDataDialogClass(parent);
}


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


// Print a string to the log window.
void ExportLog::printf(const char * format, ...)
{
	if (_LogDialog != nullptr) {
		va_list arguments;
		va_start(arguments, format);
		_LogDialog->printf(format,arguments);
	}
}


// Print a string over the last line printed.
void ExportLog::rprintf(const char * format, ...)
{
	if (_LogDialog != nullptr) {
		va_list arguments;
		va_start(arguments, format);
		_LogDialog->rprintf(format,arguments);
	}
}


void ExportLog::updatebar(float position, float total)
{
	if (_LogDialog != nullptr) {
		_LogDialog->updatebar(position,total);
	}
}


