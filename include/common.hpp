#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#ifdef _WIN32
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
  using socket_t = SOCKET;
  static constexpr socket_t kInvalidSocket = INVALID_SOCKET;

  inline int last_error() { return WSAGetLastError(); }

  [[noreturn]] inline void die(const std::string& msg) {
      int e = last_error();
      std::cerr << "[FATAL] " << msg << " | WSAGetLastError=" << e << "\n";
      std::exit(1);
  }

  inline void close_socket(socket_t s) {
      if (s != kInvalidSocket) ::closesocket(s);
  }

  struct NetInit {
      NetInit() {
          WSADATA wsa{};
          if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) die("WSAStartup");
      }
      ~NetInit() { WSACleanup(); }
      NetInit(const NetInit&) = delete;
      NetInit& operator=(const NetInit&) = delete;
  };

#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #include <errno.h>

  using socket_t = int;
  static constexpr socket_t kInvalidSocket = -1;

  inline int last_error() { return errno; }

  [[noreturn]] inline void die(const std::string& msg) {
      int e = last_error();
      std::cerr << "[FATAL] " << msg << " | errno=" << e
                << " (" << std::strerror(e) << ")\n";
      std::exit(1);
  }

  inline void close_socket(socket_t s) {
      if (s != kInvalidSocket) ::close(s);
  }

  struct NetInit {
      NetInit() = default;
      ~NetInit() = default;
      NetInit(const NetInit&) = delete;
      NetInit& operator=(const NetInit&) = delete;
  };
#endif

class Socket {
public:
    Socket() = default;
    explicit Socket(socket_t s) : s_(s) {}
    ~Socket() { reset(); }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept : s_(other.s_) { other.s_ = kInvalidSocket; }
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            reset();
            s_ = other.s_;
            other.s_ = kInvalidSocket;
        }
        return *this;
    }

    socket_t get() const { return s_; }
    void reset(socket_t news = kInvalidSocket) {
        if (s_ != kInvalidSocket) close_socket(s_);
        s_ = news;
    }
    explicit operator bool() const { return s_ != kInvalidSocket; }

private:
    socket_t s_ = kInvalidSocket;
};
