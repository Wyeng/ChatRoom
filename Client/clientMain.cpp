#include"ChatClient.h"
#include<string>
#include<thread>
#include<atomic>
#include <conio.h>
#include<chrono>


void redrawInput(
    const std::string& username,
    const std::string& message)
{
    std::cout << "\r"
        << username << ": "
        << message
        << " " << std::flush;
}


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

    // 输入用户名
    std::string username;

    std::cout << "Please enter your username: ";
    std::getline(std::cin, username);

    // 不允许用户名为空
    while (username.empty())
    {
        std::cout << "Username cannot be empty. Please enter again: ";
        std::getline(std::cin, username);
    }

    // 将用户名发送给服务器
    if (!chatClient.sendMessage(username.c_str()))
    {
        std::cerr << "Failed to send username." << std::endl;
        chatClient.cleanup();
        return 1;
    }

    std::cout << "Username sent successfully." << std::endl;

    std::atomic<bool> receiveThreadFinished{ false };  // 目前用于通知主线程接收线程已经结束
    std::atomic<bool> clientClosing{ false };   // 表示客户端是否正在主动关闭连接
    std::atomic<bool> receiveError{ false };    // 接收线程是否发生了接收错误

    std::mutex consoleMutex;    // 互斥访问控制台

    std::string message;

    std::thread receiveThread(
        [&chatClient, &receiveThreadFinished, &clientClosing,
        &receiveError, &consoleMutex, & message, &username]()
        {
            while (true)
            {
                int errorCode = 0;
                ReceiveStatus status = chatClient.receiveMessage(
                    errorCode,
                    consoleMutex,
                    username,
                    message
                );

                if (status == ReceiveStatus::Success)
                {
                    continue;
                }

                if (status == ReceiveStatus::Disconnected)
                {
                    {
                        std::lock_guard<std::mutex> lock(consoleMutex);
                        std::cout << "\n[System] Server disconnected normally."
                            << std::endl;
                    }
                    break;
                }

                if (status == ReceiveStatus::Error)
                {
                    if (!clientClosing.load())
                    {
                        receiveError.store(true);

                        {
                            std::lock_guard<std::mutex> lock(consoleMutex);
                            std::cerr << "\n[Error] Receiving from server failed. Error code: "
                                << errorCode << std::endl;
                        }
                    }
                    break;
                }
            }

            receiveThreadFinished.store(true);
        });


    int exitCode = 0;

    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "\n" << username << ": " << std::flush;
    }

    while (!receiveThreadFinished.load())
    {
        if (_kbhit())
        {
            int ch = _getch();



            if (ch == '\r')
            {
                std::string messageToSend;

                {
                    std::lock_guard<std::mutex> lock(consoleMutex);

                    // 先换行，结束当前输入行
                    std::cout << std::endl;

                    // 保存待发送内容，再清空输入缓冲区
                    messageToSend = message;
                    message.clear();
                }

                // 输入 exit 时退出
                if (messageToSend == "exit")
                {
                    break;
                }

                // 发送非空消息
                if (!messageToSend.empty())
                {
                    if (!chatClient.sendMessage(messageToSend.c_str()))
                    {
                        std::lock_guard<std::mutex> lock(consoleMutex);
                        std::cerr << "Failed to send message."
                            << std::endl;

                        exitCode = 1;
                        break;
                    }
                }

                // 只恢复一次输入提示
                {
                    std::lock_guard<std::mutex> lock(consoleMutex);
                    redrawInput(username, message);
                }
            }



            else if (ch == '\b')
            {
                std::lock_guard<std::mutex> lock(consoleMutex);

                if (!message.empty())
                {
                    message.pop_back();
                    redrawInput(username, message);
                }
            }
            else if (ch >= 32 && ch <= 126)
            {
                std::lock_guard<std::mutex> lock(consoleMutex);

                message.push_back(static_cast<char>(ch));

                redrawInput(username, message);
            }

        }
        else
        {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(20));
        }
    }

    if (receiveError.load())
    {
        exitCode = 1;
    }

    message.clear();

    clientClosing.store(true);

    shutdown(chatClient.clientSocket, SD_BOTH);

    if (receiveThread.joinable())
    {
        receiveThread.join();
    }

    chatClient.cleanup();
    std::cout << "Client exited." << std::endl;
    return exitCode;
}