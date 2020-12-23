#pragma once
#include <string.h>
#include "../TcpSocketDll/CTcpServer.h"
#include "../MysqlDll/CMysql.h"
#include "CThreadPool.h"
#pragma comment(lib, "../Debug/TcpSocketDll.lib")
#pragma comment(lib, "../Debug/MysqlDll.lib")
using namespace std;

class CServer
{
public:
    CServer();
    void SendResByteAry(MYSQL_RES* res, sockaddr addr);

private:
    bool Init();
    bool CreateTables();

public:
    CMysql m_mysql;
    CTcpServer m_tcpServer;
    CThreadPool m_threadPool;
};

class CSqlTask : public ITask
{
public:
    CSqlTask(CServer* pServer, sockaddr addr, string sql);
    virtual int Execute();
    virtual int Abort();

private:
    CServer* m_pServer;
    sockaddr m_addr;
    string m_sql;
};