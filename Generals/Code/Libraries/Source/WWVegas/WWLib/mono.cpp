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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/mono.cpp                                           $* 
 *                                                                                             * 
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   MonoClass::Clear -- Clears the monochrome screen object.                                  *
 *   MonoClass::Draw_Box -- Draws a box using the IBM linedraw characters.                     *
 *   MonoClass::Fill_Attrib -- Fill a block with specified attribute.                          *
 *   MonoClass::MonoClass -- The default constructor for monochrome screen object.             *
 *   MonoClass::Pan -- Scroll the window right or left.                                        *
 *   MonoClass::Print -- Prints the text string at the current cursor coordinates.             *
 *   MonoClass::Print -- Simple print of text number.                                          *
 *   MonoClass::Printf -- Prints a formatted string to the monochrome screen.                  *
 *   MonoClass::Printf -- Prints formatted text using text string number.                      *
 *   MonoClass::Scroll -- Scroll the monochrome screen up by the specified lines.              *
 *   MonoClass::Set_Cursor -- Sets the monochrome cursor to the coordinates specified.         *
 *   MonoClass::Sub_Window -- Partitions the mono screen into a sub-window.                    *
 *   MonoClass::Text_Print -- Prints text to the monochrome object at coordinates indicated.   *
 *   MonoClass::Text_Print -- Simple text printing from text number.                           *
 *   MonoClass::View -- Brings the mono object to the main display.                            *
 *   MonoClass::operator = -- Handles making one mono object have the same imagery as another. *
 *   MonoClass::~MonoClass -- The default destructor for a monochrome screen object.           *
 *   Mono_Clear_Screen -- Clear the currently visible monochrome page.                         *
 *   Mono_Draw_Rect -- Draws rectangle to monochrome screen.                                   *
 *   Mono_Print -- Prints simple text to monochrome screen.                                    *
 *   Mono_Printf -- Prints formatted text to visible page.                                     *
 *   Mono_Text_Print -- Prints text to location specified.                                     *
 *   Mono_X -- Fetches the X cursor position for current visible mono page.                    *
 *   Mono_Y -- Fetches the Y cursor position for current mono page.                            *
 *   MonoClass::Set_Default_Attribute -- Set the default attribute for this window.            * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"data.h"
#include	"MONO.H"
#include	"MONODRVR.H"
#include	<stdio.h>


/*
**	Global flag to indicate whether mono output is enabled. If it is not enabled,
**	then no mono output will occur.
*/
bool MonoClass::Enabled = false;


/*
**	This points to the current mono displayed screen.
*/
MonoClass * MonoClass::Current;


/***********************************************************************************************
 * MonoClass::MonoClass -- The default constructor for monochrome screen object.               *
 *                                                                                             *
 *    This is the constructor for monochrome screen objects. It handles allocating a free      *
 *    monochrome page. If there are no more pages available, then this is a big error. The     *
 *    page allocated may not be the visible one. Call the View function in order to bring      *
 *    it to the displayed page.                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
MonoClass::MonoClass(void) :
	Handle(INVALID_HANDLE_VALUE)
{
#ifdef _WINDOWS
	Handle = CreateFile("\\\\.\\MONO", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (Current == NULL) {
		Current = this;
	}
#endif
}


/***********************************************************************************************
 * MonoClass::~MonoClass -- The default destructor for a monochrome screen object.             *
 *                                                                                             *
 *    This is the default destructor for a monochrome screen object.                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
MonoClass::~MonoClass(void)
{
#ifdef _WINDOWS
	if (Handle != INVALID_HANDLE_VALUE)  {
		CloseHandle(Handle);
		Handle = INVALID_HANDLE_VALUE;
	}
	if (Current == this) {
		Current = NULL;
	}
#endif
}


/***********************************************************************************************
 * MonoClass::Pan -- Scroll the window right or left.                                          *
 *                                                                                             *
 *    This routine will scroll the window to the right or left as indicated by the number of   *
 *    rows.                                                                                    *
 *                                                                                             *
 * INPUT:   cols  -- The number of columns to pan the window. Positive numbers pan to the left *
 *                   while negative numbers pan to the right.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/05/1996 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::Pan(int )
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		unsigned long retval;
		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_PAN, NULL, 0, NULL, 0, &retval, 0);
	}
#endif
}


/***********************************************************************************************
 * MonoClass::Sub_Window -- Partitions the mono screen into a sub-window.                      *
 *                                                                                             *
 *    This routine is used to partition the monochrome screen so that only a sub-window will   *
 *    be processed. By using this, a separate rectangle of the screen can be cleared,          *
 *    scrolled, panned, or printed into.                                                       *
 *                                                                                             *
 * INPUT:   x,y   -- The upper left corner of the new sub-window.                              *
 *                                                                                             *
 *          w,h   -- Dimensions of the sub-window specified in characters.                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The parameters are clipped as necessary.                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/05/1996 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::Sub_Window(int x, int y, int w, int h)
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		struct subwindow {
			int X,Y,W,H;
		} subwindow;
		unsigned long retval;

		subwindow.X = x;
		subwindow.Y = y;
		subwindow.W = w;
		subwindow.H = h;
		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_SET_WINDOW, &subwindow, sizeof(subwindow), NULL, 0, &retval, 0);
	}
#endif
}


/***********************************************************************************************
 * MonoClass::Set_Cursor -- Sets the monochrome cursor to the coordinates specified.           *
 *                                                                                             *
 *    Use this routine to set the monochrome's cursor position to the coordinates specified.   *
 *    This is the normal way of controlling where the next Print or Printf will output the     *
 *    text to.                                                                                 *
 *                                                                                             *
 * INPUT:   x,y   -- The coordinate to position the monochrome cursor. 0,0 is the upper left   *
 *                   corner.                                                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::Set_Cursor(int x, int y)
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		struct  {
			int X,Y;
		} cursor;
		unsigned long retval;

		cursor.X = x;
		cursor.Y = y;
		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_SET_CURSOR, &cursor, sizeof(cursor), NULL, 0, &retval, 0);
	}
#endif
}


/***********************************************************************************************
 * MonoClass::Clear -- Clears the monochrome screen object.                                    *
 *                                                                                             *
 *    This routine will fill the monochrome screen object with spaces. It is clearing the      *
 *    screen of data, making it free for output. The cursor is positioned at the upper left    *
 *    corner of the screen by this routine.                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::Clear(void)
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		unsigned long retval;

		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_CLEAR_SCREEN, NULL, 0, NULL, 0, &retval, 0);
	}
#endif
}


/***********************************************************************************************
 * MonoClass::Fill_Attrib -- Fill a block with specified attribute.                            *
 *                                                                                             *
 *    This routine will give the specified attribute to the characters within the block        *
 *    but will not change the characters themselves. You can use this routine to change the    *
 *    underline, blink, or inverse characteristics of text.                                    *
 *                                                                                             *
 * INPUT:   x,y      -- The upper left coordinate of the region to change.                     *
 *                                                                                             *
 *          w,h      -- The dimensions of the region to change (in characters).                *
 *                                                                                             *
 *          attrib   -- The attribute to fill into the region specified.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/1996 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::Fill_Attrib(int x, int y, int w, int h, MonoAttribute attrib)
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		unsigned long retval;
		struct fillcontrol  {
			int X,Y,W,H,A;
		} fillcontrol;


		fillcontrol.X = x;
		fillcontrol.Y = y;
		fillcontrol.W = w;
		fillcontrol.H = h;
		fillcontrol.A = attrib;
		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_FILL_ATTRIB, &fillcontrol, sizeof(fillcontrol), NULL, 0, &retval, 0);
	}
#endif
}


/***********************************************************************************************
 * MonoClass::Scroll -- Scroll the monochrome screen up by the specified lines.                *
 *                                                                                             *
 *    Use this routine to scroll the monochrome screen up by the number of lines specified.    *
 *    This routine is typically called by the printing functions so that the monochrome screen *
 *    behaves in the expected manner -- printing at the bottom of the screen scrolls it up     *
 *    to make room for new text.                                                               *
 *                                                                                             *
 * INPUT:   lines -- The number of lines to scroll the monochrome screen.                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::Scroll(int )
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		unsigned long retval;
		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_SCROLL, NULL, 0, NULL, 0, &retval, 0);
	}
#endif
}


/***********************************************************************************************
 * MonoClass::Printf -- Prints a formatted string to the monochrome screen.                    *
 *                                                                                             *
 *    Use this routine to output a formatted string, using the standard formatting options,    *
 *    to the monochrome screen object's current cursor position.                               *
 *                                                                                             *
 * INPUT:   text  -- Pointer to the text to print.                                             *
 *                                                                                             *
 *          ...   -- Any optional parameters to supply in formatting the text.                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The total formatted text length must not exceed 255 characters.                 *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MonoClass::Printf(char const *text, ...)
{
#ifdef _WINDOWS
	va_list	va;
	/*
	**	The buffer object is placed at the end of the local variable list
	**	so that if the sprintf happens to spill past the end, it isn't likely
	**	to trash anything (important). The buffer is then manually truncated
	**	to maximum allowed size before being printed.
	*/
	char buffer[256];

	if ( !Enabled || (Handle == INVALID_HANDLE_VALUE) ) return;

	va_start(va, text);
	vsprintf(buffer, text, va);
	buffer[sizeof(buffer)-1] = '\0';

	Print(buffer);
	va_end(va);
#endif
}


/***********************************************************************************************
 * MonoClass::Printf -- Prints formatted text using text string number.                        *
 *                                                                                             *
 *    This routine will take the given text string number and print the formatted text to      *
 *    the monochrome screen.                                                                   *
 *                                                                                             *
 * INPUT:   text  -- The text number to convert into real text (by way of external function).  *
 *                                                                                             *
 *          ...   -- Additional parameters as needed.                                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void MonoClass::Printf(int text, ...)
{
#ifdef _WINDOWS
	va_list	va;

	/*
	**	The buffer object is placed at the end of the local variable list
	**	so that if the sprintf happens to spill past the end, it isn't likely
	**	to trash anything (important). The buffer is then manually truncated
	**	to maximum allowed size before being printed.
	*/
	char buffer[256];

	if ( !Enabled || (Handle == INVALID_HANDLE_VALUE) ) return;

	va_start(va, text);
	vsprintf(buffer, Fetch_String(text), va);
	buffer[sizeof(buffer)-1] = '\0';

	Print(buffer);
	va_end(va);
#endif
}


/***********************************************************************************************
 * MonoClass::Print -- Prints the text string at the current cursor coordinates.               *
 *                                                                                             *
 *    Use this routine to output the specified text string at the monochrome object's current  *
 *    text coordinate position.                                                                *
 *                                                                                             *
 * INPUT:   ptr   -- Pointer to the string to print.                                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::Print(char const * ptr)
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		unsigned long retval;
		WriteFile(Handle, ptr, strlen(ptr), &retval, NULL);
	}
#endif
}


/*********************************************************************************************** 
 * MonoClass::Set_Default_Attribute -- Set the default attribute for this window.              * 
 *                                                                                             * 
 *    This will change the default attribute to that specified. All future text will use       * 
 *    this new attribute.                                                                      * 
 *                                                                                             * 
 * INPUT:   attrib   -- The attribute to make the current default.                             * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/06/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
void MonoClass::Set_Default_Attribute(MonoAttribute attrib)
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		unsigned long retval;
		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_SET_ATTRIBUTE, &attrib, 1, NULL, 0, &retval, 0);
	}
#endif
}	


/***********************************************************************************************
 * MonoClass::Text_Print -- Prints text to the monochrome object at coordinates indicated.     *
 *                                                                                             *
 *    Use this routine to output text to the monochrome object at the X and Y coordinates      *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   text  -- Pointer to the text string to display.                                    *
 *                                                                                             *
 *          x,y   -- The X and Y character coordinates to start the printing at.               *
 *                                                                                             *
 *          attrib-- Optional parameter that specifies what text attribute code to use.        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::Text_Print(char const *text, int x, int y, MonoAttribute attrib)
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		unsigned long retval;

		Set_Cursor(x, y);
		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_SET_ATTRIBUTE, &attrib, 1, NULL, 0, &retval, 0);
		Print(text);
	}
#endif
}


/***********************************************************************************************
 * MonoClass::Text_Print -- Simple text printing from text number.                             *
 *                                                                                             *
 *    This will print the text (represented by the text number) to the location on the         *
 *    monochrome screen specified.                                                             *
 *                                                                                             *
 * INPUT:   text  -- The text number to print (converted to real text by external routine).    *
 *                                                                                             *
 *          x,y   -- The coordinates to begin the printing at.                                 *
 *                                                                                             *
 *          attrib-- The character attribute to use while printing.                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void MonoClass::Text_Print(int text, int x, int y, MonoAttribute attrib)
{
	Text_Print(Fetch_String(text), x, y, attrib);
}


/***********************************************************************************************
 * MonoClass::Print -- Simple print of text number.                                            *
 *                                                                                             *
 *    Prints text represented by the text number specified.                                    *
 *                                                                                             *
 * INPUT:   text  -- The text number to print (converted to real text by external routine).    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void MonoClass::Print(int text)
{
	Print(Fetch_String(text));
}


/***********************************************************************************************
 * MonoClass::View -- Brings the mono object to the main display.                              *
 *                                                                                             *
 *    Use this routine to display the mono object on the monochrome screen. It is possible     *
 *    that the mono object exists on some background screen memory. Calling this routine will  *
 *    perform the necessary memory swapping to bring the data to the front. The mono object    *
 *    that was currently being viewed is not destroyed by this function. It is merely moved    *
 *    off to some background page. It can be treated normally, except that is just isn't       *
 *    visible.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *   01/06/1997 JLB : Updated to WindowsNT style of mono output.                               * 
 *=============================================================================================*/
void MonoClass::View(void)
{
#ifdef _WINDOWS
	if ( Enabled && (Handle != INVALID_HANDLE_VALUE) ) {
		unsigned long retval;
		DeviceIoControl(Handle, (DWORD)IOCTL_MONO_BRING_TO_TOP, NULL, 0, NULL, 0, &retval, 0);
		Current = this;
	}
#endif
}

