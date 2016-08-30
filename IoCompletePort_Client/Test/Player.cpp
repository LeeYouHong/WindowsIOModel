#include "Player.h"
#include <string>
#include <iostream>
#include "Timer.h"

CPlayer::CPlayer(int id, int cid)
	:m_PID(id)
	, m_ConnectionID(cid)
{

}

CPlayer::~CPlayer()
{

}

void CPlayer::ProcessMessage(CMessage msg)
{
	std::string str(msg.data, msg.len);
	std::cout << str << std::endl;

	s_TimerManager::Instance().AddTimer(JuneNet::CTimer(1000, [=]()
	{
		SendMsg("ACK", sizeof("ACK"));
	}));
}

void CPlayer::SendMsg(char* data, int len)
{
	CMyConnection* pConn = s_MyConnManager::Instance().FindConn(m_ConnectionID);
	if (nullptr == pConn)
	{
		std::cout << "Player no connection, cid=" << m_ConnectionID << std::endl;
		return;
	}
	pConn->m_SocketItem.Send(data, len);
}

void CPlayerManager::AddPlayer(CPlayer* ply)
{
	m_PlayerList.push_back(ply);
}

CPlayer* CPlayerManager::FindPlayer(int cid)
{
	for (auto ply : m_PlayerList)
	{
		if (ply->m_ConnectionID == cid)
		{
			return ply;
		}
	}

	return nullptr;
}

