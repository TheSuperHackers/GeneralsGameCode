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

// FILE: DownloadManager.cpp //////////////////////////////////////////////////////
// Generals download manager code
// Author: Matthew D. Campbell, July 2002

#include "debug.h"
#include "CHATAPI.h"
#include "DownloadManager.h"
#include "RESOURCE.h"

namespace patchget
{

DownloadManager *TheDownloadManager = nullptr;

DownloadManager::DownloadManager()
{
	m_download = new CDownload(this);
	m_wasError = m_sawEnd = false;
	m_statusString = Fetch_String(FTP_StatusIdle);

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

HRESULT DownloadManager::downloadFile( std::string server, std::string username, std::string password, std::string file, std::string localfile, std::string regkey, bool tryResume )
{
	return m_download->DownloadFile( server.c_str(), username.c_str(), password.c_str(), file.c_str(), localfile.c_str(), regkey.c_str(), tryResume );
}

void DownloadManager::queueFileForDownload( std::string server, std::string username, std::string password, std::string file, std::string localfile, std::string regkey, bool tryResume )
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

std::string DownloadManager::getLastLocalFile( void )
{
	char buf[256] = "";
	m_download->GetLastLocalFile(buf, 256);
	return buf;
}

HRESULT DownloadManager::OnError( int error )
{
	m_wasError = true;
	std::string s = Fetch_String(FTP_UnknownError);
	switch (error)
	{
		case DOWNLOADEVENT_NOSUCHSERVER:
			s = Fetch_String(FTP_NoSuchServer);
			break;
		case DOWNLOADEVENT_COULDNOTCONNECT:
			s = Fetch_String(FTP_CouldNotConnect);
			break;
		case DOWNLOADEVENT_LOGINFAILED:
			s = Fetch_String(FTP_LoginFailed);
			break;
		case DOWNLOADEVENT_NOSUCHFILE:
			s = Fetch_String(FTP_NoSuchFile);
			break;
		case DOWNLOADEVENT_LOCALFILEOPENFAILED:
			s = Fetch_String(FTP_LocalFileOpenFailed);
			break;
		case DOWNLOADEVENT_TCPERROR:
			s = Fetch_String(FTP_TCPError);
			break;
		case DOWNLOADEVENT_DISCONNECTERROR:
			s = Fetch_String(FTP_DisconnectError);
			break;
	}
	m_errorString = s;
	DEBUG_LOG(("DownloadManager::OnError(): %s(%d)", s.c_str(), error));
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

HRESULT DownloadManager::OnProgressUpdate( int bytesread, int totalsize, int timetaken, int timeleft )
{
	DEBUG_LOG(("DownloadManager::OnProgressUpdate(): %d/%d %d/%d", bytesread, totalsize, timetaken, timeleft));
	return S_OK;
}

HRESULT DownloadManager::OnStatusUpdate( int status )
{
	std::string s = Fetch_String(FTP_StatusNone);
	switch (status)
	{
		case DOWNLOADSTATUS_CONNECTING:
			s = Fetch_String(FTP_StatusConnecting);
			break;
		case DOWNLOADSTATUS_LOGGINGIN:
			s = Fetch_String(FTP_StatusLoggingIn);
			break;
		case DOWNLOADSTATUS_FINDINGFILE:
			s = Fetch_String(FTP_StatusFindingFile);
			break;
		case DOWNLOADSTATUS_QUERYINGRESUME:
			s = Fetch_String(FTP_StatusQueryingResume);
			break;
		case DOWNLOADSTATUS_DOWNLOADING:
			s = Fetch_String(FTP_StatusDownloading);
			break;
		case DOWNLOADSTATUS_DISCONNECTING:
			s = Fetch_String(FTP_StatusDisconnecting);
			break;
		case DOWNLOADSTATUS_FINISHING:
			s = Fetch_String(FTP_StatusFinishing);
			break;
		case DOWNLOADSTATUS_DONE:
			s = Fetch_String(FTP_StatusDone);
			break;
	}
	m_statusString = s;
	DEBUG_LOG(("DownloadManager::OnStatusUpdate(): %s(%d)", s.c_str(), status));
	return S_OK;
}

} // namespace patchget
