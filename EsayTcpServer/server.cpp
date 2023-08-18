// server.cpp : This file contains the 'main' function. Program execution begins and ends there.
// g++ server.cpp -std=c++11 -o server

// TODO: accept command line argument to set up port number

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "EasyTcpServer.hpp"
#include "Message.hpp"
#include <thread>

int main() {

    EasyTcpServer server;

    server.initSocket();

    server.bindPort(nullptr,4567);

    server.listenNumber(5);

    // create an thread for reading server input
    std::thread serverCmdThread(cmdThread);
    serverCmdThread.detach();

    while (server.isRun() && isRun) {
        if (!server.listenClient()) break;
    }

    server.closeSock();

    std::cout << "server closed" << std::endl;
    //Sleep(1000);
    return 0;
}
