#pragma once

class CStudentDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStudentDlg)

public:
	CStudentDlg(CWnd* pParent = nullptr);
	virtual ~CStudentDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_STUDENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	CString m_strStuId;
	CString m_strStuName;
	CString m_strClassId;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
