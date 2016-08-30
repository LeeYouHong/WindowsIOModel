#pragma once

#include <vector>
#include <cassert>

//缓冲区长度（1024*8）
#define MAX_BUFFER_LEN			8192

//在完成端口上投递的操作类型
enum OPERATION_TYPE
{
	enOperation_SEND,				//发送操作
	enOperation_RECV,				//接收操作
};


//单IO数据结构
class CIoContext
{

public:
	OVERLAPPED		m_Overlapped;
	WSABUF			m_wsaBuf;
	char			m_szBuffer[MAX_BUFFER_LEN];
	OPERATION_TYPE	m_OpType;

public:
	//初始化
	CIoContext()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
		ZeroMemory(m_szBuffer, sizeof(m_szBuffer));
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = sizeof(m_szBuffer);
	}

	//释放
	~CIoContext()
	{

	}
};

class CSocketItem
{
public:
	SOCKET								m_Socket;				//每一个客户端连接的套接字
	std::vector<CIoContext*>		m_VectorIoContext;		//客户端网络操作的IO上下文

public:
	// 初始化
	CSocketItem()
	{
		m_Socket = INVALID_SOCKET;
	}

	// 释放资源
	~CSocketItem()
	{
		if( m_Socket!=INVALID_SOCKET )
		{
			closesocket( m_Socket );
			m_Socket = INVALID_SOCKET;
		}
		// 释放掉所有的IO上下文数据
		for( UINT i=0;i<m_VectorIoContext.size();i++ )
		{
			delete m_VectorIoContext.at(i);
		}
		m_VectorIoContext.clear();
	}

	// 获取一个新的IoContext
	CIoContext* GetNewIoContext()
	{
		CIoContext* p = new CIoContext;

		m_VectorIoContext.push_back( p );

		return p;
	}
};