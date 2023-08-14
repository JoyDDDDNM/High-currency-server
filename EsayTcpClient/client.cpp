// client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define WIN32_LEAN_AND_MEAN // macro to avoid including duplicate macro when include <windows.h> and <WinSock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <windows.h>  // windows system api
#include <WinSock2.h> // windows socket api 
#include <thread>
#include <vector>

enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
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

struct NewUserJoin : public DataHeader {
    NewUserJoin() {
        length = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        cSocket = 0;
    }
    int cSocket;
};


int receiveServerMessage(SOCKET _cSock) {

    // buffer for receiving data, this is still a fixed length buffer
    char szRecv[1024] = {};

    // 5. keeping reading message from clients
    //we only read header info from the incoming message
    int nLen = recv(_cSock, (char*)szRecv, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szRecv;
    if (nLen <= 0) {
        // connection has closed
        std::cout << "server disconnected" << std::endl;
        return -1;
    }

    switch (header->cmd) {
        case CMD_LOGIN_RESULT: {
            recv(_cSock, szRecv + sizeof(DataHeader), header -> length - sizeof(DataHeader), 0);
            LoginRet* loginRet = (LoginRet*)szRecv;
            std::cout << "Receive message from server: login successfully" << std::endl;
            std::cout << "Message length: " << loginRet->length << std::endl;
            break;
        }
        case CMD_LOGOUT_RESULT: {
            recv(_cSock, szRecv + sizeof(DataHeader), header->length - sizeof(DataHeader), 0);
            LogoutRet* logoutRet = (LogoutRet*)szRecv;
            std::cout << "Receive message from server: logout successfully" << std::endl;
            std::cout << "Message length: " << logoutRet->length << std::endl;
            break;
        }
        case CMD_NEW_USER_JOIN: {
            recv(_cSock, szRecv + sizeof(DataHeader), header->length - sizeof(DataHeader), 0);
            NewUserJoin* newUser = (NewUserJoin*)szRecv;
            std::cout << "Receive message from server: new user join into server" << std::endl;
            std::cout << "user id: " << newUser -> cSocket << std::endl;
            std::cout << "Message length: " << newUser->length << std::endl;
            break;
        }
        default: {
       
            break;
        }
    }

    return 0;
}

bool isNotExit = true;

void cmdThread(SOCKET _sock) {
    while (true) {
        char cmdBuf[256] = {};
        if (strcmp(cmdBuf, "exit") == 0) {
            std::cout << "sub thread finished" << std::endl;
            // tell main thread that the sub thread is finished
            isNotExit = false;
            break;
        }
        else if (strcmp(cmdBuf, "login") == 0) {
            Login login;
            strcpy_s(login.userName, "account");
            strcpy_s(login.password, "password");
            send(_sock,(const char*)&login, sizeof(Login),0);
        }
        else if (strcmp(cmdBuf, "logout") == 0) {
            Logout logout;
            strcpy_s(logout.userName, "account");
            send(_sock, (const char*)&logout, sizeof(Logout), 0);
        }
        else {
            std::cout << "not valid command" << std::endl;
        }
    }
}

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
    // https://stackoverflow.com/questions/21099041/why-do-we-cast-sockaddr-in-to-sockaddr-when-calling-bind
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
    
    
    // create an thread for reading client input
    std::thread  clientCmdThread(cmdThread,_sock);
    
    // Separates the thread of execution from the thread object, 
    // allowing execution to continue independently
    clientCmdThread.detach();

    // 3.process user input request
    while (isNotExit) {

        fd_set fdRead;
        FD_ZERO(&fdRead);
        FD_SET(_sock, &fdRead);

        timeval t = { 1, 0 };

        // client is blocked
        int ret = select(_sock, &fdRead, 0, 0, &t);
        if (ret < 0) {
            std::cout << "server is closed " << std::endl;
            break;
        }

        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);

            if (receiveServerMessage(_sock) == -1) {
                break;
            };
        }

        std::cout << "Client is idle and able to deal with other tasks" << std::endl;
    }

    // 5. close socket 
    closesocket(_sock);
    WSACleanup();

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
