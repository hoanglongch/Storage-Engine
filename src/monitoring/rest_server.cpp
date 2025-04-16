#include "rest_server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <iostream>

RestServer::RestServer(int port, std::function<std::string()> metricsCallback)
    : port(port), server_fd(-1), running(false), getMetrics(metricsCallback) {
}

RestServer::~RestServer() {
    stop();
}

bool RestServer::start() {
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "[RestServer] Socket creation failed" << std::endl;
        return false;
    }
    
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "[RestServer] setsockopt failed" << std::endl;
        return false;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "[RestServer] Bind failed" << std::endl;
        return false;
    }
    
    if (listen(server_fd, 3) < 0) {
        std::cerr << "[RestServer] Listen failed" << std::endl;
        return false;
    }
    
    running = true;
    std::thread([this]() { this->serverThread(); }).detach();
    
    std::cout << "[RestServer] Server started on port " << port << std::endl;
    return true;
}

void RestServer::stop() {
    if (running) {
        running = false;
        if (server_fd >= 0) {
            close(server_fd);
        }
        std::cout << "[RestServer] Server stopped." << std::endl;
    }
}

int RestServer::serverThread() {
    while (running) {
        struct sockaddr_in client_address;
        socklen_t client_addrlen = sizeof(client_address);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_addrlen);
        if (client_fd < 0) {
            if (running) {
                std::cerr << "[RestServer] Accept failed" << std::endl;
            }
            continue;
        }
        
        // Read the HTTP request (simplified).
        char buffer[1024] = {0};
        read(client_fd, buffer, 1024);
        
        // Check if the request is GET /metrics
        if (std::strstr(buffer, "GET /metrics") != nullptr) {
            std::string metrics = getMetrics();
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(metrics.size()) + "\r\n\r\n" + metrics;
            send(client_fd, response.c_str(), response.size(), 0);
        } else {
            std::string notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(client_fd, notFound.c_str(), notFound.size(), 0);
        }
        
        close(client_fd);
    }
    return 0;
}
