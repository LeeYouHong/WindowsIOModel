#include "MyConnection.h"

#include <iostream>
#include <string>
#include "MessageManager.h"

CMyConnection::CMyConnection(int cid)
	:m_Cid(cid)
{

}

CMyConnection::~CMyConnection()
{

}

void CMyConnection::OnConnect()
{
	std::cout << "连接成功" << std::endl;

	// 发送第一条数据
	m_SocketItem.Send("hello", 5);
}

void CMyConnection::OnSend()
{
	std::cout << "发送完成" << std::endl;

}

void CMyConnection::OnRecv(char* data, int len)
{
	std::string str(data, len);
	std::cout << "收到消息:" << str << std::endl;

	CMessage msg = {};
	memcpy(msg.data, data, len);
	msg.len = len;
	msg.cid = m_Cid;

	s_MessageManager::Instance().Add(msg);
}

void CMyConnection::OnClose()
{
	std::cout << "连接关闭" << std::endl;

}

void CMyConnManager::AddConnection(CMyConnection* pConn)
{
	m_ConnList.push_back(pConn);
}

CMyConnection* CMyConnManager::FindConn(int cid)
{
	for (auto conn : m_ConnList)
	{
		if (conn->m_Cid == cid)
		{
			return conn;
		}
	}

	return nullptr;
}
