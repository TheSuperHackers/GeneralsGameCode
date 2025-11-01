#include "stdafx.h"
#include "ToastDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// static members
std::vector<CToastDialog*> CToastDialog::s_activeToasts;
const int CToastDialog::kToastSpacing = 10;

CToastDialog::CToastDialog(const CString& message, int durationMs, bool showButtons, CWnd* pParent)
    : CDialog(IDD, pParent),
      m_message(message),
      m_durationMs(durationMs),
      m_showButtons(showButtons),
      m_nTimerID(0),
      m_offsetY(0)
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

    // Register this toast
    s_activeToasts.push_back(this);

    // Calculate vertical stacking position (C-style loops for old compilers)
    int baseY = 100;
    m_offsetY = baseY;
    for (size_t i = 0; i < s_activeToasts.size(); ++i)
    {
        if (s_activeToasts[i] == this)
            break;

        CRect rect;
        s_activeToasts[i]->GetWindowRect(&rect);
        m_offsetY += rect.Height() + kToastSpacing;
    }

    // Move window (no size change)
    SetWindowPos(NULL, 10, m_offsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    // Set message
    CStatic* pStatic = (CStatic*)GetDlgItem(IDC_HINT_TEXT);
    if (pStatic)
        pStatic->SetWindowText(m_message);

    // Hide buttons if requested
    if (!m_showButtons) {
        if (CWnd* pOK = GetDlgItem(IDOK)) pOK->ShowWindow(SW_HIDE);
        if (CWnd* pCancel = GetDlgItem(IDCANCEL)) pCancel->ShowWindow(SW_HIDE);
    }

    // Start timer
    m_nTimerID = SetTimer(1, m_durationMs, NULL);

    return TRUE;
}

BOOL CToastDialog::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.dwExStyle |= WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
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

    // Remove from active list (manual erase to avoid <algorithm> usage)
    for (size_t i = 0; i < s_activeToasts.size(); ++i)
    {
        if (s_activeToasts[i] == this)
        {
            s_activeToasts.erase(s_activeToasts.begin() + i);
            break;
        }
    }

    // Re-stack remaining toasts upward
    int y = 60;
    for (size_t b = 0; b < s_activeToasts.size(); ++b) {
        s_activeToasts[b]->SetWindowPos(NULL, 10, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        CRect r;
        s_activeToasts[b]->GetWindowRect(&r);
        y += r.Height() + kToastSpacing;
    }

    CDialog::OnDestroy();
}
