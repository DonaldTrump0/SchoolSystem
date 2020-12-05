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

    // ��ʼ���׽���(ʹ��UDPЭ��)
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == m_socket)
    {
        PrintErrMsg("socket init");
        return false;
    }

    // ��ʼ������IP�Ͷ˿�
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(nPort);
    inet_pton(AF_INET, szIp, &serverAddr.sin_addr);

    // �󶨶˿�
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

    // ��ʼ���׽���(ʹ��UDPЭ��)
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == m_socket)
    {
        PrintErrMsg("socket init");
        return false;
    }

    // ���Ϳͻ������Ӱ����������������
    Packet sendPkg(ESTABLISH, conn.m_nNextSendSeq++);
    if (SOCKET_ERROR == sendto(m_socket, (char*)&sendPkg, sizeof(Packet), 0, &addr, sizeof(sockaddr)))
    {
        PrintErrMsg("sendto");
        return 0;
    }
    m_sendList.push_back(SendListNode(clock(), addr, &sendPkg));

    CreateThread(0, 0, SendThreadProc, this, 0, 0);

    // ���շ�������Ӱ�
    Packet recvPkg;
    sockaddr fromaddr = { 0 };
    int nLen = sizeof(fromaddr);
    if (SOCKET_ERROR == recvfrom(m_socket, (char*)&recvPkg, sizeof(recvPkg), 0, &fromaddr, &nLen))
    {
        PrintErrMsg("recvfrom");
        return false;
    }
    conn.m_nNextRecvSeq = recvPkg.m_nSeq + 1;
    // ��մ���������
    m_sendListLock.Lock();
    m_sendList.clear();
    m_sendListLock.UnLock();

    // ����ȷ�ϰ�
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

    // �����ֳɼ�����
    int nPkgCnt = nLen / DATA_LEN;

    // �������������
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
            // �����¼�������İ����ط���ʱ��
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
                // ����ʱ��
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
                // ���յ�һ�����֣���ʼ��
                conn.m_nNextSendSeq = 0;
                conn.m_nNextRecvSeq = packet->m_nSeq + 1;

                // ��Ҫ���͵ĵڶ������ּ������������
                Packet* p = new Packet(ESTABLISH, conn.m_nNextSendSeq++);
                pThis->m_sendListLock.Lock();
                pThis->m_sendList.push_back(SendListNode(0, fromAddr, p));
                pThis->m_sendListLock.UnLock();

                delete packet;
            }
            else
            {
                // ���������֣�����ȷ�ϰ�
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
            // ����ȷ�ϰ�
            Packet ackPkt(ACK, packet->m_nSeq);
            if (SOCKET_ERROR == sendto(pThis->m_socket, (char*)&ackPkt, sizeof(ackPkt), 0, &fromAddr, sizeof(sockaddr)))
            {
                PrintErrMsg("sendto");
                return 0;
            }

            // �յ�������һ����
            if (packet->m_nSeq == conn.m_nNextRecvSeq)
            {
                // д�뵽������
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

                // �����հ�����������İ�д�뵽������
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

                        // ɾ��������İ�
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
            // ���յ�����İ�����˳����뵽�հ�����
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
                    // ������յ�����������
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
