#include "CInitSocketLib.h"

CInitSocketLib::CInitSocketLib()
{
    // ��ʼ���׽��ֿ�
    WSADATA wsaData = { 0 };
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        return;
    }
}

CInitSocketLib::~CInitSocketLib()
{
	//����ʼ��
	WSACleanup();
}

// ��̬��Ա��ʵ����һ�������������ù��������
// �Ӷ�ʵ�ֳ�ʼ��socket��ͷ���ʼ����ʡ�����������ط�ʵ��������
CInitSocketLib CInitSocketLib::m_sock;