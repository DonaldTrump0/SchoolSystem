#include "pch.h"
#include "SchoolSystem.h"
#include "CClassDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CClassDlg, CDialogEx)

CClassDlg::CClassDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CLASS, pParent)
	, m_strClassId(_T(""))
	, m_strClassName(_T(""))
{
}

CClassDlg::~CClassDlg()
{
}

void CClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CLASS_ID, m_strClassId);
	DDX_Text(pDX, IDC_EDIT_CLASS_NAME, m_strClassName);
}

BEGIN_MESSAGE_MAP(CClassDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CClassDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CClassDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	UpdateData(FALSE);

	return TRUE;
}


void CClassDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	CDialogEx::OnOK();
}
