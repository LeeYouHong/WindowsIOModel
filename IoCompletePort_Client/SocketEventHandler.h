#pragma once

#ifndef __SOCKETEVENTHANDLER_H__
#define __SOCKETEVENTHANDLER_H__

class ISocketEventHandler
{
public:
	ISocketEventHandler()
	{
	};
	~ISocketEventHandler()
	{
	};

public:
	virtual void OnConnect() = 0 ;
	virtual void OnSend() = 0;
	virtual void OnRecv(char* data, int len) = 0;
	virtual void OnClose() = 0;
};

#endif