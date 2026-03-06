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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: DownloadManager.h //////////////////////////////////////////////////////
// Generals download class definitions
// Author: Matthew D. Campbell, July 2002

#pragma once

#include "WWDownload/downloaddefs.h"
#include "WWDownload/Download.h"

class CDownload;
class QueuedDownload
{
public:
	AsciiString server;
	AsciiString userName;
	AsciiString password;
	AsciiString file;
	AsciiString localFile;
	AsciiString regKey;
	Bool tryResume;
};

/////////////////////////////////////////////////////////////////////////////
// DownloadManager

class DownloadManager : public IDownload
{
public:
	DownloadManager();
	virtual ~DownloadManager();

public:
	void init();
	HRESULT update();
	void reset();

	virtual HRESULT OnError( Int error );
	virtual HRESULT OnEnd();
	virtual HRESULT OnQueryResume();
	virtual HRESULT OnProgressUpdate( Int bytesread, Int totalsize, Int timetaken, Int timeleft );
	virtual HRESULT OnStatusUpdate( Int status );

	virtual HRESULT downloadFile( AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString regkey, Bool tryResume );
	AsciiString getLastLocalFile();

	Bool isDone() { return m_sawEnd || m_wasError; }
	Bool isOk() { return m_sawEnd; }
	Bool wasError() { return m_wasError; }

	UnicodeString getStatusString() { return m_statusString; }
	UnicodeString getErrorString() { return m_errorString; }

	void queueFileForDownload( AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString regkey, Bool tryResume );
	Bool isFileQueuedForDownload() { return !m_queuedDownloads.empty(); }
	HRESULT downloadNextQueuedFile();

private:
	Bool m_winsockInit;
	CDownload *m_download;
	Bool m_wasError;
	Bool m_sawEnd;
	UnicodeString m_errorString;
	UnicodeString m_statusString;

protected:
	std::list<QueuedDownload> m_queuedDownloads;
};

extern DownloadManager *TheDownloadManager;
