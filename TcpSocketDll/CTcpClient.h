#pragma once
#include "CTcpSocket.h"

class TCPSOCKETDLL_API CTcpClient : public CTcpSocket
{
public:
    bool Connect(const char* pIp, int nPort);    // 客户端连接
    int Send(const char* pBuf, int nLen);        // 发送
    int Recv(char* pBuf, int nBufLen);           // 接收

private:
    sockaddr m_serverAddr;
};