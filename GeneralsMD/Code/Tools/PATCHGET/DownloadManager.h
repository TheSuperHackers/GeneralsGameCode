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

// FILE: DownloadManager.h //////////////////////////////////////////////////////
// Generals download class definitions
// Author: Matthew D. Campbell, July 2002

#pragma once

#ifndef __DOWNLOADMANAGER_H__
#define __DOWNLOADMANAGER_H__

#include "WWDownload/downloaddefs.h"
#include "WWDownload/Download.h"
#include <string>
#include <list>

class CDownload;

namespace patchget
{

class QueuedDownload
{
public:
	std::string server;
	std::string userName;
	std::string password;
	std::string file;
	std::string localFile;
	std::string regKey;
	bool tryResume;
};

/////////////////////////////////////////////////////////////////////////////
// DownloadManager

class DownloadManager : public IDownload
{
public:
	DownloadManager();
	virtual ~DownloadManager();
	
public:
	void init( void );
	HRESULT update( void );
	void reset( void );

	virtual HRESULT OnError( int error );
	virtual HRESULT OnEnd();
	virtual HRESULT OnQueryResume();
	virtual HRESULT OnProgressUpdate( int bytesread, int totalsize, int timetaken, int timeleft );
	virtual HRESULT OnStatusUpdate( int status );

	virtual HRESULT downloadFile( std::string server, std::string username, std::string password, std::string file, std::string localfile, std::string regkey, bool tryResume );
	std::string getLastLocalFile( void );

	bool isDone( void ) { return m_sawEnd || m_wasError; }
	bool isOk( void ) { return m_sawEnd; }
	bool wasError( void ) { return m_wasError; }

	std::string getStatusString( void ) { return m_statusString; }
	std::string getErrorString( void ) { return m_errorString; }

	void queueFileForDownload( std::string server, std::string username, std::string password, std::string file, std::string localfile, std::string regkey, bool tryResume );
	bool isFileQueuedForDownload( void ) { return !m_queuedDownloads.empty(); }
	HRESULT downloadNextQueuedFile( void );

private:
	bool m_winsockInit;
	CDownload *m_download;
	bool m_wasError;
	bool m_sawEnd;
	std::string m_errorString;
	std::string m_statusString;

protected:
	std::list<QueuedDownload> m_queuedDownloads;
};

extern DownloadManager *TheDownloadManager;

} // namespace patchget

#endif // __DOWNLOADMANAGER_H__
