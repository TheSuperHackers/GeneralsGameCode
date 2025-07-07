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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: DownloadManager.cpp //////////////////////////////////////////////////////
// Generals download manager code
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/GameText.h"
#include "GameNetwork/DownloadManager.h"

DownloadManager *TheDownloadManager;

DownloadManager::DownloadManager()
{
	m_download = NEW CDownload(this);
	m_wasError = m_sawEnd = false;
	
	//Added By Sadullah Nader
	//Initializations missing and needed
	
	m_queuedDownloads.clear();
	
	//

	m_statusString = TheGameText->fetch("FTP:StatusIdle");

	// ----- Initialize Winsock -----
	m_winsockInit = true;
	WORD verReq = MAKEWORD(2, 2);
	WSADATA wsadata;

	int err = WSAStartup(verReq, &wsadata);
	if (err != 0)
	{
		m_winsockInit = false;
	}
	else
	{
		if ((LOBYTE(wsadata.wVersion) != 2) || (HIBYTE(wsadata.wVersion) !=2))
		{
			WSACleanup();
			m_winsockInit = false;
		}
	}

}

DownloadManager::~DownloadManager()
{
	delete m_download;
	if (m_winsockInit)
	{
		WSACleanup();
		m_winsockInit = false;
	}
}

void DownloadManager::init( void )
{
}

void DownloadManager::reset( void )
{
}

HRESULT DownloadManager::update( void )
{
	return m_download->PumpMessages();
}

HRESULT DownloadManager::downloadFile( AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString regkey, Bool tryResume )
{
	return m_download->DownloadFile( server.str(), username.str(), password.str(), file.str(), localfile.str(), regkey.str(), tryResume );
}

void DownloadManager::queueFileForDownload( AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString regkey, Bool tryResume )
{
	QueuedDownload q;
	q.file = file;
	q.localFile = localfile;
	q.password = password;
	q.regKey = regkey;
	q.server = server;
	q.tryResume = tryResume;
	q.userName = username;

	m_queuedDownloads.push_back(q);
}

HRESULT DownloadManager::downloadNextQueuedFile( void )
{
	QueuedDownload q;
	std::list<QueuedDownload>::iterator it = m_queuedDownloads.begin();
	if (it != m_queuedDownloads.end())
	{
		q = *it;
		m_queuedDownloads.pop_front();
		m_wasError = m_sawEnd = false;
		return downloadFile( q.server, q.userName, q.password, q.file, q.localFile, q.regKey, q.tryResume );
	}
	else
	{
		DEBUG_CRASH(("Starting non-existent download!"));
		return S_OK;
	}
}

AsciiString DownloadManager::getLastLocalFile( void )
{
	char buf[256] = "";
	m_download->GetLastLocalFile(buf, 256);
	return buf;
}

HRESULT DownloadManager::OnError( Int error )
{
	m_wasError = true;
	AsciiString s = "FTP:UnknownError";
	switch (error)
	{
		case DOWNLOADEVENT_NOSUCHSERVER:
			s = "FTP:NoSuchServer";
			break;
		case DOWNLOADEVENT_COULDNOTCONNECT:
			s = "FTP:CouldNotConnect";
			break;
		case DOWNLOADEVENT_LOGINFAILED:
			s = "FTP:LoginFailed";
			break;
		case DOWNLOADEVENT_NOSUCHFILE:
			s = "FTP:NoSuchFile";
			break;
		case DOWNLOADEVENT_LOCALFILEOPENFAILED:
			s = "FTP:LocalFileOpenFailed";
			break;
		case DOWNLOADEVENT_TCPERROR:
			s = "FTP:TCPError";
			break;
		case DOWNLOADEVENT_DISCONNECTERROR:
			s = "FTP:DisconnectError";
			break;
	}
	m_errorString = TheGameText->fetch(s);
	DEBUG_LOG(("DownloadManager::OnError(): %s(%d)", s.str(), error));
	return S_OK;
}

HRESULT DownloadManager::OnEnd()
{
	m_sawEnd = true;
	DEBUG_LOG(("DownloadManager::OnEnd()"));
	return S_OK;
}

HRESULT DownloadManager::OnQueryResume()
{
	DEBUG_LOG(("DownloadManager::OnQueryResume()"));
	//return DOWNLOADEVENT_DONOTRESUME;
	return DOWNLOADEVENT_RESUME;
}

HRESULT DownloadManager::OnProgressUpdate( Int bytesread, Int totalsize, Int timetaken, Int timeleft )
{
	DEBUG_LOG(("DownloadManager::OnProgressUpdate(): %d/%d %d/%d", bytesread, totalsize, timetaken, timeleft));
	return S_OK;
}

HRESULT DownloadManager::OnStatusUpdate( Int status )
{
	AsciiString s = "FTP:StatusNone";
	switch (status)
	{
		case DOWNLOADSTATUS_CONNECTING:
			s = "FTP:StatusConnecting";
			break;
		case DOWNLOADSTATUS_LOGGINGIN:
			s = "FTP:StatusLoggingIn";
			break;
		case DOWNLOADSTATUS_FINDINGFILE:
			s = "FTP:StatusFindingFile";
			break;
		case DOWNLOADSTATUS_QUERYINGRESUME:
			s = "FTP:StatusQueryingResume";
			break;
		case DOWNLOADSTATUS_DOWNLOADING:
			s = "FTP:StatusDownloading";
			break;
		case DOWNLOADSTATUS_DISCONNECTING:
			s = "FTP:StatusDisconnecting";
			break;
		case DOWNLOADSTATUS_FINISHING:
			s = "FTP:StatusFinishing";
			break;
		case DOWNLOADSTATUS_DONE:
			s = "FTP:StatusDone";
			break;
	}
	m_statusString = TheGameText->fetch(s);
	DEBUG_LOG(("DownloadManager::OnStatusUpdate(): %s(%d)", s.str(), status));
	return S_OK;
}
