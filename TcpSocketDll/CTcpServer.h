#pragma once
#include "CTcpSocket.h"

class TCPSOCKETDLL_API CTcpServer : public CTcpSocket
{
public:
    bool Listen(const char* pIp, int nPort);    // ����˿�������
    sockaddr Select();
    void UnSelect(sockaddr addr);

private:

};