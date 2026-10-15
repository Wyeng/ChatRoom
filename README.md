# ChatRoom - C++ TCP 聊天室

## 项目简介

这是一个基于 C++ 和 TCP Socket 的控制台聊天室项目，使用 CMake 组织构建，基于 Windows Winsock 网络编程接口实现客户端与服务端通信。

本项目用于学习 C++ 网络编程、TCP 通信以及客户端/服务端架构，并在后续逐步完善多客户端聊天功能。

## 技术栈

- **编程语言：** C++
- **网络协议：** TCP
- **网络接口：** Windows Winsock2
- **构建工具：** CMake
- **开发环境：** Windows、Visual Studio

## 项目结构

```text
ChatRoom/
├── CMakeLists.txt
├── .gitignore
├── README.md
├── ChatRoom.h
├── Client/
│   ├── main.cpp
│   ├── ChatClient.h
│   └── ChatClient.cpp
└── Server/
    ├── main.cpp
    ├── ChatServer.h
    └── ChatServer.cpp
```

## 已实现功能

- [x] Winsock 初始化与清理
- [x] 创建 TCP Socket
- [x] 服务端绑定端口并监听连接
- [x] 客户端连接服务端
- [x] 客户端发送消息
- [x] 服务端接收并打印消息
- [ ] 支持用户输入聊天消息
- [ ] 支持多客户端连接
- [ ] 实现客户端之间的消息转发
- [ ] 使用多线程处理客户端
- [ ] 完善异常处理与连接管理

## 构建与运行

### 环境要求

- Windows
- 支持 C++17 的编译器
- CMake 3.20 或更高版本

### 构建项目

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build --config Debug
```

### 运行项目

使用 Visual Studio 或终端分别启动服务端和客户端。

先启动服务端，再启动客户端。

服务端默认监听 TCP 端口 `8888`，客户端连接本机地址 `127.0.0.1:8888`。

## 学习目标

通过逐步开发本项目，学习：

- TCP Socket 通信流程
- C++ 类与资源管理
- STL 容器在连接管理中的应用
- 多线程与互斥锁
- CMake 项目组织与构建

## 项目状态

当前处于基础 TCP 通信阶段，已实现客户端与服务端之间的一次消息发送和接收。多客户端管理及消息转发功能仍在开发中。
