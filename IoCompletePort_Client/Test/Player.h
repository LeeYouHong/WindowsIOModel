#pragma once

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <vector>
#include "MyConnection.h"
#include "Singleton.h"
#include "MessageManager.h"

class CPlayer
{
public:
	CPlayer(int id, int cid);
	~CPlayer();

public:
	void ProcessMessage(CMessage msg);

private:
	void SendMsg(char* data, int len);
public:
	int m_ConnectionID;
	int m_PID;
};

class CPlayerManager
{
public:
	void AddPlayer(CPlayer* ply);

	CPlayer* FindPlayer(int cid);

	std::vector<CPlayer*> m_PlayerList;
};

using s_PlayerManager = JuneNet::Singleton < CPlayerManager > ;

#endif