#include "stdafx.h"
#include "ToastDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CToastDialog::CToastDialog(const CString& message, int durationMs, bool showButtons, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD, pParent),
      m_message(message),
      m_durationMs(durationMs),
      m_showButtons(showButtons),
      m_nTimerID(0)
{
}

void CToastDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CToastDialog, CDialog)
    ON_WM_TIMER()
    ON_WM_DESTROY()
END_MESSAGE_MAP()


BOOL CToastDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set position
    SetWindowPos(NULL, 10, 60, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    // Set message
    CStatic* pStatic = (CStatic*)GetDlgItem(IDC_HINT_TEXT);
    if (pStatic)
        pStatic->SetWindowText(m_message);

    // Hide buttons if requested
    if (!m_showButtons) {
        CWnd* pOK = GetDlgItem(IDOK);
        CWnd* pCancel = GetDlgItem(IDCANCEL);
        if (pOK) pOK->ShowWindow(SW_HIDE);
        if (pCancel) pCancel->ShowWindow(SW_HIDE);
    }

    // Start timer
    m_nTimerID = SetTimer(1, m_durationMs, NULL);

    return TRUE;
}

BOOL CToastDialog::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.dwExStyle |= 0x08000000L | WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    return CDialog::PreCreateWindow(cs);
}

void CToastDialog::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1) {
        KillTimer(m_nTimerID);
        m_nTimerID = 0;
        DestroyWindow(); // modeless
    }
    CDialog::OnTimer(nIDEvent);
}

void CToastDialog::OnOK()
{
    if (m_nTimerID)
        KillTimer(m_nTimerID);
    m_nTimerID = 0;
    DestroyWindow();
}

void CToastDialog::OnCancel()
{
    if (m_nTimerID)
        KillTimer(m_nTimerID);
    m_nTimerID = 0;
    DestroyWindow();
}

void CToastDialog::OnDestroy()
{
    if (m_nTimerID)
        KillTimer(m_nTimerID);
    CDialog::OnDestroy();
}
