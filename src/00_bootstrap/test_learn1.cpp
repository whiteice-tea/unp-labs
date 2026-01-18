#include "common.hpp"
#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include <iostream>
#include <string>

int main(){
    std::cout<<"started"<<std::endl;
    WSADATA wsaData{};
    

    // WSAStartup作用：开启 Winsock 环境（Windows 上 socket API “开机”）。
    // 怎么用：WSAStartup(MAKEWORD(2,2), &wsaData)
    // MAKEWORD(2,2)：请求使用 Winsock 2.2
    // wsaData：系统写入一些信息（你暂时不用读）
    // 失败判断：返回值 ret != 0 就失败，这个 ret 本身就是错误码。
    int ret = WSAStartup(MAKEWORD(2,2),&wsaData);
    if(ret != 0){
        std::cerr<<"WSAStartup failed with error: "<<ret<<std::endl;
        return 1;
    }

    std::cout<<"WSAStartup success"<<std::endl;


    SOCKET sock =socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    

    // socket作用：创建一个通信端点（你可以理解为“网络句柄”）。
    // 参数：
    // AF_INET：IPv4地址族
    // SOCK_STREAM：TCP（面向连接的字节流）
    // IPPROTO_TCP：明确 TCP 协议
    // 失败判断：返回 INVALID_SOCKET，再用 WSAGetLastError() 查错误码。
    
    if(sock == INVALID_SOCKET){
        std::cerr<<"socket failed with error: "<<WSAGetLastError()<<std::endl;
        WSACleanup();
        return 1;
    }

    std::cout<<"socket created"<<std::endl;


    const char* serverIp="127.0.0.1";
    unsigned short serverPort=9000;

    sockaddr_in serverAddr{};
    serverAddr.sin_family=AF_INET; // IPv4
    serverAddr.sin_port=htons(serverPort); 

    int pton_rc = InetPtonA(AF_INET,serverIp,&serverAddr.sin_addr);
    
    // InetPtonA作用：把 "127.0.0.1" 这种字符串 IP 转成二进制写入 serverAddr.sin_addr。
    // 返回值：
    // 1 成功
    // 0 IP 字符串格式不对
    // -1 调用错误（看 WSAGetLastError()）

    if(pton_rc !=1){
        std::cerr<<"InetPtonA failed,rc="<<pton_rc<<", error: "<<WSAGetLastError()<<std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    int crc = connect(sock,reinterpret_cast<sockaddr*>(&serverAddr),sizeof(serverAddr));
    
    // connect作用：主动连接到服务器的 IP:port。
    // 成功意义：这一刻，server 端的 accept() 就会立刻返回，拿到 clientSock。
    // 常见错误码：
    // 10061：连接被拒绝（server 没开 / 端口不对 / server 没 listen）
    // 10060：超时（网络不可达、防火墙等）
    if(crc == SOCKET_ERROR){
        std::cerr<<"connect failed with error: "<<WSAGetLastError()<<std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout<<"connected to "<<serverIp<<":"<<serverPort<<std::endl;



    std::cout << "Type a line and press Enter (type 'exit' to quit)\n";

    while(true){
        std::string line;
        if(!std::getline(std::cin,line)){
            break;
        }
        if(line == "exit"){
            std::cout<<"exiting"<<std::endl;
            break;
        }

        int sent = send(sock,line.data(),static_cast<int>(line.size()),0);
        if(sent == SOCKET_ERROR){
            std::cerr<<"send failed with error: "<<WSAGetLastError()<<std::endl;
            break;
        }

        char buf[4096];
        int n = recv(sock,buf,static_cast<int>(sizeof(buf)),0);
        if(n=0){
            std::cout<<"server disconnected"<<std::endl;
            break;
        }
        if(n == SOCKET_ERROR){
            std::cerr<<"recv failed with error: "<<WSAGetLastError()<<std::endl;
            break;
        }

        std::cout<<"[client] echo:";
        std::cout.write(buf,n);
        std::cout<<std::endl;


    }



    closesocket(sock);
    WSACleanup();
    std::cout<<"[client] shutdown"<<std::endl;
    return 0;
}