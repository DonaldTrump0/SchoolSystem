#pragma once

class CCourseDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCourseDlg)

public:
	CCourseDlg(CWnd* pParent = nullptr);
	virtual ~CCourseDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_COURSE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	CString m_strCourseId;
	CString m_strCourseName;
	afx_msg void OnBnClickedOk();
};
