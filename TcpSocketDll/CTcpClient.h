#pragma once
#include "CTcpSocket.h"

class TCPSOCKETDLL_API CTcpClient : public CTcpSocket
{
public:
    bool Connect(const char* pIp, int nPort);    // �ͻ�������
    int Send(const char* pBuf, int nLen);        // ����
    int Recv(char* pBuf, int nBufLen);           // ����

private:
    sockaddr m_serverAddr;
};