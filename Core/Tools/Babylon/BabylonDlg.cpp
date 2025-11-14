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


// BabylonDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Babylon.h"
#include "BabylonDlg.h"
#include "VIEWDBSII.h"
#include "VerifyDlg.h"
#include "ExportDlg.h"
#include "Report.h"
#include "MatchDlg.h"
#include "RetranslateDlg.h"
#include "GenerateDlg.h"
#include "DlgProxy.h"
#include "XLStuff.h"
#include "fileops.h"
#include <time.h>
#include "iff.h"
#include "loadsave.h"
#include "expimp.h"
#include "ProceedDlg.h"
#include "transcs.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static char buffer[100*1024];
static char buffer2[100*1024];
static char buffer3[100*1024];
static const int INCREMENTS = 100;
static const int MAX_INFO_LEN = 2*1024;
static int found_error;
static int cb_count;
static CBabylonDlg	*mainDlg;

static void print_to_log ( const char *text )
{
	if ( !found_error )
	{
		mainDlg->Log ("FAILED", SAME_LINE );
		found_error = TRUE;
	}

	sprintf ( buffer, "String %s", text);
	mainDlg->Log ( buffer );


}

static void print_to_log_and_update_progress ( const char *text )
{
	print_to_log ( text );
	cb_count++;
	mainDlg->SetProgress ( cb_count );
}

static void cb_progress ( void )
{
	cb_count++;
	mainDlg->SetProgress ( cb_count );

}

typedef struct
{
	char comment[MAX_INFO_LEN+1];
	char context[MAX_INFO_LEN+1];
	char speaker[MAX_INFO_LEN+1];
	char listener[MAX_INFO_LEN+1];
	char wave[MAX_INFO_LEN+1];
	int maxlen;

} INFO;

static INFO global_info;
static INFO local_info;

static void init_info ( INFO *info )
{
	info->comment[0] = 0;
	info->context[0] = 0;
	info->speaker[0] = 0;
	info->listener[0] = 0;
	info->wave[0] = 0;
	info->maxlen = 0;
}

static int progress_count;

static void progress_cb ( void )
{
	progress_count++;
	if ( MainDLG )
	{
		MainDLG->SetProgress ( progress_count );
	}
}


static void removeLeadingAndTrailing ( char *buffer )
{
	char *first, *ptr;
	char ch;

	ptr = first = buffer;

	while ( (ch = *first) && iswspace ( ch ))
	{
			first++;
	}

	while ( *ptr++ = *first++ );

	ptr -= 2;;

	while ( (ptr > buffer) && (ch = *ptr) && iswspace ( ch ) )
	{
		ptr--;
	}

	ptr++;
	*ptr = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBabylonDlg dialog

IMPLEMENT_DYNAMIC(CBabylonDlg, CDialog);

CBabylonDlg::CBabylonDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBabylonDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBabylonDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = NULL;
}

CBabylonDlg::~CBabylonDlg()
{
	// If there is an automation proxy for this dialog, set
	//  its back pointer to this dialog to NULL, so it knows
	//  the dialog has been deleted.
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;

}

void CBabylonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBabylonDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBabylonDlg, CDialog)
	//{{AFX_MSG_MAP(CBabylonDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_VIEWDBS, OnViewdbs)
	ON_BN_CLICKED(IDC_RELOAD, OnReload)
	ON_BN_CLICKED(IDC_UPDATE, OnUpdate)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_WARNINGS, OnWarnings)
	ON_BN_CLICKED(IDC_ERRORS, OnErrors)
	ON_BN_CLICKED(IDC_CHANGES, OnChanges)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_GENERATE, OnGenerate)
	ON_BN_CLICKED(IDC_DIALOG, OnVerifyDialog)
	ON_BN_CLICKED(IDC_TRANSLATIONS, OnTranslations)
	ON_CBN_SELCHANGE(IDC_COMBOLANG, OnSelchangeCombolang)
	ON_BN_CLICKED(IDC_REPORTS, OnReports)
	ON_CBN_DBLCLK(IDC_COMBOLANG, OnDblclkCombolang)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_SENT, OnSent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBabylonDlg message handlers

BOOL CBabylonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
		progress = (CProgressCtrl *) GetDlgItem ( IDC_PROGRESS1 );
		percent = (CStatic *) GetDlgItem ( IDC_PERCENT );
		SetWindowText ( AppTitle );
		Log ( AppTitle );
	{
		char buffer[100];
		char date[50];
		char time[50];
		_strtime ( time );
		_strdate ( date );
		sprintf ( buffer, "Session Date: %s %s\n", date, time);
		Log ( buffer );
	}

	Status ("Initializing dialog");
	operate_always = FALSE;
	combo = ( CComboBox *)GetDlgItem ( IDC_COMBOLANG );

	int index = 0;
	int lang_index = 0;
	LANGINFO *info;
	while ( (info = GetLangInfo ( lang_index )) )
	{
		combo->InsertString ( index, info->name );
		combo->SetItemDataPtr ( index, info );
		index++;
		lang_index++;
	}
	max_index = index;
	combo->SetCurSel ( 0 );

	// do any initialization
#if 0
	// initialize audio
	if ( !AIL_quick_startup ( TRUE, FALSE, 22050, 16, 2 ) )
	{
		sprintf ( buffer, "Falied to init audio.\n\nReason:%s\n", AIL_last_error ());
		AfxMessageBox ( buffer );

	}
	else
	{
		onexit ( (_onexit_t ) AIL_quick_shutdown );
	}
#endif


	Ready();;

	PostMessage ( WM_COMMAND, MAKEWPARAM ( IDC_RELOAD, BN_CLICKED ));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBabylonDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBabylonDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBabylonDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

void CBabylonDlg::OnClose()
{
	if (CanExit())
	{
		if ( !CanProceed ())
		{
			return ;
		}

		CDialog::OnOK();
		if ( !SaveLog () )
		{
			AfxMessageBox ("Failed to save log!\n\nMake sure babylon.log is writable");
		}
	}

}

//DEL void CBabylonDlg::OnOK()
//DEL {
//DEL 	if (CanExit())
//DEL 		CDialog::OnOK();
//DEL }

//DEL void CBabylonDlg::OnCancel()
//DEL {
//DEL 	if (CanExit())
//DEL 		CDialog::OnCancel();
//DEL }

BOOL CBabylonDlg::CanExit()
{
	// If the proxy object is still around, then the automation
	//  controller is still holding on to this application.  Leave
	//  the dialog around, but hide its UI.
	if (m_pAutoProxy != NULL)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}

//DEL void CBabylonDlg::OnBrowse()
//DEL {
//DEL   static char szFilter[] = "XL Files (*.XLS)\0*.xls\0\0\0";
//DEL
//DEL 	// TODO: Add your control notification handler code here
//DEL 	CFileDialog *dlg = new CFileDialog ( TRUE, "xls", "*.xls", OFN_FILEMUSTEXIST, szFilter, this );
//DEL 	if ( dlg )
//DEL 	{
//DEL 		dlg->DoModal ();
//DEL 		if ( dlg->GetPathName() != "*.xls" )
//DEL 		{
//DEL 			SelectFile ( LPCTSTR ( dlg->GetPathName() ));
//DEL 		}
//DEL 		else
//DEL 		{
//DEL 			SelectFile ( NULL );
//DEL 		}
//DEL 		delete dlg;
//DEL 	}
//DEL }

//DEL void CBabylonDlg::OnChangeXlFilename()
//DEL {
//DEL 	// TODO: If this is a RICHEDIT control, the control will not
//DEL 	// send this notification unless you override the CDialog::OnInitDialog()
//DEL 	// function and call CRichEditCtrl().SetEventMask()
//DEL 	// with the ENM_CHANGE flag ORed into the mask.
//DEL
//DEL 	// TODO: Add your control notification handler code here
//DEL
//DEL }

void CBabylonDlg::OnExport()
{
	if ( CanOperate ())
	{
		CExportDlg dlg;

		if ( dlg.DoModal () == IDOK )
		{
			ExportTranslations ( MainDB, dlg.Filename (), dlg.Language(), dlg.Options(), this );
		}
	}
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here

	char string[200];

	sprintf ( string, "Built: %s, %s", __DATE__, __TIME__ );
	SetDlgItemText ( IDC_BUILD, string );


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBabylonDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default
//	char buffer[1024];
//
//	if ( DragQueryFile(hDropInfo, 0, buffer, sizeof ( buffer )-1))
//	{
//
//		if ( ConvertStrFile ( buffer ) )
//		{
//				 return;
//		}
//
//		if ( SelectFile ( buffer ) )
//		{
//			OnExport ();
//		}
//	}
}


//DEL int CBabylonDlg::SelectFile ( const char *buffer )
//DEL {
//DEL 	char *p;
//DEL 	CWnd *wnd = GetDlgItem ( IDC_EXPORT );
//DEL
//DEL 	if ( buffer && (p = strchr ( buffer, '.' )) && !stricmp ( p, ".xls"))
//DEL 	{
//DEL 		SetDlgItemText ( IDC_XLFILE, buffer );
//DEL 		SetDlgItemText ( IDC_STATUS, "File selected: Click 'convert' to start process");
//DEL 		wnd->EnableWindow  ( TRUE );
//DEL 		return TRUE;
//DEL 	}
//DEL 	if ( buffer )
//DEL 	{
//DEL 			AfxMessageBox ("Must be an Excel file");
//DEL 	}
//DEL 	wnd->EnableWindow  ( FALSE );
//DEL 	SetDlgItemText ( IDC_STATUS, "Select excel file...");
//DEL 	SetDlgItemText ( IDC_XLFILE, "Browse or drop excel file to be convertered" );
//DEL
//DEL 	return FALSE;
//DEL }

//DEL int CBabylonDlg::ConvertStrFile ( const char *buffer )
//DEL {
//DEL 	char *p;
//DEL 	char filename[400];
//DEL
//DEL 	if ( buffer && (p = strchr ( buffer, '.' )) && !stricmp ( p, ".str"))
//DEL 	{
//DEL 		ParseDB	db;
//DEL 		SetDlgItemText ( IDC_XLFILE, buffer );
//DEL 		EnableWindow  ( FALSE );
//DEL 		SetDlgItemText ( IDC_STATUS, "Parsing .str file");
//DEL
//DEL 		if ( db.ParseStrFile ( buffer, this ) )
//DEL 		{
//DEL 			SetDlgItemText ( IDC_STATUS, "Creating .xls file");
//DEL 			strcpy ( filename, buffer );
//DEL 			p = strchr ( filename, '.' );
//DEL 			strcpy ( p, ".xls" );
//DEL
//DEL 			if ( db.CreateXLFile ( filename, this ) )
//DEL 			{
//DEL 				SetDlgItemText ( IDC_STATUS, "Created .xls file");
//DEL 			}
//DEL 			else
//DEL 			{
//DEL 				SetDlgItemText ( IDC_STATUS, "Falied to create .xls file");
//DEL 			}
//DEL
//DEL 		}
//DEL 		else
//DEL 		{
//DEL 		SetDlgItemText ( IDC_STATUS, "Failed to parse .str file");
//DEL
//DEL 		}
//DEL
//DEL 		EnableWindow  ( TRUE );
//DEL 		return TRUE;
//DEL 	}
//DEL 	return FALSE;
//DEL }

static int bytes_copied;
static int done_copy;
static int start_copy;
static LogFormat cpy_format;

static DWORD CALLBACK streamin_cb (  DWORD dwCookie, LPBYTE pbBuff, LONG bytes,  LONG *transfered )
{
	char *src = (char *) dwCookie;
	int count = 0;

	src += bytes_copied;

	if ( bytes && start_copy )
	{
		if ( cpy_format == NEW_LINE )
		{
			*pbBuff++ = '\n';
			count++;
			bytes--;
		}
		start_copy = FALSE;
	}


	while ( bytes-- )
	{
		if ( !*src )
		{
			done_copy = TRUE;
			break;
		}
		*pbBuff++ = *src++;
		bytes_copied++;
		count++;
	}
	*transfered = count;
	return 0;
}

static DWORD CALLBACK streamout_cb (  DWORD dwCookie, LPBYTE pbBuff, LONG bytes,  LONG *transfered )
{
	FILE *log = (FILE *) dwCookie;
	int count = 0;

	*transfered = fwrite ( pbBuff, 1, bytes, log );
	return *transfered == -1;
}

void CBabylonDlg::Log( const char *string, LogFormat format)
{
	CRichEditCtrl *rec;
	EDITSTREAM es;
	int lines;
	int end_pos;

	rec = (CRichEditCtrl*)GetDlgItem ( IDC_LOG );

	lines = rec->GetLineCount ( );
	end_pos = rec->LineIndex ( lines );

	rec->SetSel ( end_pos, end_pos );


	//rec->SetReadOnly ( FALSE );
	es.dwCookie = (DWORD) string;
	es.dwError = 0;
	es.pfnCallback = streamin_cb;
	bytes_copied = 0;
	done_copy = FALSE;
	start_copy = TRUE;
	cpy_format = format;

	rec->StreamIn ( SF_TEXT | SFF_SELECTION, es );
	//rec->SetReadOnly ( TRUE );
	lines = rec->GetLineCount ( );
	rec->LineScroll ( -lines, 0 );
	rec->LineScroll ( lines - 10, 0 );
}


void CBabylonDlg::Status( const char *string, int log )
{
	char buffer[200];
	int max_len;

	if ( log )
	{
		Log ( string );
	}

	max_len = sizeof ( buffer ) -1;

	strcpy ( buffer, "Status: ");
	max_len -= strlen ( buffer );
	strncat ( buffer, string, max_len );

	SetDlgItemText ( IDC_STATUS, buffer );

}

int CBabylonDlg::SaveLog()
{
	FILE *log = NULL;
	EDITSTREAM es;
 	CRichEditCtrl *rec = (CRichEditCtrl *) GetDlgItem ( IDC_LOG );
	int ok = FALSE;


	if ( ! (log = fopen ("babylon.log", "a+t" )))
	{
		goto error;
	}

	{
		const char *buffer = "\nLOG START ******************\n\n";
		fwrite ( buffer, 1, strlen ( buffer ), log );
	}

	es.dwCookie = (DWORD) log;
	es.dwError = 0;
	es.pfnCallback = streamout_cb;
	bytes_copied = 0;
	done_copy = FALSE;

	rec->StreamOut ( SF_TEXT, es );
	if ( es.dwError )
	{
		goto error;
	}

	{
		const char *buffer = "\nQuiting Babylon\n\nLOG END ******************\n\n";
		fwrite ( buffer, 1, strlen ( buffer ), log );
	}

	ok = TRUE;

error:

	if ( log )
	{
		fclose ( log );
	}


	return ok;
}

void CBabylonDlg::OnViewdbs()
{
	// TODO: Add your control notification handler code here
	VIEWDBSII dlg;

	ViewChanges = FALSE;

	dlg.DoModal ();


}

static int readToEndOfQuote( FILE *file, char *in, char *out, char *wavefile, int maxBufLen, int in_comment  )
{
	int slash = FALSE;
	int state = 0;
	int line_start = FALSE;
	char ch;
	int new_lines = 0;
	int ccount = 0;

	while ( maxBufLen )
	{
		// get next char

		if ( in )
		{
			if ( !(ch = *in++))
			{
				in = NULL; // have exhausted the input buffer
				ch = getc ( file );
			}
		}
		else
		{
			ch = getc ( file );
		}

		if ( ch == EOF )
		{
			AfxMessageBox ( "Missing terminating quote");
			return new_lines;
		}

		if ( ch == '\n' )
		{
			line_start = TRUE;
			if ( !in )
			{
				new_lines++;
			}
			slash = FALSE;
			ccount = 0;
			ch = ' ';
		}
		else if ( line_start && ( ch == '/' || iswspace ( ch )) )
		{
			continue;
		}
		else if ( ch == '\\' && !slash)
		{
			slash = TRUE;
		}
		else if ( ch == '\\' && slash)
		{
			slash = FALSE;
		}
		else if ( ch == '"' && !slash )
		{
			break; // done
		}
		else
		{
			slash = FALSE;
		}

		if ( iswspace ( ch ))
		{
			ch = ' ';
		}
		else
		{
			line_start = FALSE;
		}

		*out++ = ch;
		maxBufLen--;
	}

	*out = 0;

	if ( !wavefile )
	{
		return new_lines;
	}

	int len = 0;
	while ( TRUE )
	{
		// get next char

		if ( in )
		{
			if ( !(ch = *in++))
			{
				in = NULL; // have exhausted the input buffer
				ch = getc ( file );
			}
		}
		else
		{
			ch = getc ( file );
		}

		if ( ch == '\n' || ch == EOF )
		{
			if ( !in )
			{
				new_lines++;
			}
			break;
		}

		switch ( state )
		{

			case 0:
				if ( iswspace ( ch ) || ch == '=' )
				{
					break;
				}

				state = 1;
			case 1:
				if ( (( ch >= 'a' && ch <= 'z') || ( ch >= 'A' && ch <='Z') || (ch >= '0' && ch <= '9') || ch == '_') )
				{
					*wavefile++ = ch;
					len++;
					break;
				}
				state = 2;
			case 2:
				break;
		}
	}

	*wavefile = 0;
	if ( len )
	{
		if ( ( ch = *(wavefile-1)) < '0' || ch > '9' )
		{
			// remove last character
			*(wavefile-1) = 0;
		}

	}

	return new_lines;
}

enum
{
	START,
	TOKEN,
	COLON,
	ARG
};

static int getString ( FILE *file, char *in, char *out )
{
	int bytes = MAX_INFO_LEN;
	int new_lines = 0;
	char ch;
	char *ptr = out;

	{

		while ( (ch = *in++) && ch != '\n' && bytes )
		{
			*ptr++ = ch;
			bytes--;
		}
	}

	*ptr = 0;

	ConvertMetaChars ( out );
	StripSpaces ( out );
	return new_lines;
}

static char *getToken ( char *buffer, char *token, int bytes )
{
	char ch;
	int state = START;
	*token = 0;

	while ( (ch = *buffer) && ch != '\n' && bytes )
	{
		switch ( state )
		{
			case START:
				if ( ch == '/' || iswspace ( ch ))
				{
					break;
				}
				state = TOKEN;
			case TOKEN:
				if ( !iswspace ( ch ) && ch !=':' )
				{
					*token++ = ch;
					bytes--;
					break;
				}
				*token = 0;
				state = COLON;
			case COLON:
				if ( ch != ':' )
				{
					break;
				}
				state = ARG;
				break;
			case ARG:
				if ( iswspace ( ch  ) )
				{
					break;
				}
				return buffer;
		}
		buffer++;
	}

	*token = 0;
	return buffer;

}

static int parseComment ( FILE *file, char *buffer, INFO *info )
{
	char token[256];
	int new_lines = 0;

	buffer = getToken ( buffer, token, sizeof (token) -1 );

	if ( !token )
	{
		return new_lines;
	}

	if ( !stricmp ( token, "COMMENT" ) )
	{
		new_lines += getString ( file, buffer, info->comment );
	}
	else if ( !stricmp ( token, "CONTEXT" ) )
	{
		new_lines += getString ( file, buffer, info->context );
	}
	else if ( !stricmp ( token, "SPEAKER" ) )
	{
		new_lines += getString ( file, buffer, info->speaker );
	}
	else if ( !stricmp ( token, "LISTENER" ) )
	{
		new_lines += getString ( file, buffer, info->listener );
	}
	else if ( !stricmp ( token, "MAXLEN" ) )
	{
		info->maxlen = atoi ( buffer );
	}
	else if ( !stricmp ( token, "WAVE" ) )
	{
		new_lines += getString ( file, buffer, info->wave );
	}


	return new_lines;
}

static int getLabelCount( char *filename )
{
	int count = 0;
	FILE *fp;

	if ( ! ( fp = fopen ( filename, "rt" )))
	{
		return 0;
	}

	while(TRUE)
	{
		if( fscanf( fp, "%s", buffer ) == EOF )
			break;

		if ( !stricmp( buffer, "END" ) )
		{
			count++;
		}
	}

	fclose ( fp );

	return count;
}

int CBabylonDlg::LoadStrFile ( TransDB *db, const char *filename, void (*cb) ( void ) )
{
	FILE *file = NULL;
	BabylonLabel *label = NULL;
	int status = FALSE;
	int line_number = 0;
	int label_count = 0;
	int	text_dup_count = 0;
	int label_dup_count = 0;

	init_info ( &global_info );

	if ( !(file = fopen ( filename, "rt" ) ))
	{
		goto exit;
	}

	while ( fgets ( buffer, sizeof(buffer)-1, file ) )
	{

		line_number++;
		removeLeadingAndTrailing ( buffer );

		if ( !buffer[0] || (buffer[0] == '/' && buffer[1] == '/') )
		{
			line_number += parseComment ( file, buffer, &global_info );
			continue;
		}

		label = new BabylonLabel ( );
		label->SetName ( buffer );
		label->LockName ();
		label->SetLineNumber ( line_number );
		db->AddLabel ( label );

		local_info = global_info;
		local_info.wave[0] = 0; // wave file name is only locally set
		while( TRUE )
		{
			if ( !fgets ( buffer, sizeof(buffer)-1, file ))
			{
				AfxMessageBox ( "Unexpected end of file" );
				goto exit;
			}

			line_number++;
			removeLeadingAndTrailing ( buffer );

			if ( !stricmp ( buffer, "END" )	)
			{
				break;
			}

			if ( !buffer[0] || (buffer[0] == '/' && buffer[1] == '/') )
			{
				line_number += parseComment ( file, buffer, &local_info );
				continue;
			}

			if ( buffer[0] == '"' )
			{
				int line = line_number;
				strcat ( buffer, "\n" );
				line_number += readToEndOfQuote( file, &buffer[1], buffer2, buffer3, sizeof(buffer2)-1, FALSE );
				BabylonText *text = new BabylonText ( );
				text->Set ( buffer2 );
				text->FormatMetaString ();
				text->LockText ();
				if ( buffer3[0] )
				{
					text->SetWave ( buffer3 );
				}
				else
				{
					text->SetWave ( local_info.wave );
				}
				text->SetLineNumber ( line );
				label->AddText ( text );


			}
		}

		label->SetComment ( local_info.comment );
		label->SetContext ( local_info.context );
		label->SetSpeaker ( local_info.speaker );
		label->SetListener ( local_info.listener );
		label->SetMaxLen ( local_info.maxlen );

		if ( cb )
		{
			cb ();
		}
		label = NULL;

	}
	status = TRUE;


exit:

	db->ClearChanges ();

	delete label;

	if ( file )
	{
		fclose ( file );
	}

	return status;

}

int		CBabylonDlg::CanProceed ( void )
{

	if ( MainDB->IsChanged ())
	{

retry:
		int result = AfxMessageBox ( "Main database has changed!\n\n Do you wish to save it before proceeding?", MB_YESNOCANCEL );

		if ( result == IDCANCEL )
		{
			return FALSE;
		}
		else if ( result == IDYES )
		{
			if ( !SaveMainDB () )
			{
				AfxMessageBox ("Save failed!\n\nCanceling operation");
				return FALSE;
			}
		}
		else
		{
			int result = AfxMessageBox ( "Are you sure you don't want to save?\n\nAll current changes will be lost", MB_YESNO );
			if ( result == IDNO )
			{
				goto retry;
			}
		}
	}

	return TRUE;
}


int		CBabylonDlg::CanOperate ( void )
{
	if ( operate_always )
	{
		return TRUE;
	}

	if ( BabylonstrDB->IsChanged() || BabylonstrDB->HasErrors () )
	{
		const char *string = "Unknown problem!\n\n\nProceed anyway?";

		if ( BabylonstrDB->HasErrors ())
		{
				string = "Generals.str has errors! As a result the translation database is not up to date!\n\nRecommend you fix problems in Generals.str before proceeding.\n\n\n\nDo you wish to continue anyway?";
		}

		if ( BabylonstrDB->IsChanged ())
		{
				string = "The translation database is not up to date! Generals.str has changed since the last time the database was updated.\n\nRecommend you update the database before proceeding.\n\n\n\nDo you wish to continue anyway?";
		}

		ProceedDlg dlg ( string );

		int result = dlg.DoModal ();

		if ( result == IDALWAYS )
		{
			operate_always = TRUE;
		}

		return result != IDNO;
	}

	return TRUE;
}

void CBabylonDlg::OnReload()
{
	int num_errors;
	int num_warnings;
	int count = 0;
	int str_loaded = FALSE;
	int db_loaded = FALSE;
	int db_readonly = FALSE;
	int db_error = FALSE;
	int do_update = FALSE;
	int errors;
	CWnd *win;

	// TODO: Add your control notification handler code here

	if ( !CanProceed ())
	{
		return;
	}
	BabylonstrDB->Clear ();
	BabylonstrDB->ClearChanges ();
	MainDB->Clear ();
	MainDB->ClearChanges ();

	count += getLabelCount ( BabylonstrFilename );
	count += GetLabelCountDB ( MainXLSFilename );
	progress_count = 0;

	win = GetDlgItem ( IDC_ERRORS );
	win->EnableWindow ( FALSE );
	win = GetDlgItem ( IDC_WARNINGS );
	win->EnableWindow ( FALSE );
	win = GetDlgItem ( IDC_UPDATE );
	win->EnableWindow ( TRUE);
	win = GetDlgItem ( IDC_SAVE );
	win->EnableWindow ( TRUE);
	win = GetDlgItem ( IDC_IMPORT );
	win->EnableWindow ( TRUE);
	win = GetDlgItem ( IDC_EXPORT );
	win->EnableWindow ( TRUE);

	InitProgress ( count );
	Log ("" );

	if ( FileExists ( BabylonstrFilename ))
	{
		if ( (errors = ValidateStrFile ( BabylonstrFilename )) )
		{
			if ( errors == -1 )
			{
				if ( AfxMessageBox ( "Unable to verify string file!\n\nMake sure \"strcheck.exe\" is in your path and \"strcheck.rst\" is writeable.\n\nDo you wish to continue loading? \n\nWarning: Any errors in the string file could cause inappropiate updates to the database.", MB_YESNO  ) == IDYES )
				{
					errors = 0;
				}
			}
			else
			{
				sprintf ( buffer, "\"%s\" has %d formating error%s!\n\nFile will not be loaded.", BabylonstrFilename, errors, errors == 1 ? "" : "s" );
				AfxMessageBox ( buffer );
			}
		}
		if ( !errors )
		{
			sprintf ( buffer, "Loading \"%s\"...", BabylonstrFilename );
			Status ( buffer );

			if ( !(str_loaded = LoadStrFile ( BabylonstrDB, BabylonstrFilename, progress_cb )) )
			{
				Log ( "FAILED", SAME_LINE );
				BabylonstrDB->Clear ();
				BabylonstrDB->ClearChanges ();
			}
		}
		else
		{
			sprintf ( buffer, "Loading \"%s\"...NOT LOADED", BabylonstrFilename );
			Log ( buffer );
		}
	}
	else
	{
		sprintf ( buffer, "Loading \"%s\"...", BabylonstrFilename );
		Status ( buffer );
		Log ( "FILE NOT FOUND", SAME_LINE );
	}

	if ( str_loaded )
	{
		sprintf ( buffer, "Validating \"%s\"...", BabylonstrFilename );
		Status ( buffer, FALSE );

		if ( (num_errors = BabylonstrDB->Errors ( )))
		{
			sprintf ( buffer, "Generals.str has %d error(s):\n\nClick \"Errors\" for a detailed list.\n\nAll errors must be fixed before \"Update\" will be enabled.", num_errors );
			AfxMessageBox ( buffer );
			win = GetDlgItem ( IDC_UPDATE );
			win->EnableWindow ( FALSE);
			win = GetDlgItem ( IDC_ERRORS );
			win->EnableWindow ( TRUE );
		}

		if ( (num_warnings = BabylonstrDB->Warnings()))
		{
			win = GetDlgItem ( IDC_WARNINGS );
			win->EnableWindow ( TRUE );
		}

		if ( !num_errors && !num_warnings )
		{
			Log ( "OK", SAME_LINE );
		}
		else
		{
			sprintf ( buffer, "%d errors, %d warnings OK", num_errors, num_warnings );
			Log ( buffer, SAME_LINE );
		}
	}

	sprintf ( buffer, "Loading \"%s\"...", MainXLSFilename );
	Status ( buffer );

	if ( FileExists ( MainXLSFilename ))
	{
		if ( !(db_loaded = LoadMainDB ( MainDB, MainXLSFilename, progress_cb )) )
		{
			Log ( "FAILED", SAME_LINE );
			MainDB->Clear ();
			MainDB->ClearChanges ();
			db_error = TRUE;
			win = GetDlgItem ( IDC_UPDATE );
			win->EnableWindow ( FALSE);
			win = GetDlgItem ( IDC_SAVE );
			win->EnableWindow ( FALSE);
			win = GetDlgItem ( IDC_EXPORT );
			win->EnableWindow ( FALSE);
			win = GetDlgItem ( IDC_IMPORT );
			win->EnableWindow ( FALSE);
		}
		else
		{
			if ( FileAttribs ( MainXLSFilename ) & FA_READONLY )
			{
				AfxMessageBox ( "Database file is readonly!\n\nNo updates will be allowed.\n\nCheckout the database file and reload.");
				db_readonly = TRUE;
				win = GetDlgItem ( IDC_UPDATE );
				win->EnableWindow ( FALSE);
				win = GetDlgItem ( IDC_SAVE );
				win->EnableWindow ( FALSE);
				win = GetDlgItem ( IDC_IMPORT );
				win->EnableWindow ( FALSE);
				Log ( "READONLY", SAME_LINE );
			}
			else
			{
				Log ( "OK", SAME_LINE );
			}
		}
	}
	else
	{
		Log ( "FILE NOT FOUND", SAME_LINE );
	}

	if ( str_loaded && !db_error && !num_errors )
	{
		if ( UpdateDB ( BabylonstrDB, MainDB, FALSE ) )
		{
			BabylonstrDB->Changed ();
			if ( db_loaded )
			{
				if ( db_readonly )
				{
					sprintf ( buffer, "\"%s\" has changed!\n\nHowever, as the database is READ ONLY you cannot update the changes.", BabylonstrFilename);
				}
				else
				{
					sprintf ( buffer, "\"%s\" has changed!\n\nRecomended that you update the database with these new changes.\n\nDo you wish to update now?", BabylonstrFilename);
				}
			}
			else
			{
				sprintf ( buffer, "New Database!\n\nRecomended that you update the new database.\n\nDo you wish to update now?", BabylonstrFilename);
			}

			if ( db_readonly )
			{
				do_update = FALSE;
				AfxMessageBox ( buffer );

			}
			else
			{
				do_update = (AfxMessageBox ( buffer, MB_YESNO ) == IDYES);
			}
		}
	}
	ProgressComplete ();
	Ready ();

	if ( do_update )
	{
		OnUpdate ();
	}
}

void CBabylonDlg::InitProgress(int range)
{

	if ( (progress_range = range) <= 0 )
	{
		progress_range = 1;
	}

	progress->SetRange ( 0, INCREMENTS );
	progress_pos = -1;
	progress->SetPos ( 0 );
}

void CBabylonDlg::SetProgress(int pos)
{
	char string[20];

	int new_pos = (pos * 100 ) / progress_range;

	if ( new_pos > 100 )
	{
		new_pos = 100;
	}
	else if ( new_pos < 0 )
	{
		new_pos = 0;
	}

	if ( new_pos == progress_pos )
	{
		return;
	}

	progress->SetPos ( new_pos );
	progress_pos = new_pos;
	sprintf ( string, "%d%% ", progress_pos );
	percent->SetWindowText ( string );

}

void CBabylonDlg::ProgressComplete()
{
	progress->SetPos ( 100 );
	percent->SetWindowText ( "100% ");
}

void CBabylonDlg::OnUpdate()
{
	// TODO: Add your control notification handler code here

	UpdateDB ( BabylonstrDB, MainDB );

}

#define MACRO_UPDATE(field,count)	{ if ( wcsicmp ( source->##field () , destination->##field ())) \
															{ 																														\
																if ( update )																								\
																{																														\
																	destination->Set##field ( source->##field () );						\
																}																														\
																label_modified = TRUE;																			\
																info.changes++;																							\
																(count)++;																									\
															}																															\
														 }


int CBabylonDlg::UpdateLabel( BabylonLabel *source, BabylonLabel *destination, UPDATEINFO &info, int update, int skip )
{
	BabylonText *stext, *dtext;
	ListSearch sh;
	TransDB *destDB, *srcDB;
	int label_modified = FALSE;

	destination->ClearMatched ();
	source->ClearMatched ();
	destDB = destination->DB();
	srcDB = source->DB ();

	// first go through and match up as many strings as possible

	stext = source->FirstText ( sh );

	while ( stext )
	{

		dtext = destDB->FindText ( stext->Get ());

		// remember FindText() spans labels so keep looking till we find
		// one that belongs to the label we are checking
		while ( dtext && (dtext->Label () != destination) )
		{
			dtext = destDB->FindNextText ();
		}

		if ( dtext && dtext->Matched ())
		{
			AfxMessageBox ( "Fatal error: substring already matched" );
			return FALSE;
		}

		if ( dtext )
		{
			// we have a matching string so mark it
			dtext->Match ( stext );
			stext->Match ( dtext );

		}

		stext = source->NextText ( sh );
	}


	// ask the user to resolve remaing unmatched strings

	{

		stext = source->FirstText ( sh );

		while ( stext )
		{
			if ( destination->AllMatched ())
			{
				// no point trying to match anymore
				break;
			}

			if ( !stext->Matched () )
			{
				int result;
				BabylonText *match = NULL;

				if ( update && !skip )
				{
					if ( destination->DB()->MultiTextAllowed())
					{
						result = MatchText ( stext, destination, &match );
					}
					else
					{
						ListSearch tsh;
						BabylonText *oldtext = destination->FirstText ( tsh );
						if ( !oldtext )
						{
							break;
						}
						result = RetranslateText ( stext, oldtext );
						match = oldtext;
					}
				}
				else
				{
					result = IDSKIP;
				}

				if ( result == IDCANCEL || result == IDSKIP)
				{
					return result;
				}

				if ( match )
				{
					stext->Match ( match );
					match->Match ( stext );
				}
				stext->Processed ();
			}
			stext = source->NextText ( sh );
		}
	}


	// go through all matched strings and update them accordingly

	dtext = destination->FirstText ( sh );

	while ( dtext )
	{

		if ( (stext = (BabylonText *) dtext->Matched ()) )
		{
			// stext is the newer version;
			if ( wcscmp ( dtext->Get (), stext->Get ()))
			{
				if ( update )
				{
					dtext->Set ( stext->Get ());
				}
				info.modified_strings++;
				label_modified = TRUE;
				info.changes ++;
			}
			if ( wcsicmp ( dtext->Wave (), stext->Wave ()))
			{
				if ( update )
				{
					dtext->SetWave ( stext->Wave ());
				}
				info.updated_waves++;
				label_modified = TRUE;
				info.changes ++;
			}
			if ( dtext->Retranslate ())
			{
				if ( update )
				{
					dtext->IncRevision ();
				}
				label_modified = TRUE;
			}
			dtext->SetRetranslate ( FALSE );
		}

		dtext = destination->NextText ( sh );
	}


	// any remaining umatched text in the source are new strings
	// any remaining umatched text in the destination are now obsolete

	// delete old strings	from destination

	dtext = destination->FirstText ( sh );

	while ( dtext )
	{
		BabylonText *next = destination->NextText ( sh );

		if ( !dtext->Matched ())
		{
			if ( update )
			{
				dtext->Remove ();
				destDB->AddObsolete ( dtext );
			}
			info.deleted_strings++;
			label_modified = TRUE;
			info.changes ++;
		}

		dtext = next;
	}

	// add new strings from source

	stext = source->FirstText ( sh );

	while ( stext )
	{

		if ( !stext->Matched ())
		{
			if ( update )
			{
				dtext = stext->Clone ();
				destination->AddText ( dtext );
			}
			info.new_strings++;
			label_modified = TRUE;
			info.changes ++;
		}

		stext = source->NextText ( sh );
	}

	// finally update label info
	MACRO_UPDATE(Comment, info.updated_comments);
	MACRO_UPDATE(Context, info.updated_contexts);
	MACRO_UPDATE(Speaker, info.updated_speakers);
	MACRO_UPDATE(Listener, info.updated_listeners);

	if ( destination->MaxLen () != source->MaxLen ())
	{
		if ( update )
		{
			destination->SetMaxLen ( source->MaxLen ());
		}
		label_modified = TRUE;
		info.updated_maxlen++;
		info.changes ++;
	}


	if ( label_modified )
	{
		if ( update )
		{
			source->ClearChanges ();
		}
		else
		{
			source->Changed ();
		}

		info.modified_labels ++;
	}

	return IDOK;

}

int CBabylonDlg::UpdateDB(TransDB *source, TransDB *destination, int update )
{
	BabylonLabel	*slabel;
	BabylonLabel	*dlabel;
	ListSearch sh;
	int	count = 0;
	int result = IDOK;
	UPDATEINFO	info;
	int changes = FALSE;
	int diffs = 0;
	int skip_all = FALSE;

	memset ( &info, 0, sizeof ( info ));
	if ( update )
	{
		sprintf ( buffer, "Updating \"%s\" from \"%s\"...", destination->Name(), source->Name());
		Log("");
		Status ( buffer );
	}
	else
	{
		Status ("Checking for changes...", FALSE );
	}

	source->ClearProcessed ();
	destination->ClearProcessed ();

	if ( update )
	{
		InitProgress ( source->NumLabels() );
	}
	slabel = source->FirstLabel ( sh );

	while ( slabel )
	{
		if ( (dlabel = destination->FindLabel ( slabel->Name ())))
		{
			dlabel->Processed ();

			result = UpdateLabel ( slabel, dlabel, info, update, skip_all );

			if ( result == IDCANCEL )
			{
				skip_all = TRUE;
			}

			if ( result == IDOK )
			{
				if ( update )
				{
					slabel->ClearChanges ();
				}
			}
			else
			{
				info.skipped_labels ++;
				info.changes ++;
				if ( !update )
				{
					slabel->Changed();
				}
			}
		}
		else
		{
			BabylonLabel *clone;

			if ( update )
			{
				clone = slabel->Clone ();
				destination->AddLabel ( clone );
				clone->Processed ();
				slabel->ClearChanges ();
			}
			else
			{
				slabel->Changed ();
			}
			info.new_strings += slabel->NumStrings ();
			info.changes += slabel->NumStrings ();
			info.new_labels++;
			info.changes++;
		}

		count++;
		if ( update )
		{
			SetProgress ( count );
		}

		slabel->Processed ();
		slabel = source->NextLabel ( sh );
	}

	// go through all unprocessed labels in the destination database and remove them.
	dlabel = destination->FirstLabel ( sh );

	while ( dlabel )
	{
		BabylonLabel *next_label = destination->NextLabel ( sh );

		if ( !dlabel->IsProcessed ())
		{
			// this label was not matched so is obsolete
			ListSearch sh_text;
			BabylonText *dtext = dlabel->FirstText ( sh_text);

			while ( dtext )
			{
				BabylonText *next = dlabel->NextText ( sh_text );

				if ( update )
				{
					dtext->Remove ();
					destination->AddObsolete ( dtext );
				}

				info.deleted_strings++;
				info.changes ++;

				dtext = next;
			}

			if ( update )
			{
				dlabel->Remove ();
				delete dlabel;
			}

			info.deleted_labels++;
		}

		dlabel = next_label;
	}


	if ( update )
	{
		if ( info.new_labels )
		{
			sprintf ( buffer, "Added %d new label%c", info.new_labels, info.new_labels==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.deleted_labels )
		{
			sprintf ( buffer, "Deleted %d label%c", info.deleted_labels, info.deleted_labels==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.modified_labels )
		{
			sprintf ( buffer, "Modified %d label%c", info.modified_labels, info.modified_labels==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.new_strings )
		{
			sprintf ( buffer, "Added %d new string%c", info.new_strings, info.new_strings==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.deleted_strings )
		{
			sprintf ( buffer, "Deleted %d string%c", info.deleted_strings, info.deleted_strings==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.modified_strings )
		{
			sprintf ( buffer, "Modified %d string%c", info.modified_strings, info.modified_strings==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.skipped_labels )
		{
			sprintf ( buffer, "Skipped %d label%c", info.skipped_labels, info.skipped_labels==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.updated_comments )
		{
			sprintf ( buffer, "Updated %d comment%c", info.updated_comments, info.updated_comments==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.updated_contexts )
		{
			sprintf ( buffer, "Updated %d context%c", info.updated_contexts, info.updated_contexts==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.updated_speakers )
		{
			sprintf ( buffer, "Updated %d speaker%c", info.updated_speakers, info.updated_speakers==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.updated_listeners )
		{
			sprintf ( buffer, "Updated %d listener%c", info.updated_listeners, info.updated_listeners==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.updated_maxlen )
		{
			sprintf ( buffer, "Updated %d max length%c", info.updated_maxlen, info.updated_maxlen==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( info.updated_waves )
		{
			sprintf ( buffer, "Updated %d speech file%c", info.updated_waves, info.updated_waves==1?' ':'s' );
			Log ( buffer );
			changes = TRUE;
		}

		if ( !changes  )
		{
			if ( !slabel && !info.skipped_labels)
			{
				Log ( "No differences found" );
			}
			else
			{
				Log ( "No changes made" );
			}
		}

		if ( result == IDCANCEL )
		{
			Log ("Update aborted by user!" );
			InitProgress ( 100 );
		}
		else
		{
			if ( !info.skipped_labels )
			{
				source->ClearChanges ();
			}
		}
	}


	Ready ();

	return info.changes;
}

void CBabylonDlg::OnSave()
{

	if ( CanOperate ())
	{
		SaveMainDB ( );
	}
}

int CBabylonDlg::SaveMainDB( )
{
	TransDB *db = MainDB;
	const char *filename = MainXLSFilename;
	int attribs;

	if ( !db )
	{
		return TRUE;
	}

	if ( !db->IsChanged ())
	{
		return TRUE;
	}

	attribs = FileAttribs ( filename );

	if ( attribs & FA_READONLY )
	{
		char buffer[100];
		sprintf ( buffer, "Cannot save changes!\n\nFile \"%s\" is read only", filename );
		AfxMessageBox ( buffer );
		sprintf ( buffer, "Cannot save to \"%s\". File is read only", filename );
		Log ( buffer );
		Status ("Save failed", FALSE );
		return FALSE;
	}

	Log("");
	Status ( "Saving main database..." );

	if ( attribs != FA_NOFILE )
	{
		MakeBackupFile ( filename );
	}

	if ( !WriteMainDB ( db, filename, this ) )
	{
		RestoreBackupFile ( filename );
		Log ("FAILED", SAME_LINE );
		Status ("Save failed", FALSE );
		return FALSE;
	}
	else
	{
		Log ("OK", SAME_LINE );
	}

	Ready();
	return TRUE;
}


void CBabylonDlg::OnWarnings()
{
	// TODO: Add your control notification handler code here
	if ( BabylonstrDB )
	{
		BabylonstrDB->Warnings ( this );
	}

}


void CBabylonDlg::OnErrors()
{
	// TODO: Add your control notification handler code here
	if ( BabylonstrDB )
	{
		BabylonstrDB->Errors ( this );
	}
}


int CBabylonDlg::MatchText ( BabylonText *text, BabylonLabel *label, BabylonText **match )
{
	CMatchDlg dlg;
	int result;

	*match = NULL;
	sprintf ( buffer, "Text: %s\n\nLabel:%s\n", text->GetSB (), label->NameSB () );

	// TODO: Add your control notification handler code here

	MatchOriginalText = text;
	MatchLabel = label;

	result = dlg.DoModal ();

	if ( result != IDCANCEL )
	{
		*match = MatchingBabylonText;
	}

	return result;

}

int CBabylonDlg::RetranslateText ( BabylonText *newtext, BabylonText *oldtext )
{
	RetranslateDlg dlg;
	int result;

	// TODO: Add your control notification handler code here

	dlg.newtext = newtext;
	dlg.oldtext = oldtext;

	result = dlg.DoModal ( );

	return result;

}
void CBabylonDlg::OnChanges()
{
	// TODO: Add your control notification handler code here

	VIEWDBSII dlg;

	ViewChanges = TRUE;

	dlg.DoModal ();



}

void CBabylonDlg::OnImport()
{
	if ( CanOperate ())
	{
		CFileDialog fd ( TRUE , NULL, "*.xls",  OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR );

		if ( fd.DoModal () == IDOK )
		{
			if (ImportTranslations ( MainDB, (LPCSTR ) fd.GetPathName (), this ) == -1 )
			{
//				ProcessWaves ( BabylonstrDB, fd.GetPathName (), this );
			}
		}
	}
}

void CBabylonDlg::OnGenerate()
{
	if ( CanOperate ())
	{
		CGenerateDlg dlg;

		if ( dlg.DoModal () == IDOK )
		{
			GenerateGameFiles ( MainDB, dlg.FilePrefix(), dlg.Options(), dlg.Langauges(), this );
		}
	}
}

int CBabylonDlg::ValidateStrFile( const char *filename)
{
	STARTUPINFO StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo;
	const char *results = "strcheck.rst";
	int errors = 0;
	FILE *file = NULL;

	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_SHOWMINNOACTIVE;

	Log ("");
	sprintf ( buffer, "Verifying \"%s\"...", filename );
	Status ( buffer );
	if ( FileExists ( results ))
	{
		DeleteFile ( results );
	}

	sprintf ( buffer, "strcheck %s %s", filename, results );

	if (!CreateProcess(
			NULL,
			buffer,
			NULL,
			NULL,
			FALSE,
			0,
			NULL,
			NULL,
			&StartupInfo,
			&ProcessInfo))
	{
		goto error;
	}


	WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

	//this->SetForegroundWindow ();
	//this->RedrawWindow ();

	if ( !(file = fopen ( results, "rt" )))
	{
		goto error;
	}

	while ( fgets ( buffer, sizeof(buffer), file ) )
	{
		strlwr ( buffer );
		if ( strstr ( buffer, "error") || strstr ( buffer, "warning" ))
		{
			errors++;
		}
	}

	if ( errors )
	{
		sprintf ( buffer, "%d ERROR%s", errors, errors == 1 ? "" : "S" );
		Log (buffer, SAME_LINE );
		fseek ( file, 0, SEEK_SET );
		while ( fgets ( buffer, sizeof(buffer), file ) )
		{
			sprintf ( buffer2, "   strcheck> %s", buffer );
			strlwr ( buffer );
			if ( strstr ( buffer2, "error") || strstr ( buffer, "warning" ) )
			{
				int len = strlen ( buffer2 );

				if ( buffer2[len-1] == '\n' )
				{
					buffer2[len-1] = 0;
				}
				Log ( buffer2 );
			}
		}


	}
	else
	{
		Log ("OK", SAME_LINE );
	}


done:

	if ( FileExists ( results ))
	{
		DeleteFile ( results );
	}

	if (file )
	{
		fclose (file );
	}

	return errors;

error:
		Log ( "UNABLE TO VERIFY", SAME_LINE );
		errors = -1;
	goto done;
}

void CBabylonDlg::OnVerifyDialog()
{
	if ( MainDB && CanOperate () )
	{
		VerifyDialog ( MainDB, CurrentLanguage );
	}

}

void CBabylonDlg::VerifyDialog( TransDB *db, LangID langid )
{
	BabylonLabel *label;
	ListSearch sh_label;
	int count = 0;
	DLGREPORT _info;
	DLGREPORT *info = &_info;

	Log ("");

	sprintf ( buffer, "Verifying %s dialog...", GetLangName ( langid ) );
	Status ( buffer );

	InitProgress ( db->NumLabels () );
	cb_count = 0;
	mainDlg = this;
	db->VerifyDialog ( langid, cb_progress );
	db->ReportDialog ( info, langid);

	if ( info->unresolved )
	{
		Status ( "Verification", FALSE);

		InitProgress ( info->unresolved );

		label = db->FirstLabel ( sh_label );

		while ( label )
		{
			BabylonText *text;
			ListSearch sh_text;

			text = label->FirstText ( sh_text );

			while ( text )
			{
				if ( text->IsDialog ())
				{
					if ( text->DialogIsPresent ( DialogPath, langid ))
					{
						if ( !text->DialogIsValid ( DialogPath, langid, FALSE ) )
						{
						 	VerifyDlg dlg(text, langid, DialogPath);
						 	int result;

						 	result = dlg.DoModal ();

							if ( result == IDCANCEL )
							{
								goto done;
							}
							if ( result == IDOK )
							{
								text->ValidateDialog ( DialogPath, langid );
							}

							count++;
							SetProgress ( count );
						}
					}
				}

				text = label->NextText ( sh_text );
			}

			label = db->NextLabel ( sh_label );
		}
	}

done:
	Ready ();

	Status ( "Collecting stats...", FALSE );

	count = db->ReportDialog ( info, langid );

	if ( count < 100 )
	{
		InitProgress ( count );

		cb_count = 0;
		found_error = FALSE;
		mainDlg = this;
		db->ReportDialog ( info, langid, print_to_log_and_update_progress );
	}

	if ( info->numdialog )
	{
		if ( info->errors || info->missing || info->unresolved )
		{
			if ( !found_error )
			{
				Log ( "FAILED", SAME_LINE );
			}

			if ( info->errors )
			{
				sprintf ( buffer, "Errors           : %d", info->errors );
				Log ( buffer );
			}

			if ( info->missing )
			{
				sprintf ( buffer, "Missing dialog   : %d", info->missing );
				Log ( buffer );
			}

			if ( info->unresolved )
			{
				sprintf ( buffer, "Unverified dialog: %d", info->unresolved );
				Log ( buffer );
			}

			if ( info->resolved )
			{
				sprintf ( buffer, "Verified dialog  : %d", info->resolved );
				Log ( buffer );
			}

		}
		else
		{
			Log ( "OK", SAME_LINE );

			if ( info->resolved )
			{
				sprintf ( buffer, "Verified dialog  : %d", info->resolved );
				Log ( buffer );
			}
		}
		sprintf ( buffer, "Total dialog      : %d", info->numdialog );
		Log ( buffer );
	}
	else
	{
		Log ( "NO DIALOG FOUND", SAME_LINE );
	}

	Ready();

}

void CBabylonDlg::VerifyTranslations( TransDB *db, LangID langid )
{
	int count = 0;
	TRNREPORT _info;
	TRNREPORT *info = &_info;

	Log ("");
	sprintf ( buffer, "Verifying %s text...", GetLangName ( langid ) );
	Status ( buffer );

	count = db->ReportTranslations ( info, langid );

	if ( count < 100 )
	{
		InitProgress ( count);
		cb_count = 0;
		found_error = FALSE;
		mainDlg = this;
		db->ReportTranslations ( info, langid, print_to_log_and_update_progress );
	}

	if ( info->numstrings )
	{
		if ( info->too_big || info->missing || info->retranslate || info->bad_format )
		{

			if ( info->missing )
			{
				sprintf ( buffer, "Missing translations: %d", info->missing );
				Log ( buffer );
			}

			if ( info->too_big )
			{
				sprintf ( buffer, "Oversized strings   : %d", info->too_big );
				Log ( buffer );
			}

			if ( info->retranslate )
			{
				sprintf ( buffer, "Retranslations       : %d", info->retranslate);
				Log ( buffer );
			}

			if ( info->bad_format )
			{
				sprintf ( buffer, "Badly formated translations: %d", info->bad_format);
				Log ( buffer );
			}

			if ( langid == LANGID_US )
			{
				sprintf  ( buffer, "Recommemd editing \"%s\" to fix problems and reload", BabylonstrFilename );
			}
			else
			{
				sprintf  ( buffer, "Recommemd exporting translations for update and re-import" );
			}
			Log ( buffer );
		}
		else
		{
			Log ( "OK", SAME_LINE );

		}
	}
	else
	{
		Log ( "NO TEXT", SAME_LINE );
	}

	Ready();
}

void CBabylonDlg::OnTranslations()
{
	if ( MainDB && CanOperate () )
	{

		VerifyTranslations ( MainDB, CurrentLanguage );
	}


}

void CBabylonDlg::OnSelchangeCombolang()
{
	LANGINFO *info = NULL;
	int index;

	index = combo->GetCurSel ();

	if ( index >= 0  && index < max_index )
	{
		info = (LANGINFO *) combo->GetItemDataPtr ( index );
	}

	if ( info )
	{
		CurrentLanguage = info->langid;
	}
	else
	{
		CurrentLanguage = LANGID_UNKNOWN;
	}

}

void CBabylonDlg::OnReports()
{
	// TODO: Add your control notification handler code here
	if ( CanOperate ())
	{
		CReport dlg;

		if ( dlg.DoModal () == IDOK )
		{
			GenerateReport ( MainDB, dlg.Filename(), dlg.Options(), dlg.Langauges(), this );
		}
	}

}

void CBabylonDlg::OnDblclkCombolang()
{
	// TODO: Add your control notification handler code here
}

void CBabylonDlg::OnReset()
{
	// TODO: Add your control notification handler code here

	if ( CurrentLanguage != LANGID_UNKNOWN )
	{
		sprintf ( buffer, "Are you sure you want to invalidate all %s dialog?", GetLangName ( CurrentLanguage ));
		if ( AfxMessageBox ( buffer, MB_YESNO) == IDYES )
		{
			MainDB->InvalidateDialog ( CurrentLanguage );
		}
	}
}

void CBabylonDlg::OnSent()
{
	// TODO: Add your control notification handler code here
	if ( CanOperate ())
	{
		CFileDialog fd ( TRUE , NULL, "*.xls",  OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR );

		if ( fd.DoModal () == IDOK )
		{
			UpdateSentTranslations ( MainDB, (LPCSTR ) fd.GetPathName (), this );
		}
	}
}

void CAboutDlg::OnButton1()
{
	// TODO: Add your control notification handler code here

	CreateTranslationTable ( );
}

