// CCourseDlg.cpp : implementation file
//

#include "pch.h"
#include "SchoolSystem.h"
#include "CCourseDlg.h"
#include "afxdialogex.h"


// CCourseDlg dialog

IMPLEMENT_DYNAMIC(CCourseDlg, CDialogEx)

CCourseDlg::CCourseDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_COURSE, pParent)
	, m_strCourseId(_T(""))
	, m_strCourseName(_T(""))
{

}

CCourseDlg::~CCourseDlg()
{
}

void CCourseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COURSE_ID, m_strCourseId);
	DDX_Text(pDX, IDC_EDIT_COURSE_NAME, m_strCourseName);
}


BEGIN_MESSAGE_MAP(CCourseDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCourseDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCourseDlg message handlers


void CCourseDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	CDialogEx::OnOK();
}
