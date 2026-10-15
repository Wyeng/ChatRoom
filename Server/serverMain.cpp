#include <iostream>
#include "ChatServer.h"

// 服务端-socket()-bind()-listen()-accept()(建立和客户端的连接，三次握手)-recv()/send()(与客户端进行数据交流)-close()

int main()
{
    std::cout << "===== Chat Room Server =====" << std::endl;
    std::cout << "Server is starting..." << std::endl;

    ChatServer chatServer;

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

    SOCKET clientSocket = chatServer.acceptClient();
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to accept client." << std::endl;
        chatServer.cleanup();
        return 1;
    }
    std::cout << "Client socket accepted successfully." << std::endl;

    chatServer.receiveMessage(clientSocket);

    closesocket(clientSocket);
    chatServer.cleanup();
    return 0;
}