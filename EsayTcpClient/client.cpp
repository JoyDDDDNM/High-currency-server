// client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// compile command in UNIX-like environment:
// g++ client.cpp -std=c++11 -pthread -o client

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "EsayTcpClient.hpp"
#include <thread>

int main()
{
    EasyTcpClient client;

    client.initSocket();

    /*char ipAddr[20];
    std::cout << "Enter Ip for connection: ";

    std::cin >> ipAddr;*/

    client.connectServer("127.0.0.1", 4567);

    // we can add more clients to connnect different servers or ports
    // EasyTcpClient client2
    // client2.initSocket
    // client2.connectServer("127.0.0.1", 4567);
    // std::thread  clientCmdThread2(cmdThread, &client2);

    // create an thread for reading client input
    std::thread clientCmdThread(cmdThread,  &client);
    
    // Separates the thread of execution from the thread object, 
    // allowing execution to continue independently
    clientCmdThread.detach();

    Login login;
    strcpy(login.userName, "account");
    strcpy(login.password, "password");

    // 3.process user input request
    while (client.isRun()) {
        client.listenServer();
        //client.sendMessage(&login);
        //std::cout << "Client is idle and able to deal with other tasks" << std::endl;
    }

    //client.closeSock();

    std::cout << "Client exit" << std::endl;
    std::cout << "Press Enter to exit" << std::endl;
    getchar();
    return 0;
}
