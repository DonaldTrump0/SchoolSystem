#include "CTcpClient.h"
#include <time.h>

bool CTcpClient::Connect(const char* pIp, int nPort)
{
    // 初始化远程主机IP和端口
    sockaddr_in* pServerAddr = (sockaddr_in*)&m_serverAddr;
    pServerAddr->sin_family = AF_INET;
    pServerAddr->sin_port = htons(nPort);
    inet_pton(AF_INET, pIp, &pServerAddr->sin_addr);

    Conn& conn = m_connectMap[m_serverAddr];
    conn.m_nNextSendSeq = 0;

    // 初始化套接字(使用UDP协议)
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == m_socket)
    {
        PrintErrMsg("socket init");
        return false;
    }

    // 发送客户端连接包，加入待发包链表
    Packet sendPkg(ESTABLISH_1, conn.m_nNextSendSeq++);
    if (SOCKET_ERROR == sendto(m_socket, (char*)&sendPkg, sizeof(Packet), 0, &m_serverAddr, sizeof(sockaddr)))
    {
        PrintErrMsg("sendto");
        return 0;
    }
    m_sendList.push_back(SendListNode(clock(), m_serverAddr, &sendPkg));

    CreateThread(0, 0, SendThreadProc, this, 0, 0);

    // 接收服务端连接包
    Packet recvPkg;
    sockaddr fromaddr = { 0 };
    int nLen = sizeof(fromaddr);
    if (SOCKET_ERROR == recvfrom(m_socket, (char*)&recvPkg, sizeof(recvPkg), 0, &fromaddr, &nLen))
    {
        PrintErrMsg("recvfrom");
        return false;
    }
    conn.m_nNextRecvSeq = recvPkg.m_nSeq + 1;
    // 清空待发送链表
    m_sendListLock.Lock();
    m_sendList.clear();
    m_sendListLock.UnLock();

    // 发送确认包
    Packet AckPkg(ACK, recvPkg.m_nSeq);
    if (SOCKET_ERROR == sendto(m_socket, (char*)&AckPkg, sizeof(AckPkg), 0, &m_serverAddr, sizeof(sockaddr)))
    {
        PrintErrMsg("sendto");
        return false;
    }

    CreateThread(0, 0, RecvThreadProc, this, 0, 0);

    return true;
}

int CTcpClient::Send(const char* pBuf, int nLen)
{
    return CTcpSocket::Send(m_serverAddr, pBuf, nLen);
}

int CTcpClient::Recv(char* pBuf, int nBufLen)
{
    return CTcpSocket::Recv(m_serverAddr, pBuf, nBufLen);
}
