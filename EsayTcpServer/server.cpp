// server.cpp : This file contains the 'main' function. Program execution begins and ends there.
// g++ server.cpp -std=c++11 -o server

// TODO: accept command line argument to set up port number

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "EasyTcpServer.hpp"
#include "Message.hpp"

int main() {

    EasyTcpServer server;

    server.initSocket();

    server.bindPort(NULL,4567);

    server.listenNumber(5);

    while (server.isRun()) {
                
        server.listenClient();
    }

    server.closeSock();

    std::cout << "server closed" << std::endl;
    Sleep(1000);
    return 0;
}
