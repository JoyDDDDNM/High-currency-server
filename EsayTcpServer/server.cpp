// server.cpp : This file contains the 'main' function. Program execution begins and ends there.

// TODO: accept command line argument to set up port number

#define WIN32_LEAN_AND_MEAN // macro to avoid including duplicate macro when include <windows.h> and <WinSock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <windows.h>  // windows system api
#include <WinSock2.h> // windows socket api 

struct DataPackage {
    int age;
    char name[32];
};

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
        std::cout << "successfully bind to the specified port" << std::endl;
    }
    
    // setup the length of queue for pending connection to 5
    if (SOCKET_ERROR == listen(_sock, 5)) {
        std::cout << "ERROR, listen to port failed" << std::endl;
    }
    else {
        std::cout << "listen to port successully" << std::endl;
    }


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
        
    // ion converts an (Ipv4) Internet network address into an ASCII string in Internet standard dotted-decimal format
    std::cout << "new client connected: socket = " << _cSock << ", ip = " << inet_ntoa(clientAddr.sin_addr) << std::endl;

    // tell client connection succeeded
    // send(_cSock,msgBuf,strlen(msgBuf)+1,0);

    char _recvBuf[128] = {};
    DataPackage dp = {};
    while (true) {
        // 5. keeping reading message from clients
        int nLen = recv(_cSock,_recvBuf,128,0);
        if (nLen <= 0) {
            break;
        }
        
        // 6.process client request and send data to client
        // strlen doesn't count the trailiing '\0'
        std::cout << "received message from client: " << _recvBuf << std::endl;
        if (strcmp(_recvBuf, "getInfo") == 0) {
            // using strcpy() to a buffer which is not large enough to contain it,
            // it will cause a buffer overflow. strcpy_s() is a security enhanced 
            // version of strcpy() 
            dp = {80,"rain"};
        }
        else {
            dp = {10000,"not valid"};            
        }

        send(_cSock, (const char*)&dp, sizeof(DataPackage), 0);

        
    }

    // 7. close socket
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
