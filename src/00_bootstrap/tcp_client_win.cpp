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

// 其余代码保持不变
static sockaddr_in make_addr(const char* ip, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr.sin_addr) != 1) die("inet_pton(client)");
    return addr;
}

int main(int argc, char** argv) {
    NetInit net; // Windows 必须；Linux 无副作用

    if (argc != 3) {
        std::cerr << "Usage: tcp_client <ip> <port>\n";
        return 2;
    }
    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    Socket sock(::socket(AF_INET, SOCK_STREAM, 0));
#ifdef _WIN32
    if (sock.get() == INVALID_SOCKET) die("socket");
#else
    if (sock.get() < 0) die("socket");
#endif

    sockaddr_in addr = make_addr(ip, port);
#ifdef _WIN32
    if (::connect(sock.get(), (sockaddr*)&addr, (int)sizeof(addr)) == SOCKET_ERROR) die("connect");
#else
    if (::connect(sock.get(), (sockaddr*)&addr, sizeof(addr)) < 0) die("connect");
#endif

    std::cerr << "[client] connected to " << ip << ":" << port << "\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        line.push_back('\n');

        size_t off = 0;
        while (off < line.size()) {
#ifdef _WIN32
            int n = ::send(sock.get(), line.data() + (int)off, (int)(line.size() - off), 0);
            if (n == SOCKET_ERROR) die("send");
#else
            ssize_t n = ::send(sock.get(), line.data() + off, line.size() - off, 0);
            if (n < 0) die("send");
#endif
            off += (size_t)n;
        }

        char buf[4096];
#ifdef _WIN32
        int r = ::recv(sock.get(), buf, (int)sizeof(buf), 0);
        if (r == 0) { std::cerr << "[client] server closed\n"; break; }
        if (r == SOCKET_ERROR) die("recv");
        std::cout.write(buf, r);
#else
        ssize_t r = ::recv(sock.get(), buf, sizeof(buf), 0);
        if (r == 0) { std::cerr << "[client] server closed\n"; break; }
        if (r < 0) die("recv");
        std::cout.write(buf, r);
#endif
        std::cout.flush();
    }
}
