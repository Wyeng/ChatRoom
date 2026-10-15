#include"ChatClient.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

ChatClient::ChatClient() :clientSocket(INVALID_SOCKET),winsockInitialized(false)
{

}

bool ChatClient::initialize()
{
	// 实例化对象,windows提供的socket网络编程接口
	WSADATA wsaData;

	// 请求使用Winsock2.2，并且接受返回值判断是否初始化成功
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return false;
	}
	winsockInitialized = true;
	std::cout << "Winsock initialized successfully." << std::endl;
	return true;
}

bool ChatClient::createSocket()
{
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "socket failed : "<< WSAGetLastError() << std::endl;
		return false;
	}
	std::cout << "TCP socket created successfully." << std::endl;
	return true;
}

bool ChatClient::connectServer(const char* ip, unsigned short port)
{
	sockaddr_in serverAddr{};

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	int result = inet_pton(AF_INET, ip, &serverAddr.sin_addr);
	if (result != 1)
	{
		std::cerr << "Invalid IP address" << std::endl;
		return false;
	}
	int resultConnect = connect(
		clientSocket,
		reinterpret_cast<const sockaddr*>(&serverAddr),
		sizeof(serverAddr)
	);

	if (resultConnect == SOCKET_ERROR)
	{
		std::cerr << "connect failed: "
			<< WSAGetLastError() << std::endl;
		return false;
	}

	std::cout << "Connected to server successfully." << std::endl;
	return true;
}

bool ChatClient::sendMessage(const char* message)
{
	int bytesSent = send(
		clientSocket,
		message,
		static_cast<int>(strlen(message)),
		0
	);

	if (bytesSent == SOCKET_ERROR)
	{
		std::cerr << "send failed: "
			<< WSAGetLastError() << std::endl;
		return false;
	}

	std::cout << "Message sent successfully." << std::endl;
	return true;
}

void ChatClient::cleanup()
{
	if (clientSocket != INVALID_SOCKET)
	{
		closesocket(clientSocket);
		clientSocket = INVALID_SOCKET;
	}
	if (winsockInitialized)
	{
		WSACleanup();
		winsockInitialized = false;
	}
}