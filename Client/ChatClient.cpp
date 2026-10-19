#include"ChatClient.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include<string>

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
	// 添加 '\n' 作为消息分隔符，便于接收端识别消息边界
	std::string sendData = message;
	sendData += '\n';
	int totalBytes = static_cast<int>(sendData.size());
	int totalSent = 0;

	while (totalSent < totalBytes)
	{
		int bytesSent = send(
			clientSocket,
			sendData.c_str()+totalSent,	// 从尚未发送的位置继续
			totalBytes-totalSent,	// 只发送剩余字符
			0
		);
		// 如果send失败进行处理
		if (bytesSent == SOCKET_ERROR)
		{
			std::cerr << "send failed: "
				<< WSAGetLastError() << std::endl;
			return false;
		}

		if (bytesSent == 0)
		{
			std::cerr << "send returned 0." << std::endl;
			return false;
		}

		// 更新totalSent
		totalSent += bytesSent;
	}

	//std::cout << "Message sent successfully." << std::endl;
	return true;
}

ReceiveStatus ChatClient::receiveMessage(int& errorCode, std::mutex& consoleMutex, const std::string& username, const std::string& message)
{
	errorCode = 0;
	char buffer[1024]{};

	int bytesReceived = recv(
		clientSocket,
		buffer,
		static_cast<int>(sizeof(buffer) - 1),
		0
	);

	if (bytesReceived > 0)
	{
		buffer[bytesReceived] = '\0';
		{
			std::lock_guard<std::mutex> lock(consoleMutex);

			// 清除当前输入行并换行
			std::cout << "\r" << std::string(120, ' ') << "\r";

			// 显示收到的消息
			std::cout << buffer;

			// 恢复当前输入行
			std::cout << username << ": "
				<< message << std::flush;
		}
		return ReceiveStatus::Success;
	}

	if (bytesReceived == 0)
	{
		std::cout << "Server disconnected." << std::endl;
		return ReceiveStatus::Disconnected;
	}

	errorCode = WSAGetLastError();
	return ReceiveStatus::Error;
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