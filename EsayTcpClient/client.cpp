// client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define WIN32_LEAN_AND_MEAN // macro to avoid including duplicate macro when include <windows.h> and <WinSock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <windows.h>  // windows system api
#include <WinSock2.h> // windows socket api 


enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR
};

struct DataHeader {
    short length;
    short cmd;
};

struct Login : public DataHeader {
    Login() {
        length = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    char userName[32];
    char password[32];
};

struct LoginRet : public DataHeader {
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

int main()
{
    // launch windows socket 2.x environment
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;

    // initiates use of the Winsock DLL by program.
    WSAStartup(ver, &dat);

    // 1. build socket
    SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);
    if (INVALID_SOCKET == _sock) {
        std::cout << "socket create failed" << std::endl;
    }
    else {
        std::cout << "socket create succeed" << std::endl;
    }

    // 2. connect server
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567); 
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    int ret = connect(_sock,(sockaddr*)&_sin, sizeof(sockaddr_in));

    if (SOCKET_ERROR == ret) {
        std::cout << "server connect failed" << std::endl;
    }
    else {
        std::cout << "server connect succeed" << std::endl;
    }
    
    // 3.process user input request
    char cmdBuf[128] = {};
    while (true) {
        std::cout << ">> ";
        std::cin >> cmdBuf;
        if (strcmp(cmdBuf, "exit") == 0) {
            // client exit
            break;
        } else if  (strcmp(cmdBuf, "login") == 0) {
            Login login;
            strcpy_s(login.userName, "account");
            strcpy_s(login.password, "password");

            send(_sock, (char*)&login, sizeof(Login), 0);

            LoginRet loginRet= {};            
            recv(_sock, (char*)&loginRet, sizeof(LoginRet), 0);
            std::cout << "Received message from server: ";
            std::cout << "Login result : " << loginRet.result << std::endl;

        } else if (strcmp(cmdBuf, "logout") == 0) {
            Logout logout = {};
            strcpy_s(logout.userName, "account");
            send(_sock, (char*)&logout, sizeof(Logout), 0);

            LogoutRet logoutRet = {};
            recv(_sock, (char*)&logoutRet, sizeof(LogoutRet), 0);
            std::cout << "Received message from server: ";
            std::cout << "Logout result : " << logoutRet.result << std::endl;
        }
        else {
            // not a valid command, listen next command
            std::cout << "Not valid command" << std::endl;
            continue;
        }
    }

    // 5. close socket 
    closesocket(_sock);
    WSACleanup();

    /*std::cout << "exit" << std::endl;
    std::cout << "press Enter to exit" << std::endl;
    getchar();*/
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
