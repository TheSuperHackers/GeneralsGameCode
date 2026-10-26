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
	void init();
	HRESULT update();
	void reset();

	virtual HRESULT OnError(int error);
	virtual HRESULT OnEnd();
	virtual HRESULT OnQueryResume();
	virtual HRESULT OnProgressUpdate(int bytesread, int totalsize, int timetaken, int timeleft);
	virtual HRESULT OnStatusUpdate(int status);

	virtual HRESULT downloadFile(std::string server, std::string username, std::string password, std::string file, std::string localfile, std::string regkey, bool tryResume);
	std::string getLastLocalFile();

	bool isDone() { return m_sawEnd || m_wasError; }
	bool isOk() { return m_sawEnd; }
	bool wasError() { return m_wasError; }

	std::string getStatusString() { return m_statusString; }
	std::string getErrorString() { return m_errorString; }

	void queueFileForDownload(std::string server, std::string username, std::string password, std::string file, std::string localfile, std::string regkey, bool tryResume);
	bool isFileQueuedForDownload() { return !m_queuedDownloads.empty(); }
	HRESULT downloadNextQueuedFile();

private:
	bool m_winsockInit;
	CDownload* m_download;
	bool m_wasError;
	bool m_sawEnd;
	std::string m_errorString;
	std::string m_statusString;

protected:
	std::list<QueuedDownload> m_queuedDownloads;
};

extern DownloadManager* TheDownloadManager;

}    // namespace patchget
