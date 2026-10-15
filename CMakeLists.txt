
cmake_minimum_required(VERSION 3.20)

project(ChatRoom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 服务端
add_executable(ChatServer
    Server/serverMain.cpp
    Server/ChatServer.cpp
    Server/ChatServer.h
)

# 链接 Windows Socket 库
target_link_libraries(ChatServer PRIVATE Ws2_32)

# 客户端
add_executable(ChatClient
    Client/clientMain.cpp
    Client/ChatClient.cpp
    Client/ChatClient.h
)

target_link_libraries(ChatClient PRIVATE Ws2_32)
