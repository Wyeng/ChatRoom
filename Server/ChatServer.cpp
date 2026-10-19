#include"ChatServer.h"
#include<iostream>
#include<string>

// 构造函数,初始时将Socket状态初始为无效状态，避免成员变量处于未初始化状态
ChatServer::ChatServer() :chatSocket(INVALID_SOCKET),winsockInitialized(false)
{

}
bool ChatServer::initialize()
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

// 创建服务器端Socket,并返回是否成功
bool ChatServer::createSocket()
{
	// 创建套接字 使用IPv4地址，使用面向连接的字节流通信，使用TCP协议
	this->chatSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->chatSocket == INVALID_SOCKET)
	{
		std::cerr << "socket failed: "<< WSAGetLastError() << std::endl;
		return false;
	}
	std::cout << "TCP socket created successfully." << std::endl;
	return true;
}

// 将服务器绑定到端口8888
bool ChatServer::bindSocket()
{
	// 创建sockaddr_in对象来保存IPv4网络地址信息
	sockaddr_in serverAddr{};

	// 设置地址和端口
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8888);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(chatSocket, reinterpret_cast<const sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	std::cout << "bind created successfully" << std::endl;
	return true;
}

// 初始化listen函数，使用服务器socket开始监听
bool ChatServer::startListening()
{
	if (listen(chatSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "listen failed: " <<WSAGetLastError()<< std::endl;
		return false;
	}
	std::cout << "listen started successfully" << std::endl;
	return true;
}


SOCKET ChatServer::acceptClient()
{
	sockaddr_in clientAddr{};
	int addrlen = sizeof(clientAddr);

	SOCKET clientSocket = accept(
		chatSocket,
		reinterpret_cast<sockaddr*>(&clientAddr),
		&addrlen
	);

	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "accept failed: "
			<< WSAGetLastError()
			<< std::endl;

		return INVALID_SOCKET;
	}

	// 设置发送超时时间：3000 毫秒
	DWORD sendTimeoutMs = 3000;

	int result = setsockopt(
		clientSocket,
		SOL_SOCKET,
		SO_SNDTIMEO,
		reinterpret_cast<const char*>(&sendTimeoutMs),
		sizeof(sendTimeoutMs)
	);

	if (result == SOCKET_ERROR)
	{
		std::cerr << "Failed to set send timeout: "
			<< WSAGetLastError()
			<< std::endl;

		closesocket(clientSocket);
		return INVALID_SOCKET;
	}

	std::cout << "Client accepted successfully."
		<< std::endl;

	std::cout << "Send timeout set to 3000 ms."
		<< std::endl;

	return clientSocket;
}


bool ChatServer::receiveMessage(ClientSession& client)
{
	char buffer[1024] = {};	// 定义缓冲区大小为1024个字符

	int bytesReceived = recv(
		client.clientSocket,
		buffer,
		sizeof(buffer) - 1,
		0
	);

	if (bytesReceived == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();

		std::cerr << "recv failed: "
			<< errorCode
			<< ", socket: "
			<< client.clientSocket
			<< std::endl;

		return false;
	}

	if (bytesReceived == 0)
	{
		std::cout << "Client disconnected." << std::endl;
		return false;
	}

	// 将本次收到的数据追加到成员缓冲区
	client.receiveBuffer.append(buffer, bytesReceived);

	std::string::size_type pos;

	while ((pos = client.receiveBuffer.find('\n')) != std::string::npos)
	{

		std::string message = client.receiveBuffer.substr(0, pos);

		// 第一次收到完整消息：将其作为用户名
		if (client.username.empty())
		{
			client.username = message;

			std::cout << "Client username registered: "
				<< client.username << std::endl;
		}
		else
		{
			// 后续消息才作为聊天消息广播
			std::cout << "[" << client.username
				<< "] " << message << std::endl;

			std::string broadcastData =
				client.username + ": " + message + "\n";

			if (!broadcastMessage(client.clientSocket, broadcastData))
			{
				std::cerr << "Failed to broadcast message."
					<< std::endl;
			}
		}

		client.receiveBuffer.erase(0, pos + 1);

	}
	return true;
}

void ChatServer::stopListening()
{
	if (chatSocket != INVALID_SOCKET)
	{
		shutdown(chatSocket, SD_BOTH);	// 停止Socket的收发操作
		closesocket(chatSocket);	// 释放监听socket
		chatSocket = INVALID_SOCKET;	// 记录socket已关闭，避免重复关闭
	}
}


WaitResult ChatServer::waitForClient(int timeoutMs)
{
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(chatSocket, &readSet);

	timeval timeout{};
	timeout.tv_sec = timeoutMs / 1000;
	timeout.tv_usec = (timeoutMs % 1000) * 1000;

	int result = select(0, &readSet, nullptr, nullptr, &timeout);

	if (result > 0)
	{
		return WaitResult::Ready;
	}

	if (result == 0)
	{
		return WaitResult::Timeout;
	}

	std::cerr << "select failed: "
		<< WSAGetLastError() << std::endl;

	return WaitResult::Error;
}

bool ChatServer::addClient(
	std::shared_ptr<ClientSession> client)
{
	if (!client ||
		client->clientSocket == INVALID_SOCKET)
	{
		return false;
	}

	std::lock_guard<std::mutex> lock(clientsMutex);	// 加锁互斥访问临界资源

	clients.push_back(client);

	return true;
}

// 关闭客户端则将socket从vector中移除

void ChatServer::removeClient(std::shared_ptr<ClientSession> client)
{
	std::lock_guard<std::mutex> lock(clientsMutex);

	auto it = std::find(
		clients.begin(),
		clients.end(),
		client
	);

	if (it != clients.end())
	{
		clients.erase(it);
	}
}

// 将消息进行广播，实现群聊的功能

bool ChatServer::broadcastMessage(
	SOCKET senderSocket,
	const std::string& message)
{
	// 复制在线客户端会话列表
	std::vector<std::shared_ptr<ClientSession>> currentClients;

	{
		std::lock_guard<std::mutex> lock(clientsMutex);
		currentClients = clients;
	}

	bool allSucceeded = true;

	// 遍历在线客户端
	for (const auto& client : currentClients)
	{
		// 锁住当前客户端的 Socket
		std::lock_guard<std::mutex> lock(client->socketMutex);

		// 跳过已关闭的客户端
		if (client->socketClosed.load() ||
			client->clientSocket == INVALID_SOCKET)
		{
			continue;
		}

		// 不向发送者本人广播
		if (client->clientSocket == senderSocket)
		{
			continue;
		}

		int totalSent = 0;
		int totalBytes = static_cast<int>(message.size());

		// 循环处理部分发送
		while (totalSent < totalBytes)
		{
			int bytesSent = send(
				client->clientSocket,
				message.c_str() + totalSent,
				totalBytes - totalSent,
				0
			);

			if (bytesSent == SOCKET_ERROR)
			{
				int errorCode = WSAGetLastError();

				std::cerr << "Broadcast failed. Error code: "
					<< errorCode << std::endl;

				allSucceeded = false;
				break;
			}

			if (bytesSent == 0)
			{
				std::cerr << "Broadcast failed: send returned 0."
					<< std::endl;

				allSucceeded = false;
				break;
			}

			totalSent += bytesSent;
		}
	}

	return allSucceeded;
}

// 回收客户端工作线程
void ChatServer::closeClientSocket(SOCKET clientSocket)
{
	if (clientSocket != INVALID_SOCKET)
	{
		shutdown(clientSocket, SD_BOTH);
		closesocket(clientSocket);
	}
}

void ChatServer::cleanup()
{
	// 关闭监听socket
	if (chatSocket != INVALID_SOCKET)
	{
		closesocket(chatSocket);
		chatSocket=INVALID_SOCKET;
	}
	if (winsockInitialized)
	{
		WSACleanup();
		winsockInitialized = false;
	}
}