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

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
