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

#include <shellapi.h>

#define FILE_PREFIX     L"file://"


///////////////////////////////////////////////////////////////////////////////
HRESULT MakeEscapedURL( LPWSTR pszInURL, LPWSTR *ppszOutURL )
{
    if( ( nullptr == pszInURL ) || ( nullptr == ppszOutURL ) )
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

        if( nullptr == pchToEscape )
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

    if( nullptr == *ppszOutURL )
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

        if( nullptr == pchToEscape )
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
HRESULT LaunchURL( LPCWSTR pszURL )
{
    if( pszURL == nullptr || pszURL[ 0 ] == L'\0' )
    {
        return E_INVALIDARG;
    }

    HINSTANCE result = ShellExecuteW( nullptr, L"open", pszURL, nullptr, nullptr, SW_SHOWNORMAL );
    if( reinterpret_cast<INT_PTR>( result ) <= 32 )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    return S_OK;
}
