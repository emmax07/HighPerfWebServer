#ifndef SOCKET_COMPAT_HPP
#define SOCKET_COMPAT_HPP

#include <iostream>
#include <string>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")

    using socket_t = SOCKET;
    constexpr socket_t INVALID_SOCKET_VAL = INVALID_SOCKET;
    constexpr int SOCKET_ERROR_VAL = SOCKET_ERROR;

    inline bool init_sockets() {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "[Network Error] WSAStartup failed: " << result << std::endl;
            return false;
        }
        return true;
    }

    inline void cleanup_sockets() {
        WSACleanup();
    }

    inline void close_socket(socket_t s) {
        closesocket(s);
    }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <cstring>

    using socket_t = int;
    constexpr socket_t INVALID_SOCKET_VAL = -1;
    constexpr int SOCKET_ERROR_VAL = -1;

    inline bool init_sockets() { return true; }
    inline void cleanup_sockets() {}

    inline void close_socket(socket_t s) {
        close(s);
    }
#endif

#endif // SOCKET_COMPAT_HPP