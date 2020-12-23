#pragma once


// CSelectDlg dialog

class CSelectDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectDlg)

public:
	CSelectDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CSelectDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SELECT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strStuId;
	CString m_strCourseId;
	CString m_strScore;
	afx_msg void OnBnClickedOk();
};
