#ifndef SERVER_HPP
#define SERVER_HPP

#include "socket_compat.hpp"
#include <string>
#include <cstdint>

class TcpServer {
public:
    TcpServer(const std::string& host, uint16_t port);
    ~TcpServer();

    // Prevent copying
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    bool start();
    void stop();
    void run_single_threaded();

private:
    std::string host_;
    uint16_t port_;
    socket_t listen_fd_{INVALID_SOCKET_VAL};
    bool is_running_{false};

    bool bind_and_listen();
    void handle_client(socket_t client_fd);
};

#endif // SERVER_HPP