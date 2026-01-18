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


int main() {
    std::cout<<"server started"<<std::endl;
    WSADATA wsaData{};

    int rser = WSAStartup(MAKEWORD(2,2),&wsaData);
    //作用：打开 Winsock 环境。你可以理解为“Windows 的网络 API 先上电初始化”。
    if(rser != 0){
        std::cerr<<"WSAStartup failed with error: "<<rser<<std::endl;
        return 1;
    }
    std::cout<<"WSAStartup success"<<std::endl;

    SOCKET sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //作用：创建一个套接字，用于网络通信。
    //AF_INET：地址族，表示 IPv4
    //SOCK_STREAM：套接字类型，表示面向连接的 TCP 套接字
    //IPPROTO_TCP：协议，表示使用 TCP 协议

    //检查 socket 是否创建成功
    if(sock == INVALID_SOCKET){
        std::cerr<<"socket failed with error: "<<WSAGetLastError()<<std::endl;
        WSACleanup();
        return 1;
    }
    std::cout<<"socket created"<<std::endl;

    const char* bindIp = "127.0.0.1";
    unsigned short bindPort = 9000; 

    sockaddr_in addr{};
    //作用：定义一个 IPv4 地址结构体，里面会放三件关键东西,地址族（IPv4 / IPv6）,端口,IP 地址
    addr.sin_family = AF_INET; //地址族,ipv4,IPv6 就是 AF_INET6
    addr.sin_port = htons(bindPort); //端口,注意网络字节序转换
    //把本机字节序转换成网络字节序（大端）

    int pton_rc = InetPtonA(AF_INET, bindIp, &addr.sin_addr);
    //作用：把点分十进制的 IP 地址字符串转换成二进制形式，存放在 addr.sin_addr 中

    if(pton_rc != 1){
        std::cerr<<"InetPtonA failed with error: "<<WSAGetLastError()<<std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    int brc = bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    //作用：把套接字和一个本地地址（IP 地址 + 端口）绑定在一起
    //这是 C/C++ socket API 的历史遗留：统一用 sockaddr* 接口

    if(brc == SOCKET_ERROR){
        std::cerr<<"bind failed with error: "<<WSAGetLastError()<<std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    std::cout<<"bind success"<<bindIp<<":"<<bindPort<<std::endl;

//int listen(SOCKET s, int backlog);
//s：你前面 socket() 创建、并且 bind() 成功的那个“监听套接字”
//backlog：允许排队等待 accept 的连接数量上限（简单理解）
// backlog = 排队区容量。
// 客户端 connect() 过来时：
// 如果 server 正忙、还没 accept() 接它
// 系统会让它先进入“等待队列”
// backlog 就是队列最多能塞多少个“等着被 accept 的连接”。
    int lrc = listen(sock, SOMAXCONN);
    //作用：把一个普通套接字变成“监听套接字”，准备接受连接请求
    //SOMAXCONN 是一个常量，表示允许的最大排队连接数,让系统帮你选一个“合理的最大排队长度,也可以写具体数字

    if(lrc == SOCKET_ERROR){
        std::cerr<<"listen failed with error: "<<WSAGetLastError()<<std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout<<"listening on "<<bindIp<<":"<<bindPort<<"waiting for connections..."<<std::endl;


    sockaddr_in clientAddr{};
    //这是一个“盒子”，用来让系统把**客户端的地址信息（IP+端口）**写进去。
    int clientLen = sizeof(clientAddr);
    //accept() 需要你告诉它：这个盒子有多大，方便它写入。
    SOCKET clientSock = accept(sock, reinterpret_cast<sockaddr*>(&clientAddr),&clientLen);
    //accept() 的作用
    // 它会阻塞等待（卡在这不动）直到真的有客户端连进来。
    // 一旦有连接，它会返回一个新的 socket：clientSock

    // listenSock(此代码中为sock)：还是“门口接待处”，用来继续接受未来的新连接
    //clientSock：是“你和这个客户端专属的通话线路”    
    if(clientSock == INVALID_SOCKET){
        std::cerr<<"accept failed with error: "<<WSAGetLastError()<<std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    std::cout<<"client connected"<<std::endl;

    std::cout<<"start echo loop"<<std::endl;
    while (true){
    // 服务器保持循环：不断接收客户端发来的数据，直到出错或对方关闭。
        char buf[4096];//准备一个缓冲区，存放收到的数据。4096 是常见大小
        int n = recv(clientSock, buf, sizeof(buf),0);
        //作用：从套接字接收数据
        // recv() 作用从 已经连接的 socket（clientSock） 里读数据。
        // 参数解释：
        // clientSock：你和这个客户端的专属连接（注意不是 listenSock）
        // buf：把数据写进这个数组
        // sizeof(buf)：最多读多少字节
        // 0：flags，常用写 0

        // 返回值：
        // n > 0：成功，读到了 n 个字节
        // n == 0：对方正常关闭连接（客户端退出了）
        // n == SOCKET_ERROR：出错，用 WSAGetLastError() 看原因

        if (n==0)  
        {
            std::cout<<"client disconnected"<<std::endl;
            break;
        }

        if(n== SOCKET_ERROR){
            std::cerr<<"recv failed with error: "<<WSAGetLastError()<<std::endl;
            break;
        }
        // 这里 buf[0..n-1] 是本次收到的字节
        std::cout<<"recv"<<n<<" bytes"<<std::endl;

        int sent = send(clientSock,buf,n,0);
        // send() 作用把数据发回给客户端（这就是 echo）。
        // 参数解释：
        // clientSock：同一条连接
        // buf：要发的数据起始地址
        // n：发多少字节（这里把刚收到的原样发回去）
        // 0：flags，flags 就是 send() 的“模式开关”，告诉系统：这次发送要不要用特殊规则。
        // 返回值：
        // 成功：返回实际发送的字节数
        // 失败：SOCKET_ERROR

        if(sent == SOCKET_ERROR){
            std::cerr<<"send failed with error: "<<WSAGetLastError()<<std::endl;
            break;
        }
        std::cout<<"sent "<<sent<<" bytes"<<std::endl;
        
    }
    
        closesocket(clientSock);
        closesocket(sock);
        WSACleanup();
        std::cout << "[server] shutdown\n";
        return 0;

    
    
    
    
    return 0;
}
