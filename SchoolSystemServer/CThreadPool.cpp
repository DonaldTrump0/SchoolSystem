#include "CThreadPool.h"

CThreadPool::CThreadPool()
    : m_nCurThreads(0)
    , m_nIdleThreads(0)
{
    // 获取cpu线程数
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    m_nMaxThreads = sysinfo.dwNumberOfProcessors;

    m_pThreadHandles = new HANDLE[m_nMaxThreads];
    ZeroMemory(m_pThreadHandles, m_nMaxThreads * sizeof(HANDLE));

    m_hSem[0] = CreateEvent(NULL, TRUE, 0, NULL);
    m_hSem[1] = CreateSemaphore(NULL, 0, m_nMaxThreads * 1000, NULL);
    InitializeCriticalSection(&m_cs);
}

CThreadPool::CThreadPool(unsigned int nMaxThreads)
    : CThreadPool()
{
    if (nMaxThreads >= 1 && nMaxThreads <= m_nMaxThreads)
    {
        m_nMaxThreads = nMaxThreads;
    }
}

CThreadPool::~CThreadPool()
{
    SetEvent(m_hSem[0]);

    WaitForMultipleObjects(m_nCurThreads, m_pThreadHandles, TRUE, INFINITE);

    for (int i = 0; i < m_nCurThreads; i++)
    {
        CloseHandle(m_pThreadHandles[i]);
    }

    while (!m_taskQueue.empty())
    {
        ITask* pTask = m_taskQueue.front();
        m_taskQueue.pop();
        pTask->Abort();
        delete pTask;
    }

    delete[] m_pThreadHandles;

    CloseHandle(m_hSem[0]);
    CloseHandle(m_hSem[1]);
    DeleteCriticalSection(&m_cs);
}

int CThreadPool::AddTask(ITask* pTask)
{
    EnterCriticalSection(&m_cs);
    // 信号量达到上限，则丢弃任务
    if (ReleaseSemaphore(m_hSem[1], 1, NULL))
    {
        m_taskQueue.push(pTask);
    }
    LeaveCriticalSection(&m_cs);

    if (m_nIdleThreads == 0 && m_nCurThreads < m_nMaxThreads)
    {
        m_pThreadHandles[m_nCurThreads++] = CreateThread(0, 0, ThreadProc, this, 0, 0);
    }

    return 0;
}

DWORD WINAPI CThreadPool::ThreadProc(LPVOID lpThreadParameter)
{
    CThreadPool* pThreadPool = (CThreadPool*)lpThreadParameter;

    while (true)
    {
        InterlockedIncrement(&pThreadPool->m_nIdleThreads);

        DWORD dwIndex = WaitForMultipleObjects(2, pThreadPool->m_hSem, FALSE, INFINITE);
        if (dwIndex == 0)
        {
            return 0;
        }

        InterlockedDecrement(&pThreadPool->m_nIdleThreads);

        EnterCriticalSection(&pThreadPool->m_cs);
        ITask* pTask = pThreadPool->m_taskQueue.front();
        pThreadPool->m_taskQueue.pop();
        LeaveCriticalSection(&pThreadPool->m_cs);

        pTask->Execute();
        delete pTask;
    }

    return 0;
}
