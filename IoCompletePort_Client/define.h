#pragma once

#ifndef __DEFINE_H__
#define __DEFINE_H__

//#include <windows.h>
#include <winsock2.h>
#include <MSWSock.h>
#include <Ws2tcpip.h>
#include <cassert>

//»º³åÇø³¤¶È£¨1024*8£©
#define MAX_BUFFER_LEN			8 * 1024

#define MAX_RECV_BUF_LEN		16 * 1024

#define SAFE_DEL(obj)	if((obj) != NULL) {delete (obj); (obj) = NULL;}
#define ARRAY_DEL(obj)   if((obj) != NULL) {delete[] (obj); (obj) = NULL;}

#endif