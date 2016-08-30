#pragma once

#ifndef __SOCKETITEM_H__
#define __SOCKETITEM_H__

#include "define.h"
#include <vector>


//����ɶ˿���Ͷ�ݵĲ�������
enum OPERATION_TYPE
{
	enOperation_NONE,				//δ�������
	enOperation_SEND,				//���Ͳ���
	enOperation_RECV,				//���ղ���
	enOperation_CONNECT,			//���Ӳ���
};


//��IO���ݽṹ
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
	char		m_Buffer[MAX_RECV_BUF_LEN];		// ��󻺴��С
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
	// ��ȡһ���µ�IoContext
	CIOContext* GetFreeIoContext();

	// ����һ��IO������
	void FreeIOContext(CIOContext* pIOCtx);

public:
	void Close();

	void Send(char* data, int len);

public:
	SOCKET							m_Socket;				// ÿһ���ͻ������ӵ��׽���
	CBuffer							m_RecvBuffer;			// ���ܻ�����
	CConnection*					m_pConnection;
	CIOCPManager*					m_pIOCPManager;			// IOCP������
private:
	std::vector<CIOContext*>		m_IOContextList;		// �ͻ������������IO������
	std::vector<CIOContext*>		m_FreeIOContextList;	// ���еĿͻ������������IO������
};

#endif