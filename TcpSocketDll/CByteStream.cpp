#include "CByteStream.h"

void CByteStream::Write(const char* pBuf, int nSize)
{
    for (int i = 0; i < nSize; i++)
    {
        m_vtBuf.push_back(pBuf[i]);
    }
}

void CByteStream::Read(char* pBuf, int nSize)
{
    memcpy(pBuf, m_vtBuf.data(), nSize);
    auto itrBegin = m_vtBuf.begin();
    auto itrEnd = itrBegin + nSize;
    m_vtBuf.erase(itrBegin, itrEnd);
}

void CByteStream::Peek(char* pBuf, int nSize)
{
    memcpy(pBuf, m_vtBuf.data(), nSize);
}

int CByteStream::GetSize() const
{
    return m_vtBuf.size();
}
