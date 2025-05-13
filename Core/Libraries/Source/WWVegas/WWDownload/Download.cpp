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

// Download.cpp : Implementation of CDownload
#include "DownloadDebug.h"
#include "Download.h"
#include <mmsystem.h>
#include <assert.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

/////////////////////////////////////////////////////////////////////////////
// CDownload


//
// Kick off a download - copys the parameters for the download to the appropriate
// data members, then sets the status flag to start the download when PumpMessages()
// is called.
//

HRESULT CDownload::DownloadFile(LPCSTR server, LPCSTR username, LPCSTR password, LPCSTR file, LPCSTR localfile, LPCSTR regkey, bool tryresume)
{
 	// Check we're not in the middle of another download.
	if((m_Status != DOWNLOADSTATUS_DONE ) && (m_Status != DOWNLOADSTATUS_FINDINGFILE))
	{
		return( DOWNLOAD_STATUSERROR );
	}

	// If we're still connected, make sure we're on the right server
	if (m_Status == DOWNLOADSTATUS_FINDINGFILE)
	{
		if ((strcmp(m_Server, server)) || (strcmp(m_Login, username)))
		{
			// Damn, a server switch.  Close conn & fix state
			m_Ftp->DisconnectFromServer();
			m_Status=DOWNLOADSTATUS_DONE;
		}
	}

	// Check all parameters are non-null.

	if( ( server == NULL ) || ( username == NULL ) ||
		( password == NULL ) || ( file == NULL ) ||
		( localfile == NULL ) || ( regkey == NULL ) )
	{
     //////////DBGMSG("Download Paramerror");
		return( DOWNLOAD_PARAMERROR );
	}

	// Make sure we have a download directory
	_mkdir("download");

	// Copy parameters to member variables.
	strncpy( m_Server, server, sizeof( m_Server ) );
	strncpy( m_Login, username, sizeof( m_Login ) );
	strncpy( m_Password, password, sizeof( m_Password ) );
	strncpy( m_File, file, sizeof( m_File ) );
	strncpy( m_LocalFile, localfile, sizeof( m_LocalFile ) );

	strncpy( m_LastLocalFile, localfile, sizeof( m_LastLocalFile ) );

	strncpy( m_RegKey, regkey, sizeof( m_RegKey ) );
	m_TryResume = tryresume;
	m_StartPosition=0;

/*********
	// figure out file offset
	if (m_TryResume == false)
	{
		m_StartPosition=0;
	}
	else if(( m_StartPosition = m_Ftp->FileRecoveryPosition( m_LocalFile, m_RegKey ) ) != 0 )
	{
		if( Listener->OnQueryResume() == DOWNLOADEVENT_DONOTRESUME )
		{
			m_Ftp->RestartFrom( 0 );
			m_StartPosition = 0;
		}
	}
************/

	// Set status so we start to connect at the next PumpMessages()
	if (m_Status != DOWNLOADSTATUS_FINDINGFILE)
		m_Status = DOWNLOADSTATUS_GO;

   //////////DBGMSG("Ready to download");
	return S_OK;
}


//
// Get the local filename of the last file we requested to download....
//
HRESULT CDownload::GetLastLocalFile(char *local_file, int maxlen) {
	if (local_file==0)
		return(E_FAIL);

	strncpy(local_file, m_LastLocalFile, maxlen);
	local_file[maxlen-1]=0;

	return(S_OK);
}


//
// Abort the current download - disconnects from the server and sets the
// status flag to stop PumpMessages() from doing anything.
//

HRESULT CDownload::Abort()
{

	if( m_Status != DOWNLOADSTATUS_NONE )
	{
		m_Ftp->DisconnectFromServer();
		m_Status = DOWNLOADSTATUS_NONE;
	}

	m_TimeStarted		= 0;
	m_StartPosition		= 0;
	m_FileSize			= 0;
	m_BytesRead			= 0;
	m_Server[ 0 ]		= '\0';
	m_Login[ 0 ]		= '\0';
	m_Password[ 0 ]		= '\0';
	m_File[ 0 ]			= '\0';
	m_LocalFile[ 0 ]	= '\0';
	m_RegKey[ 0 ]		= '\0';

	return S_OK;
}


//
// PumpMessages() does all the work - checks for possible resumption of a previous
// download, connects to the server, finds the file and downloads it.
//

HRESULT CDownload::PumpMessages()
{
	int iResult = 0;
	int timetaken = 0;
	int averagetimepredicted = -1;
	static int reenter = 0;

	/* Avoid reentrancy. */

   ////DBGMSG("Download Pump");

	if( reenter != 0 )
	{
		return( DOWNLOAD_REENTERERROR );
	}

	reenter = 1;


	if( m_Status == DOWNLOADSTATUS_GO )
	{
		// Check to see whether the file already exists locally 

/***********
		if (m_TryResume == false)
		{
			m_StartPosition = 0;
		}
		else if(	( m_StartPosition = m_Ftp->FileRecoveryPosition( m_LocalFile, m_RegKey ) ) != 0 )
		{
			if( Listener->OnQueryResume() == DOWNLOADEVENT_DONOTRESUME )
			{
				m_Ftp->RestartFrom( 0 );
				m_StartPosition = 0;
			}
		}
************/

		// Tell the client that we're starting to connect.

		Listener->OnStatusUpdate( DOWNLOADSTATUS_CONNECTING );
		
		m_Status = DOWNLOADSTATUS_CONNECTING;
	}


	if( m_Status == DOWNLOADSTATUS_CONNECTING )
	{
		// Connect to the server
      //////////DBGMSG("Connect to Server");
				
		iResult = m_Ftp->ConnectToServer( m_Server );
      ////////////DBGMSG("out of FTP connect");

		if( iResult == FTP_SUCCEEDED )
		{
			m_Status = DOWNLOADSTATUS_LOGGINGIN;
		}
		else
		{
			if( iResult == FTP_FAILED )
			{
				// Tell the client we couldn't connect
            ///////////DBGMSG("Couldn't connect");
				Listener->OnError( DOWNLOADEVENT_COULDNOTCONNECT );
				reenter = 0;
				return DOWNLOAD_NETWORKERROR;
			}
		}
		reenter = 0;
		return DOWNLOAD_SUCCEEDED;
	}

	if( m_Status == DOWNLOADSTATUS_LOGGINGIN )
	{
		// Login to the server
      ////////////DBGMSG("Login to server");

		iResult = m_Ftp->LoginToServer( m_Login, m_Password );

		if( iResult == FTP_SUCCEEDED )
		{
			Listener->OnStatusUpdate( DOWNLOADSTATUS_FINDINGFILE );
			m_Status = DOWNLOADSTATUS_FINDINGFILE;
		}

		if( iResult == FTP_FAILED )
		{
			// Tell the client we couldn't log in

			Listener->OnError( DOWNLOADEVENT_LOGINFAILED );
			reenter = 0;
			return( DOWNLOAD_NETWORKERROR );
		}

		reenter = 0;
		return DOWNLOAD_SUCCEEDED;
	}

	if(( m_Status == DOWNLOADSTATUS_FINDINGFILE ) && (strlen(m_File)))
	{
		
		// Find the file on the server
      ///////////DBGMSG("Find File");

		if( m_Ftp->FindFile( m_File, &m_FileSize ) == FTP_FAILED )
		{
			// Tell the client we couldn't find the file

			Listener->OnError( DOWNLOADEVENT_NOSUCHFILE );
			reenter = 0;
			return( DOWNLOAD_FILEERROR );
		}

		if( m_FileSize > 0 )
		{
			// First check to see if we already have the complete file in the proper dest location...
			//
			// Note, we will only skip the download if the file is a patch.  Patch files should
			//   never ever change so this is not a concern.
			//
			// We identify patches because they are written into the patches folder.
			struct _stat statdata;
			if (	(_stat(m_LocalFile, &statdata) == 0) && 
					(statdata.st_size == m_FileSize) && 
					(_strnicmp(m_LocalFile, "patches\\", strlen("patches\\"))==0)) {
				// OK, no need to download this again....

				m_Status				= DOWNLOADSTATUS_FINDINGFILE;  // ready to find another file
				m_TimeStarted		= 0;
				m_StartPosition	= 0;
				m_FileSize			= 0;
				m_BytesRead			= 0;
				m_File[ 0 ]			= '\0';
				m_LocalFile[ 0 ]	= '\0';
			
				Listener->OnEnd();

				reenter = 0;
				return DOWNLOAD_SUCCEEDED;
			}


			//
			// Check if we can do a file resume
			//
			if (m_TryResume == false)
			{
				m_StartPosition = 0;
			}
			else if(	( m_StartPosition = m_Ftp->FileRecoveryPosition( m_LocalFile, m_RegKey ) ) != 0 )
			{
				if( Listener->OnQueryResume() == DOWNLOADEVENT_DONOTRESUME )
				{
					m_Ftp->RestartFrom( 0 );
					m_StartPosition = 0;
				}
			}
			// end resume check


			m_Status = DOWNLOADSTATUS_DOWNLOADING;

			// Tell the client we're starting to download

			Listener->OnStatusUpdate( DOWNLOADSTATUS_DOWNLOADING );
		}

		reenter = 0;
		return DOWNLOAD_SUCCEEDED;
	}

	if( m_Status == DOWNLOADSTATUS_DOWNLOADING )
	{

		// Get the next chunk of the file
      ///DBGMSG("Get Next File Block");

		iResult = m_Ftp->GetNextFileBlock( m_LocalFile, &m_BytesRead );

		if( m_TimeStarted == 0 )
		{
			// This is the first time through here - record the starting time.
			m_TimeStarted = timeGetTime();
		}

		if( iResult == FTP_SUCCEEDED )
		{
			m_Status = DOWNLOADSTATUS_FINISHING;
		}
		else
		{
			if( iResult == FTP_FAILED )
			{
				Listener->OnError( DOWNLOADEVENT_TCPERROR );
				reenter = 0;
				return( DOWNLOAD_NETWORKERROR );
			}
		}

		// Calculate time taken so far, and predict how long there is left.
		// The prediction returned is the average of the last 8 predictions.

		timetaken = ( timeGetTime() - m_TimeStarted ) / 1000;

		//////////if( m_BytesRead > 0 ) // NAK - RP said this is wrong
      if( ( m_BytesRead - m_StartPosition ) > 0 )
		{
			// Not the first read.
			int predictionIndex = ( m_predictions++ ) & 0x7;
			m_predictionTimes[ predictionIndex ] = MulDiv( timetaken, (m_FileSize - m_BytesRead), (m_BytesRead - m_StartPosition) );
			//__int64 numerator = ( m_FileSize - m_BytesRead )  * timetaken;
			//__int64 denominator = ( m_BytesRead - m_StartPosition );
			//m_predictionTimes[ predictionIndex ] = numerator/denominator;
			//m_predictionTimes[ predictionIndex ] = ( ( m_FileSize - m_BytesRead )  * timetaken ) / ( m_BytesRead - m_StartPosition );
			//m_predictionTimes[ predictionIndex ] = ( ( m_FileSize - m_BytesRead ) / ( m_BytesRead - m_StartPosition ) * timetaken );

			DEBUG_LOG(("About to OnProgressUpdate() - m_FileSize=%d, m_BytesRead=%d, timetaken=%d, numerator=%d",
				m_FileSize, m_BytesRead, timetaken, ( ( m_FileSize - m_BytesRead )  * timetaken )));
			DEBUG_LOG((", m_startPosition=%d, denominator=%d, predictionTime=%d\n",
				m_StartPosition, ( m_BytesRead - m_StartPosition ), predictionIndex));
			DEBUG_LOG(("vals are %d %d %d %d %d %d %d %d\n",
				m_predictionTimes[ 0 ], m_predictionTimes[ 1 ], m_predictionTimes[ 2 ], m_predictionTimes[ 3 ],
				m_predictionTimes[ 4 ], m_predictionTimes[ 5 ], m_predictionTimes[ 6 ], m_predictionTimes[ 7 ]));

			if( m_predictions > 8 )
			{
				// We've had enough time to build up a few predictions, so calculate
				// an average.
				averagetimepredicted = ( m_predictionTimes[ 0 ] + m_predictionTimes[ 1 ] +
										 m_predictionTimes[ 2 ] + m_predictionTimes[ 3 ] +
										 m_predictionTimes[ 4 ] + m_predictionTimes[ 5 ] +
										 m_predictionTimes[ 6 ] + m_predictionTimes[ 7 ] ) / 8;
			}
			else
			{
				// Wait a while longer before passing a "real" prediction
				averagetimepredicted = -1;
			}

		}
		else
		{
			averagetimepredicted = -1;
		}

		// Update the client's progress bar (or whatever)

		Listener->OnProgressUpdate( m_BytesRead, m_FileSize, timetaken, averagetimepredicted + 1);
	}


	if( m_Status == DOWNLOADSTATUS_FINISHING )
	{
		if (m_Ftp->m_iCommandSocket)
			m_Status = DOWNLOADSTATUS_FINDINGFILE;  // ready to find another file
		else
			m_Status = DOWNLOADSTATUS_DONE;			// command channel closed, connect again...

		m_TimeStarted		= 0;
		m_StartPosition		= 0;
		m_FileSize			= 0;
		m_BytesRead			= 0;
		m_File[ 0 ]			= '\0';
		m_LocalFile[ 0 ]	= '\0';
	
		Listener->OnEnd();
	}

	reenter = 0;
	return DOWNLOAD_SUCCEEDED;
}
