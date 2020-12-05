#include "CSem.h"

CSem::CSem()
{
    m_sem = CreateSemaphore(NULL, 0, MAXINT, NULL);
}

CSem::~CSem()
{
    CloseHandle(m_sem);
}

void CSem::ReleaseSem()
{
    ReleaseSemaphore(m_sem, 1, NULL);
}

void CSem::WaitForSem()
{
    WaitForSingleObject(m_sem, INFINITE);
}
