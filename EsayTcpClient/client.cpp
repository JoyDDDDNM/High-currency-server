// client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// compile command in UNIX-like environment:
// g++ client.cpp -std=c++11 -pthread -o client

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "EsayTcpClient.hpp"
#include <thread>

// number of threads
const int tCount = 4;

// number of clients per thread
const int cCount = 1000;

// clients array, it needs to be global so that each client in child thread can access it
EasyTcpClient* clients[cCount];

void childThread(int id) {

	int begin = (id - 1) * (cCount / tCount);
	int end = (id) * (cCount / tCount);

	for (int n = begin; n < end; n++) {
		if (!isRun) return;
		clients[n] = new EasyTcpClient();
	}

	for (int n = begin; n < end; n++) {
		if (!isRun) return;
		clients[n]->initSocket();
		clients[n]->connectServer("127.0.0.1", 4567);
	}

	Login login;
	strcpy(login.userName, "account");
	strcpy(login.password, "password");

	while (isRun) {
		// listen message from server
		//client.listenServer();

		//client.sendMessage(&login);
		//std::cout << "Client is idle and able to deal with other tasks" << std::endl;
		for (int n = begin; n < end; n++) {
			clients[n]->sendMessage(&login);
		}

	}

	for (int n = begin; n < end; n++) {
		clients[n]->closeSock();
	}
}

int main()
{	
	// UI thread: create an thread for reading client input
	std::thread clientCmdThread(cmdThread);

	// Separates the thread of execution from the thread object, 
	// allowing execution to continue independently
	clientCmdThread.detach();

	// initialize threads
	for (int n = 0; n < tCount; n++) {
		std::thread t1(childThread, n+1);
		t1.detach();
	}

	while (isRun) continue;

	std::cout << "Client exit" << std::endl;
	std::cout << "Press Enter to exit" << std::endl;
	getchar();
	return 0;
}
