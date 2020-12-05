#include "CTcpSocket.h"
#include <stdio.h>
#include <time.h>

void PrintErrMsg(const char* szPreMsg)
{
    char errMsg[256] = { 0 };
    char buf[256] = { 0 };
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, errMsg, sizeof(errMsg), 0);
    sprintf(buf, "%s: %s\r\n", szPreMsg, errMsg);
    OutputDebugString(buf);
}

bool CTcpSocket::Listen(const char* szIp, short nPort)
{
    m_bServer = true;

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
    inet_pton(AF_INET, szIp, &serverAddr.sin_addr);

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

sockaddr CTcpSocket::Accept()
{
    m_connectSem.WaitForSem();

    m_connectListLock.Lock();
    sockaddr addr = m_connectList.front();
    m_connectList.erase(m_connectList.begin());
    m_connectListLock.UnLock();

    return addr;
}

bool CTcpSocket::Connect(sockaddr addr)
{
    m_bServer = false;
    Conn& conn = m_connectMap[addr];
    conn.m_nNextSendSeq = 0;

    // 初始化套接字(使用UDP协议)
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == m_socket)
    {
        PrintErrMsg("socket init");
        return false;
    }

    // 发送客户端连接包，加入待发包链表
    Packet sendPkg(ESTABLISH, conn.m_nNextSendSeq++);
    if (SOCKET_ERROR == sendto(m_socket, (char*)&sendPkg, sizeof(Packet), 0, &addr, sizeof(sockaddr)))
    {
        PrintErrMsg("sendto");
        return 0;
    }
    m_sendList.push_back(SendListNode(clock(), addr, &sendPkg));

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
    if (SOCKET_ERROR == sendto(m_socket, (char*)&AckPkg, sizeof(AckPkg), 0, &addr, sizeof(addr)))
    {
        PrintErrMsg("sendto");
        return false;
    }

    CreateThread(0, 0, RecvThreadProc, this, 0, 0);

    return true;
}

sockaddr CTcpSocket::Select()
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

void CTcpSocket::UnSelect(sockaddr addr)
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

int CTcpSocket::Send(sockaddr addr, const char* pBuf, int nLen)
{
    Conn& conn = m_connectMap[addr];

    // 计算拆分成几个包
    int nPkgCnt = nLen / DATA_LEN;

    // 加入待发包链表
    m_sendListLock.Lock();
    for (int i = 0; i < nPkgCnt; i++)
    {
        DataPacket* pkt = new DataPacket(DATA, conn.m_nNextSendSeq++, DATA_LEN, pBuf);
        m_sendList.push_back(SendListNode(0, addr, pkt));
        pBuf += DATA_LEN;
    }
    if (nLen % DATA_LEN != 0)
    {
        DataPacket* pkt = new DataPacket(DATA, conn.m_nNextSendSeq++, nLen % DATA_LEN, pBuf);
        m_sendList.push_back(SendListNode(0, addr, pkt));
    }
    m_sendListLock.UnLock();

    return nLen;
}

int CTcpSocket::Recv(sockaddr addr, char* pBuf, int nBufLen)
{
    Conn& conn = m_connectMap[addr];
    int nStreamSize = 0;

    while (0 == (nStreamSize = conn.m_byteStream.GetSize()))
    {
        Sleep(20);
    }

    int nReadLen = min(nStreamSize, nBufLen);
    conn.m_byteStreamLock.Lock();
    conn.m_byteStream.Read(pBuf, nReadLen);
    conn.m_byteStreamLock.UnLock();

    return nReadLen;
}

DWORD WINAPI CTcpSocket::SendThreadProc(LPVOID lpParam)
{
    CTcpSocket* pThis = (CTcpSocket*)lpParam;

    while (true)
    {
        pThis->m_sendListLock.Lock();
        for (SendListNode& n : pThis->m_sendList)
        {
            // 发送新加入链表的包，重发超时包
            if ((n.m_time == 0) || (clock() - n.m_time >= 200))
            {
                switch (n.m_packet->m_nType)
                {
                case ESTABLISH:
                {
                    if (SOCKET_ERROR == sendto(pThis->m_socket, (char*)n.m_packet, 
                        sizeof(Packet), 0, &n.m_addr, sizeof(sockaddr)))
                    {
                        PrintErrMsg("sendto");
                        return 0;
                    }
                    break;
                }
                case DATA:
                {
                    if (SOCKET_ERROR == sendto(pThis->m_socket, (char*)n.m_packet, 
                        ((DataPacket*)n.m_packet)->Size(), 0, &n.m_addr, sizeof(sockaddr)))
                    {
                        PrintErrMsg("sendto");
                        return 0;
                    }
                    break;
                }
                default:
                    break;
                }
                // 更新时间
                n.m_time = clock();
            }
        }
        pThis->m_sendListLock.UnLock();

        Sleep(20);
    }

    return 0;
}

DWORD WINAPI CTcpSocket::RecvThreadProc(LPVOID lpParam)
{
    CTcpSocket* pThis = (CTcpSocket*)lpParam;

    while (true)
    {
        DataPacket* packet = new DataPacket();
        sockaddr fromAddr = { 0 };
        int nFromLen = sizeof(fromAddr);
        if (SOCKET_ERROR == recvfrom(pThis->m_socket, (char*)packet, sizeof(DataPacket), 0, &fromAddr, &nFromLen))
        {
            PrintErrMsg("recvfrom");
            return 0;
        }

        Conn& conn = pThis->m_connectMap[fromAddr];

        switch (packet->m_nType)
        {
        case ESTABLISH:
        {
            if (pThis->m_bServer)
            {
                // 接收第一次握手，初始化
                conn.m_nNextSendSeq = 0;
                conn.m_nNextRecvSeq = packet->m_nSeq + 1;

                // 将要发送的第二次握手加入待发送链表
                Packet* p = new Packet(ESTABLISH, conn.m_nNextSendSeq++);
                pThis->m_sendListLock.Lock();
                pThis->m_sendList.push_back(SendListNode(0, fromAddr, p));
                pThis->m_sendListLock.UnLock();

                delete packet;
            }
            else
            {
                // 第三次握手，发送确认包
                Packet ackPkt(ACK, packet->m_nSeq);
                if (SOCKET_ERROR == sendto(pThis->m_socket, (char*)&ackPkt, sizeof(ackPkt), 0, &fromAddr, sizeof(sockaddr)))
                {
                    PrintErrMsg("sendto");
                    return 0;
                }

                delete packet;
            }
            break;
        }
        case DATA:
        {
            // 发送确认包
            Packet ackPkt(ACK, packet->m_nSeq);
            if (SOCKET_ERROR == sendto(pThis->m_socket, (char*)&ackPkt, sizeof(ackPkt), 0, &fromAddr, sizeof(sockaddr)))
            {
                PrintErrMsg("sendto");
                return 0;
            }

            // 收到的是下一个包
            if (packet->m_nSeq == conn.m_nNextRecvSeq)
            {
                // 写入到数据流
                conn.m_byteStreamLock.Lock();
                conn.m_byteStream.Write(packet->m_data, packet->m_nLen);
                conn.m_byteStreamLock.UnLock();

                conn.m_statusLock.Lock();
                if (0 == conn.m_nStatus)
                {
                    conn.m_nStatus = 1;
                }
                conn.m_statusLock.UnLock();

                conn.m_nNextRecvSeq++;
                delete packet;

                // 遍历收包链表，将后面的包写入到数据流
                auto it = conn.m_recvList.begin();
                while (it != conn.m_recvList.end())
                {
                    if ((*it)->m_nSeq == conn.m_nNextRecvSeq)
                    {
                        conn.m_byteStreamLock.Lock();
                        conn.m_byteStream.Write((*it)->m_data, (*it)->m_nLen);
                        conn.m_byteStreamLock.UnLock();
                        conn.m_nNextRecvSeq++;

                        conn.m_statusLock.Lock();
                        if (0 == conn.m_nStatus)
                        {
                            conn.m_nStatus = 1;
                        }
                        conn.m_statusLock.UnLock();

                        // 删除处理过的包
                        auto t = it;
                        it++;
                        delete *t;
                        conn.m_recvList.erase(t);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            // 先收到后面的包，则按顺序插入到收包链表
            else if (packet->m_nSeq > conn.m_nNextRecvSeq)
            {
                auto it = conn.m_recvList.begin();
                for (; it != conn.m_recvList.end(); it++)
                {
                    if ((*it)->m_nSeq > packet->m_nSeq)
                    {
                        break;
                    }
                }
                conn.m_recvList.insert(it, packet);
            }

            break;
        }
        case ACK:
        {
            pThis->m_sendListLock.Lock();
            for (auto it = pThis->m_sendList.begin(); it != pThis->m_sendList.end(); it++)
            {
                if (it->m_packet->m_nSeq == packet->m_nSeq)
                {
                    // 服务端收到第三次握手
                    if (packet->m_nSeq == 0)
                    {
                        pThis->m_connectListLock.Lock();
                        pThis->m_connectList.push_back(fromAddr);
                        pThis->m_connectListLock.UnLock();
                        pThis->m_connectSem.ReleaseSem();
                    }

                    delete it->m_packet;
                    pThis->m_sendList.erase(it);
                    break;
                }
            }
            pThis->m_sendListLock.UnLock();

            delete packet;
            break;
        }
        case FIN:
        {

            delete packet;
            break;
        }
        }
    }

    return 0;
}
