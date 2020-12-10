#ifdef TCPSOCKETDLL_EXPORTS
#define TCPSOCKETDLL_API __declspec(dllexport)
#else
#define TCPSOCKETDLL_API __declspec(dllimport)
#endif

#pragma once
#include <list>
#include <map>
#include <ws2tcpip.h>
#include "CByteStream.h"
#include "CLock.h"
#include "CSem.h"
using namespace std;

// 一个包携带的最大字节数
#define DATA_LEN 1024

class TCPSOCKETDLL_API CTcpSocket
{
public:
    bool Listen(const char* szIp, short nPort); // 监听，服务端调用
    //sockaddr Accept();                          // 接受连接，服务端调用
    bool Connect(sockaddr addr);                // 发起连接，客户端调用
    sockaddr Select();
    void UnSelect(sockaddr addr);
    int Send(sockaddr addr, const char* pBuf, int nLen);
    int Recv(sockaddr addr, char* pBuf, int nBufLen);

    bool Close();

private:
    static DWORD WINAPI SendThreadProc(LPVOID lpParam); // 发送数据的线程函数
    static DWORD WINAPI RecvThreadProc(LPVOID lpParam); // 接收数据的线程函数

private:
    enum PackageType
    {
        ESTABLISH,  // 三次握手
        DATA,       // 数据包
        ACK,        // 确认包
        FIN         // 四次挥手
    };

    struct Packet
    {
        int m_nType;
        int m_nSeq;

        Packet() : m_nType(0), m_nSeq(0) {}
        Packet(int nType, int nSeq) : m_nType(nType), m_nSeq(nSeq) {}
    };

    struct DataPacket : public Packet
    {
        int m_nLen;
        char m_data[DATA_LEN];

        DataPacket() : m_nLen(0)
        {
            memset(m_data, 0, sizeof(m_data));
        }

        DataPacket(int nType, int nSeq, int nLen, const char* pData)
            : Packet(nType, nSeq), m_nLen(nLen)
        {
            memcpy(m_data, pData, nLen);
        }

        int Size() { return sizeof(m_nType) + sizeof(m_nSeq) + sizeof(m_nLen) + m_nLen; }
    };

    struct SendListNode
    {
        time_t m_time;
        sockaddr m_addr;
        Packet* m_packet;

        SendListNode(time_t time, sockaddr addr, Packet* packet)
            : m_time(time), m_addr(addr), m_packet(packet) {}
    };

    struct Conn
    {
        int m_nNextSendSeq;             // 下一个发送的包序号
        int m_nNextRecvSeq;             // 下一个接收的包序号
        list<DataPacket*> m_recvList;   // 收包链表 
        CByteStream m_byteStream;       // 待读取缓冲区
        CLock m_byteStreamLock;
        int m_nStatus;                  // 0表示没数据，1表示有数据，2表示数据正在被接收
        CLock m_statusLock;

        Conn() : m_nNextSendSeq(0), m_nNextRecvSeq(0), m_nStatus(0) {}
    };

    // 用于m_connectMap
    struct Compare
    {
        bool operator()(const sockaddr& left, const sockaddr& right) const
        {
            sockaddr_in* pLeft = (sockaddr_in*)&left;
            sockaddr_in* pRight= (sockaddr_in*)&right;

            if (pLeft->sin_addr.S_un.S_addr == pRight->sin_addr.S_un.S_addr)
            {
                return pLeft->sin_port < pRight->sin_port;
            }

            return pLeft->sin_addr.S_un.S_addr < pRight->sin_addr.S_un.S_addr;
        }
    };


private:
    SOCKET m_socket;
    bool m_bServer;                             // 是否为服务端

    map<sockaddr, Conn, Compare> m_connectMap;  // 客户端连接信息

    list<SendListNode> m_sendList;              // 待发包链表
    CLock m_sendListLock;                       // 待发包链表锁

    list<sockaddr> m_connectList;               // 待获取的连接
    CLock m_connectListLock;                    // 待获取的连接锁
    CSem m_connectSem;                          // 待获取的连接数
};