#pragma once
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>

[[noreturn]] inline void die(const std::string& msg) {
    int e = errno;
    std::cerr << "[FATAL] " << msg << " | errno=" << e
              << " (" << std::strerror(e) << ")\n";
    std::exit(1);
}

class Fd {
public:
    Fd() = default;
    explicit Fd(int fd) : fd_(fd) {}
    ~Fd() { reset(); }

    Fd(const Fd&) = delete;
    Fd& operator=(const Fd&) = delete;

    Fd(Fd&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    Fd& operator=(Fd&& other) noexcept {
        if (this != &other) {
            reset();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    int get() const { return fd_; }
    void reset(int newfd = -1) {
        if (fd_ >= 0) ::close(fd_);
        fd_ = newfd;
    }
    explicit operator bool() const { return fd_ >= 0; }

private:
    int fd_ = -1;
};
