#include "pch.h"
#include "SchoolSystem.h"
#include "CStudentDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CStudentDlg, CDialogEx)

CStudentDlg::CStudentDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_STUDENT, pParent)
	, m_strStuId(_T(""))
	, m_strStuName(_T(""))
	, m_strClassId(_T(""))
{
}

CStudentDlg::~CStudentDlg()
{
}

void CStudentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_STU_NAME, m_strStuName);
	DDX_Text(pDX, IDC_EDIT_CLASS_ID, m_strClassId);
	DDX_Text(pDX, IDC_EDIT_STU_ID, m_strStuId);
}

BEGIN_MESSAGE_MAP(CStudentDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CStudentDlg::OnBnClickedOk)
END_MESSAGE_MAP()

void CStudentDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	CDialogEx::OnOK();
}


BOOL CStudentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	UpdateData(FALSE);

	return TRUE;
}
