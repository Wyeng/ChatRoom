#include"ChatServer.h"
#include<iostream>

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
	// 创建客户端地址结构体
	sockaddr_in clientAddr{};
	// 设置地址结构体长度
	int addrlen = sizeof(clientAddr);
	// 调用accept()
	SOCKET clientSocket = accept(chatSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrlen);
	// 判断是否接受连接成功
	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "accept failed: "<< WSAGetLastError() << std::endl;
		return INVALID_SOCKET;
	}
	std::cout << "client accept successfully" << std::endl;
	return clientSocket;
}

bool ChatServer::receiveMessage(SOCKET clientSocket)
{
	char buffer[1024] = {};	// 定义缓冲区大小为1024个字符

	int bytesReceived = recv(
		clientSocket,
		buffer,
		sizeof(buffer) - 1,
		0
	);

	if (bytesReceived == SOCKET_ERROR)
	{
		std::cerr << "recv failed: "
			<< WSAGetLastError() << std::endl;
		return false;
	}

	if (bytesReceived == 0)
	{
		std::cout << "Client disconnected." << std::endl;
		return false;
	}

	std::cout << "Received message: " << buffer << std::endl;
	return true;
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