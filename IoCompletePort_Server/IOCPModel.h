#pragma once

#include <vector>
#include <cassert>

//���������ȣ�1024*8��
#define MAX_BUFFER_LEN			8192

//����ɶ˿���Ͷ�ݵĲ�������
enum OPERATION_TYPE
{
	enOperation_SEND,				//���Ͳ���
	enOperation_RECV,				//���ղ���
};


//��IO���ݽṹ
class CIoContext
{

public:
	OVERLAPPED		m_Overlapped;
	WSABUF			m_wsaBuf;
	char			m_szBuffer[MAX_BUFFER_LEN];
	OPERATION_TYPE	m_OpType;

public:
	//��ʼ��
	CIoContext()
	{
		ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
		ZeroMemory(m_szBuffer, sizeof(m_szBuffer));
		m_wsaBuf.buf = m_szBuffer;
		m_wsaBuf.len = sizeof(m_szBuffer);
	}

	//�ͷ�
	~CIoContext()
	{

	}
};

class CSocketItem
{
public:
	SOCKET								m_Socket;				//ÿһ���ͻ������ӵ��׽���
	std::vector<CIoContext*>		m_VectorIoContext;		//�ͻ������������IO������

public:
	// ��ʼ��
	CSocketItem()
	{
		m_Socket = INVALID_SOCKET;
	}

	// �ͷ���Դ
	~CSocketItem()
	{
		if( m_Socket!=INVALID_SOCKET )
		{
			closesocket( m_Socket );
			m_Socket = INVALID_SOCKET;
		}
		// �ͷŵ����е�IO����������
		for( UINT i=0;i<m_VectorIoContext.size();i++ )
		{
			delete m_VectorIoContext.at(i);
		}
		m_VectorIoContext.clear();
	}

	// ��ȡһ���µ�IoContext
	CIoContext* GetNewIoContext()
	{
		CIoContext* p = new CIoContext;

		m_VectorIoContext.push_back( p );

		return p;
	}
};