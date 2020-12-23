// CSelectDlg.cpp : implementation file
//

#include "pch.h"
#include "SchoolSystem.h"
#include "CSelectDlg.h"
#include "afxdialogex.h"


// CSelectDlg dialog

IMPLEMENT_DYNAMIC(CSelectDlg, CDialogEx)

CSelectDlg::CSelectDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SELECT, pParent)
	, m_strCourseId(_T(""))
	, m_strStuId(_T(""))
	, m_strScore(_T(""))
{

}

CSelectDlg::~CSelectDlg()
{
}

void CSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COURSE_ID, m_strCourseId);
	DDX_Text(pDX, IDC_EDIT_STU_ID, m_strStuId);
	DDX_Text(pDX, IDC_EDIT_SCORE, m_strScore);
}


BEGIN_MESSAGE_MAP(CSelectDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSelectDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSelectDlg message handlers


void CSelectDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	CDialogEx::OnOK();
}
