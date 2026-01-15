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

// FILE: wolSetup.cpp //////////////////////////////////////////////////////
// Defines the entry point for the application.
// Author: Matthew D. Campbell, December 2001

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "resource.h"
#include "wolSetup.h"
#include "verchk.h"

void registerDLL(const char *dllName)
{
	HINSTANCE hLib = LoadLibrary(dllName);
	FARPROC lpDllEntryPoint;

	if (hLib < (HINSTANCE)HINSTANCE_ERROR)
	{
		return;
	}

	// Find the entry point.
	(FARPROC&)lpDllEntryPoint = GetProcAddress(hLib,
		"DllRegisterServer");
	if (lpDllEntryPoint != nullptr)
		(*lpDllEntryPoint)();
	else
		;//unable to locate entry point
}



HINSTANCE g_hInst = nullptr;

LRESULT CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	g_hInst = hInstance;

	checkInstalledWolapiVersion();

	DialogBox(g_hInst, (LPCTSTR)IDD_MAINBOX, nullptr, (DLGPROC)MainDialogProc);

	return 0;
}


// Mesage handler for generals setup box.
LRESULT CALLBACK GeneralsSetupDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			{
				SetDlgItemText(hDlg, IDC_EDIT_PATH, g_generalsFilename);
				SetDlgItemText(hDlg, IDC_EDIT_SERIAL, g_generalsSerial);
				return TRUE;
			}

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					char genPath[_MAX_PATH], genSerial[1024];
					GetDlgItemText(hDlg, IDC_EDIT_PATH, genPath, _MAX_PATH);
					GetDlgItemText(hDlg, IDC_EDIT_SERIAL, genSerial, 1024);
					setupGenerals( genPath, genSerial );
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}

			case IDCANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			}
			break;
	}
    return FALSE;
}

void updateDisplay(HWND hDlg)
{
	checkInstalledWolapiVersion();
	if (g_wolapiInstalled)
	{
		char buf[200];
		sprintf(buf, "%d.%d (%s)", MAJOR(g_wolapiRealVersion), MINOR(g_wolapiRealVersion), g_wolapiRealFilename);
		SetDlgItemText(hDlg, IDC_TEXT_WOLVER, buf);
	}
	else
	{
		SetDlgItemText(hDlg, IDC_TEXT_WOLVER, "Not installed");
	}
	SetDlgItemText(hDlg, IDC_TEXT_WOLDIR, g_wolapiRegFilename);
	SetDlgItemText(hDlg, IDC_TEXT_GENERALSDIR, g_generalsFilename);
}

// Mesage handler for main dialog box.
LRESULT CALLBACK MainDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			{
				updateDisplay(hDlg);
				return TRUE;
			}

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			case IDC_SETUP_GENERALS:
				{
					DialogBox(g_hInst, (LPCTSTR)IDD_GENERALSSETUPBOX, hDlg, (DLGPROC)GeneralsSetupDialogProc);
					updateDisplay(hDlg);
					break;
				}
			case IDC_UNINSTALL_GENERALS:
				{
					if (MessageBox(hDlg, "Are you sure you want to delete Generals registry entries?", "Warning!", MB_OKCANCEL) == IDOK)
					{
						MessageBox(hDlg, "Oops! Can't do that yet!", "Unimplemented", MB_OK);
						updateDisplay(hDlg);
					}
					break;
				}
			case IDC_DEBUG_WOLAPI:
				{
					if (g_wolapiInstalled)
					{
						if (MessageBox(hDlg, "Are you sure you want to overwrite installed WOLAPI?", "Warning!", MB_OKCANCEL) == IDOK)
						{
							registerDLL("woldbg.dll");
							updateDisplay(hDlg);
						}
					}
					else
					{
						DialogBox(g_hInst, (LPCTSTR)IDD_GENERALSSETUPBOX, hDlg, (DLGPROC)MainDialogProc);
					}
					break;
				}
			case IDC_RELEASE_WOLAPI:
				{
					if (g_wolapiInstalled)
					{
						if (MessageBox(hDlg, "Are you sure you want to overwrite installed WOLAPI?", "Warning!", MB_OKCANCEL) == IDOK)
						{
							registerDLL("wolapi.dll");
							updateDisplay(hDlg);
						}
					}
					else
					{
						DialogBox(g_hInst, (LPCTSTR)IDD_GENERALSSETUPBOX, hDlg, (DLGPROC)MainDialogProc);
						updateDisplay(hDlg);
					}
					break;
				}
			case IDC_UNINSTALL_WOLAPI:
				{
					if (g_wolapiInstalled)
					{
						MessageBox(hDlg, "Oops! Can't do that yet!", "Unimplemented", MB_OK);
						updateDisplay(hDlg);
					}
					break;
				}
			}
			break;
	}
    return FALSE;
}



