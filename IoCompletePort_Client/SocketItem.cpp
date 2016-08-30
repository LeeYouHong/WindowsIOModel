#include "SocketItem.h"
#include "IOCPManager.h"

CSocketItem::CSocketItem(CConnection* pConn)
	:m_Socket(INVALID_SOCKET)
	, m_pConnection(pConn)
{
}

CSocketItem::~CSocketItem()
{
	//if (m_Socket != INVALID_SOCKET)
	//{
	//	closesocket(m_Socket);
	//	m_Socket = INVALID_SOCKET;
	//}

	//// 释放掉所有的IO上下文数据
	//for (UINT i = 0; i < m_IOContextList.size(); i++)
	//{
	//	SAFE_DEL(m_IOContextList[i]);
	//}
	//m_IOContextList.clear();
}

CIOContext* CSocketItem::GetFreeIoContext()
{
	if (!m_FreeIOContextList.empty())
	{
		CIOContext* pFree = m_FreeIOContextList.back();
		m_FreeIOContextList.pop_back();
		return pFree;
	}

	// 新建一个
	CIOContext* ctx = new CIOContext();
	m_IOContextList.push_back(ctx);

	return ctx;
}

void CSocketItem::FreeIOContext(CIOContext* pIOCtx)
{
	pIOCtx->Clear();
	m_FreeIOContextList.push_back(pIOCtx);
}

void CSocketItem::Close()
{
	if (m_Socket != INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	// 释放掉所有的IO上下文数据
	for (UINT i = 0; i < m_IOContextList.size(); i++)
	{
		SAFE_DEL(m_IOContextList[i]);
	}
	m_IOContextList.clear();
}

void CSocketItem::Send(char* data, int len)
{
	m_pIOCPManager->SendData(this, data, len);
}

CBuffer::CBuffer()
	:m_Head(0)
	, m_Tail(-1)
	, m_MaxBufLen(sizeof(m_Buffer))
{
}

int CBuffer::GetFreeSize()
{
	if (m_Tail < 0)
	{
		return m_MaxBufLen;
	}

	return m_MaxBufLen - (m_Tail+1);
}

int CBuffer::WriteBuf(char* buf, int len)
{
	assert(len > 0);

	if (len > m_MaxBufLen)
	{
		assert(len <= m_MaxBufLen);
		return -1;
	}

	// 空闲空间不够，则往前移动来腾出空间
	if (len > GetFreeSize() && m_Head > 0)
	{
		int bufLen = GetBufSize();
		memcpy(m_Buffer, m_Buffer+m_Head, bufLen);
		m_Head = 0;
		m_Tail = bufLen;
	}

	memcpy(m_Buffer+m_Head, buf, len);
	m_Tail += len;

	return len;
}

int CBuffer::ReadBuf(char* buf, int len)
{
	assert(len > 0);

	if (len < 0)
	{
		assert(len > 0);
		return -1;
	}

	if (len > GetBufSize())
	{
		assert(len <= GetBufSize());
		return -1;
	}

	memcpy(buf, m_Buffer + m_Head, len);

	m_Head += len;

	return len;
}

int CBuffer::PeekBuf(char* buf, int len)
{
	assert(len > 0);

	if (len < 0)
	{
		assert(len > 0);
		return -1;
	}

	if (len > GetBufSize())
	{
		assert(len <= GetBufSize());
		return -1;
	}

	memcpy(buf, m_Buffer + m_Head, len);

	return len;
}

bool CBuffer::CheckSize(int len)
{
	return GetBufSize() >= len;
}

int CBuffer::GetBufSize()
{
	if (m_Tail == -1)
	{
		return 0;
	}

	return (m_Tail - m_Head) + 1;
}
