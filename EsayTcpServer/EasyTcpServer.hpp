#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN // macro to avoid including duplicate macro when include <windows.h> and <WinSock2.h>
#   include <windows.h>  // windows system api
#   include <WinSock2.h> // windows socket api 
#else
#   include <unistd.h> // unix standard system interface
#   include <arpa/inet.h>

#   define SOCKET int
#   define INVALID_SOCKET  (SOCKET)(~0)
#   define SOCKET_ERROR            (-1)
#endif

#include <iostream>
#include <vector>
#include <string>
#include "Message.hpp"


std::vector<std::string> allCommands = { "CMD_LOGIN",
										"CMD_LOGIN_RESULT",
										"CMD_LOGOUT",
										"CMD_LOGOUT_RESULT",
										"CMD_NEW_USER_JOIN",
										"CMD_ERROR" };

class EasyTcpServer
{
public:
	EasyTcpServer() :_sock{ INVALID_SOCKET } {}

	// initialize server socket
	SOCKET initSocket() {
#		ifdef _WIN32
			// launch windows socket 2.x environment
			WORD ver = MAKEWORD(2, 2);
			WSADATA dat;

			// initiates use of the Winsock DLL by program.
			WSAStartup(ver, &dat);
#		endif

		// 1.build a socket
		// first argument: address family
		// second: socket type 
		// third: protocol type
		// socket has been created, close it and create an new one
		if (INVALID_SOCKET != _sock) {
			std::cout << "socket: " << _sock << " has been created previously, close it and recreate an new socket" << std::endl;
			closeSock();
		}

		// 1. build socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			std::cout << "socket create failed" << std::endl;
			return -1;
		}
		else {
			std::cout << "socket create succeed" << std::endl;
		}

		// return socket number
		return _sock;
	}

	// bind ip and port
	int bindPort(const char* ip, unsigned short port) {
		if (INVALID_SOCKET == _sock) {
			std::cout << "server socket did not created, create socket automatically instead" << std::endl;
			initSocket();
		}

		// same struct compared with the standard argument type of bind function, we use type conversion to convert its type
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;

		// host to net unsigned short (from int to ushort)
		_sin.sin_port = htons(port);

		// bind ip address ,
		// if use 127.0.0.1, the socket will listen for incoming connections only on the loopback interface using that specific IP address
		// if use ip address of LAN, socket will be bound to that address with specficied port, and can listen to any packets from LAN or public network
		//inet_addr("127.0.0.1");
#		ifdef _WIN32
			if (ip) {
				_sin.sin_addr.S_un.S_addr = inet_addr(ip);
			}
			else {
				_sin.sin_addr.S_un.S_addr = INADDR_ANY;
			}
#		else
			if (ip) {
				_sin.sin_addr.s_addr = inet_addr(ip);
			}
			else {
				_sin.sin_addr.s_addr = INADDR_ANY;
			}
#		endif

		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));

		if (SOCKET_ERROR == ret) {
			std::cout << "ERROR, cannot bind to port: " << port << std::endl;
		}
		else {
			std::cout << "Successfully bind to port: " << port << std::endl;
		}

		return ret;
		 
	}

	// defines the maximum length to the queue of pending connections
	int listenNumber(int n) {
		int ret = listen(_sock, n);

		if (SOCKET_ERROR == ret) {
			std::cout << "ERROR, socket " << _sock << " listen to port failed" << std::endl;
		}
		else {
			std::cout << "Socket: " << _sock << " listen to port successully" << std::endl;
		}

		return ret;
	}

	// accept client connection
	SOCKET acceptClient() {
		// 4. wait until accept an new client connection
		  // The accept function fills this structure with the address information of the client that is connecting.
		sockaddr_in clientAddr = {};

		// After the function call, it will be updated with the actual size of the client's address information.
		int nAddrLen = sizeof(clientAddr);

		SOCKET _cSock = INVALID_SOCKET;

#		ifdef _WIN32
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#		else
			_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#		endif

		if (INVALID_SOCKET == _cSock) {
			std::cout << "Invalid Socket " << _sock << " accepted" << std::endl;
		}
		else {
			// send message to all client that there is an new client connected to server
			NewUserJoin client;
			client.cSocket = _cSock;
			broadcastMessage(&client);

			clients_list.push_back(_cSock);

			// ion converts an (Ipv4) Internet network address into an ASCII string in Internet standard dotted-decimal format
			std::cout << "Server socket " << _sock << " accept new client connection: client socket = " << _cSock << ", ip = " << inet_ntoa(clientAddr.sin_addr) << std::endl;
		} 

		return _cSock;
	}

	// close socket
	void closeSock() {
		if (_sock == INVALID_SOCKET) {
			return;
		}

#		ifdef _WIN32
			// close all client sockets
			for (int n = 0; n < (int)clients_list.size(); n++) {
				closesocket(clients_list[n]);
			}
			// close server socket
			closesocket(_sock);
			// terminates use of the Winsock 2 DLL (Ws2_32.dll)
			WSACleanup();
#		else
			for (int n = 0; n < (int)clients_list.size(); n++) {
				close(clients_list[n]);
			}
			close(_sock);
#		endif
	}

	// check if socket is created
	bool isRun() {
		return _sock != INVALID_SOCKET;
	}

	// listen client message
	bool listenClient() {
		if (!isRun()) {
			return false;
		}

		// fd_set: to place sockets into a "set" for various purposes, such as testing a given socket for readability using the readfds parameter of the select function
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;

		// reset the count of each set to zero
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		// add socket _sock to each set to be monitor later when using select()
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		// record the maximum number of fd in all scokets
		SOCKET maxSock = _sock;

		for (int n = (int)clients_list.size() - 1; n >= 0; n--) {
			FD_SET(clients_list[n], &fdRead);
			if (maxSock < clients_list[n]) maxSock = clients_list[n];
		}

		// setup time stamp to listen client connection, which means, 
		// our server is non-blocking, and can process other request while listening to client socket
		// set time stamp to 1 second to check if there are any connections from client
		timeval t = { 1, 0 };

		// fisrt arg: ignore, the nfds parameter is included only for compatibility with Berkeley sockets.
		// last arg is timeout: The maximum time for select to wait for checking status of sockets
		// allow a program to monitor multiple file descriptors, waiting until one or more of the file descriptors become "ready" for some class of I/O operation
		// when select find status of sockets change, it would clear all sockets and reload the sockets which has changed the status

		// drawback of select function: maximum size of fdset is 64, which means, there can be at most 64 clients connected to server

		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);

		// error happens when return value less than 0
		if (ret < 0) {
			std::cout << "=================" << std::endl;
			std::cout << "Exception happens" << std::endl;
			std::cout << "=================" << std::endl;
			closeSock();
			return false;
		}

		// the status of server socket is changed, which is, a client is trying to connect with server 
		// create an new socket for current client and add it into client list
		if (FD_ISSET(_sock, &fdRead)) {
			// Clears the bit for the file descriptor fd in the file descriptor set fdRead, so that we can .
			FD_CLR(_sock, &fdRead);

			// accept connection from client
			acceptClient();
		}

		bool stillRun = true;

		// loop through all client sockets to process command
		for (int n = (int)clients_list.size() - 1; n >= 0; n--) {
			if (FD_ISSET(clients_list[n], &fdRead)) {
				// connected client socket has been closed
				if (receiveClientMessage(clients_list[n]) == -1) {
					// find cloesd client socket and remove
					std::cout << "Client " << clients_list[n] << " exit" << std::endl;
					auto iter = clients_list.begin() + n;
					//auto iter = clients_list.begin() + n;
					if (iter != clients_list.end()) {
						// TODO: problem, should close socket before remove it 
						clients_list.erase(iter);
					}

					if (clients_list.size() == 0) {
						std::cout << "No client connected to server,\nDo you want to shut down the server ? type YES or NO" << std::endl;
						char command[12];
						std::cin >> command;
						if (strcmp(command, "YES") == 0) {
							stillRun = false;
						}
					}
				}

			}
		}

		return stillRun;
		//std::cout << "Server is idle and able to deal with other tasks" << std::endl;
	
	}

	// receive client message, solve message concatenation
	int receiveClientMessage(SOCKET _cSock) {

		// buffer for receiving data, this is still a fixed length buffer
		char szRecv[1024] = {};

		// 5. keeping reading message from clients
		//we only read header info from the incoming message
		int nLen = (int)recv(_cSock, (char*)szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;

		if (nLen <= 0) {
			// connection has closed
			return -1;
		}

		recv(_cSock, szRecv + sizeof(DataHeader), header->length - sizeof(DataHeader), 0);

		processClientMessage(_cSock,header);

		return 0;
	}

	// response client message, there can be different ways of processing messages in different kinds of server
	// we use virutal to for inheritance
	virtual void processClientMessage(SOCKET _cSock,DataHeader* header) {
		// 6.process client request and send data to client
		switch (header->cmd) {
			case CMD_LOGIN: {
				// modify pointer to points to the member of subclass, since the member of parent class
				// would be initialized before those of subclass
				// at the same time, reduce the amount of data we need to read
				Login* login = (Login*)header;
				std::cout << "Received message from client: " << allCommands[login->cmd] << " message length: " << login->length << std::endl;
				std::cout << "User: " << login->userName << " Password: " << login->password << std::endl;

				LoginRet ret;
				// TODO: needs account validation
				// return header file before sending data
				send(_cSock, (char*)&ret, sizeof(LoginRet), 0);
				break;
			}
			case CMD_LOGOUT: {
				Logout* logout = (Logout*)header;
				std::cout << "Received message from client: " << allCommands[logout->cmd] << " message length: " << logout->length << std::endl;
				std::cout << "User: " << logout->userName << std::endl;
				// TODO: needs account validation
				LogoutRet ret;
				send(_cSock, (char*)&ret, sizeof(LogoutRet), 0);
				break;
			}
			default: {
				header->length = 0;
				header->cmd = CMD_ERROR;
				send(_cSock, (char*)&header, sizeof(DataHeader), 0);
				break;
			}
		}
	}

	// send message to client
	int sendMessage(SOCKET _cSock, DataHeader* header) {
		if (isRun() && header) {
			send(_cSock, (const char*)header, header->length, 0);
		}

		return SOCKET_ERROR;
	}

	// broadcast message to all users in server
	void broadcastMessage(DataHeader* header) {
		if (isRun() && header) {
			for (int n = (int)clients_list.size() - 1; n >= 0; n--) {
				sendMessage(clients_list[n], header);
			}
		}
	}

	virtual ~EasyTcpServer(){
		closeSock();
	}

private:
	// server socket
	SOCKET _sock;

	// all client sockets connected with server
	std::vector<SOCKET> clients_list;

};

#endif // !_EsayTcpServer_hpp
