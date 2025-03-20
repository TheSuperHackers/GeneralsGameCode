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


#include "Common/urllaunch.h"

#define FILE_PREFIX     L"file://"


///////////////////////////////////////////////////////////////////////////////
HRESULT MakeEscapedURL( LPWSTR pszInURL, LPWSTR *ppszOutURL )
{
    if( ( NULL == pszInURL ) || ( NULL == ppszOutURL ) )
    {
        return( E_INVALIDARG );
    }

    //
    // Do we need to pre-pend file://?
    //
    BOOL fNeedFilePrefix = ( 0 == wcsstr( pszInURL, L"://" ) );

    //
    // Count how many characters need to be escaped
    //
    LPWSTR pszTemp = pszInURL;
    DWORD cEscapees = 0;

    while( TRUE )
    {
        LPWSTR pchToEscape = wcspbrk( pszTemp, L" #$%&\\+,;=@[]^{}" );

        if( NULL == pchToEscape )
        {
            break;
        }

        cEscapees++;

        pszTemp = pchToEscape + 1;
    }

    //
    // Allocate sufficient outgoing buffer space
    //
    int cchNeeded = wcslen( pszInURL ) + ( 2 * cEscapees ) + 1;

    if( fNeedFilePrefix )
    {
        cchNeeded += wcslen( FILE_PREFIX );
    }

    *ppszOutURL = new WCHAR[ cchNeeded ];

    if( NULL == *ppszOutURL )
    {
        return( E_OUTOFMEMORY );
    }

    //
    // Fill in the outgoing escaped buffer
    //
    pszTemp = pszInURL;

    LPWSTR pchNext = *ppszOutURL;

    if( fNeedFilePrefix )
    {
        wcscpy( *ppszOutURL, FILE_PREFIX );
        pchNext += wcslen( FILE_PREFIX );
    }

    while( TRUE )
    {
        LPWSTR pchToEscape = wcspbrk( pszTemp, L" #$%&\\+,;=@[]^{}" );

        if( NULL == pchToEscape )
        {
            //
            // Copy the rest of the input string and get out
            //
            wcscpy( pchNext, pszTemp );
            break;
        }

        //
        // Copy all characters since the previous escapee
        //
        int cchToCopy = pchToEscape - pszTemp;

        if( cchToCopy > 0 )
        {
            wcsncpy( pchNext, pszTemp, cchToCopy );

            pchNext += cchToCopy;
        }

        //
        // Expand this character into an escape code and move on
        //
        pchNext += swprintf( pchNext, L"%%%02x", *pchToEscape );

        pszTemp = pchToEscape + 1;
    }

    return( S_OK );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT GetShellOpenCommand( LPTSTR ptszShellOpenCommand, DWORD cbShellOpenCommand )
{
    LONG lResult;

    HKEY hKey = NULL;
    HKEY hFileKey = NULL;

    BOOL fFoundExtensionCommand = FALSE;

    do
    {
        //
        // Look for the file type associated with .html files
        //
        TCHAR szFileType[ MAX_PATH ];

        lResult = RegOpenKeyEx( HKEY_CLASSES_ROOT, _T( ".html" ), 0, KEY_READ, &hKey );

        if( ERROR_SUCCESS != lResult )
        {
            break;
        }

        DWORD dwLength = sizeof( szFileType );

        lResult = RegQueryValueEx( hKey, NULL, 0, NULL, (BYTE *)szFileType, &dwLength );

        if( ERROR_SUCCESS != lResult )
        {
            break;
        }

        //
        // Find the command for the shell's open verb associated with this file type
        //
        TCHAR szKeyName[ MAX_PATH + 20 ];

        wsprintf( szKeyName, _T( "%s\\shell\\open\\command" ), szFileType );

        lResult = RegOpenKeyEx( HKEY_CLASSES_ROOT, szKeyName, 0, KEY_READ, &hFileKey );

        if( ERROR_SUCCESS != lResult )
        {
            break;
        }

        dwLength = cbShellOpenCommand;

        lResult = RegQueryValueEx( hFileKey, NULL, 0, NULL, (BYTE *)ptszShellOpenCommand, &dwLength );

        if( 0 == lResult )
        {
            fFoundExtensionCommand = TRUE;
        }
    }
    while( FALSE );

    //
    // If there was no application associated with .html files by extension, look for
    // an application associated with the http protocol
    //
    if( !fFoundExtensionCommand )
    {
        if( NULL != hKey )
        {
            RegCloseKey( hKey );
        }

        do
        {
            //
            // Find the command for the shell's open verb associated with the http protocol
            //
            lResult = RegOpenKeyEx( HKEY_CLASSES_ROOT, _T( "http\\shell\\open\\command" ), 0, KEY_READ, &hKey );

            if( ERROR_SUCCESS != lResult )
            {
                break;
            }

            DWORD dwLength = cbShellOpenCommand;

            lResult = RegQueryValueEx( hKey, NULL, 0, NULL, (BYTE *)ptszShellOpenCommand, &dwLength );
        }
        while( FALSE );
    }

    if( NULL != hKey )
    {
        RegCloseKey( hKey );
    }

    if( NULL != hFileKey )
    {
        RegCloseKey( hFileKey );
    }

    return( HRESULT_FROM_WIN32( lResult ) );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT LaunchURL( LPCWSTR pszURL )
{
    HRESULT hr;

    //
    // Find the appropriate command to launch URLs with
    //
    TCHAR szShellOpenCommand[ MAX_PATH * 2 ];

    hr = GetShellOpenCommand( szShellOpenCommand, sizeof( szShellOpenCommand ) );

    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Build the appropriate command line, substituting our URL parameter
    //
    TCHAR szLaunchCommand[ 2000 ];

    LPTSTR pszParam = _tcsstr( szShellOpenCommand, _T( "\"%1\"" ) );

    if( NULL == pszParam )
    {
        pszParam = _tcsstr( szShellOpenCommand, _T( "\"%*\"" ) );
    }

    if( NULL != pszParam )
    {
        *pszParam = _T( '\0' ) ;

        wsprintf( szLaunchCommand, _T( "%s%ws%s" ), szShellOpenCommand, pszURL, pszParam + 4 );
    }
    else
    {
        wsprintf( szLaunchCommand, _T( "%s %ws" ), szShellOpenCommand, pszURL );
    }

    //
    // Find the application name, stripping quotes if necessary
    //
    TCHAR szExe[ MAX_PATH * 2 ];
    LPTSTR pchFirst = szShellOpenCommand;
    LPTSTR pchNext = NULL;

    while( _T( ' ' ) == *pchFirst )
    {
        pchFirst++;
    }

    if( _T( '"' ) == *pchFirst )
    {
        pchFirst++;

        pchNext = _tcschr( pchFirst, _T( '"' ) );
    }
    else
    {
        pchNext = _tcschr( pchFirst + 1, _T( ' ' ) );
    }

    if( NULL == pchNext )
    {
        pchNext = szShellOpenCommand + _tcslen( szShellOpenCommand );
    }

    _tcsncpy( szExe, pchFirst, pchNext - pchFirst );
    szExe[ pchNext - pchFirst ] = _T( '\0' ) ;

    //
    // Because of the extremely long length of the URLs, neither
    // WinExec, nor ShellExecute, were working correctly.  For this reason 
    // we use CreateProcess.  The CreateProcess documentation in MSDN says
    // that the most robust way to call CreateProcess is to pass the full
    // command line, where the first element is the application name, in the
    // lpCommandLine parameter.  In our case this is necesssary to get Netscape
    // to function properly.
    //
    PROCESS_INFORMATION ProcInfo;
    ZeroMemory( (LPVOID)&ProcInfo, sizeof( PROCESS_INFORMATION ) );

    STARTUPINFO StartUp;
    ZeroMemory( (LPVOID)&StartUp, sizeof( STARTUPINFO ) );

    StartUp.cb = sizeof(STARTUPINFO); 

    if( !CreateProcess( szExe, szLaunchCommand, NULL, NULL, 
                        FALSE, 0, NULL, NULL, &StartUp, &ProcInfo) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }
    else
    {
        //
        // CreateProcess succeeded and we do not need the handles to the thread 
        // or the process, so close them now.
        //
        if( NULL != ProcInfo.hThread )
        {
            CloseHandle( ProcInfo.hThread );
        }

        if( NULL != ProcInfo.hProcess )
        {
            CloseHandle( ProcInfo.hProcess );
        }
    }

    return( hr );
}
