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

// VerifyDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Babylon.h"
#include "VerifyDlg.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMERID	1000

/////////////////////////////////////////////////////////////////////////////
// VerifyDlg dialog


VerifyDlg::VerifyDlg( BabylonText *ntext, LangID langid,  const char *path, CWnd* pParent /*=NULL*/)
	: CDialog(VerifyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(VerifyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	babylon_text = ntext;
	linfo = GetLangInfo ( langid );
	sprintf ( wavefile, "%s%s\\%s%s.wav", path, linfo->character, ntext->WaveSB(), linfo->character  );
}


void VerifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VerifyDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VerifyDlg, CDialog)
	//{{AFX_MSG_MAP(VerifyDlg)
	ON_BN_CLICKED(IDC_NOMATCH, OnNomatch)
	ON_BN_CLICKED(IDOK, OnMatch)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_PAUSE, OnPause)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VerifyDlg message handlers

BOOL VerifyDlg::OnInitDialog() 
{
	//long total;
	CDialog::OnInitDialog();
	RECT rect;
	
	// TODO: Add extra initialization here

	this->GetWindowRect ( &rect );
	rect.top -= 100;
	rect.bottom -= 100;
	this->MoveWindow ( &rect );
	stop.AutoLoad ( IDC_STOP, this );
	pause.AutoLoad ( IDC_PAUSE, this );
	play.AutoLoad ( IDC_PLAY, this );

	wave = GetDlgItem ( IDC_WAVENAME );
	text = (CStatic *) GetDlgItem ( IDC_TEXT );
	slider = (CSliderCtrl *) GetDlgItem ( IDC_SLIDER );

	wave->SetWindowText ( wavefile );
	SetDlgItemText ( IDC_TEXT_TITLE, (babylon_text->Label()->NameSB()));
	if ( linfo->langid == LANGID_US )
	{
		text->SetWindowText ( babylon_text->GetSB ());
	}
	else
	{
		Translation *trans = babylon_text->GetTranslation ( linfo->langid );

		if ( trans )
		{
			text->SetWindowText ( trans->GetSB ());
		}
		else
		{
			text->SetWindowText ( "No translation!!");
		}

	}

#if 0
		HDIGDRIVER dig;
		HMDIDRIVER mdi;
		HDLSDEVICE dls;

		AIL_quick_handles ( &dig, &mdi, &dls );
		stream = AIL_open_stream ( dig, wavefile, 0 ); 
		if ( stream )
		{
			timer = SetTimer( TIMERID, 300, NULL );
			AIL_stream_ms_position ( stream, &total, NULL );
			slider->SetRange ( 0, total );
		}
#endif

	PostMessage ( WM_COMMAND, MAKEWPARAM ( IDC_PLAY, BN_CLICKED ));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void VerifyDlg::OnNomatch() 
{
	// TODO: Add your control notification handler code here
		CloseAudio ();
		this->EndDialog ( IDSKIP );
}

void VerifyDlg::OnMatch() 
{
	// TODO: Add your control notification handler code here
	CloseAudio ();
	CDialog::OnOK();
	
}

void VerifyDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CloseAudio ();
	CDialog::OnCancel();
}

void VerifyDlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	#if 0
		if ( stream )
		{
			AIL_pause_stream ( stream, TRUE );
			AIL_set_stream_ms_position ( stream, 0 );
		}
		#endif
}

void VerifyDlg::OnPlay() 
{
	// TODO: Add your control notification handler code here
	#if 0
	if ( stream )
	{
		if ( AIL_stream_status ( stream ) == SMP_STOPPED )
		{
			AIL_pause_stream ( stream, FALSE);
		}
		else
		{
			AIL_start_stream ( stream );
		}
	}
	#endif
}

void VerifyDlg::OnPause() 
{
	// TODO: Add your control notification handler code here
	#if 0
	if ( stream )
	{
		if ( AIL_stream_status ( stream ) == SMP_STOPPED )
		{
			AIL_pause_stream ( stream, FALSE);
		}
		else
		{
			AIL_pause_stream ( stream, TRUE);
		}
	}
	#endif
	
}

void VerifyDlg::CloseAudio ( void )
{
	#if 0
	if ( stream )
	{
		AIL_close_stream ( stream );
		stream = NULL;
	}
	#endif
}

void VerifyDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if ( nIDEvent == TIMERID )
	{
	#if 0
		if ( stream )
		{
			long current;
			AIL_stream_ms_position ( stream, NULL, &current );
			slider->SetPos ( current );
		}
	#endif
	}
	else
	{
		CDialog::OnTimer(nIDEvent);
	}
}

