#include "CServer.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    CServer server;
    return 0;
}


CServer::CServer()
{
    if (!Init())
    {
        return;
    }

    while (true)
    {
        sockaddr addr = m_tcpSocket.Select();

        int nLen = 0;
        m_tcpSocket.Recv(addr, (char*)&nLen, sizeof(nLen));

        char buf[0x1000] = { 0 };
        m_tcpSocket.Recv(addr, buf, nLen);

        m_threadPool.AddTask(new CSqlTask(this, addr, string(buf)));
    }
}

bool CServer::Init()
{
    // 初始化并连接mysql
    if (!m_mysql.Init("localhost", "root", "root"))
    {
        return false;
    }

    // 创建表
    if (!CreateTables())
    {
        return false;
    }

    // 初始化socket等待连接
    if (!m_tcpSocket.Listen("127.0.0.1", 5566))
    {
        return false;
    }

    return true;
}

void CServer::SendResByteAry(MYSQL_RES* res, sockaddr addr)
{
    char buf[0x1000] = { 0 };
    char* p = buf;

    // 遍历字段
    MYSQL_FIELD* fields = mysql_fetch_fields(res);
    for (int i = 0; i < mysql_num_fields(res); i++)
    {
        int nLen = strlen(fields[i].name) + 1;
        memcpy(p, fields[i].name, nLen);
        p += nLen;
    }

    // 发送字段
    int nLen = p - buf;
    m_tcpSocket.Send(addr, (char*)&nLen, sizeof(nLen));
    m_tcpSocket.Send(addr, buf, nLen);

    p = buf;

    // 遍历数据
    MYSQL_ROW row;
    while (NULL != (row = mysql_fetch_row(res)))
    {
        for (int i = 0; i < mysql_num_fields(res); i++)
        {
            int nLen = strlen(row[i]) + 1;
            memcpy(p, row[i], nLen);
            p += nLen;
        }
    }

    // 发送数据
    nLen = p - buf;
    m_tcpSocket.Send(addr, (char*)&nLen, sizeof(nLen));
    m_tcpSocket.Send(addr, buf, nLen);
}

// 如果表不存在，则创建表
bool CServer::CreateTables()
{
    // 创建数据库
    string sql = "CREATE DATABASE IF NOT EXISTS test;";
    if (m_mysql.Query(sql) < 0)
    {
        return false;
    }
    sql = "USE test;";
    if (m_mysql.Query(sql) < 0)
    {
        return false;
    }
    // 创建班级表
    sql = "CREATE TABLE IF NOT EXISTS t_class("
        "class_id varchar(64),"
        "class_name varchar(64) NOT NULL,"
        "PRIMARY KEY(class_id));";
    if (m_mysql.Query(sql) < 0)
    {
        return false;
    }
    // 创建学生表
    sql = "CREATE TABLE IF NOT EXISTS t_student("
        "stu_id varchar(64),"
        "stu_name varchar(64) NOT NULL,"
        "class_id varchar(64) NOT NULL,"
        "PRIMARY KEY(stu_id),"
        "CONSTRAINT FK_CLASSID FOREIGN KEY(class_id) REFERENCES t_class(class_id));";
    if (m_mysql.Query(sql) < 0)
    {
        return false;
    }
    // 创建课程表
    sql = "CREATE TABLE IF NOT EXISTS t_course("
        "course_id varchar(64),"
        "course_name varchar(64) NOT NULL,"
        "PRIMARY KEY(course_id));";
    if (m_mysql.Query(sql) < 0)
    {
        return false;
    }
    // 创建选课表
    sql = "CREATE TABLE IF NOT EXISTS t_select("
        "stu_id varchar(64),"
        "course_id varchar(64),"
        "score float DEFAULT NULL,"
        "PRIMARY KEY(stu_id, course_id),"
        "CONSTRAINT FK_COURSEID FOREIGN KEY(course_id) REFERENCES t_course(course_id),"
        "CONSTRAINT FK_STUID FOREIGN KEY(stu_id) REFERENCES t_student(stu_id));";
    if (m_mysql.Query(sql) < 0)
    {
        return false;
    }

    return true;
}


CSqlTask::CSqlTask(CServer* pServer, sockaddr addr, string sql)
    : m_pServer(pServer)
    , m_addr(addr)
    , m_sql(sql)
{
}

int CSqlTask::Execute()
{
    MYSQL_RES* res = NULL;
    int nRet = m_pServer->m_mysql.Query(m_sql, res);

    if (NULL == res)
    {
        // 发送insert, delete, update影响的行数
        m_pServer->m_tcpSocket.Send(m_addr, (char*)&nRet, sizeof(nRet));
    }
    else
    {
        // 发送select数据
        m_pServer->SendResByteAry(res, m_addr);
    }

    m_pServer->m_tcpSocket.UnSelect(m_addr);

    return 0;
}

int CSqlTask::Abort()
{
    return 0;
}
