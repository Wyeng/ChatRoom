#include <iostream>
#include "ChatServer.h"
#include<thread>
#include<memory>
#include<functional>
#include<string>
#include<atomic>

// 将session和worker放在一起管理
struct ClientWorker
{
    std::shared_ptr<ClientSession> session;
    std::thread worker;
};

// 检查并回收已经结束的线程
void cleanupFinishedThreads(std::vector<ClientWorker>& clientWorkers)
{
    auto it = clientWorkers.begin();

    while (it != clientWorkers.end())
    {
        if (it->session->finished.load())
        {
            if (it->worker.joinable())
            {
                it->worker.join();
            }

            it = clientWorkers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// 负责接受和处理客户端消息
void handleClient(
    ChatServer& chatServer,
    std::shared_ptr<ClientSession> client)
{
    while (chatServer.receiveMessage(*client))
    {
    }

    std::cout << "Client handler finished." << std::endl;

    // 从在线客户端列表中移除
    chatServer.removeClient(client);

    // 由工作线程负责关闭 Socket
    {
        std::lock_guard<std::mutex> lock(client->socketMutex);

        // 由工作线程负责关闭 Socket
        if (client->clientSocket != INVALID_SOCKET)
        {
            closesocket(client->clientSocket);
            client->clientSocket = INVALID_SOCKET;
            client->socketClosed.store(true);
        }
    }

    // 标记工作线程已结束
    client->finished.store(true);
}

// 服务端-socket()-bind()-listen()-accept()(建立和客户端的连接，三次握手)-recv()/send()(与客户端进行数据交流)-close()

int main()
{
    std::cout << "===== Chat Room Server =====" << std::endl;
    std::cout << "Server is starting..." << std::endl;

    ChatServer chatServer;

    // 服务器运行状态，true表示运行，false表示准备停止
    std::atomic<bool> serverRunning{ true };

    // 创建线程容器
    std::vector<ClientWorker> clientWorkers;

    if (!chatServer.initialize())
    {
        std::cerr << "Server initialization failed." << std::endl;
        return 1;
    }

    if (!chatServer.createSocket())
    {
        std::cerr << "Socket creation failed." << std::endl;
        chatServer.cleanup();   //清理winsock初始化使用的资源
        return 1;
    }

    std::cout << "Server socket is ready." << std::endl;

    if (!chatServer.bindSocket())
    {
        std::cerr << "Socket binding failed." << std::endl;
        chatServer.cleanup();
        return 1;
    }

    if (!chatServer.startListening())
    {
        std::cerr << "Failed to start listening." << std::endl;
        chatServer.cleanup();
        return 1;
    }

    // 专门监听控制台输入
    std::thread consoleThread([&serverRunning]()
        {
            std::string command;

            while (std::getline(std::cin, command))
            {
                if (command == "exit")
                {
                    serverRunning.store(false);
                    std::cout << "Server shutdown requested." << std::endl;
                    break;
                }
            }
        });

    while (serverRunning.load())    // 每次开始下一轮循环前，检查服务器的运行状态
    {
        //  每隔500ms进行一次检查
        WaitResult result = chatServer.waitForClient(500);

        if (result == WaitResult::Timeout)
        {
            continue;
        }

        if (result == WaitResult::Error)
        {
            if (serverRunning.load())
            {
                std::cerr << "Waiting for client failed." << std::endl;
                break;
            }

            // 如果已经收到停止请求，就退出循环
            break;
        }

        // 只有 Ready 状态才继续调用 acceptClient()
        SOCKET clientSocket = chatServer.acceptClient();

        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Failed to accept client." << std::endl;
            break;
        }

        std::cout << "Client socket accepted successfully." << std::endl;

        // 回收已经结束的工作线程
        cleanupFinishedThreads(clientWorkers);


        // 先创建客户端会话
        auto client = std::make_shared<ClientSession>();
        client->clientSocket = clientSocket;

        // 将客户端会话加入在线列表
        if (!chatServer.addClient(client))
        {
            std::cerr << "Failed to register client." << std::endl;
            closesocket(clientSocket);
            client->clientSocket = INVALID_SOCKET;
            continue;
        }

        // 创建客户端工作线程
        clientWorkers.push_back(
            ClientWorker{
                client,
                std::thread(handleClient, std::ref(chatServer), client)
            }
        );
    }

    // 通知所有仍在运行的客户端停止收发数据
    for (auto& clientWorker : clientWorkers)
    {
        auto& client = clientWorker.session;

        std::lock_guard<std::mutex> lock(client->socketMutex);

        if (!client->finished.load() &&
            !client->socketClosed.load() &&
            client->clientSocket != INVALID_SOCKET)
        {
            if (shutdown(client->clientSocket, SD_BOTH) == SOCKET_ERROR)
            {
                int errorCode = WSAGetLastError();

                // 退出流程中，连接可能已经断开
                std::cerr << "shutdown failed: "
                    << errorCode << std::endl;
            }
        }
    }

    // 等待所有客户端处理线程结束
    for (auto& clientWorker : clientWorkers)
    {
        if (clientWorker.worker.joinable())
        {
            clientWorker.worker.join();
        }
    }

    if (consoleThread.joinable())
    {
        consoleThread.join();
    }

    chatServer.cleanup();
    return 0;
}