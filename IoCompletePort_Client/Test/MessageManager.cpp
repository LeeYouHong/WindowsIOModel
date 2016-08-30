#include "MessageManager.h"

#include <thread>
#include <iostream>
#include "Player.h"
#include "Timer.h"

CMessageManager::CMessageManager()
	:m_IsRun(false)
{

}

CMessageManager::~CMessageManager()
{

}

void CMessageManager::Start()
{
	m_IsRun = true;
	std::thread threadPro(Run, this);
	threadPro.detach();
}

void CMessageManager::Stop()
{
	m_IsRun = false;
}

void CMessageManager::Run(CMessageManager* instance)
{
	while (instance->m_IsRun)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		// ¶¨Ê±Æ÷
		s_TimerManager::Instance().Run();

		instance->m_ListMutex.lock();
		if (instance->m_MessageList.empty())
		{
			instance->m_ListMutex.unlock();
			continue;
		}
		std::vector<CMessage> tmpMsgList;
		tmpMsgList.swap(instance->m_MessageList);
		instance->m_ListMutex.unlock();

		for (auto msg : tmpMsgList)
		{
			CPlayer* ply = s_PlayerManager::Instance().FindPlayer(msg.cid);
			if (nullptr == ply)
			{
				std::cout << "No player, cid=" << msg.cid << std::endl;
				continue;
			}
			ply->ProcessMessage(msg);
		}

		tmpMsgList.clear();

	}
}

void CMessageManager::Add(CMessage msg)
{
	m_ListMutex.lock();
	m_MessageList.push_back(msg);
	m_ListMutex.unlock();
}
