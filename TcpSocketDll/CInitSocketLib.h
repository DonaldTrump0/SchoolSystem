#pragma once
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")

class CInitSocketLib
{
public:
	CInitSocketLib();
	~CInitSocketLib();

private:
	static CInitSocketLib m_sock;
};

