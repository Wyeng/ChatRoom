#include"ChatClient.h"

int main()
{
    std::cout << "===== Chat Room Client =====" << std::endl;

    ChatClient chatClient;

    if (!chatClient.initialize())
    {
        std::cerr << "Client initialization failed." << std::endl;
        return 1;
    }

    if (!chatClient.createSocket())
    {
        std::cerr << "Socket creation failed." << std::endl;
        chatClient.cleanup();
        return 1;
    }

    if (!chatClient.connectServer("127.0.0.1", 8888))
    {
        std::cerr << "Failed to connect to server." << std::endl;
        chatClient.cleanup();
        return 1;
    }
    std::cout << "Client connected successfully." << std::endl;

    chatClient.sendMessage("hello Server");

    chatClient.cleanup();
    return 0;
}