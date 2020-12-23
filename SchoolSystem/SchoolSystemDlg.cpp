#include "pch.h"
#include "framework.h"
#include "SchoolSystem.h"
#include "SchoolSystemDlg.h"
#include "afxdialogex.h"
#include "CStudentDlg.h"
#include "CCourseDlg.h"
#include "CClassDlg.h"
#include "CSelectDlg.h"
#include <string>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSchoolSystemDlg::CSchoolSystemDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SCHOOLSYSTEM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_fieldMap["stu_id"] = "学生ID";
	m_fieldMap["stu_name"] = "学生姓名";
	m_fieldMap["class_id"] = "班级ID";
	m_fieldMap["class_name"] = "班级名";
	m_fieldMap["course_id"] = "课程ID";
	m_fieldMap["course_name"] = "课程名";
	m_fieldMap["score"] = "成绩";
}

void CSchoolSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
	DDX_Control(pDX, IDC_LIST, m_list);
}

BEGIN_MESSAGE_MAP(CSchoolSystemDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CSchoolSystemDlg::OnTcnSelchangeTab)
	ON_BN_CLICKED(IDC_BUTTON_INSERT, &CSchoolSystemDlg::OnBnClickedButtonInsert)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CSchoolSystemDlg::OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CSchoolSystemDlg::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, &CSchoolSystemDlg::OnBnClickedButtonSelect)
	ON_BN_CLICKED(IDC_BUTTON_UNION_SELECT, &CSchoolSystemDlg::OnBnClickedButtonUnionSelect)
END_MESSAGE_MAP()

void CSchoolSystemDlg::ShowList()
{
	// 清空所有字段跟记录
	m_list.DeleteAllItems();
	while (m_list.DeleteColumn(0))
	{
	}
	
	// 接收字段总字节数
	int nBytes = 0;
	m_tcpClient.Recv((char*)&nBytes, sizeof(nBytes));

	// 接收字段
	char pBuf[0x1000] = { 0 };
	m_tcpClient.Recv(pBuf, nBytes);

	// 插入列
	char* p = pBuf;
	int nCol = 0;
	while (p < pBuf + nBytes)
	{
		m_list.InsertColumn(nCol, m_fieldMap[p].c_str(), 0, 150);
		nCol++;
		p += strlen(p) + 1;
	}

	// 接收记录总字节数
	m_tcpClient.Recv((char*)&nBytes, sizeof(nBytes));

	if (0 == nBytes)
	{
		return;
	}

	// 接收记录
	m_tcpClient.Recv(pBuf, nBytes);

	// 插入行
	p = pBuf;
	int nRow = 0;
	while (p < pBuf + nBytes)
	{
		m_list.InsertItem(nRow, NULL);
		for (int i = 0; i < nCol; i++)
		{
			m_list.SetItemText(nRow, i, p);
			p += strlen(p) + 1;
		}
		nRow++;
	}
}

void CSchoolSystemDlg::SendSql(CString sql)
{
	int nLen = sql.GetLength() + 1;
	// 发送sql语句长度
	m_tcpClient.Send((char*)&nLen, sizeof(nLen));
	// 发送sql语句
	m_tcpClient.Send(sql, nLen);
}

BOOL CSchoolSystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_nCurTabSel = 0;

	m_tab.InsertItem(0, "学生");
	m_tab.InsertItem(1, "课程");
	m_tab.InsertItem(2, "班级");
	m_tab.InsertItem(3, "选课");

	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 创建socket
	if (!m_tcpClient.Connect("127.0.0.1", 5566))
	{
		AfxMessageBox("连接服务器失败");
		EndDialog(0);
	}

	// 显示所有学生
	SendSql("SELECT * FROM t_student");
	ShowList();

	// return TRUE  unless you set the focus to a control
	return TRUE;
}

// If you add a minimize button to your dialog, you will need the code below to draw the icon. 
void CSchoolSystemDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags the minimized window.
HCURSOR CSchoolSystemDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSchoolSystemDlg::OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	m_nCurTabSel = m_tab.GetCurSel();

	CString sql;
	switch (m_nCurTabSel)
	{
	// 查询所有学生
	case 0:
	{
		sql = "SELECT * FROM t_student";
		break;
	}
	// 查询所有课程
	case 1:
	{
		sql = "SELECT * FROM t_course";
		break;
	}
	// 查询所有班级
	case 2:
	{
		sql = "SELECT * FROM t_class";
		break;
	}
	// 查询所有选课
	case 3:
	{
		sql = "SELECT * FROM t_select";
		break;
	}
	}
	
	SendSql(sql);
	ShowList();
}


void CSchoolSystemDlg::OnBnClickedButtonInsert()
{
	switch (m_nCurTabSel)
	{
	case 0:
	{
		CStudentDlg studentDlg;
		if (IDOK == studentDlg.DoModal())
		{
			CString sql;
			sql.Format("INSERT INTO t_student (stu_id, stu_name, class_id) VALUES ('%s', '%s', '%s')",
				studentDlg.m_strStuId, studentDlg.m_strStuName, studentDlg.m_strClassId);
			SendSql(sql);

			int nSuccess = 0;
			m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
			if (-1 != nSuccess)
			{
				// 插入行
				m_list.InsertItem(0, NULL);
				m_list.SetItemText(0, 0, studentDlg.m_strStuId);
				m_list.SetItemText(0, 1, studentDlg.m_strStuName);
				m_list.SetItemText(0, 2, studentDlg.m_strClassId);
			}
			else
			{
				AfxMessageBox("添加失败");
			}
		}
		break;
	}
	case 1:
	{
		CCourseDlg courseDlg;
		if (IDOK == courseDlg.DoModal())
		{
			CString sql;
			sql.Format("INSERT INTO t_course (course_id, course_name) VALUES ('%s', '%s')",
				courseDlg.m_strCourseId, courseDlg.m_strCourseName);
			SendSql(sql);

			int nSuccess = 0;
			m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
			if (-1 != nSuccess)
			{
				// 插入行
				m_list.InsertItem(0, NULL);
				m_list.SetItemText(0, 0, courseDlg.m_strCourseId);
				m_list.SetItemText(0, 1, courseDlg.m_strCourseName);
			}
			else
			{
				AfxMessageBox("添加失败");
			}
		}
		break;
	}
	case 2:
	{
		CClassDlg classDlg;
		if (IDOK == classDlg.DoModal())
		{
			CString sql;
			sql.Format("INSERT INTO t_class (class_id, class_name) VALUES ('%s', '%s')",
				classDlg.m_strClassId, classDlg.m_strClassName);
			SendSql(sql);

			int nSuccess = 0;
			m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
			if (-1 != nSuccess)
			{
				// 插入行
				m_list.InsertItem(0, NULL);
				m_list.SetItemText(0, 0, classDlg.m_strClassId);
				m_list.SetItemText(0, 1, classDlg.m_strClassName);
			}
			else
			{
				AfxMessageBox("添加失败");
			}
		}
		break;
	}
	case 3:
	{
		CSelectDlg selectDlg;
		if (IDOK == selectDlg.DoModal())
		{
			CString sql;
			sql.Format("INSERT INTO t_select (stu_id, course_id, score) VALUES ('%s', '%s', '%s')",
				selectDlg.m_strStuId, selectDlg.m_strCourseId, selectDlg.m_strScore);
			SendSql(sql);

			int nSuccess = 0;
			m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
			if (-1 != nSuccess)
			{
				// 插入行
				m_list.InsertItem(0, NULL);
				m_list.SetItemText(0, 0, selectDlg.m_strStuId);
				m_list.SetItemText(0, 1, selectDlg.m_strCourseId);
				m_list.SetItemText(0, 2, selectDlg.m_strScore);
			}
			else
			{
				AfxMessageBox("添加失败");
			}
		}
		break;
	}
	}
}


void CSchoolSystemDlg::OnBnClickedButtonDelete()
{
	int nCurSelRow = m_list.GetSelectionMark();
	if (-1 == nCurSelRow)
	{
		AfxMessageBox("请选择一行进行删除");
		return;
	}
	CString strId = m_list.GetItemText(nCurSelRow, 0);

	CString sql;
	switch (m_nCurTabSel)
	{
	case 0:
		sql.Format("DELETE FROM t_student WHERE stu_id = '%s'", strId);
		break;
	case 1:
		sql.Format("DELETE FROM t_course WHERE course_id = '%s'", strId);
		break;
	case 2:
		sql.Format("DELETE FROM t_class WHERE class_id = '%s'", strId);
		break;
	case 3:
	{
		CString strCourseId = m_list.GetItemText(nCurSelRow, 1);
		sql.Format("DELETE FROM t_select WHERE stu_id = '%s' AND course_id = '%s'", strId, strCourseId);
		break;
	}
	}
	SendSql(sql);

	int nSuccess = 0;
	m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
	if (-1 != nSuccess)
	{
		// 删除行
		m_list.DeleteItem(nCurSelRow);
	}
	else
	{
		AfxMessageBox("删除失败");
	}
}


void CSchoolSystemDlg::OnBnClickedButtonUpdate()
{
	int nCurSelRow = m_list.GetSelectionMark();
	if (-1 == nCurSelRow)
	{
		AfxMessageBox("请选择一行进行修改");
		return;
	}
	CString strId = m_list.GetItemText(nCurSelRow, 0);

	switch (m_nCurTabSel)
	{
	case 0:
	{
		CStudentDlg studentDlg;
		studentDlg.m_strStuId = m_list.GetItemText(nCurSelRow, 0);
		studentDlg.m_strStuName = m_list.GetItemText(nCurSelRow, 1);
		studentDlg.m_strClassId = m_list.GetItemText(nCurSelRow, 2);

		if (IDOK == studentDlg.DoModal())
		{
			CString sql;
			sql.Format("UPDATE t_student SET stu_id = '%s', stu_name = '%s', class_id = '%s' WHERE (stu_id = '%s');"
				, studentDlg.m_strStuId, studentDlg.m_strStuName, studentDlg.m_strClassId, strId);
			SendSql(sql);

			int nSuccess = 0;
			m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
			if (-1 != nSuccess)
			{
				m_list.SetItemText(nCurSelRow, 0, studentDlg.m_strStuId);
				m_list.SetItemText(nCurSelRow, 1, studentDlg.m_strStuName);
				m_list.SetItemText(nCurSelRow, 2, studentDlg.m_strClassId);
			}
			else
			{
				AfxMessageBox("修改失败");
			}
		}
		break;
	}
	case 1:
	{
		CCourseDlg courseDlg;
		courseDlg.m_strCourseId = m_list.GetItemText(nCurSelRow, 0);
		courseDlg.m_strCourseName = m_list.GetItemText(nCurSelRow, 1);

		if (IDOK == courseDlg.DoModal())
		{
			CString sql;
			sql.Format("UPDATE t_course SET course_id = '%s', course_name = '%s' WHERE (course_id = '%s');"
				, courseDlg.m_strCourseId, courseDlg.m_strCourseName, strId);
			SendSql(sql);

			int nSuccess = 0;
			m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
			if (-1 != nSuccess)
			{
				m_list.SetItemText(nCurSelRow, 0, courseDlg.m_strCourseId);
				m_list.SetItemText(nCurSelRow, 1, courseDlg.m_strCourseName);
			}
			else
			{
				AfxMessageBox("修改失败");
			}
		}
		break;
	}
	case 2:
	{
		CClassDlg classDlg;
		classDlg.m_strClassId = m_list.GetItemText(nCurSelRow, 0);
		classDlg.m_strClassName = m_list.GetItemText(nCurSelRow, 1);

		if (IDOK == classDlg.DoModal())
		{
			CString sql;
			sql.Format("UPDATE t_class SET class_id = '%s', class_name = '%s' WHERE (class_id = '%s');"
				, classDlg.m_strClassId, classDlg.m_strClassName, strId);
			SendSql(sql);

			int nSuccess = 0;
			m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
			if (-1 != nSuccess)
			{
				m_list.SetItemText(nCurSelRow, 0, classDlg.m_strClassId);
				m_list.SetItemText(nCurSelRow, 1, classDlg.m_strClassName);
			}
			else
			{
				AfxMessageBox("修改失败");
			}
		}
		break;
	}
	case 3:
	{
		CSelectDlg selectDlg;
		selectDlg.m_strStuId = m_list.GetItemText(nCurSelRow, 0);
		selectDlg.m_strCourseId = m_list.GetItemText(nCurSelRow, 1);
		selectDlg.m_strScore = m_list.GetItemText(nCurSelRow, 2);

		if (IDOK == selectDlg.DoModal())
		{
			CString sql;
			sql.Format("UPDATE t_select SET score = '%s' WHERE (stu_id = '%s') and (course_id = '%s');"
				, selectDlg.m_strScore, selectDlg.m_strStuId, selectDlg.m_strCourseId);
			SendSql(sql);

			int nSuccess = 0;
			m_tcpClient.Recv((char*)&nSuccess, sizeof(nSuccess));
			if (-1 != nSuccess)
			{
				m_list.SetItemText(nCurSelRow, 2, selectDlg.m_strScore);
			}
			else
			{
				AfxMessageBox("修改失败");
			}
		}
		break;
	}
	}
}


void CSchoolSystemDlg::OnBnClickedButtonSelect()
{
	switch (m_nCurTabSel)
	{
	case 0:
	{
		CStudentDlg studentDlg;

		if (IDOK == studentDlg.DoModal())
		{
			CString sql = "SELECT * FROM t_student WHERE true";

			if (!studentDlg.m_strStuId.IsEmpty())
			{
				sql += " AND stu_id = '" + studentDlg.m_strStuId + "'";
			}
			if (!studentDlg.m_strStuName.IsEmpty())
			{
				sql += " AND stu_name = '" + studentDlg.m_strStuName + "'";
			}
			if (!studentDlg.m_strClassId.IsEmpty())
			{
				sql += " AND class_id = '" + studentDlg.m_strClassId + "'";
			}

			SendSql(sql);
			ShowList();
		}
		break;
	}
	case 1:
	{
		CCourseDlg courseDlg;

		if (IDOK == courseDlg.DoModal())
		{
			CString sql = "SELECT * FROM t_course WHERE true";

			if (!courseDlg.m_strCourseId.IsEmpty())
			{
				sql += " AND course_id = '" + courseDlg.m_strCourseId + "'";
			}
			if (!courseDlg.m_strCourseName.IsEmpty())
			{
				sql += " AND course_name = '" + courseDlg.m_strCourseName + "'";
			}

			SendSql(sql);
			ShowList();
		}
		break;
	}
	case 2:
	{
		CClassDlg classDlg;

		if (IDOK == classDlg.DoModal())
		{
			CString sql = "SELECT * FROM t_class WHERE true";

			if (!classDlg.m_strClassId.IsEmpty())
			{
				sql += " AND class_id = '" + classDlg.m_strClassId + "'";
			}
			if (!classDlg.m_strClassName.IsEmpty())
			{
				sql += " AND class_name = '" + classDlg.m_strClassName + "'";
			}

			SendSql(sql);
			ShowList();
		}
		break;
	}
	case 3:
	{
		CSelectDlg selectDlg;

		if (IDOK == selectDlg.DoModal())
		{
			CString sql = "SELECT * FROM t_select WHERE true";

			if (!selectDlg.m_strStuId.IsEmpty())
			{
				sql += " AND stu_id = '" + selectDlg.m_strStuId + "'";
			}
			if (!selectDlg.m_strCourseId.IsEmpty())
			{
				sql += " AND course_id = '" + selectDlg.m_strCourseId + "'";
			}
			if (!selectDlg.m_strScore.IsEmpty())
			{
				sql += " AND score = '" + selectDlg.m_strScore + "'";
			}


			SendSql(sql);
			ShowList();
		}
		break;
	}
	}
}


void CSchoolSystemDlg::OnBnClickedButtonUnionSelect()
{
	CString sql = 
		"select A.*, B.course_id, B.course_name, B.score from (select t_student.*, t_class.class_name from t_student left join t_class on t_student.class_id = t_class.class_id) as A "
		"inner join(select t_select.stu_id, t_select.course_id, t_course.course_name, t_select.score from t_select left join t_course on t_select.course_id = t_course.course_id) as B "
		"on A.stu_id = B.stu_id;";
	SendSql(sql);
	ShowList();
}


void CSchoolSystemDlg::OnOK()
{
}
