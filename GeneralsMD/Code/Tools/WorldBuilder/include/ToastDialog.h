#pragma once

#include <afxwin.h>

#define IDD_TOAST 242
#define IDC_HINT_TEXT 256

class CToastDialog : public CDialog
{
public:
    CToastDialog(const CString& message, int durationMs = 3000, bool showButtons = true, CWnd* pParent = NULL);


    enum { IDD = IDD_TOAST };

protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg void OnCancel();
    afx_msg void OnOK();
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

private:
    CString m_message;
    int m_durationMs;
    UINT_PTR m_nTimerID;
    bool m_showButtons;
};