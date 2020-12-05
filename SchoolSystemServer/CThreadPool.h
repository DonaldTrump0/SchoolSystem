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
    unsigned int m_nMaxThreads;     // 最大线程数
    unsigned int m_nCurThreads;     // 当前线程数
    unsigned int m_nIdleThreads;    // 空闲线程数
    HANDLE* m_pThreadHandles;       // 线程句柄数组
    queue<ITask*> m_taskQueue;      // 任务队列
    HANDLE m_hSem[2];               // 0代表线程池销毁信号event，1代表任务信号量
    CRITICAL_SECTION m_cs;          // 任务队列临界区
};

