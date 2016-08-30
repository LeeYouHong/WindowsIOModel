#include "Connection.h"

#include <iostream>
#include <string>
#include "SocketEventHandler.h"

CConnection::CConnection()
	:m_SocketItem(this)
	, m_ISocketHandler(nullptr)
{

}

CConnection::~CConnection()
{
	m_SocketItem.Close();
}

void CConnection::OnConnect()
{


	if (nullptr != m_ISocketHandler)
	{
		m_ISocketHandler->OnConnect();
	}
}

void CConnection::OnSend()
{
	if (nullptr != m_ISocketHandler)
	{
		m_ISocketHandler->OnSend();
	}
}

void CConnection::OnRecv(char* data, int len)
{

	if (nullptr != m_ISocketHandler)
	{
		m_ISocketHandler->OnRecv(data, len);
	}
}

void CConnection::OnClose()
{
	if (nullptr != m_ISocketHandler)
	{
		m_ISocketHandler->OnClose();
	}
}

void CConnection::SetSocketEventHandler(ISocketEventHandler* pHandler)
{
	m_ISocketHandler = pHandler;
}
