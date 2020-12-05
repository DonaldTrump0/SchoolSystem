#pragma once
#include <Windows.h>
#include <queue>
using namespace std;

class ITask
{
public:
    virtual int Execute() = 0;
    virtual int Abort() = 0;
};

class CThreadPool
{
public:
    CThreadPool();
    CThreadPool(unsigned int nMaxThreads);
    ~CThreadPool();
    int AddTask(ITask* pTask);
    static DWORD WINAPI ThreadProc(LPVOID lpThreadParameter);

private:
    unsigned int m_nMaxThreads;     // ����߳���
    unsigned int m_nCurThreads;     // ��ǰ�߳���
    unsigned int m_nIdleThreads;    // �����߳���
    HANDLE* m_pThreadHandles;       // �߳̾������
    queue<ITask*> m_taskQueue;      // �������
    HANDLE m_hSem[2];               // 0�����̳߳������ź�event��1���������ź���
    CRITICAL_SECTION m_cs;          // ��������ٽ���
};

