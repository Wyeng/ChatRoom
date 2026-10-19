#pragma once

#include<WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include<cstring>
#include<mutex>
#include<string>

enum class ReceiveStatus
{
	Success,        // 成功收到数据
	Disconnected,   // 对端关闭连接
	Error           // 发生接收错误
};

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
	ReceiveStatus receiveMessage(
		int& errorCode,
		std::mutex& consoleMutex,
		const std::string& username,
		const std::string& message
	);
	void cleanup();
};