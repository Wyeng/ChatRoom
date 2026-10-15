#pragma once

#include<WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include<cstring>

class ChatClient
{
public:
	SOCKET clientSocket;
	bool winsockInitialized;
public:
	ChatClient();
	bool initialize();
	bool createSocket();
	bool connectServer(const char* ip, unsigned short port);
	bool sendMessage(const char* message);
	void cleanup();
};