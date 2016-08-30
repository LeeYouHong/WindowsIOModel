#pragma once

#ifndef __SOCKETITEM_H__
#define __SOCKETITEM_H__

#include "define.h"
#include <vector>


//在完成端口上投递的操作类型
enum OPERATION_TYPE
{
	enOperation_NONE,				//未定义操作
	enOperation_SEND,				//发送操作
	enOperation_RECV,				//接收操作
	enOperation_CONNECT,			//连接操作
};


//单IO数据结构
class CIOContext
{

public:
	OVERLAPPED		m_Overlapped;
	WSABUF			m_wsaBuf;
	char			m_Buf[MAX_BUFFER_LEN];
	OPERATION_TYPE	m_OpType;

public:
	void SetWsaBuf(char* buf, int len)
	{
		m_wsaBuf.buf = buf;
		m_wsaBuf.len = len;
	}

	void Clear()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
		m_wsaBuf.buf = m_Buf;
		m_wsaBuf.len = sizeof(m_Buf);
		m_OpType = enOperation_NONE;;
	}

	CIOContext()
	{
		Clear();
	}
};

class CBuffer
{
public:
	CBuffer();

	int GetFreeSize();

	int WriteBuf(char* buf, int len);

	int ReadBuf(char* buf, int len);

	int PeekBuf(char* buf, int len);

	bool CheckSize(int len);

	int GetBufSize();
private:
	char		m_Buffer[MAX_RECV_BUF_LEN];		// 最大缓存大小
	int			m_Head;
	int			m_Tail;
	int			m_MaxBufLen;
};

class CConnection;
class CIOCPManager;

class CSocketItem
{
public:
	CSocketItem(CConnection* pConn);
	~CSocketItem();

public:
	// 获取一个新的IoContext
	CIOContext* GetFreeIoContext();

	// 回收一个IO上下文
	void FreeIOContext(CIOContext* pIOCtx);

public:
	void Close();

	void Send(char* data, int len);

public:
	SOCKET							m_Socket;				// 每一个客户端连接的套接字
	CBuffer							m_RecvBuffer;			// 接受缓冲区
	CConnection*					m_pConnection;
	CIOCPManager*					m_pIOCPManager;			// IOCP管理类
private:
	std::vector<CIOContext*>		m_IOContextList;		// 客户端网络操作的IO上下文
	std::vector<CIOContext*>		m_FreeIOContextList;	// 空闲的客户端网络操作的IO上下文
};

#endif