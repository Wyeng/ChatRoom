#pragma once
#include<WinSock2.h>
class ChatServer
{
private:
	SOCKET chatSocket;
	bool winsockInitialized;
public:
	ChatServer();
	bool initialize();
	bool createSocket();
	bool bindSocket();
	bool startListening();
	SOCKET acceptClient();
	void cleanup();	// 清理服务器端资源
	bool receiveMessage(SOCKET clientSocket);
};