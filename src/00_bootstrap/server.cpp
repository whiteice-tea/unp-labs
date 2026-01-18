#include "common.hpp"
#include <iostream>

static sockaddr_in make_addr(const char* ip, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr.sin_addr) != 1) die("inet_pton(server)");
    return addr;
}

int main(int argc, char** argv) {
    NetInit net;

    if (argc != 3) {
        std::cerr << "Usage: tcp_server <ip> <port>\n";
        return 2;
    }
    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    Socket listen_sock(::socket(AF_INET, SOCK_STREAM, 0));
#ifdef _WIN32
    if (listen_sock.get() == INVALID_SOCKET) die("socket");
#else
    if (listen_sock.get() < 0) die("socket");
#endif

    int yes = 1;
#ifdef _WIN32
    if (::setsockopt(listen_sock.get(), SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes)) == SOCKET_ERROR)
        die("setsockopt(SO_REUSEADDR)");
#else
    if (::setsockopt(listen_sock.get(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        die("setsockopt(SO_REUSEADDR)");
#endif

    sockaddr_in addr = make_addr(ip, port);
#ifdef _WIN32
    if (::bind(listen_sock.get(), (sockaddr*)&addr, (int)sizeof(addr)) == SOCKET_ERROR) die("bind");
    if (::listen(listen_sock.get(), 128) == SOCKET_ERROR) die("listen");
#else
    if (::bind(listen_sock.get(), (sockaddr*)&addr, sizeof(addr)) < 0) die("bind");
    if (::listen(listen_sock.get(), 128) < 0) die("listen");
#endif

    std::cerr << "[server] listening on " << ip << ":" << port << "\n";

    for (;;) {
        sockaddr_in cli{};
#ifdef _WIN32
        int len = sizeof(cli);
        SOCKET c = ::accept(listen_sock.get(), (sockaddr*)&cli, &len);
        if (c == INVALID_SOCKET) die("accept");
        Socket client_sock(c);
#else
        socklen_t len = sizeof(cli);
        int c = ::accept(listen_sock.get(), (sockaddr*)&cli, &len);
        if (c < 0) die("accept");
        Socket client_sock(c);
#endif

        char cip[INET_ADDRSTRLEN]{};
        ::inet_ntop(AF_INET, &cli.sin_addr, cip, sizeof(cip));
        std::cerr << "[server] client " << cip << ":" << ntohs(cli.sin_port) << " connected\n";

        char buf[4096];
        for (;;) {
#ifdef _WIN32
            int n = ::recv(client_sock.get(), buf, (int)sizeof(buf), 0);
            if (n == 0) { std::cerr << "[server] client disconnected\n"; break; }
            if (n == SOCKET_ERROR) die("recv");

            int off = 0;
            while (off < n) {
                int m = ::send(client_sock.get(), buf + off, n - off, 0);
                if (m == SOCKET_ERROR) die("send");
                off += m;
            }
#else
            ssize_t n = ::recv(client_sock.get(), buf, sizeof(buf), 0);
            if (n == 0) { std::cerr << "[server] client disconnected\n"; break; }
            if (n < 0) die("recv");

            ssize_t off = 0;
            while (off < n) {
                ssize_t m = ::send(client_sock.get(), buf + off, n - off, 0);
                if (m < 0) die("send");
                off += m;
            }
#endif
        }
    }
}
