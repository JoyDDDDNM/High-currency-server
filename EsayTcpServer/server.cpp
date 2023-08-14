// server.cpp : This file contains the 'main' function. Program execution begins and ends there.

// TODO: accept command line argument to set up port number

#define WIN32_LEAN_AND_MEAN // macro to avoid including duplicate macro when include <windows.h> and <WinSock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <windows.h>  // windows system api
#include <WinSock2.h> // windows socket api 
#include <vector>
#include <string>

enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};

std::vector<std::string> allCommands = { "CMD_LOGIN",
                                        "CMD_LOGIN_RESULT",
                                        "CMD_LOGOUT",
                                        "CMD_LOGOUT_RESULT",
                                        "CMD_NEW_USER_JOIN",
                                        "CMD_ERROR"};

struct DataHeader {
    short length; 
    short cmd;
};

struct Login: public DataHeader {
    Login () {
        length = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    char userName[32];
    char password[32];
};

struct LoginRet: public DataHeader {
    LoginRet() {
        length = sizeof(LoginRet);
        cmd = CMD_LOGIN_RESULT;
        result = 0;
    }
    int result;
};

struct Logout : public DataHeader {
    Logout() {
        length = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }
    char userName[32];
};

struct LogoutRet : public DataHeader {
    LogoutRet() {
        length = sizeof(LogoutRet);
        cmd = CMD_LOGOUT_RESULT;
        result = 0;
    }
    int result;
};

struct NewUserJoin : public DataHeader {
    NewUserJoin() {
        length = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        cSocket = 0;
    }
    int cSocket;
};

int processClient(SOCKET _cSock) {

    // buffer for receiving data, this is still a fixed length buffer
    char szRecv[1024] = {};

    // 5. keeping reading message from clients
    //we only read header info from the incoming message
    int nLen = recv(_cSock, (char*)szRecv, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szRecv;
    if (nLen <= 0) {
        // connection has closed
        return -1;
    }

    //if (nLen >= header->length) {};
    // 6.process client request and send data to client
    switch (header->cmd) {
        case CMD_LOGIN: {
            // modify pointer to points to the member of subclass, since the member of parent class
            // would be initialized before those of subclass
            // at the same time, reduce the amount of data we need to read
            recv(_cSock, szRecv + sizeof(DataHeader), header->length - sizeof(DataHeader), 0);
            Login* login = (Login*)szRecv;
            std::cout << "Received message from client: " << allCommands[login->cmd] << " message length: " << login->length << std::endl;
            std::cout << "User: " << login->userName << " Password: " << login->password << std::endl;

            LoginRet ret;
            // TODO: needs account validation
            // return header file before sending data
            send(_cSock, (char*)&ret, sizeof(LoginRet), 0);

            break;
        }
        case CMD_LOGOUT: {
            recv(_cSock, szRecv + sizeof(DataHeader), header->length - sizeof(DataHeader), 0);
            Logout* logout = (Logout*)szRecv;
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

    return 0;
}

std::vector<SOCKET> clients_list = {};

int main() {
    // launch windows socket 2.x environment
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    
    // initiates use of the Winsock DLL by program.
    WSAStartup(ver, &dat);

    // 1.build a socket
    // first argument: address family
    // second: socket type
    // third: protocol type
    SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (INVALID_SOCKET == _sock) {
        std::cout << "soecket create failed" << std::endl;
    }

    // 2.bind network port
    // same struct compared with the standard argument type of bind function, we use type conversion to convert its type
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;

    // host to net unsigned short (from int to ushort)
    _sin.sin_port = htons(4567);
    
    // bind ip address ,
    // if use 127.0.0.1, the socket will listen for incoming connections only on the loopback interface using that specific IP address
    // if use ip address of LAN, socket will be bound to that address with specficied port, and can listen to any packets from LAN or public network
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
    //inet_addr("127.0.0.1");
    
    // 3. listen port 
    // determine if we bind port successfully
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
        std::cout << "ERROR, cannot bind to the specified port number" << std::endl;
    }
    else {
        std::cout << "Successfully bind to the specified port" << std::endl;
    }
    
    // setup the length of queue for pending connection to 5
    if (SOCKET_ERROR == listen(_sock, 5)) {
        std::cout << "ERROR, listen to port failed" << std::endl;
    }
    else {
        std::cout << "Listen to port successully" << std::endl;
    }

    while (true) {
        
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

        for (int n = (int)clients_list.size() - 1; n >= 0; n--) {
            FD_SET(clients_list[n], &fdRead);
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
        int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);

        // error happens when return value less than 0
        if (ret < 0) {
            std::cout << "=================" << std::endl;
            std::cout << "Exception happens" << std::endl;
            std::cout << "=================" << std::endl;
            break;
        }

        // the status of server socket is changed, which is, a client is trying to connect with server 
        // create an new socket for current client and add it into client list
        if (FD_ISSET(_sock, &fdRead)) {
            // Clears the bit for the file descriptor fd in the file descriptor set fdRead, so that we can .
            FD_CLR(_sock, &fdRead);

            // 4. wait until accept an new client connection
            // The accept function fills this structure with the address information of the client that is connecting.
            sockaddr_in clientAddr = {};

            // After the function call, it will be updated with the actual size of the client's address information.
            int nAddrLen = sizeof(clientAddr);

            SOCKET _cSock = INVALID_SOCKET;

            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
            if (INVALID_SOCKET == _cSock) {
                std::cout << "Invalid Socket accepted" << std::endl;
            }
            else {
                // send message to all client that there is an new client connected to server
                for (int n = (int)clients_list.size() - 1; n >= 0; n--) {
                    NewUserJoin client;
                    client.cSocket = _cSock;
                    send(clients_list[n], (const char*)&client, sizeof(client), 0);
                }

                clients_list.push_back(_cSock);
                // ion converts an (Ipv4) Internet network address into an ASCII string in Internet standard dotted-decimal format
                std::cout << "New client connected: socket = " << _cSock << ", ip = " << inet_ntoa(clientAddr.sin_addr) << std::endl;
            }
        }

        bool isClosed = false;
        // loop through all client sockets to process command
        for (int n = 0; n < (int)fdRead.fd_count; n++) {
            // connected client socket has been closed
            if (processClient(fdRead.fd_array[n]) == -1) {
                // find cloesd client socket and remove
                std::cout << "client " << fdRead.fd_array[n] << " exit" << std::endl;
                auto iter = find(clients_list.begin(), clients_list.end(), fdRead.fd_array[n]);
                if (iter != clients_list.end() ) {
                    // TODO: problem, should close socket before remove it 
                    clients_list.erase(iter);
                }
                 
                if (clients_list.size() == 0) {
                    std::cout << "No client connected to server,\nDo you want to shut down the server ? type YES or NO" << std::endl;
                    char command[12];
                    std::cin >> command;
                    if (strcmp(command, "YES") == 0) {
                        isClosed = true;
                    }
                }
            }
        }

        if (isClosed) break;

        //std::cout << "Server is idle and able to deal with other tasks" << std::endl;
    }

    // close all client sockets
    for (int n = 0 ; n < (int)clients_list.size(); n++) {
        closesocket(clients_list[n]);
    }

    // close server socket
    closesocket(_sock);

    // terminates use of the Winsock 2 DLL (Ws2_32.dll)
    WSACleanup();

    std::cout << "server closed" << std::endl;
    std::cout << "press Enter to exit" << std::endl;
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
