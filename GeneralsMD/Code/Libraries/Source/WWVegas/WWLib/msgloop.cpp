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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/msgloop.cpp                            $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/05/02 1:17p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Add_Accelerator -- Adds a keyboard accelerator to the message handler.                    *
 *   Add_Modeless_Dialog -- Adds a modeless dialog box to the message handler.                 *
 *   Remove_Accelerator -- Removes an accelerator from the message processor.                  *
 *   Remove_Modeless_Dialog -- Removes the dialog box from the message tracking handler.       *
 *   Windows_Message_Handler -- Handles windows message.                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"Vector.H"
#include	"win.h"


/*
**	Tracks modeless dialog box messages by keeping a record of all active modeless dialog
**	box handles and then determining if the windows message applies to the dialog box. If it
**	does, then the default message handling should not be performed.
*/
static DynamicVectorClass<HWND> _ModelessDialogs;


/*
**	Tracks windows accelerators with this structure.
*/
struct AcceleratorTracker {
	AcceleratorTracker(HWND window = NULL, HACCEL accelerator = NULL) : Accelerator(accelerator), Window(window) {}

	int operator == (AcceleratorTracker const & acc) const {return(Accelerator == acc.Accelerator && Window == acc.Window);}
	int operator != (AcceleratorTracker const & acc) const {return(!(*this == acc));}

	HACCEL Accelerator;
	HWND Window;
};
static DynamicVectorClass<AcceleratorTracker> _Accelerators;


/*
**	In those cases where message intercept needs to occur but not for purposes
**	of a modeless dialog box or a windows accelerator, then this is a function
**	pointer to than message intercept handler.
*/
bool (*Message_Intercept_Handler)(MSG &msg) = NULL;


/***********************************************************************************************
 * Windows_Message_Handler -- Handles windows message.                                         *
 *                                                                                             *
 *    This routine will take all messages that have accumulated in the message queue and       *
 *    dispatch them to their respective recipients. When the message queue has been emptied,   *
 *    then this routine will return. By using this routine, it is possible to have the main    *
 *    program run in the main thread and yet still have it behave like a normal program as     *
 *    far as message handling is concerned. To achieve this, this routine must be called on    *
 *    a semi-frequent basis (a few times a second is plenty).                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/17/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Windows_Message_Handler(void)
{
	MSG msg;

	/*
	**	Process windows messages until the message queue is exhuasted.
	*/
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (!GetMessage( &msg, NULL, 0, 0 )) {
			return;
		}

		/*
		**	Pass the message through any loaded accelerators. If the message
		**	was processed by an accelerator, then it doesn't need to be
		**	processed by the normal message handling procedure.
		*/
		bool processed = false;
		for (int aindex = 0; aindex < _Accelerators.Count(); aindex++) {
			if (_Accelerators[aindex].Window) {
				if (TranslateAccelerator(_Accelerators[aindex].Window, _Accelerators[aindex].Accelerator, &msg)) {
					processed = true;
				}
			}
			break;
		}
		if (processed) continue;

		/*
		**	Pass the windows message through any modeless dialogs that may
		**	be active. If one of the dialogs processes the message, then
		**	it must not be processed by the normal window message handler.
		*/
		for (int index = 0; index < _ModelessDialogs.Count(); index++) {
			if (IsDialogMessage(_ModelessDialogs[index], &msg)) {
				processed = true;
				break;
			}
		}
		if (processed) continue;

		/*
		**	If the message was not handled by any normal intercept handlers, then
		**	submit the message to a custom message handler if one has been provided.
		*/
		if (Message_Intercept_Handler != NULL) {
			processed = Message_Intercept_Handler(msg);
		}
		if (processed) continue;

		/*
		**	If the message makes it to this point, then it must be a normal message. Process
		**	it in the normal fashion. The message will appear in the window message handler
		**	for the window that it was directed to.
		*/
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


/***********************************************************************************************
 * Add_Modeless_Dialog -- Adds a modeless dialog box to the message handler.                   *
 *                                                                                             *
 *    When a modeless dialog box becomes active, the messages processed by the main message    *
 *    handler must be handled different. This routine is used to inform the message handler    *
 *    that a dialog box is active and messages must be fed to it as appropriate.               *
 *                                                                                             *
 * INPUT:   dialog   -- Handle to the modeless dialog box.                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The modeless dialog box must be removed from the tracking system by calling     *
 *             Remove_Modeless_Dialog. Failure to do so when the dialog is destroyed will      *
 *             result in undefined behavior.                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/17/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Add_Modeless_Dialog(HWND dialog)
{
	_ModelessDialogs.Add(dialog);
}


/***********************************************************************************************
 * Remove_Modeless_Dialog -- Removes the dialog box from the message tracking handler.         *
 *                                                                                             *
 *    This routine must be called when a modeless dialog is being removed.                     *
 *                                                                                             *
 * INPUT:   dialog   -- Handle to the modeless dialog that was previously submitted to         *
 *                      Add_Modeless_Dialog().                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Failure to call this routine will result in undefined behavior when the dialog  *
 *             is destroyed.                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/17/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Remove_Modeless_Dialog(HWND dialog)
{
	_ModelessDialogs.Delete(dialog);
}


/***********************************************************************************************
 * Add_Accelerator -- Adds a keyboard accelerator to the message handler.                      *
 *                                                                                             *
 *    This routine will add a keyboard accelerator to the tracking process for the message     *
 *    handler. If the incoming message is processed by an accelerator, then the normal         *
 *    processing must be altered. By using this routine, the proper behavior of accelerators   *
 *    is maintained.                                                                           *
 *                                                                                             *
 * INPUT:   window   -- The window that the accelerator belongs to. Each accelerator must be   *
 *                      assigned to a window.                                                  *
 *                                                                                             *
 *          accelerator -- The handler to the windows accelerator.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   When the accelerator is no longer valid (or the controlling window as been      *
 *             destroyed), the Remove_Accelerator function must be called.                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/17/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Add_Accelerator(HWND window, HACCEL accelerator)
{
	_Accelerators.Add(AcceleratorTracker(window, accelerator));
}


/***********************************************************************************************
 * Remove_Accelerator -- Removes an accelerator from the message processor.                    *
 *                                                                                             *
 *    This routine must be called when the accelerator or the window it was attached to has    *
 *    been destroyed.                                                                          *
 *                                                                                             *
 * INPUT:   accelerator -- The accelerator to remove from the tracking system.                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine presumes that the accelerator will not be shared between windows.  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/17/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Remove_Accelerator(HACCEL accelerator)
{
	for (int index = 0; index < _Accelerators.Count(); index++) {
		if (_Accelerators[index].Accelerator == accelerator) {
			_Accelerators.Delete(index);
			break;
		}
	}
}