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

// һ����Я��������ֽ���
#define DATA_LEN 1024

class TCPSOCKETDLL_API CTcpSocket
{
public:
    bool Listen(const char* szIp, short nPort); // ����������˵���
    //sockaddr Accept();                          // �������ӣ�����˵���
    bool Connect(sockaddr addr);                // �������ӣ��ͻ��˵���
    sockaddr Select();
    void UnSelect(sockaddr addr);
    int Send(sockaddr addr, const char* pBuf, int nLen);
    int Recv(sockaddr addr, char* pBuf, int nBufLen);

    bool Close();

private:
    static DWORD WINAPI SendThreadProc(LPVOID lpParam); // �������ݵ��̺߳���
    static DWORD WINAPI RecvThreadProc(LPVOID lpParam); // �������ݵ��̺߳���

private:
    enum PackageType
    {
        ESTABLISH,  // ��������
        DATA,       // ���ݰ�
        ACK,        // ȷ�ϰ�
        FIN         // �Ĵλ���
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
        int m_nNextSendSeq;             // ��һ�����͵İ����
        int m_nNextRecvSeq;             // ��һ�����յİ����
        list<DataPacket*> m_recvList;   // �հ����� 
        CByteStream m_byteStream;       // ����ȡ������
        CLock m_byteStreamLock;
        int m_nStatus;                  // 0��ʾû���ݣ�1��ʾ�����ݣ�2��ʾ�������ڱ�����
        CLock m_statusLock;

        Conn() : m_nNextSendSeq(0), m_nNextRecvSeq(0), m_nStatus(0) {}
    };

    // ����m_connectMap
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
    bool m_bServer;                             // �Ƿ�Ϊ�����

    map<sockaddr, Conn, Compare> m_connectMap;  // �ͻ���������Ϣ

    list<SendListNode> m_sendList;              // ����������
    CLock m_sendListLock;                       // ������������

    list<sockaddr> m_connectList;               // ����ȡ������
    CLock m_connectListLock;                    // ����ȡ��������
    CSem m_connectSem;                          // ����ȡ��������
};