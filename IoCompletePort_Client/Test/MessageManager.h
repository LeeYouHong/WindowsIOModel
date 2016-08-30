#pragma once

#ifndef __MESSAGEMANAGER_H__
#define __MESSAGEMANAGER_H__

#include <vector>
#include <mutex>
#include "Singleton.h"


class CMessage
{
public:
	char data[4096];
	int len;
	int cid;
};

class CMessageManager
{
public:
	CMessageManager();
	~CMessageManager();

public:
	void Start();

	void Stop();

	static void Run(CMessageManager* instance);

	void Add(CMessage msg);

	std::vector<CMessage> m_MessageList;
	std::mutex m_ListMutex;

	bool m_IsRun;
};

using s_MessageManager = JuneNet::Singleton < CMessageManager >;

#endif