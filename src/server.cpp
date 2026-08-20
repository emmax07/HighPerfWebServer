#include "server/server.hpp"
#include <iostream>
#include <vector>

TcpServer::TcpServer(const std::string& host, uint16_t port, size_t thread_count)
    : host_(host), port_(port) {
    init_sockets();
    thread_pool_ = std::make_unique<ThreadPool>(thread_count);
}

TcpServer::~TcpServer() {
    stop();
    cleanup_sockets();
}

bool TcpServer::bind_and_listen() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd_ == INVALID_SOCKET_VAL) {
        std::cerr << "[TcpServer] Failed to create socket." << std::endl;
        return false;
    }

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    
    if (host_ == "0.0.0.0" || host_.empty()) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, host_.c_str(), &addr.sin_addr);
    }

    if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR_VAL) {
        std::cerr << "[TcpServer] Failed to bind to " << host_ << ":" << port_ << std::endl;
        close_socket(listen_fd_);
        listen_fd_ = INVALID_SOCKET_VAL;
        return false;
    }

    if (listen(listen_fd_, SOMAXCONN) == SOCKET_ERROR_VAL) {
        std::cerr << "[TcpServer] Failed to listen on socket." << std::endl;
        close_socket(listen_fd_);
        listen_fd_ = INVALID_SOCKET_VAL;
        return false;
    }

    return true;
}

bool TcpServer::start() {
    if (!bind_and_listen()) {
        return false;
    }
    is_running_ = true;
    std::cout << "[TcpServer] Multithreaded server listening on http://" << host_ << ":" << port_ << std::endl;
    return true;
}

void TcpServer::stop() {
    if (is_running_) {
        is_running_ = false;
        if (listen_fd_ != INVALID_SOCKET_VAL) {
            close_socket(listen_fd_);
            listen_fd_ = INVALID_SOCKET_VAL;
        }
        if (thread_pool_) {
            thread_pool_->stop();
        }
        std::cout << "[TcpServer] Server stopped." << std::endl;
    }
}

void TcpServer::handle_client(socket_t client_fd) {
    std::vector<char> buffer(4096);
    int bytes_received = recv(client_fd, buffer.data(), static_cast<int>(buffer.size() - 1), 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';

        // Minimal HTTP Response including active Thread ID
        std::string body = "Hello from Concurrent C++ Server! Thread ID: " + 
                           std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;

        send(client_fd, response.c_str(), static_cast<int>(response.size()), 0);
    }

    close_socket(client_fd);
}

void TcpServer::run() {
    while (is_running_) {
        sockaddr_in client_addr{};
        #ifdef _WIN32
        int addr_len = sizeof(client_addr);
        #else
        socklen_t addr_len = sizeof(client_addr);
        #endif

        socket_t client_fd = accept(listen_fd_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
        if (client_fd == INVALID_SOCKET_VAL) {
            if (!is_running_) break;
            std::cerr << "[TcpServer] Accept failed." << std::endl;
            continue;
        }

        // Delegate client socket processing to the ThreadPool worker queue
        thread_pool_->enqueue([this, client_fd]() {
            this->handle_client(client_fd);
        });
    }
}