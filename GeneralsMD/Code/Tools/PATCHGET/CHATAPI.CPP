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

//
// This module takes care of all the Chat API stuff
//

#include <list>
#include "gamespy/ghttp/ghttp.h"
#include "DownloadManager.h"

#include "CHATAPI.H"

//#include "api/wolapi_i.c"  // This should only be in one .cpp file
#include <objbase.h>
#include <windows.h>
#include <initguid.h>
#include <olectl.h>
#include <mapicode.h>
#include "RESOURCE.H"
#include "WINBLOWS.H"
#include <crtdbg.h>
#include "process.h"
#include "WWDownload/Registry.h"
#include "WWDownload/urlBuilder.h"
#include "debug.h"

namespace patchget
{

enum EVENT_TYPES
{
  NOUPDATE_EVENT=0,    // don't need to update
  ABORT_EVENT,
  NUM_EVENTS       // keep last
};

#define GAME_NAME	"Command & Conquer"

HANDLE       Events[NUM_EVENTS];

char         g_UpdateString[256];   // for the filename
char         g_DLTimeRem[80];
char         g_DLBytesLeft[80];
char         g_DLBPS[80];

int          g_Finished=0;


HWND         g_DownloadWindow;
HWND         g_ContactWindow;
HWND         g_PrimaryWindow;

static bool checkingForPatch = false;
static int checksLeft = 0;
static bool cantConnect = false;
static std::list<QueuedDownload> queuedDownloads;

BOOL CALLBACK downloadDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
    LPARAM lParam );

///////////////////////////////////////////////////////////////////////////////////////

static void startOnline( void );

///////////////////////////////////////////////////////////////////////////////////////

QueuedDownload TheDownload;

class DownloadManagerMunkee : public DownloadManager
{
public:
	DownloadManagerMunkee() { }
	virtual HRESULT OnError( int error );
	virtual HRESULT OnEnd();
	virtual HRESULT OnProgressUpdate( int bytesread, int totalsize, int timetaken, int timeleft );
	virtual HRESULT OnStatusUpdate( int status );
	virtual HRESULT downloadFile( std::string server, std::string username, std::string password, std::string file, std::string localfile, std::string regkey, bool tryResume );
};

///////////////////////////////////////////////////////////////////////////////////////

HRESULT DownloadManagerMunkee::downloadFile( std::string server, std::string username, std::string password, std::string file, std::string localfile, std::string regkey, bool tryResume )
{
	/*
	if (staticTextFile)
	{
		UnicodeString fileString;
		fileString.translate(file);
		GadgetStaticTextSetText(staticTextFile, fileString);
	}
	*/
	return DownloadManager::downloadFile( server, username, password, file, localfile, regkey, tryResume );
}
HRESULT DownloadManagerMunkee::OnError( int error )
{
	HRESULT ret = DownloadManager::OnError( error );
  g_Finished = -1;
	return ret;
}
HRESULT DownloadManagerMunkee::OnEnd()
{
	HRESULT ret = DownloadManager::OnEnd();
  g_Finished = 1;
	return ret;
}
HRESULT DownloadManagerMunkee::OnProgressUpdate( int bytesread, int totalsize, int timetaken, int timeleft )
{
	HRESULT ret = DownloadManager::OnProgressUpdate( bytesread, totalsize, timetaken, timeleft );

  SendDlgItemMessage( g_DownloadWindow, IDC_PROGRESS, PBM_SETPOS, (WPARAM)(bytesread * 100) / totalsize, 0 );
  char temp[256];

  if( timeleft > 0 )
  {
    //DBGMSG("Bytes read: "<<bytesread<<".    Time left: "<<timeleft<<" seconds");
 	 LoadString(Global_instance, TXT_TIME_REMAIN, temp, sizeof(temp));
    sprintf(g_DLTimeRem,temp,(timeleft/60),(timeleft%60));

    LoadString(Global_instance, TXT_BPS, temp, sizeof(temp));
    sprintf(g_DLBPS,temp,bytesread/timetaken);
  }
  LoadString(Global_instance, TXT_BYTES_READ, temp, sizeof(temp));
  sprintf(g_DLBytesLeft,temp,bytesread,totalsize);
	return ret;
}
HRESULT DownloadManagerMunkee::OnStatusUpdate( int status )
{
	HRESULT ret = DownloadManager::OnStatusUpdate( status );
	SetWindowText(g_DownloadWindow, getStatusString().c_str());
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK simpleDialogProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch( uMsg )
  {
    case WM_INITDIALOG:
      return(TRUE);
    break;
    case WM_CLOSE:
      DestroyWindow(hwnd);
      PostQuitMessage(0);
      exit(0);
    break;
  }
  return(FALSE);
}

static void startOnline( void )
{
	checkingForPatch = false;

  // Close that contacting window
  DestroyWindow(g_ContactWindow);
  g_ContactWindow=NULL;

	if (cantConnect)
	{
		MessageBox(NULL, "Can't Connect", GAME_NAME, MB_OK);
		exit(0);
	}
	else if (queuedDownloads.size())
	{
		if (MessageBox(NULL, "Patches Available.  Download?", GAME_NAME, MB_YESNO) == IDYES)
		{
			DEBUG_LOG(("Downloading patches\n"));
			while (queuedDownloads.size())
			{
				TheDownload = queuedDownloads.front();
				queuedDownloads.pop_front();
				TheDownloadManager = new DownloadManagerMunkee;
				/**/
		    int retVal = DialogBox(Global_instance, MAKEINTRESOURCE(IDD_DOWNLOAD_DIALOG), g_PrimaryWindow, downloadDialogProc);
				if (retVal)
				{
					DEBUG_LOG(("Error %d\n", GetLastError()));
				}
				/**/
				/*
				{
					//char *res = MAKEINTRESOURCE(IDD_CONNECTING);
					char *res = MAKEINTRESOURCE(IDD_DOWNLOAD_DIALOG1);
					g_DownloadWindow=CreateDialog(Global_instance,res,g_PrimaryWindow,simpleDialogProc);
					ShowWindow(g_DownloadWindow,SW_SHOWNORMAL);
					SetForegroundWindow(g_DownloadWindow);
				}
				*/
				delete TheDownloadManager;
				TheDownloadManager = NULL;

				if (g_Finished != 1)
				{
					// Download failed
					//DBGMSG("Download failed: "<<retval);
					SetEvent(Events[ABORT_EVENT]);
					return;
				}
			}
			exit(0);
		}
		else
		{
			exit(0);
		}
	}
	else
	{
		MessageBox(NULL, "No Patches Available", GAME_NAME, MB_OK);
		exit(0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////

static std::string trim(std::string s, const std::string& delim)
{
	unsigned int i;
	i = s.find_first_not_of(delim);
	if (i != s.npos)
	{
		s = s.substr(i);
	}

	i = s.find_last_not_of(delim);
	if (i>=0 && i<s.npos)
	{
		s = s.substr(0, i+1);
	}

	return s;
}

///////////////////////////////////////////////////////////////////////////////////////

static std::string getNextLine(std::string in, std::string& remainder)
{
	int lineEnd;
	lineEnd = in.find_first_of("\n\r", 0);
	if (lineEnd < 1)
	{
		remainder = "";
		return in;
	}

	std::string out = in.substr(0, lineEnd);
	remainder = in.substr(lineEnd+1);

	remainder = trim(remainder, "\r\n\t ");
	out = trim(out, "\r\t\n ");
}

///////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
inline const char* skipSeps(const char* p, const char* seps)
{
	while (*p && strchr(seps, *p) != NULL)
		++p;
	return p;
}

//-----------------------------------------------------------------------------
inline const char* skipNonSeps(const char* p, const char* seps)
{
	while (*p && strchr(seps, *p) == NULL)
		++p;
	return p;
}

//-----------------------------------------------------------------------------
bool nextToken(std::string& base, std::string& tok, const char* seps = NULL)
{
	if (base.empty())
		return false;

	if (seps == NULL)
		seps = " \n\r\t";

	const char* start = skipSeps(base.c_str(), seps);
	const char* end = skipNonSeps(start, seps);

	if (end > start)
	{
		int len = end - start;
		char* tmp = new char[len+1];
		memcpy(tmp, start, len);
		tmp[len] = 0;

		tok = tmp;
		delete[] tmp;

		base = end;

		return true;
	}
	else
	{
		base = tok = "";
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

static void queuePatch(bool mandatory, std::string downloadURL)
{
	// downloadURL is of the form "ftp://ftp.ea.com:user@pass/pub/munkee/bananna.rtp"
	QueuedDownload q;
	bool success = true;

	std::string connectionType;
	success = success && nextToken(downloadURL, connectionType, ":");

	std::string server;
	success = success && nextToken(downloadURL, server, ":/");

	std::string user;
	success = success && nextToken(downloadURL, user, ":@");

	std::string pass;
	success = success && nextToken(downloadURL, pass, "@/");

	std::string filePath;
	success = success && nextToken(downloadURL, filePath, "");

	if (!success && !user.empty())
	{
		// no user/pass combo - move the file into it's proper place
		filePath = user;
		user = ""; // LFeenanEA - Credentials removed as per Security requirements
		pass = "";
		success = true;
	}

	std::string fileStr = filePath;
	unsigned int slashPos = filePath.find_last_of('/');
	std::string fileDir = "patches\\";
	std::string fileName = "";
	if (slashPos == filePath.npos)
	{
		fileName = filePath;
	}
	else
	{
		fileName = filePath.substr(slashPos+1);
	}
	fileDir.append(fileName);

	DEBUG_LOG(("download URL split: %d [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		success, connectionType.c_str(), server.c_str(), user.c_str(), pass.c_str(),
		filePath.c_str(), fileName.c_str(), fileDir.c_str()));

	if (!success)
		return;

	q.file = filePath;
	q.localFile = fileDir;
	q.password = pass;
	q.regKey = "";
	q.server = server;
	q.tryResume = true;
	q.userName = user;

	std::list<QueuedDownload>::iterator it = queuedDownloads.begin();
	while (it != queuedDownloads.end())
	{
		if (it->localFile == q.localFile)
			return; // don't add it if it exists already (because we can check multiple times)
		++it;
	}

	queuedDownloads.push_back(q);
}

///////////////////////////////////////////////////////////////////////////////////////

static GHTTPBool patchCheckCallback( GHTTPRequest request, GHTTPResult result, char * buffer, GHTTPByteCount bufferLen, void * param )
{
	--checksLeft;
	DEBUG_ASSERTCRASH(checksLeft>=0, ("Too many callbacks"));

	DEBUG_LOG(("Result=%d, buffer=[%s], len=%d\n", result, buffer, bufferLen));
	if (result != GHTTPSuccess)
	{
		cantConnect = true;
		if (!checksLeft)
		{
			startOnline();
		}
		return GHTTPTrue;
	}

	std::string message = buffer;
	std::string line;
	while (nextToken(message, line, "\r\n"))
	{
		std::string type, req, url;
		bool ok = true;
		ok = ok && nextToken(line, type, " ");
		ok = ok && nextToken(line, req, " ");
		ok = ok && nextToken(line, url, " ");
		if (ok && type == "patch")
		{
			DEBUG_LOG(("Saw a patch: %d/[%s]\n", atoi(req.c_str()), url.c_str()));
			queuePatch( atoi(req.c_str()), url );
		}
		else if (ok && type == "server")
		{
		}
	}

	if (!checksLeft)
	{
		startOnline();
	}

	return GHTTPTrue;
}

///////////////////////////////////////////////////////////////////////////////////////

static void StartPatchCheck( void )
{
	checkingForPatch = true;
	std::string gameURL, mapURL;
	std::string configURL, motdURL;

	FormatURLFromRegistry(gameURL, mapURL, configURL, motdURL);

	std::string proxy;
	if (GetStringFromRegistry("", "Proxy", proxy))
	{
		if (!proxy.empty())
		{
			ghttpSetProxy(proxy.c_str());
		}
	}

	// check for a patch first
	checksLeft = 2;
	cantConnect = false;
	ghttpGet(gameURL.c_str(), GHTTPFalse, patchCheckCallback, NULL);
	ghttpGet(mapURL.c_str(), GHTTPFalse, patchCheckCallback, NULL);

	DEBUG_LOG(("Started looking for patches at '%s' && '%s'\n", gameURL.c_str(), mapURL.c_str()));
}

///////////////////////////////////////////////////////////////////////////////////////


BOOL CALLBACK downloadDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	//   HRESULT res;
	
	int cmd = LOWORD(wParam);
	switch( uMsg )
	{
		case WM_COMMAND:
			if ( cmd == IDC_DLABORT )
			{
				
				char abort[128];
				char abortdload[256];
				LoadString(Global_instance, TXT_ABORT_DOWNLOAD, abortdload, sizeof(abortdload));
				LoadString(Global_instance, TXT_ABORT, abort, sizeof(abort));
				
				if (MessageBox(g_PrimaryWindow,abortdload,abort,MB_YESNO)==IDYES)
				{
					TheDownloadManager->reset();
					
					EndDialog( hwndDlg, g_Finished );
					DestroyWindow(hwndDlg);
				}
			}
			else
			{
				return FALSE;
			}
			break;
			
		case WM_INITDIALOG:
			//SetupDownload();
			
			SendMessage(hwndDlg, WM_SETICON,(WPARAM)ICON_SMALL,
				(LPARAM)LoadIcon(Global_instance, MAKEINTRESOURCE(IDI_ICON1)));
			
			g_DLTimeRem[0]=0;
			g_DLBytesLeft[0]=0;
			g_DLBPS[0]=0;
			
			//SetDlgItemText( hwndDlg, IDC_DOWNLOADTITLE, g_UpdateString);
			//SetWindowText(hwndDlg, g_UpdateString);
			
			SetDlgItemText( hwndDlg, IDC_TIMEREM, g_DLTimeRem);
			SetDlgItemText( hwndDlg, IDC_BYTESLEFT, g_DLBytesLeft);
			// SetDlgItemText( hwndDlg, IDC_BPS, g_DLBPS );
			
			// Work out the full file name
			//char    fullpath[_MAX_PATH];
			//char    localfile[_MAX_PATH];
			//sprintf( fullpath, "%s/%s", g_Update->patchpath,g_Update->patchfile);
			//sprintf(localfile,"%s\\%s",g_Update->localpath,g_Update->patchfile);
			
			// Create the directory
			//CreateDirectory((char *)g_Update->localpath, NULL );
			
			TheDownloadManager->downloadFile(TheDownload.server, TheDownload.userName, TheDownload.password,
				TheDownload.file, TheDownload.localFile, TheDownload.regKey, TheDownload.tryResume);
				/*
				res=pDownload->DownloadFile((char *)g_Update->server, (char *)g_Update->login, (char *)g_Update->password,
				fullpath, localfile, APP_REG_KEY);
				
			*/
			g_DownloadWindow = hwndDlg;
			g_Finished = 0;
			SetTimer( hwndDlg, 1, 200, NULL );    // was 50
			
			break;
			
		case WM_TIMER:
			DEBUG_LOG(("TIMER\n"));
			if( g_Finished == 0 )
			{
				DEBUG_LOG(("Entering PumpMsgs\n"));
				TheDownloadManager->update();
				/*
				pDownload->PumpMessages();
				*/
				DEBUG_LOG(("Done with PumpMsgs\n"));
				if (strlen(g_DLTimeRem))
					SetDlgItemText( hwndDlg, IDC_TIMEREM, g_DLTimeRem );
				if (strlen(g_DLBytesLeft))
					SetDlgItemText( hwndDlg, IDC_BYTESLEFT, g_DLBytesLeft );
				//if (strlen(g_DLBPS))
				//  SetDlgItemText( hwndDlg, IDC_BPS, g_DLBPS );
			}
			else
			{
				DEBUG_LOG(("TIMER: Finished\n"));
				EndDialog( hwndDlg, g_Finished );
				DestroyWindow( hwndDlg );
			}
			break;
			
		case WM_DESTROY:
			KillTimer( hwndDlg, 1 );
			//ClosedownDownload();
			
			//DBGMSG("WM_DESTROY");
			break;
			
		case WM_SETFONT:
			return TRUE;
			
		default:
			return FALSE;
	}
	return TRUE;
}



DWORD        dwChatAdvise;
DWORD        dwDownloadAdvise;

//Update      *g_Update;

uint32       g_AppVer=-1;


BOOL CALLBACK Download_Dialog_Proc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
    LPARAM lParam );

BOOL CALLBACK Simple_Dialog_Proc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
    LPARAM lParam );

HWND CreatePrimaryWin(void);

char const * Fetch_String(int id);


//
// Create a primary window
//
HWND CreatePrimaryWin(void)
{
  HWND                hwnd;
  WNDCLASS            wc;
  char                name[256];

  sprintf(name,Fetch_String(TXT_TITLE));

  //DBGMSG("CreatePrimary: "<<name);

  /*
  ** set up and register window class
  */
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = DefWindowProc;
  wc.cbClsExtra = 0;            // Don't need any extra class data
  wc.cbWndExtra = 0;            // No extra win data
  wc.hInstance = Global_instance;
  wc.hIcon=LoadIcon(Global_instance, MAKEINTRESOURCE(IDI_ICON1));
  wc.hCursor = NULL;  /////////LoadCursor( NULL, IDC_ARROW );
  wc.hbrBackground = NULL;
  wc.lpszMenuName = name;
  wc.lpszClassName = name;
  RegisterClass( &wc );
    
  /*
  ** create a window
  */
  hwnd = CreateWindowEx(
      WS_EX_APPWINDOW,
      name,
      name,
      WS_POPUP,
      0, 0,

      //GetSystemMetrics( SM_CXSCREEN ),
      //GetSystemMetrics( SM_CYSCREEN ),
      0,0,

      NULL,
      NULL,
      Global_instance,
      NULL );

  SendMessage(hwnd,WM_SETICON,(WPARAM)ICON_SMALL,
      (LPARAM)LoadIcon(Global_instance, MAKEINTRESOURCE(IDI_ICON1)));

  ShowWindow(hwnd,SW_SHOWNORMAL);

  return(hwnd);
}



//
// Dispatch pending windows events
//
void DispatchEvents(void)
{
  MSG msg;
  int counter=0;
  while(PeekMessage(&msg,NULL,0,0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
	 counter++;
	 if (counter==256)  // just in case
	   break;
  }
}

//
// Check for patches
//
int main(int argc, char *argv[])
{
	InitCommonControls();

	/*
  g_PrimaryWindow=CreatePrimaryWin();  // Create the main window
  DispatchEvents();  // process some win messages 
	*/

/*
  // Check if they've registered before, if not ask them if they want to
  bool have_registered=false;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,REGISTER_REG_KEY,0,KEY_READ,&rKey)==ERROR_SUCCESS)
  {
    char username[64];
    valuesize=sizeof(username);
    if (RegQueryValueEx(rKey,"UserName",NULL,&type,(uint8 *)username,&valuesize)==ERROR_SUCCESS)
      have_registered=true;
    RegCloseKey(rKey);
  }
  if (!have_registered)
  {
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT,NICK_REG_KEY,0,KEY_READ,&rKey)==ERROR_SUCCESS)
    {
      have_registered=true;
      RegCloseKey(rKey);
    }
  }

  if (!have_registered)
  {
    if (MessageBox(NULL,Fetch_String(TXT_REGNOW),Fetch_String(TXT_TITLE),MB_YESNO)==IDNO)
      have_registered=true;  // pretend they've alredy registered
  }

  if (!have_registered)
  {
    // figure out where the registration app is installed & launch it, continue
    //    after it exits.
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,REGISTER_REG_APP,0,KEY_READ,&rKey)==ERROR_SUCCESS)
    {
      char regapp[300];
      valuesize=sizeof(regapp);
      if ((RegQueryValueEx(rKey,"InstallPath",NULL,&type,(uint8 *)regapp,&valuesize)==ERROR_SUCCESS)&&
             (strlen(regapp) > 8))
      {
        // Launch the process
        SHELLEXECUTEINFO info;
        memset(&info,0,sizeof(info));
        info.cbSize=sizeof(info);
        info.fMask=SEE_MASK_NOCLOSEPROCESS;
        info.hwnd=g_PrimaryWindow;
        info.lpVerb=NULL;
        info.lpFile=regapp;
        info.lpParameters=NULL;
        info.lpDirectory=".";
        info.nShow=SW_SHOW;
        ShellExecuteEx(&info);

        // Can't wait infinite or the other process will never create its window
        //   Only Bill himself knows why this is happening
        while(1)  // Wait for completion
        {
          DispatchEvents();
          if (WaitForSingleObject(info.hProcess,500)!=WAIT_TIMEOUT)
            break;
        }
      }
      RegCloseKey(rKey);
    }
  }
  // OK, done with that crap go on to the task at hand now....
*/


  // Find the game version
  g_AppVer = -1;
	if (!GetUnsignedIntFromRegistry("", "Version", g_AppVer))
	{
    MessageBox(g_PrimaryWindow,Fetch_String(TXT_INSTALL_PROBLEM),Fetch_String(TXT_ERROR),MB_OK);
    exit(0);
	}
  // OK, have the current game version now

  g_PrimaryWindow=CreatePrimaryWin();  // Create the main window
  DispatchEvents();  // process some win messages 

  // Popup the "contacting" window
  g_ContactWindow=CreateDialog(Global_instance,MAKEINTRESOURCE(IDD_CONNECTING),g_PrimaryWindow,Simple_Dialog_Proc);
  ShowWindow(g_ContactWindow,SW_SHOWNORMAL);
  SetForegroundWindow(g_ContactWindow);
  DispatchEvents();  // process some win messages


  // Setup the Westwood Online stuff
  Startup_Chat();

  Update_If_Required();

  Shutdown_Chat();

  return(0);
}


typedef struct SRecord {
	int ID;						// ID number of the string resource.
	int TimeStamp;				// 'Time' that this string was last requested.
	char String[2048];			// Copy of string resource.

	SRecord(void) : ID(-1), TimeStamp(-1) {}
} SRecord;


/***********************************************************************************************
 * Fetch_String -- Fetches a string resource.                                                  *
 *                                                                                             *
 *    Fetches a string resource and returns a pointer to its text.                             *
 *                                                                                             *
 * INPUT:   id -- The ID number of the string resource to fetch.                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the actual text of the string resource.                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/25/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * Fetch_String(int id)
{
	static SRecord _buffers[64];
	static int _time = 0;

	/*
	**	Determine if the string ID requested is valid. If not then return an empty string pointer.
	*/
	if (id == -1 || id == TXT_NONE) return("");

	/*
	**	Adjust the 'time stamp' tracking value. This is an artificial value used merely to track
	**	the relative age of the strings requested.
	*/
	_time = _time+1;

	/*
	**	Check to see if the requested string has already been fetched into a buffer. If so, then
	**	return a pointer to that string (update the time stamp as well).
	*/
	for (int index = 0; index < ARRAY_SIZE(_buffers); index++) {
		if (_buffers[index].ID == id) {
			_buffers[index].TimeStamp = _time;
			return(_buffers[index].String);
		}
	}

	/*
	**	Find a suitable buffer to hold the string to be fetched. The buffer should either be
	**	empty or have the oldest fetched string.
	*/
	int oldest = -1;
	int oldtime = -1;
	for (int text = 0; text < ARRAY_SIZE(_buffers); text++) {
		if (oldest == -1 || oldtime > _buffers[text].TimeStamp) {
			oldest = text;
			oldtime = _buffers[text].TimeStamp;
			if (oldtime == -1 || _buffers[text].ID == -1) break;
		}
	}

	/*
	**	A suitable buffer has been found so fetch the string resource and then return a pointer
	**	to the string.
	*/
	char * stringptr = _buffers[oldest].String;
	_buffers[oldest].ID = id;
	_buffers[oldest].TimeStamp = _time;

   
	if (LoadString(Global_instance, id, stringptr, sizeof(_buffers[oldest].String)) == 0) {
		return("");
	}
   
   /******
   char resname[32];
   sprintf(resname,"#%d",id);
   HMODULE hmod=GetModuleHandle(NULL);
   HRSRC hrsrc=FindResourceEx(hmod, RT_STRING, MAKEINTRESOURCE(id), LANGID);
   if (hrsrc==0)
   {
     char message_buffer[256];
	  FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &message_buffer[0], 256, NULL );
     
   }
   HGLOBAL resdata=LoadResource(NULL,hrsrc);
   LPVOID vdata=LockResource(resdata);
   strcpy(stringptr,(char *)vdata);
   *********/

	stringptr[sizeof(_buffers[oldest].String)-1] = '\0';
	return(stringptr);
}




void LogMsg(char *msg)
{
#ifdef _DEBUG
  FILE *out=fopen("register.log","a");
  fprintf(out,"%s\n",msg);
  fflush(out);
  fclose(out);
#endif
}




void Startup_Chat(void)
{
	/*
  //////CComObject<CChatEventSink>* g_pChatSink;
  HRESULT          hRes;
  g_pChatSink=NULL;

  CoCreateInstance(CLSID_Chat, NULL, CLSCTX_INPROC_SERVER,
      IID_IChat, (void**)&pChat);


  if (pChat==NULL)
  {
    char error[128];
    char apimissing[256];
  	 LoadString(Global_instance, TXT_API_MISSING, apimissing, sizeof(apimissing));
	 LoadString(Global_instance, TXT_ERROR, error, sizeof(error));
    MessageBox(g_PrimaryWindow,apimissing,error,MB_OK);
    exit(-5);
  }

  g_pChatSink=new CChatEventSink;

  // Get a connection point from the chat class
  IConnectionPoint           *pConnectionPoint=NULL;
  IConnectionPointContainer  *pContainer=NULL;

  dwChatAdvise=0;
  hRes=pChat->QueryInterface(IID_IConnectionPointContainer,(void**)&pContainer);
  _ASSERTE(SUCCEEDED(hRes));
  hRes=pContainer->FindConnectionPoint(IID_IChatEvent,&pConnectionPoint);
  _ASSERTE(SUCCEEDED(hRes));
  hRes=pConnectionPoint->Advise((IChatEvent *)g_pChatSink,&dwChatAdvise);
  _ASSERTE(SUCCEEDED(hRes));


  pChat->SetAttributeValue("RegPath",APP_REG_KEY);

  // ADD pConnectionPoint->Release();
	*/
}

void Shutdown_Chat(void)
{
	/*
    /////AtlUnadvise(pChat, IID_IChatEvent, dwChatAdvise);

    IConnectionPoint           *pConnectionPoint=NULL;
    IConnectionPointContainer  *pContainer=NULL;
    HRESULT                     hRes;

    hRes=pChat->QueryInterface(IID_IConnectionPointContainer,(void**)&pContainer);
    _ASSERTE(SUCCEEDED(hRes));
    hRes=pContainer->FindConnectionPoint(IID_IChatEvent,&pConnectionPoint);
    _ASSERTE(SUCCEEDED(hRes));
    pConnectionPoint->Unadvise(dwChatAdvise);

    pChat->Release();

    /////delete(g_pChatSink);   This appears to be bad....
    // ADD g_pChatSink->Release();
    // ADD pConnectionPoint->Release();
    // ADD pContainer->Release();
		*/
}



//
// Download a patch for the registration client if required
//   This uses the chat API for ver checking and FTP.
//
void Update_If_Required(void)
{
  int   retval;
  int   i;
  // Create the events
  for (i=0; i<NUM_EVENTS; i++)
    Events[i]=CreateEvent(NULL,FALSE,FALSE,NULL);

	StartPatchCheck();

	while (1)
	{
		ghttpThink();
    MSG msg;
    while(PeekMessage(&msg,NULL,0,0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    retval=WaitForMultipleObjectsEx(NUM_EVENTS,Events,FALSE,50,FALSE);
    if (retval==WAIT_TIMEOUT)
      continue;
    //DBGMSG("An event was set");
    retval-=WAIT_OBJECT_0;
    break;
  }

  //DBGMSG("Out of the loop")

  if (retval==ABORT_EVENT)
  {
     exit(0);
  }
  else
  {
    //DBGMSG("NO update required");
  }

  //DBGMSG("Shutting down");

  // close all the event objects
  for (i=0; i<NUM_EVENTS; i++)
    CloseHandle(Events[i]);

	/*
  Startup_Chat();
  int   retval;
  int   i;


  // Create the events
  for (i=0; i<NUM_EVENTS; i++)
    Events[i]=CreateEvent(NULL,FALSE,FALSE,NULL);

  /// For Testing....
  ///pChat->RequestServerList(1000,262364,"register","regpas98",15);
  ///pChat->RequestServerList(1000,300,"register","regpas98",15);

  pChat->RequestServerList(g_AppSku,g_AppVer,"register","regpas98",40);

  while(1)
  {
    pChat->PumpMessages();
    MSG msg;
    while(PeekMessage(&msg,NULL,0,0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    retval=WaitForMultipleObjectsEx(NUM_EVENTS,Events,FALSE,50,FALSE);
    if (retval==WAIT_TIMEOUT)
      continue;
    //DBGMSG("An event was set");
    retval-=WAIT_OBJECT_0;
    break;
  }

  //DBGMSG("Out of the loop")

  if (retval==ABORT_EVENT)
  {
     exit(0);
  }
  else
  {
    //DBGMSG("NO update required");
  }

  //DBGMSG("Shutting down");

  // close all the event objects
  for (i=0; i<NUM_EVENTS; i++)
    CloseHandle(Events[i]);

  Shutdown_Chat();
	*/
}






/*
CChatEventSink::CChatEventSink()
{
  m_cRef=0;  // init the refrence count
}


///////////////////////////////////////////////////////////
//
//			Interface IUnknown Methods
//
///////////////////////////////////////////////////////////
//
// QueryInterface
//
HRESULT __stdcall
CChatEventSink::QueryInterface(const IID& iid, void** ppv)
{
	if ((iid == IID_IUnknown) ||(iid == IID_IChatEvent))
	{
		*ppv = static_cast<IChatEvent*>(this) ;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	(reinterpret_cast<IUnknown*>(*ppv))->AddRef() ;
	return S_OK ;
}

///////////////////////////////////////////////////////////
//
// AddRef
//
ULONG __stdcall
CChatEventSink::AddRef()
{
	return InterlockedIncrement(&m_cRef) ;
}

///////////////////////////////////////////////////////////
//
// Release
//
ULONG __stdcall
CChatEventSink::Release()
{
   if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this ;
      return 0 ;
	}
	return m_cRef;
}





///// DOWNLOAD

CDownloadEventSink::CDownloadEventSink()
{
  m_cRef=0;   // Ref counter
}

///////////////////////////////////////////////////////////
//
//			Interface IUnknown Methods
//
///////////////////////////////////////////////////////////
//
// QueryInterface
//
HRESULT __stdcall
CDownloadEventSink::QueryInterface(const IID& iid, void** ppv)
{
	if ((iid == IID_IUnknown) ||(iid == IID_IDownloadEvent))
	{
		*ppv = static_cast<IDownloadEvent*>(this) ;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	(reinterpret_cast<IUnknown*>(*ppv))->AddRef() ;
	return S_OK ;
}

///////////////////////////////////////////////////////////
//
// AddRef
//
ULONG __stdcall
CDownloadEventSink::AddRef()
{
	return InterlockedIncrement(&m_cRef) ;
}

///////////////////////////////////////////////////////////
//
// Release
//
ULONG __stdcall
CDownloadEventSink::Release()
{
   if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this ;
      return 0 ;
	}
	return m_cRef;
}

*/






//// FTP Download stuff


void SetupDownload( void )
{
	/*
  HRESULT              hRes;

  g_pDownloadSink=NULL;

  CoCreateInstance(CLSID_Download, NULL, CLSCTX_INPROC_SERVER,
      IID_IDownload, (void**)&pDownload);
  _ASSERTE(pDownload);
  g_pDownloadSink=new CDownloadEventSink;

  // Get a connection point from the chat class
  IConnectionPoint           *pConnectionPoint=NULL;
  IConnectionPointContainer  *pContainer=NULL;
  dwDownloadAdvise = 0;

  hRes=pDownload->QueryInterface(IID_IConnectionPointContainer,(void**)&pContainer);
  _ASSERTE(SUCCEEDED(hRes));
  hRes=pContainer->FindConnectionPoint(IID_IDownloadEvent,&pConnectionPoint);
  _ASSERTE(SUCCEEDED(hRes));
  hRes=pConnectionPoint->Advise((IDownloadEvent *)g_pDownloadSink,&dwDownloadAdvise);
  _ASSERTE(SUCCEEDED(hRes));
	*/
}



void ClosedownDownload( void )
{
/*
  // AtlUnadvise(pDownload, IID_IDownloadEvent, dwDownloadAdvise);

  IConnectionPoint           *pConnectionPoint=NULL;
  IConnectionPointContainer  *pContainer=NULL;
  HRESULT                     hRes;

  hRes=pDownload->QueryInterface(IID_IConnectionPointContainer,(void**)&pContainer);
  _ASSERTE(SUCCEEDED(hRes));
  hRes=pContainer->FindConnectionPoint(IID_IDownloadEvent,&pConnectionPoint);
  _ASSERTE(SUCCEEDED(hRes));
  pConnectionPoint->Unadvise(dwDownloadAdvise);

	pDownload->Release();

   //////delete(g_pDownloadSink);  This appears to be bad....
*/
}



BOOL CALLBACK Download_Dialog_Proc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
//	char    fullpath[ 256 ];
//   char    localfile[ 256];
//   HRESULT res;

	switch( uMsg )
	{
		case WM_COMMAND:
			switch( LOWORD( wParam ) )
			{
				case IDC_DLABORT:
            {

               char abort[128];
               char abortdload[256];
  	            LoadString(Global_instance, TXT_ABORT_DOWNLOAD, abortdload, sizeof(abortdload));
               LoadString(Global_instance, TXT_ABORT, abort, sizeof(abort));

               if (MessageBox(g_PrimaryWindow,abortdload,abort,MB_YESNO)==IDYES)
               {
/*
					  pDownload->Abort();
*/
                 EndDialog( hwndDlg, g_Finished );
					  DestroyWindow(hwndDlg);
               }
            }
            break;

				default:
					return FALSE;
			}
			break;

		case WM_INITDIALOG:
			SetupDownload();

         SendMessage(hwndDlg, WM_SETICON,(WPARAM)ICON_SMALL,
           (LPARAM)LoadIcon(Global_instance, MAKEINTRESOURCE(IDI_ICON1)));

         g_DLTimeRem[0]=0;
         g_DLBytesLeft[0]=0;
         g_DLBPS[0]=0;

			//SetDlgItemText( hwndDlg, IDC_DOWNLOADTITLE, g_UpdateString);
         //SetWindowText(hwndDlg, g_UpdateString);

         SetDlgItemText( hwndDlg, IDC_TIMEREM, g_DLTimeRem);
         SetDlgItemText( hwndDlg, IDC_BYTESLEFT, g_DLBytesLeft);
         // SetDlgItemText( hwndDlg, IDC_BPS, g_DLBPS );

/*
         // Work out the full file name
         sprintf( fullpath, "%s/%s", g_Update->patchpath,g_Update->patchfile);
         sprintf(localfile,"%s\\%s",g_Update->localpath,g_Update->patchfile);

         // Create the directory
         CreateDirectory((char *)g_Update->localpath, NULL );

         res=pDownload->DownloadFile((char *)g_Update->server, (char *)g_Update->login, (char *)g_Update->password,
            fullpath, localfile, APP_REG_KEY);

*/
			g_DownloadWindow = hwndDlg;
			g_Finished = 0;
			SetTimer( hwndDlg, 1, 200, NULL );    // was 50

			break;

		case WM_TIMER:
      LogMsg("TIMER");
			if( g_Finished == 0 )
			{
            LogMsg("Entering PumpMsgs");
/*
				pDownload->PumpMessages();
*/
            LogMsg("Done with PumpMsgs");
            if (strlen(g_DLTimeRem))
              SetDlgItemText( hwndDlg, IDC_TIMEREM, g_DLTimeRem );
            if (strlen(g_DLBytesLeft))
              SetDlgItemText( hwndDlg, IDC_BYTESLEFT, g_DLBytesLeft );
            //if (strlen(g_DLBPS))
            //  SetDlgItemText( hwndDlg, IDC_BPS, g_DLBPS );
			}
			else
			{
            LogMsg("TIMER: Finished");
            EndDialog( hwndDlg, g_Finished );
				DestroyWindow( hwndDlg );
			}
			break;

		case WM_DESTROY:
			KillTimer( hwndDlg, 1 );
			ClosedownDownload();

         //DBGMSG("WM_DESTROY");
			break;

		case WM_SETFONT:
			return TRUE;

		default:
			return FALSE;
	}
	return TRUE;
}



// Whoeee this is an exciting one...
BOOL CALLBACK Simple_Dialog_Proc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch( uMsg )
  {
    case WM_INITDIALOG:
      return(TRUE);
    break;
    case WM_CLOSE:
      DestroyWindow(hwnd);
      PostQuitMessage(0);
      exit(0);
    break;
  }
  return(FALSE);
}




/////////////////////////////////////////////////////////////////////////////
// CDownloadEventSink
//////////////////////////////////////////////////////////////////////////////

/*

STDMETHODIMP CDownloadEventSink::OnEnd(void)
{
  LogMsg("Finished!");
  g_Finished = 1;
  return(S_OK);
}

STDMETHODIMP CDownloadEventSink::OnError(int error)
{
  LogMsg("ERROR");
  g_Finished = -1;
  return(S_OK);
}


STDMETHODIMP CDownloadEventSink::OnProgressUpdate(int bytesread, int totalsize,
  int timetaken, int timeleft)
{
  SendDlgItemMessage( g_DownloadWindow, IDC_PROGRESS, PBM_SETPOS, (WPARAM)(bytesread * 100) / totalsize, 0 );
  char temp[256];

  if( timeleft > 0 )
  {
    //DBGMSG("Bytes read: "<<bytesread<<".    Time left: "<<timeleft<<" seconds");
 	 LoadString(Global_instance, TXT_TIME_REMAIN, temp, sizeof(temp));
    sprintf(g_DLTimeRem,temp,(timeleft/60),(timeleft%60));

    LoadString(Global_instance, TXT_BPS, temp, sizeof(temp));
    sprintf(g_DLBPS,temp,bytesread/timetaken);
  }
  LoadString(Global_instance, TXT_BYTES_READ, temp, sizeof(temp));
  sprintf(g_DLBytesLeft,temp,bytesread,totalsize);

  return(S_OK);
}


STDMETHODIMP CDownloadEventSink::OnStatusUpdate(int status)
{
  switch( status )
  {
    case DOWNLOADSTATUS_CONNECTING:
    {
	  //LogMsg( "Connecting..." );
     SetWindowText(g_DownloadWindow, Fetch_String(TXT_CONNECTING));
    }
    break;

    case DOWNLOADSTATUS_FINDINGFILE:
	  //LogMsg( "Finding patch..." );
     SetWindowText(g_DownloadWindow, g_UpdateString);
    break;

    case DOWNLOADSTATUS_DOWNLOADING:
	  //LogMsg( "Downloading patch..." );
     SetWindowText(g_DownloadWindow, g_UpdateString);
    break;

    default:
     //LogMsg("Unknown status update!");
    break;
  }
  return(S_OK);
}

//
// Just tell the FTP module to go ahead and resume
//
STDMETHODIMP CDownloadEventSink::OnQueryResume(void)
{
  return(DOWNLOADEVENT_RESUME);
}




/////////////////////////////////////////////////////////////////////////////
// CChatEventSink
//////////////////////////////////////////////////////////////////////////////


STDMETHODIMP CChatEventSink::OnServerList(HRESULT res, Server* servers)
{
  // If we get this, then we don't need to patch
  // Close that contacting window
  DestroyWindow(g_ContactWindow);
  g_ContactWindow=NULL;

  LogMsg("Server List");

  MessageBox(g_PrimaryWindow,Fetch_String(TXT_NO_PATCHES),Fetch_String(TXT_AUTO_UPDATE),MB_OK);

  SetEvent(Events[NOUPDATE_EVENT]);
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnServerBannedYou(HRESULT, time_t)
{
 return(S_OK);
}


STDMETHODIMP CChatEventSink::OnPageSend(HRESULT)
{
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnPaged(HRESULT, User *, LPCSTR)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnFind(HRESULT, Channel *)
{
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnLogout(HRESULT, User *)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnBusy(HRESULT)
{
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnIdle(HRESULT)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnConnection(HRESULT, LPCSTR)
{
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnUserFlags(HRESULT,LPCSTR,unsigned int, unsigned int)
{
 return(S_OK);
}


STDMETHODIMP CChatEventSink::OnChannelCreate(HRESULT, Channel *)
{
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnChannelModify(HRESULT, Channel *)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnChannelJoin(HRESULT, Channel *, User *)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnChannelLeave(HRESULT, Channel *, User *)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnChannelTopic(HRESULT, Channel *, LPCSTR)
{
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnChannelBan(HRESULT,  LPCSTR, int)
{
 return(S_OK);
}


STDMETHODIMP CChatEventSink::OnGroupList(HRESULT, Group *)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnPublicMessage(HRESULT, Channel *, User *, LPCSTR)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnPrivateMessage(HRESULT, User *,LPCSTR)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnSystemMessage(HRESULT, LPCSTR)
{
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnUserLocale(long, struct User *)
{
   return(S_OK);
}

STDMETHODIMP CChatEventSink::OnUserTeam(long, struct User *)
{
   return(S_OK);
}

STDMETHODIMP CChatEventSink::OnSetLocale(long, enum Locale)
{
   return(S_OK);
}

STDMETHODIMP CChatEventSink::OnSetTeam(long, int)
{
   return(S_OK);
}

STDMETHODIMP CChatEventSink::OnNetStatus(HRESULT hr)
{
  char string[60];
  sprintf(string,"NetStatus %ld",hr);
  LogMsg(string);

  if ((hr==CHAT_E_CON_NETDOWN)||(hr==CHAT_E_CON_LOOKUP_FAILED)||
    (hr==CHAT_E_CON_ERROR)||(hr==CHAT_E_TIMEOUT))
  {
    char error[128];
    char cantconn[256];
  	 LoadString(Global_instance, TXT_CANT_CONTACT, cantconn, sizeof(cantconn));
	 LoadString(Global_instance, TXT_ERROR, error, sizeof(error));
    MessageBox(g_PrimaryWindow,cantconn,error,MB_OK);
    exit(-1);
  }

  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnChannelList(HRESULT, Channel*)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnUserList(HRESULT, Channel*, User*)
{
  return(S_OK);
}



//
// We got a list of updates to apply
//
STDMETHODIMP CChatEventSink::OnUpdateList(HRESULT r, Update * updates)
{
  Update     *tmp;
  int         numupdates=0;
  int         i=0;
  int         retval;

  static int  alreadyGotOne=0;

  LogMsg("Update list");

  // Close that contacting window
  DestroyWindow(g_ContactWindow);
  g_ContactWindow=NULL;

  if( FAILED(r) )
  {
    LogMsg("Failed");
    SetEvent(Events[ABORT_EVENT]);   // An error occurred, bail out
  }

  if( updates == NULL )  // shouldn't happen
	return S_OK;


  if (alreadyGotOne)  // Should only get one update list
    return(S_OK);
  alreadyGotOne=1;


  // Count the updates;
  tmp = updates;
  while( tmp != NULL )
  {
    tmp = tmp->next;
    numupdates++;
  }

  // Got a list of updates - If an update is required, the user must either
  //   patch or quit.

  tmp = updates;

  LogMsg("Found an update");

  // We have a required update

  char upreq[256];
  char title[128];

  LoadString(Global_instance, TXT_AN_UPGRADE_AVAILABLE, upreq, sizeof(upreq));
  strcat(upreq,"\n");
  LoadString(Global_instance, TXT_DOWNLOAD_NOW, upreq+strlen(upreq), sizeof(upreq));
  LoadString(Global_instance, TXT_UPGRADE_AVAILABLE, title, sizeof(title));

  if( MessageBox(g_PrimaryWindow,upreq,title, MB_YESNO ) == IDNO )
  {
    // If they don't want to patch now, just exit...
    //DBGMSG("Must patch to continue, so exit");
    exit(0);
  }

  // Do the downloads
  while( tmp != NULL )
  {
    g_Update = tmp;

    char dloading[256];
  	 LoadString(Global_instance, TXT_DOWNLOADING_FILE, dloading, sizeof(dloading));
    sprintf( g_UpdateString, dloading, ++i, numupdates );

    LogMsg("Creating Download dialog box");
    //if( (retval=DialogBox(Global_instance, MAKEINTRESOURCE(IDD_DOWNLOAD_DIALOG), g_PrimaryWindow,
    //    (DLGPROC)Download_Dialog_Proc)) != 1 )

    retval=DialogBox(Global_instance, MAKEINTRESOURCE(IDD_DOWNLOAD_DIALOG), g_PrimaryWindow,(DLGPROC)Download_Dialog_Proc);

    if (g_Finished != 1)
    {
      // Download failed
      //DBGMSG("Download failed: "<<retval);
      SetEvent(Events[ABORT_EVENT]);
      return(E_FAIL);
    }
    tmp = tmp->next;
  }
  // Quit so the launcher can apply the patches
  exit(0);
  return(S_OK);  // make silly compiler happy
}






STDMETHODIMP CChatEventSink::OnServerError(HRESULT, LPCSTR)
{
  LogMsg("Server Error");
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnMessageOfTheDay(HRESULT, LPCSTR)
{
  return(S_OK);
}



STDMETHODIMP CChatEventSink::OnPrivateAction(HRESULT, User *,  LPCSTR)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnPublicAction(HRESULT, Channel *, User *,  LPCSTR)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnPrivateGameOptions(HRESULT, User *,  LPCSTR)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnPublicGameOptions(HRESULT, Channel *, User *,  LPCSTR)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnGameStart(HRESULT, Channel *, User *,  int)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnUserKick(HRESULT, Channel *, User *,  User *)
{
  return(S_OK);
}


STDMETHODIMP CChatEventSink::OnUserIP(HRESULT, User *)
{
  return(S_OK);
}

STDMETHODIMP CChatEventSink::OnSquadInfo(HRESULT, unsigned long, Squad *)
{
  return(S_OK);
}

*/

} // namespace patchget
