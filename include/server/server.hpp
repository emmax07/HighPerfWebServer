#ifndef SERVER_HPP
#define SERVER_HPP

#include "socket_compat.hpp"
#include "thread_pool.hpp"
#include <string>
#include <cstdint>
#include <memory>

class TcpServer {
public:
    TcpServer(const std::string& host, uint16_t port, size_t thread_count = std::thread::hardware_concurrency());
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    bool start();
    void stop();
    void run();

private:
    std::string host_;
    uint16_t port_;
    socket_t listen_fd_{INVALID_SOCKET_VAL};
    bool is_running_{false};

    std::unique_ptr<ThreadPool> thread_pool_;

    bool bind_and_listen();
    void handle_client(socket_t client_fd);
};

#endif // SERVER_HPP