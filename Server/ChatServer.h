#pragma once
#include<WinSock2.h>
#include<string>
#include<atomic>
#include<vector>
#include<mutex>
#include<algorithm>
#include<memory>

struct ClientSession
{
	std::string username;
	SOCKET clientSocket = INVALID_SOCKET;	// 保存客户端连接
	std::string receiveBuffer;	// 暂存TCP数据
	std::atomic<bool> finished{ false };	//标记工作线程是否结束
	std::atomic<bool> socketClosed{ false };	//记录socket的关闭状态
	std::mutex socketMutex;  // 保护 Socket 的访问和关闭
};

enum class WaitResult
{
	Ready,    // 有连接请求
	Timeout,  // 等待超时
	Error     // 发生错误
};

class ChatServer
{
private:
	SOCKET chatSocket;
	bool winsockInitialized;

	std::vector<std::shared_ptr<ClientSession>> clients;
	std::mutex clientsMutex;
public:
	ChatServer();
	bool initialize();
	bool createSocket();
	bool bindSocket();
	bool startListening();
	SOCKET acceptClient();
	WaitResult waitForClient(int timeoutMs);	// 循环是定时检查服务器的状态，看是否退出
	void closeClientSocket(SOCKET clientSocket);
	bool addClient(std::shared_ptr<ClientSession> client);
	void removeClient(std::shared_ptr<ClientSession> client);
	bool broadcastMessage(SOCKET senderSocket, const std::string& message);
	void cleanup();	// 清理服务器端资源
	bool receiveMessage(ClientSession& client);
	void stopListening();
};