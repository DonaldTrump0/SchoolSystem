#pragma once
#include <string>
#include <map>
#include "../TcpSocketDll/CTcpSocket.h"
#pragma comment(lib, "../Debug/TcpSocketDll.lib")
using namespace std;

class CSchoolSystemDlg : public CDialogEx
{
public:
	CSchoolSystemDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCHOOLSYSTEM_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

public:
	HICON m_hIcon;
	CTabCtrl m_tab;
	CListCtrl m_list;
	int m_nCurTabSel;
	CTcpSocket m_tcpSocket;
	sockaddr m_addr;
	map<string, string> m_fieldMap;	// ±í×Ö¶ÎÓ³Éä

	void ShowList();
	void SendSql(CString sql);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButtonInsert();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonSelect();
	afx_msg void OnBnClickedButtonUnionSelect();
};
