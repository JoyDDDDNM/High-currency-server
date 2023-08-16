// client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// compile command in UNIX-like environment:
// g++ client.cpp -std=c++11 -pthread -o client

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "EsayTcpClient.hpp"
#include <thread>

void cmdThread(EasyTcpClient* client) {
    while (true) {
        char cmdBuf[256] = {};
        std::cin >> cmdBuf;
        if (strcmp(cmdBuf, "exit") == 0) {
            std::cout << "sub thread finished" << std::endl;
            // tell main thread that the sub thread is finished
            client->closeSock();
            break;
        }
        else if (strcmp(cmdBuf, "login") == 0) {
            Login login;
            strcpy(login.userName, "account");
            strcpy(login.password, "password");
            client->sendMessage(&login);
        }
        else if (strcmp(cmdBuf, "logout") == 0) {
            Logout logout;
            strcpy(logout.userName, "account");
            client->sendMessage(&logout);
        }
        else {
            std::cout << "not valid command" << std::endl;
        }
    }
}

int main()
{
    EasyTcpClient client;

    client.initSocket();

    client.connectServer("127.0.0.1",4567);

    // we can add more clients to connnect different servers or ports
    // EasyTcpClient client2
    // client2.initSocket
    // client2.connectServer("127.0.0.1", 4567);
    // std::thread  clientCmdThread2(cmdThread, &client2);

    // create an thread for reading client input
    std::thread  clientCmdThread(cmdThread,  &client);
    
    // Separates the thread of execution from the thread object, 
    // allowing execution to continue independently
    clientCmdThread.detach();

    // 3.process user input request
    while (client.isRun()) {
        client.listenServer();
       
        //std::cout << "Client is idle and able to deal with other tasks" << std::endl;
    }

    client.closeSock();

    std::cout << "Client exit" << std::endl;
    std::cout << "Press Enter to exit" << std::endl;
    getchar();
    return 0;
}
