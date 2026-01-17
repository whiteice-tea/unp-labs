#include "common.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>
#include <string>

static sockaddr_in make_addr(const char* ip, uint16_t port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, ip, &addr.sin_addr) != 1) die("inet_pton(client)");
    return addr;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: tcp_client <ip> <port>\n";
        return 2;
    }
    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

    Fd fd(::socket(AF_INET, SOCK_STREAM, 0));
    if (!fd) die("socket");

    sockaddr_in addr = make_addr(ip, port);
    if (::connect(fd.get(), (sockaddr*)&addr, sizeof(addr)) < 0) die("connect");

    std::cerr << "[client] connected to " << ip << ":" << port << "\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        line.push_back('\n');

        size_t off = 0;
        while (off < line.size()) {
            ssize_t n = ::write(fd.get(), line.data() + off, line.size() - off);
            if (n < 0) die("write");
            off += (size_t)n;
        }

        char buf[4096];
        ssize_t r = ::read(fd.get(), buf, sizeof(buf));
        if (r == 0) { std::cerr << "[client] server closed\n"; break; }
        if (r < 0) die("read");
        std::cout.write(buf, r);
        std::cout.flush();
    }
}
