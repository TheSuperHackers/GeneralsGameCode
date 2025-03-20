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

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   WPAudio
//
// Module:    AUD
//
// File name: AUD_Windows.cpp
//
// Created:   5/09/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmreg.h>

#include <wpaudio/thread.h>
#include <wpaudio/memory.h>
#include <wpaudio/Source.h>
#include <wsys/file.h>
#include <wpaudio/device.h>
#include <wpaudio/profiler.h>
#include <wpaudio/win32.h>

// 'assignment within condition expression'.
#pragma warning(disable : 4706)


//----------------------------------------------------------------------------
//         Externals                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines                                                         
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types                                                     
//----------------------------------------------------------------------------

struct _aud_thread
{
	char										name[200];
	volatile int						quit;				/* thread must quit */
	volatile int						count;
	volatile int						leaving;		/* thread is quiting */
	volatile TimeStamp			interval;		/* itask interval */
	volatile int						running;		/* thread is running */
	HANDLE									handle;			/* threads handle (windows) */
	void										*data;
	AUD_ThreadCB						*code;
	DWORD										id;					/* thread id (windows) */
	CRITICAL_SECTION				access;
	AudioServiceInfo				update;
	ProfileCPU							cpu;

	DBG_TYPE()
};

DBG_DECLARE_TYPE ( AUD_Thread )


//----------------------------------------------------------------------------
//         Private Data                                                     
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data                                                      
//----------------------------------------------------------------------------

static HWND audioMainWindowHandle = NULL;

//----------------------------------------------------------------------------
//         Private Prototypes                                               
//----------------------------------------------------------------------------

static DWORD		WINAPI		AUD_service_thread ( VOID *data );


//----------------------------------------------------------------------------
//         Private Functions                                               
//----------------------------------------------------------------------------

static DWORD		WINAPI		AUD_service_thread ( VOID *data )
{
	AUD_Thread	*thread = (AUD_Thread *) data;

	if ( !thread )
	{
		return 0;
	}

	AUD_ThreadBeginCriticalSection ( thread );

	thread->running = TRUE;
	thread->leaving = FALSE;

	while ( !thread->quit )
	{
		if ( thread->code ( thread, thread->data ))
		{
			AUD_ThreadEndCriticalSection ( thread );
			Sleep ( (unsigned long ) thread->interval );
			AUD_ThreadBeginCriticalSection ( thread );
		}
		else
		{
			AUD_ThreadEndCriticalSection ( thread );
			Sleep ( MSECONDS(5));
			AUD_ThreadBeginCriticalSection ( thread );
		}

		thread->count++;
	}

	AUD_ThreadEndCriticalSection ( thread );
	thread->leaving = TRUE;
	return 0;
}


//----------------------------------------------------------------------------
//         Public Functions                                                
//----------------------------------------------------------------------------

AUD_Thread*	AUD_ThreadCreate ( const char *name, AUD_ThreadPriority pri, AUD_ThreadCB *code )
{
	AUD_Thread *thread;

	ALLOC_STRUCT ( thread, AUD_Thread );

	DBG_SET_TYPE ( thread, AUD_Thread );

	if ( !name )
	{
		name = "no name given";
	}

	strncpy ( thread->name, name, ArrayLen(thread->name));
	ArrayEnd(thread->name);
	thread->quit = FALSE;
	thread->leaving = FALSE;
	thread->running = FALSE;
	thread->count = 0;
	thread->code = code;
	AudioServiceInfoInit ( &thread->update );
	ProfileCPUInit ( thread->cpu );
	InitializeCriticalSection ( &thread->access );


	AUD_ThreadSetInterval ( thread, SECONDS(1)/30 );

	if ( !(thread->handle = CreateThread ( NULL, 4*1024,  AUD_service_thread, thread, 0, &thread->id )))
	{
		DBGPRINTF (( "ERROR: Failed to create audio thread: '%s'\n", thread->name ));
		return NULL;
	}

	int set;

	switch (pri)
	{
		case AUD_THREAD_PRI_NORMAL:
			set = TRUE;
			break;
		case AUD_THREAD_PRI_HIGH:
			set = SetThreadPriority ( thread->handle, THREAD_PRIORITY_HIGHEST );
			break;
		case AUD_THREAD_PRI_REALTIME:
			set = SetThreadPriority ( thread->handle, THREAD_PRIORITY_TIME_CRITICAL );
			break;
		default:
			DBG_MSGASSERT ( FALSE, ("Illegal thread priority"));
			set = TRUE;
	}

	if ( !set )
	{
		char buffer[1024];

		FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buffer, sizeof(buffer), NULL);
		DBGPRINTF (( "Unable to change the priority of the thread - %s\n", buffer ));
		msg_assert ( FALSE, ( "Unable to change the priority of the thread - %s\n", buffer ));
	}

	DBGPRINTF (( "Created audio thread: '%s'\n", thread->name ));

	return thread;
}

//============================================================================
// AUD_ThreadDestroy 
//============================================================================

void				AUD_ThreadDestroy ( AUD_Thread *thread )
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );

	thread->quit = TRUE;

	while ( !thread->leaving );

	WaitForSingleObject ( thread->handle, SECONDS(5));
	CloseHandle ( thread->handle );
	DeleteCriticalSection ( &thread->access );
	DBGPRINTF (( "Removed audio thread: '%s'\n", thread->name ));
	DBG_INVALIDATE_TYPE ( thread );
	AudioMemFree ( thread );
}

//============================================================================
// AUD_ThreadBeginCriticalSection 
//============================================================================

void				AUD_ThreadBeginCriticalSection ( AUD_Thread *thread)
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );
	EnterCriticalSection ( &thread->access );
}

//============================================================================
// AUD_ThreadEndCriticalSection 
//============================================================================

void				AUD_ThreadEndCriticalSection ( AUD_Thread *thread )
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );
	LeaveCriticalSection ( &thread->access );
}

//============================================================================
// AUD_ThreadSetData 
//============================================================================

void				AUD_ThreadSetData ( AUD_Thread *thread, void *data )
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );
	AUD_ThreadBeginCriticalSection ( thread );
	thread->data = data;
	AUD_ThreadEndCriticalSection ( thread );

}

//============================================================================
// AUD_ThreadSetInterval 
//============================================================================

void				AUD_ThreadSetInterval ( AUD_Thread *thread, TimeStamp interval )
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );
	AUD_ThreadBeginCriticalSection ( thread );
	thread->interval = interval;
	AudioServiceSetInterval ( &thread->update, thread->interval );
	AudioServiceSetMustServiceInterval ( &thread->update, thread->interval*4 );
	AudioServiceSetResetInterval ( &thread->update, thread->interval*4 );
	AUD_ThreadEndCriticalSection ( thread );

}

//============================================================================
// AUD_ThreadGetInterval 
//============================================================================

TimeStamp				AUD_ThreadGetInterval ( AUD_Thread *thread )
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );

	return thread->interval;

}

//============================================================================
// AUD_ThreadName
//============================================================================

char*				AUD_ThreadName( AUD_Thread *thread )
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );
	return thread->name;
}

//============================================================================
// AUD_ThreadCPUProfile
//============================================================================

ProfileCPU*		AUD_ThreadCPUProfile( AUD_Thread *thread )
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );
	return &thread->cpu;
}

//============================================================================
// AUD_ThreadAudioServiceInfo
//============================================================================

AudioServiceInfo*		AUD_ThreadServiceInfo( AUD_Thread *thread )
{
	DBG_ASSERT_TYPE ( thread, AUD_Thread );
	return &thread->update;
}

//============================================================================
// AudioFormatReadWaveFile 
//============================================================================

int AudioFormatReadWaveFile ( File *file, AudioFormat *format, int *bytes )
{
	RIFF_HEADER rh;
	RIFF_CHUNK	chunk;
	WAVEFORMATEX *wformat = NULL;
	int got_format = FALSE;
	int result = FALSE;
	//int mp3 = FALSE;


	DBG_ASSERT_TYPE ( format, AudioFormat );


	if ( bytes )
	{
		*bytes = 0;
	}

	if ( !file )
	{
		goto done;
	}


	file->seek ( 0, File::START );

	/* read wav info */
	if (	file->read ( &rh, sizeof (rh)) != sizeof(rh) )
	{
		DBGPRINTF (( "error: cannot read file\n" ));
		goto done;
	}

	if ( rh.form != vRIFF ||	rh.type != vWAVE )
	{
		file->seek ( 0, File::START );
		result = AudioFormatReadMP3File ( file, format, bytes );
		goto done;
	}

	while (	file->read ( &chunk, sizeof (chunk) ) == sizeof(chunk) )
	{
		switch ( chunk.type )
		{
			case vFMT :

					if ( chunk.length < sizeof ( WAVEFORMATEX ) )
					{
						wformat = (WAVEFORMATEX *) malloc ( sizeof(WAVEFORMATEX));
						memset ( wformat, 0, sizeof ( WAVEFORMATEX) );
					}
					else
					{
						wformat = (WAVEFORMATEX *) malloc ( chunk.length );
						memset ( wformat, 0, chunk.length );
					}
				file->read ( wformat, chunk.length  );
				wformat->cbSize = (ushort) chunk.length;
				got_format = TRUE;
				break;
			case vDATA:
				*bytes = chunk.length;
				goto got_data;
			default: 
				file->seek ( chunk.length, File::CURRENT );
				break;
		}
	}

	DBGPRINTF (( "no data chunk found\n" ));

	goto done;

got_data:

	if ( !wformat )
	{
		DBGPRINTF (( "no format chunk found\n" ));
		goto done;
	}


	format->SampleWidth = wformat->wBitsPerSample / 8;

	if ( wformat->wFormatTag == WAVE_FORMAT_IMA_ADPCM )
	{
		format->cdata.adpcm.BlockSize = wformat->nBlockAlign;
		format->Compression = AUDIO_COMPRESS_IMA_ADPCM;
		format->SampleWidth = 2;
	}
	else if ( wformat->wFormatTag == WAVE_FORMAT_ADPCM )
	{
		ADPCMWAVEFORMAT *aformat = (ADPCMWAVEFORMAT *)wformat;

		if ( aformat->wNumCoef != 7 && memcmp ( aformat->aCoef, MSADPCM_StdCoef, sizeof ( MSADPCM_StdCoef )) )
		{
			//currently we only support MS ADPCM using the standard coef table
			goto done;
		}
		format->cdata.adpcm.BlockSize = wformat->nBlockAlign;
		format->Compression = AUDIO_COMPRESS_MS_ADPCM;
		format->SampleWidth = 2;

	}
	else if ( wformat->wFormatTag == WAVE_FORMAT_PCM )
	{
		format->Compression = AUDIO_COMPRESS_NONE;
	}
	else if ( wformat->wFormatTag == WAVE_FORMAT_MPEGLAYER3 )
	{
		result =  AudioFormatReadMP3File ( file, format, NULL );
		goto done;
	}
	else 
	{
		goto done;
	}


	format->Channels = wformat->nChannels;
	format->BytesPerSecond = wformat->nAvgBytesPerSec;
	format->Rate = wformat->nSamplesPerSec;
	format->Flags = mAUDIO_FORMAT_PCM;

	AudioFormatUpdate ( format );

	result = TRUE;

done:

	if ( wformat )
	{
		free ( wformat );
	}
		
	return result;

}

//============================================================================
// WindowsDebugPrint
//============================================================================

void	WindowsDebugPrint( const char * lpOutputString )
{
	OutputDebugStringA ( lpOutputString );
}

//============================================================================
// AudioSetWindowsHandle 
//============================================================================

void AudioSetWindowsHandle ( HWND hwnd )
{
	audioMainWindowHandle = hwnd;

}

//============================================================================
// AudioGetWindowsHandle 
//============================================================================

HWND AudioGetWindowsHandle ( void )
{
	return audioMainWindowHandle;

}
