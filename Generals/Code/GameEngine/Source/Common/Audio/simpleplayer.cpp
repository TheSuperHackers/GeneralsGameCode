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


#include "Common/simpleplayer.h"
#include "Common/urllaunch.h"


///////////////////////////////////////////////////////////////////////////////
CSimplePlayer::CSimplePlayer( HRESULT* phr )
{
    m_cRef = 1;
    m_cBuffersOutstanding = 0;

    m_pReader = NULL;

    m_pHeader = NULL;
    m_hwo = NULL;

    m_fEof = FALSE;
    m_pszUrl = NULL;

	*phr = S_OK;

    m_hOpenEvent = CreateEvent( NULL, FALSE, FALSE, SIMPLE_PLAYER_OPEN_EVENT );
	if ( NULL == m_hOpenEvent )
	{
		*phr = E_OUTOFMEMORY;
	}
    m_hCloseEvent = CreateEvent( NULL, FALSE, FALSE, SIMPLE_PLAYER_CLOSE_EVENT );
	if ( NULL == m_hCloseEvent )
	{
		*phr = E_OUTOFMEMORY;
	}

    m_hrOpen = S_OK;

	m_hCompletionEvent = NULL;

    InitializeCriticalSection( &m_CriSec );
    m_whdrHead = NULL;
}


///////////////////////////////////////////////////////////////////////////////
CSimplePlayer::~CSimplePlayer()
{
    DEBUG_ASSERTCRASH( 0 == m_cBuffersOutstanding,("CSimplePlayer destructor m_cBuffersOutstanding != 0") );

    Close();

    //
    // final remove of everything in the wave header list
    //
    RemoveWaveHeaders();
    DeleteCriticalSection( &m_CriSec );

    if( m_pHeader != NULL )
    {
        m_pHeader->Release();
        m_pHeader = NULL;
    }

    if( m_pReader != NULL )
    {
        m_pReader->Release();
        m_pReader = NULL;
    }

    if( m_hwo != NULL )
    {
        waveOutClose( m_hwo );
    }

    delete [] m_pszUrl;

	if ( m_hOpenEvent )
	{
	    CloseHandle( m_hOpenEvent );
	}
	if ( m_hCloseEvent )
	{
        CloseHandle( m_hCloseEvent );
	}
}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CSimplePlayer::QueryInterface(
    REFIID riid,
    void **ppvObject )
{
    return( E_NOINTERFACE );
}


///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CSimplePlayer::AddRef()
{
    return( InterlockedIncrement( &m_cRef ) );
}


///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CSimplePlayer::Release()
{
    ULONG uRet = InterlockedDecrement( &m_cRef );

    if( 0 == uRet )
    {
        delete this;
    }

    return( uRet );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CSimplePlayer::OnSample( 
        /* [in] */ DWORD dwOutputNum,
        /* [in] */ QWORD cnsSampleTime,
        /* [in] */ QWORD cnsSampleDuration,
        /* [in] */ DWORD dwFlags,
        /* [in] */ INSSBuffer __RPC_FAR *pSample,
        /* [in] */ VOID *pvContext )
{
    if( 0 != dwOutputNum )
    {
        return( S_OK );
    }

    HRESULT hr = S_OK;
    BYTE *pData;
    DWORD cbData;

    //
    // first UnprepareHeader and remove everthing in the ready list
    //
    RemoveWaveHeaders( );

    hr = pSample->GetBufferAndLength( &pData, &cbData );
    if ( FAILED( hr ) )
    {
        return( E_UNEXPECTED );
    }

    DEBUG_LOG(( " New Sample of length %d and PS time %d ms\n", 
              cbData, ( DWORD ) ( cnsSampleTime / 10000 ) ));

    LPWAVEHDR pwh = (LPWAVEHDR) new BYTE[ sizeof( WAVEHDR ) + cbData ];

    if( NULL == pwh )
    {
        DEBUG_LOG(( "OnSample OUT OF MEMORY! \n"));

        *m_phrCompletion = E_OUTOFMEMORY;
        SetEvent( m_hCompletionEvent );
        return( E_UNEXPECTED );
    }

    pwh->lpData = (LPSTR)&pwh[1];
    pwh->dwBufferLength = cbData;
    pwh->dwBytesRecorded = cbData;
    pwh->dwUser = 0;
    pwh->dwLoops = 0;
    pwh->dwFlags = 0;

    CopyMemory( pwh->lpData, pData, cbData );

    MMRESULT mmr;

    mmr = waveOutPrepareHeader( m_hwo, pwh, sizeof(WAVEHDR) );
    mmr = MMSYSERR_NOERROR;

    if( mmr != MMSYSERR_NOERROR )
    {
        DEBUG_LOG(( "failed to prepare wave buffer, error=%lu\n" , mmr ));
        *m_phrCompletion = E_UNEXPECTED;
        SetEvent( m_hCompletionEvent );
        return( E_UNEXPECTED );
    }

    mmr = waveOutWrite( m_hwo, pwh, sizeof(WAVEHDR) );
    mmr = MMSYSERR_NOERROR;

    if( mmr != MMSYSERR_NOERROR )
    {
        delete pwh;

        DEBUG_LOG(( "failed to write wave sample, error=%lu\n" , mmr ));
        *m_phrCompletion = E_UNEXPECTED;
        SetEvent( m_hCompletionEvent );
        return( E_UNEXPECTED );
    }

    InterlockedIncrement( &m_cBuffersOutstanding );

    return( S_OK );
}

///////////////////////////////////////////////////////////////////////////////
HRESULT CSimplePlayer::Play( LPCWSTR pszUrl, DWORD dwSecDuration, HANDLE hCompletionEvent, HRESULT *phrCompletion )
{
    HRESULT hr;

    //
    // If the URL is not a UNC path, a full path, or an Internet-style URL then assume it is
    // a relative local file name that needs to be expanded to a full path.
    //
    WCHAR wszFullUrl[ MAX_PATH ];

    if( ( 0 == wcsstr( pszUrl, L"\\\\" ) )
        && ( 0 == wcsstr( pszUrl, L":\\" ) )
        && ( 0 == wcsstr( pszUrl, L"://" ) ) )
    {
        //
        // Expand to a full path name
        //
        LPWSTR pszCheck = _wfullpath( wszFullUrl, pszUrl, MAX_PATH );

        if( NULL == pszCheck )
        {
           DEBUG_LOG(( "internal error %lu\n" , GetLastError() ));
           return E_UNEXPECTED ;
        }

		pszUrl = wszFullUrl;
    }

    //
    // Save a copy of the URL
    //
    delete[] m_pszUrl;

    m_pszUrl = new WCHAR[ wcslen( pszUrl ) + 1 ];

    if( NULL == m_pszUrl )
    {
        DEBUG_LOG(( "insufficient Memory\n"  )) ;
        return( E_OUTOFMEMORY );
    }

    wcscpy( m_pszUrl, pszUrl );

    //
    // Attempt to open the URL
    //
    m_hCompletionEvent = hCompletionEvent;

    m_phrCompletion = phrCompletion;

#ifdef SUPPORT_DRM

    hr = WMCreateReader( NULL, WMT_RIGHT_PLAYBACK, &m_pReader );

#else

    hr = WMCreateReader( NULL, 0, &m_pReader );

#endif

    if( FAILED( hr ) )
    {
        DEBUG_LOG(( "failed to create audio reader (hr=0x%08x)\n" , hr ));
        return( hr );
    }

    //
    // Open the file
    //
    hr = m_pReader->Open( m_pszUrl, this, NULL );
    if ( SUCCEEDED( hr ) )
    {
        WaitForSingleObject( m_hOpenEvent, INFINITE );
        hr = m_hrOpen;
    }
    if ( NS_E_NO_STREAM == hr )
    {
        DEBUG_LOG(( "Waiting for transmission to begin...\n" ));
        WaitForSingleObject( m_hOpenEvent, INFINITE );
        hr = m_hrOpen;
    }
    if ( FAILED( hr ) )
    {
        DEBUG_LOG(( "failed to open (hr=0x%08x)\n", hr ));
        return( hr );
    }
         

    //
    // It worked!  Display various attributes
    //
    hr = m_pReader->QueryInterface( IID_IWMHeaderInfo, ( VOID ** )&m_pHeader );
    if ( FAILED( hr ) )
    {
        DEBUG_LOG(( "failed to qi for header interface (hr=0x%08x)\n" , hr ));
        return( hr );
    }

    WORD i, wAttrCnt;

    hr = m_pHeader->GetAttributeCount( 0, &wAttrCnt );
    if ( FAILED( hr ) )
    {
        DEBUG_LOG(( "GetAttributeCount Failed (hr=0x%08x)\n" , hr ));
        return( hr );
    }

    WCHAR *pwszName = NULL;
    BYTE *pValue = NULL;

    for ( i = 0; i < wAttrCnt ; i++ )
    {
        WORD wStream = 0;
        WORD cchNamelen = 0;
        WMT_ATTR_DATATYPE type;
        WORD cbLength = 0;

        hr = m_pHeader->GetAttributeByIndex( i, &wStream, NULL, &cchNamelen, &type, NULL, &cbLength );
        if ( FAILED( hr ) ) 
        {
            DEBUG_LOG(( "GetAttributeByIndex Failed (hr=0x%08x)\n" , hr ));
            break;
        }

        pwszName = new WCHAR[ cchNamelen ];
        pValue = new BYTE[ cbLength ];

        if( NULL == pwszName || NULL == pValue )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = m_pHeader->GetAttributeByIndex( i, &wStream, pwszName, &cchNamelen, &type, pValue, &cbLength );
        if ( FAILED( hr ) ) 
        {
            DEBUG_LOG(( "GetAttributeByIndex Failed (hr=0x%08x)\n" , hr ));
            break;
        }

        switch ( type )
        {
        case WMT_TYPE_DWORD:
            DEBUG_LOG(("%ws:  %u\n" , pwszName, *((DWORD *) pValue) ));
            break;
        case WMT_TYPE_STRING:
            DEBUG_LOG(("%ws:   %ws\n" , pwszName, (WCHAR *) pValue ));
            break;
        case WMT_TYPE_BINARY:
            DEBUG_LOG(("%ws:   Type = Binary of Length %u\n" , pwszName, cbLength ));
            break;
        case WMT_TYPE_BOOL:
            DEBUG_LOG(("%ws:   %s\n" , pwszName, ( * ( ( BOOL * ) pValue) ? _T( "true" ) : _T( "false" ) ) ));
            break;
        case WMT_TYPE_WORD:
            DEBUG_LOG(("%ws:  %hu\n" , pwszName, *((WORD *) pValue) ));
            break;
        case WMT_TYPE_QWORD:
            DEBUG_LOG(("%ws:  %I64u\n" , pwszName, *((QWORD *) pValue) ));
            break;
        case WMT_TYPE_GUID:
            DEBUG_LOG(("%ws:  %I64x%I64x\n" , pwszName, *((QWORD *) pValue), *((QWORD *) pValue + 1) ));
            break;
        default:
            DEBUG_LOG(("%ws:   Type = %d, Length %u\n" , pwszName, type, cbLength ));
            break;
        }

        delete pwszName;
        pwszName = NULL;

        delete pValue;
        pValue = NULL;
    }

    delete pwszName;
    pwszName = NULL;

    delete pValue;
    pValue = NULL;

    if ( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Make sure we're audio only
    //
    DWORD cOutputs;

    hr = m_pReader->GetOutputCount( &cOutputs );
    if ( FAILED( hr ) )
    {
        DEBUG_LOG(( "failed GetOutputCount(), (hr=0x%08x)\n" , hr ));
        return( hr );
    }

    if ( cOutputs != 1 )
    {
        DEBUG_LOG(( "Not audio only (cOutputs = %d).\n" , cOutputs ));
        // return( E_UNEXPECTED );
    }

    IWMOutputMediaProps *pProps;
    hr = m_pReader->GetOutputProps( 0, &pProps );
    if ( FAILED( hr ) )
    {
        DEBUG_LOG(( "failed GetOutputProps(), (hr=0x%08x)\n" , hr ));
        return( hr );
    }

    DWORD cbBuffer = 0;

    hr = pProps->GetMediaType( NULL, &cbBuffer );
    if ( FAILED( hr ) )
    {
        pProps->Release( );
        DEBUG_LOG(( "GetMediaType failed (hr=0x%08x)\n" , hr ));
        return( hr );
    }
    
	WM_MEDIA_TYPE *pMediaType = ( WM_MEDIA_TYPE * ) new BYTE[cbBuffer] ;

	hr = pProps->GetMediaType( pMediaType, &cbBuffer );
    if ( FAILED( hr ) )
    {
        pProps->Release( );
        DEBUG_LOG(( "GetMediaType failed (hr=0x%08x)\n" , hr ));
        return( hr );
    }

    pProps->Release( );

    if ( pMediaType->majortype != WMMEDIATYPE_Audio )
    {
		delete[] (BYTE *) pMediaType ;
        DEBUG_LOG(( "Not audio only (major type mismatch).\n"  ));
        return( E_UNEXPECTED );
    }
    
    //
    // Set up for audio playback
    //
    WAVEFORMATEX *pwfx = ( WAVEFORMATEX * )pMediaType->pbFormat;
    memcpy( &m_wfx, pwfx, sizeof( WAVEFORMATEX ) + pwfx->cbSize );
    
    delete[] (BYTE *)pMediaType ;
	pMediaType = NULL ;

    MMRESULT mmr;

    mmr = waveOutOpen( &m_hwo,
                       WAVE_MAPPER, 
                       &m_wfx, 
                       (DWORD)WaveProc, 
                       (DWORD)this, 
                       CALLBACK_FUNCTION );
    mmr = MMSYSERR_NOERROR;

    if( mmr != MMSYSERR_NOERROR  )
    {
        
        DEBUG_LOG(( "failed to open wav output device, error=%lu\n" , mmr ));
        return( E_UNEXPECTED );
    }

    //
    // Start reading the data (and rendering the audio)
    //
    QWORD cnsDuration = ( QWORD ) dwSecDuration * 10000000;
    hr = m_pReader->Start( 0, cnsDuration, 1.0, NULL );

    if( FAILED( hr ) )
    {
        DEBUG_LOG(( "failed Start(), (hr=0x%08x)\n" , hr ));
        return( hr );
    }

    return( hr );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CSimplePlayer::OnStatus( 
        /* [in] */ WMT_STATUS Status,
        /* [in] */ HRESULT hr,
        /* [in] */ WMT_ATTR_DATATYPE dwType,
        /* [in] */ BYTE __RPC_FAR *pValue,
        /* [in] */ void __RPC_FAR *pvContext)
{
    switch( Status )
    {
    case WMT_OPENED:
        DEBUG_LOG(( "OnStatus( WMT_OPENED )\n"  ));
        m_hrOpen = hr;
        SetEvent( m_hOpenEvent );
        break;

    case WMT_SOURCE_SWITCH:
        DEBUG_LOG(( "OnStatus( WMT_SOURCE_SWITCH )\n"  ));
        m_hrOpen = hr;
        SetEvent( m_hOpenEvent );
        break;

    case WMT_ERROR:
        DEBUG_LOG(( "OnStatus( WMT_ERROR )\n"  ));
        break;

    case WMT_STARTED:
        DEBUG_LOG(( "OnStatus( WMT_STARTED )\n"  ));
        break;

    case WMT_STOPPED:
        DEBUG_LOG(( "OnStatus( WMT_STOPPED )\n"  ));
        break;

    case WMT_BUFFERING_START:
        DEBUG_LOG(( "OnStatus( WMT_BUFFERING START)\n"  ));
        break;

    case WMT_BUFFERING_STOP:
        DEBUG_LOG(( "OnStatus( WMT_BUFFERING STOP)\n"  ));
        break;

    case WMT_EOF:
        DEBUG_LOG(( "OnStatus( WMT_EOF )\n"  ));

        //
        // cleanup and exit
        //

        m_fEof = TRUE;

        if( 0 == m_cBuffersOutstanding )
        {
            SetEvent( m_hCompletionEvent );
        }

        break;

    case WMT_END_OF_SEGMENT:
        DEBUG_LOG(( "OnStatus( WMT_END_OF_SEGMENT )\n"  ));

        //
        // cleanup and exit
        //

        m_fEof = TRUE;

        if( 0 == m_cBuffersOutstanding )
        {
            SetEvent( m_hCompletionEvent );
        }

        break;

    case WMT_LOCATING:
        DEBUG_LOG(( "OnStatus( WMT_LOCATING )\n"  ));
        break;

    case WMT_CONNECTING:
        DEBUG_LOG(( "OnStatus( WMT_CONNECTING )\n"  ));
        break;

    case WMT_NO_RIGHTS:
        {
            LPWSTR pwszEscapedURL = NULL;

            hr = MakeEscapedURL( m_pszUrl, &pwszEscapedURL );

            if( SUCCEEDED( hr ) )
            {
                WCHAR wszURL[ 0x1000 ];

                swprintf( wszURL, L"%s&filename=%s&embedded=false", pValue, pwszEscapedURL );

                hr = LaunchURL( wszURL );

                if( FAILED( hr ) )
                {
                    DEBUG_LOG(( "Unable to launch web browser to retrieve playback license (hr=0x%08x)\n" , hr ));
                }

                delete [] pwszEscapedURL;
				pwszEscapedURL = NULL ;
            }
        }
        break;

    case WMT_MISSING_CODEC:
		{
			DEBUG_LOG(( "Missing codec: (hr=0x%08x)\n" , hr ));
			break;
		}

    case WMT_CLOSED:
        SetEvent( m_hCloseEvent );
        break;
    };

    return( S_OK );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CSimplePlayer::Close()
{
    HRESULT hr = S_OK;

    if( NULL != m_pReader )
    {
        hr = m_pReader->Close();

        if( SUCCEEDED( hr ) )
        {
            WaitForSingleObject( m_hCloseEvent, INFINITE );
        }
    }

    return( hr );
}


///////////////////////////////////////////////////////////////////////////////
void CSimplePlayer::OnWaveOutMsg( UINT uMsg, DWORD dwParam1, DWORD dwParam2 )
{
    if( WOM_DONE == uMsg )
    {
        //
        // add the wave header to ready-to-free list for the caller
        // to pick up and free in the next OnSample call
        //
        AddWaveHeader( ( LPWAVEHDR )dwParam1 );

        InterlockedDecrement( &m_cBuffersOutstanding );

        if( m_fEof && ( 0 == m_cBuffersOutstanding ) )
        {
            SetEvent( m_hCompletionEvent );
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
void CALLBACK CSimplePlayer::WaveProc( 
                                HWAVEOUT hwo, 
                                UINT uMsg, 
                                DWORD dwInstance, 
                                DWORD dwParam1, 
                                DWORD dwParam2 )
{
    CSimplePlayer *pThis = (CSimplePlayer*)dwInstance;

    pThis->OnWaveOutMsg( uMsg, dwParam1, dwParam2 );
}

//////////////////////////////////////////////////////////////////////////////
HRESULT CSimplePlayer::AddWaveHeader( LPWAVEHDR pwh )
{
    WAVEHDR_LIST *tmp = new WAVEHDR_LIST;
    if( NULL == tmp )
    {
        return( E_OUTOFMEMORY );
    }
    tmp->pwh = pwh;

    EnterCriticalSection( &m_CriSec );
    tmp->next = m_whdrHead;
    m_whdrHead = tmp;
    LeaveCriticalSection( &m_CriSec );
    return( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
void CSimplePlayer::RemoveWaveHeaders( )
{
    WAVEHDR_LIST *tmp;

    EnterCriticalSection( &m_CriSec );
    while( NULL != m_whdrHead )
    {
        tmp = m_whdrHead->next;
        DEBUG_ASSERTCRASH( m_whdrHead->pwh->dwFlags & WHDR_DONE, ("RemoveWaveHeaders!") );
        waveOutUnprepareHeader( m_hwo, m_whdrHead->pwh, sizeof( WAVEHDR ) );
        delete m_whdrHead->pwh;
        delete m_whdrHead;
        m_whdrHead = tmp;
    }
    LeaveCriticalSection( &m_CriSec );
}
