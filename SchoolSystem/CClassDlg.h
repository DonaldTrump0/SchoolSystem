#pragma once

class CClassDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClassDlg)

public:
	CClassDlg(CWnd* pParent = nullptr);
	virtual ~CClassDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CLASS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	CString m_strClassId;
	CString m_strClassName;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
