#pragma once
#include <vector>
using namespace std;

// �򵥵��ֽ���������
class CByteStream
{
public:
    // д�뻺����, �����ڻ�����ĩβ
    void Write(const char* pBuf, int nSize);
    // �ӻ�������ȡָ���ֽ���, ��ȡ�������Զ��ӻ�����ɾ��
    void Read(char* pBuf, int nSize);
    // �ӻ�������ȡָ���ֽ���, ��ȡ�����ݲ���ӻ�����ɾ��
    void Peek(char* pBuf, int nSize);
    // ��ȡ�����������ݵĴ�С
    int GetSize() const;

private:
    vector<char> m_vtBuf;
};

