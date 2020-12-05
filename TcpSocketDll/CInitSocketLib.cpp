#include "CInitSocketLib.h"

CInitSocketLib::CInitSocketLib()
{
    // 初始化套接字库
    WSADATA wsaData = { 0 };
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        return;
    }
}

CInitSocketLib::~CInitSocketLib()
{
	//反初始化
	WSACleanup();
}

// 静态成员，实例化一个对象，用来调用构造和析构
// 从而实现初始化socket库和反初始化，省略了在其他地方实例化对象
CInitSocketLib CInitSocketLib::m_sock;