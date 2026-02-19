#include <windows.h>
#include "WWDownload/Download.h"
#include "WWDownload/ftp.h"

/*
** Stub implementation of CDownload and related classes to satisfy linker
** and allow engine initialization without real FTP/Update functionality.
*/

// CDownload implementation
HRESULT CDownload::PumpMessages() { return S_OK; }

HRESULT CDownload::Abort() { return S_OK; }

HRESULT CDownload::DownloadFile(LPCSTR server, LPCSTR username, LPCSTR password,
                                LPCSTR file, LPCSTR localfile, LPCSTR regkey,
                                bool tryresume) {
  return E_FAIL;
}

HRESULT CDownload::GetLastLocalFile(char *local_file, int maxlen) {
  if (local_file && maxlen > 0) {
    local_file[0] = '\0';
  }
  return S_OK;
}

// Cftp implementation
Cftp::Cftp()
    : m_iCommandSocket(-1), m_iDataSocket(-1), m_pfLocalFile(nullptr) {}

Cftp::~Cftp() {}

HRESULT Cftp::ConnectToServer(LPCSTR szServerName) { return E_FAIL; }
HRESULT Cftp::DisconnectFromServer() { return S_OK; }
HRESULT Cftp::LoginToServer(LPCSTR szUserName, LPCSTR szPassword) {
  return E_FAIL;
}
HRESULT Cftp::LogoffFromServer(void) { return S_OK; }
HRESULT Cftp::FindFile(LPCSTR szRemoteFileName, int *piSize) { return E_FAIL; }
HRESULT Cftp::FileRecoveryPosition(LPCSTR szLocalFileName,
                                   LPCSTR szRegistryRoot) {
  return E_FAIL;
}
HRESULT Cftp::GetNextFileBlock(LPCSTR szLocalFileName, int *piTotalRead) {
  return E_FAIL;
}
HRESULT Cftp::RecvReply(LPCSTR pReplyBuffer, int iSize, int *piRetCode) {
  return E_FAIL;
}
HRESULT Cftp::SendCommand(LPCSTR pCommand, int iSize) { return E_FAIL; }

int Cftp::SendData(char *pData, int iSize) { return -1; }
int Cftp::RecvData(char *pData, int iSize) { return -1; }
int Cftp::SendNewPort() { return -1; }
int Cftp::OpenDataConnection() { return -1; }
void Cftp::CloseDataConnection() {}
int Cftp::AsyncGetHostByName(char *szName, struct sockaddr_in &address) {
  return -1;
}
void Cftp::GetDownloadFilename(const char *localname, char *downloadname,
                               size_t downloadname_size) {}
void Cftp::CloseSockets(void) {}
void Cftp::ZeroStuff(void) {}
