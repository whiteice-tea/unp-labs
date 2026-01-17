#include "common.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>

static sockaddr_in make_addr(const char* ip, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr.sin_addr) != 1) die("inet_pton(server)");
    return addr;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: tcp_server <ip> <port>\n";
        return 2;
    }
    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    Fd listen_fd(::socket(AF_INET, SOCK_STREAM, 0));
    if (!listen_fd) die("socket");

    int yes = 1;
    if (::setsockopt(listen_fd.get(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        die("setsockopt(SO_REUSEADDR)");

    sockaddr_in addr = make_addr(ip, port);
    if (::bind(listen_fd.get(), (sockaddr*)&addr, sizeof(addr)) < 0) die("bind");
    if (::listen(listen_fd.get(), 128) < 0) die("listen");

    std::cerr << "[server] listening on " << ip << ":" << port << "\n";

    for (;;) {
        sockaddr_in cli{};
        socklen_t len = sizeof(cli);
        int cfd = ::accept(listen_fd.get(), (sockaddr*)&cli, &len);
        if (cfd < 0) die("accept");
        Fd client_fd(cfd);

        char cip[INET_ADDRSTRLEN]{};
        ::inet_ntop(AF_INET, &cli.sin_addr, cip, sizeof(cip));
        std::cerr << "[server] client " << cip << ":" << ntohs(cli.sin_port) << " connected\n";

        char buf[4096];
        for (;;) {
            ssize_t n = ::read(client_fd.get(), buf, sizeof(buf));
            if (n == 0) { std::cerr << "[server] client disconnected\n"; break; }
            if (n < 0) die("read");

            ssize_t off = 0;
            while (off < n) {
                ssize_t m = ::write(client_fd.get(), buf + off, n - off);
                if (m < 0) die("write");
                off += m;
            }
        }
    }
}
