#ifndef REST_SERVER_H
#define REST_SERVER_H

#include <string>
#include <functional>

class RestServer {
public:
    RestServer(int port, std::function<std::string()> metricsCallback);
    ~RestServer();

    // Start the REST server in a separate thread.
    bool start();

    // Stop the server.
    void stop();

private:
    int port;
    int server_fd;
    bool running;
    std::function<std::string()> getMetrics; // Callback to get metrics JSON.
    int serverThread(); // Function run by the server thread.
};

#endif // REST_SERVER_H
