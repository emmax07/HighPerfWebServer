#include "server/server.hpp"
#include <iostream>

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << " Starting Multithreaded C++ Web Server      " << std::endl;
    std::cout << "============================================" << std::endl;

    // Initializes server with 8 worker threads
    TcpServer server("127.0.0.1", 8080, 8);

    if (!server.start()) {
        std::cerr << "[Error] Failed to initialize server." << std::endl;
        return 1;
    }

    std::cout << "Press CTRL+C to terminate." << std::endl;
    server.run();

    return 0;
}