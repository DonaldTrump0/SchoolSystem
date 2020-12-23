#include "CTcpServer.h"

bool CTcpServer::Listen(const char* pIp, int nPort)
{
    // 初始化套接字(使用UDP协议)
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == m_socket)
    {
        PrintErrMsg("socket init");
        return false;
    }

    // 初始化本机IP和端口
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(nPort);
    inet_pton(AF_INET, pIp, &serverAddr.sin_addr);

    // 绑定端口
    if (SOCKET_ERROR == bind(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)))
    {
        PrintErrMsg("bind");
        return false;
    }

    CreateThread(0, 0, SendThreadProc, this, 0, 0);
    CreateThread(0, 0, RecvThreadProc, this, 0, 0);

    return true;
}

sockaddr CTcpServer::Select()
{
    while (true)
    {
        for (auto& p : m_connectMap)
        {
            if (1 == p.second.m_nStatus)
            {
                p.second.m_nStatus = 2;
                return p.first;
            }
        }
        Sleep(20);
    }
}

void CTcpServer::UnSelect(sockaddr addr)
{
    Conn& conn = m_connectMap[addr];

    conn.m_statusLock.Lock();
    if (0 != conn.m_byteStream.GetSize())
    {
        conn.m_nStatus = 1;
    }
    else
    {
        conn.m_nStatus = 0;
    }
    conn.m_statusLock.UnLock();
}