#pragma once
#include <Windows.h>

class CSem
{
public:
    CSem();
    ~CSem();
    void ReleaseSem();
    void WaitForSem();

private:
    HANDLE m_sem;
};