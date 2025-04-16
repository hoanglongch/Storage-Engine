#include "replication_protocol.h"
#include "error_handler.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <functional>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Include OpenSSL headers for TLS and SHA256.
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>

// Include the generated Protocol Buffers header.
// Make sure to compile replication.proto with protoc to generate replication.pb.h and replication.pb.cc.
#include "replication.pb.h"

#define REPLICA_DEFAULT_PORT 6000
#define MAX_RETRY 3

// Helper function to compute SHA256 hash of the message.
static std::string computeSHA256(const std::string &data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    if (!SHA256_Init(&sha256))
        ErrorHandler::handleError(ErrorCode::REPLICATION_SERIALIZE_FAILURE, "SHA256_Init failed");
    if (!SHA256_Update(&sha256, data.c_str(), data.size()))
        ErrorHandler::handleError(ErrorCode::REPLICATION_SERIALIZE_FAILURE, "SHA256_Update failed");
    if (!SHA256_Final(hash, &sha256))
        ErrorHandler::handleError(ErrorCode::REPLICATION_SERIALIZE_FAILURE, "SHA256_Final failed");
    return std::string(reinterpret_cast<char*>(hash), SHA256_DIGEST_LENGTH);
}

ReplicationProtocol::ReplicationProtocol() {
    // In a real-world scenario, you might derive this from configuration.
    localNodeID = "node_local";
}

ReplicationProtocol::~ReplicationProtocol() {
    // Cleanup if needed.
}

bool ReplicationProtocol::initialize() {
    ErrorHandler::logInfo("[ReplicationProtocol] Initializing replication protocol...");
    if (!setupReplication()) {
        ErrorHandler::logError("[ReplicationProtocol] Failed to set up replication protocol.");
        return false;
    }
    ErrorHandler::logInfo("[ReplicationProtocol] Replication protocol initialized successfully.");
    return true;
}

bool ReplicationProtocol::setupReplication() {
    // Setup a simple consistent hash ring with example node addresses.
    hashRing.push_back("127.0.0.1:5001");
    hashRing.push_back("127.0.0.1:5002");
    hashRing.push_back("127.0.0.1:5003");

    // Initialize vector clocks: set counter to 0 for all nodes.
    for (const auto &node : hashRing) {
        vectorClock[node] = 0;
    }
    vectorClock[localNodeID] = 0;

    ErrorHandler::logInfo("[ReplicationProtocol] Consistent hash ring and vector clocks set up.");
    return true;
}

std::string ReplicationProtocol::chooseReplicaNode(const std::string &key) {
    // Use std::hash and modulo on the hash ring size to select a node.
    size_t hashVal = std::hash<std::string>{}(key);
    size_t index = hashVal % hashRing.size();
    return hashRing[index];
}

bool ReplicationProtocol::sendReplicationMessage(const std::string &nodeAddress, const std::string &message) {
    // Parse nodeAddress in the form "IP:port". Use default port if unspecified.
    std::string ip;
    int port = REPLICA_DEFAULT_PORT;
    size_t colonPos = nodeAddress.find(':');
    if (colonPos != std::string::npos) {
        ip = nodeAddress.substr(0, colonPos);
        port = std::stoi(nodeAddress.substr(colonPos + 1));
    } else {
        ip = nodeAddress;
    }

    // Create a plain TCP socket.
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ErrorHandler::logError("[ReplicationProtocol] Socket creation failed.");
        return false;
    }

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        ErrorHandler::logError("[ReplicationProtocol] Invalid IP address: " + ip);
        close(sockfd);
        return false;
    }

    // Connect with retries and exponential backoff.
    int retry = 0;
    bool connected = false;
    while (retry < MAX_RETRY && !connected) {
        if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == 0) {
            connected = true;
        } else {
            ErrorHandler::logWarning("[ReplicationProtocol] Connection attempt " + std::to_string(retry + 1) +
                                      " to " + nodeAddress + " failed.");
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (1 << retry)));
            retry++;
        }
    }
    if (!connected) {
        ErrorHandler::logError("[ReplicationProtocol] Failed to connect to " + nodeAddress +
                                 " after " + std::to_string(MAX_RETRY) + " attempts.");
        close(sockfd);
        return false;
    }

    // Initialize OpenSSL for a TLS client connection.
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        ErrorHandler::logError("[ReplicationProtocol] SSL_CTX_new failed.");
        close(sockfd);
        return false;
    }
    // In a complete solution, load client certificates and set verification options.
    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        ErrorHandler::logError("[ReplicationProtocol] SSL_new failed.");
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }
    SSL_set_fd(ssl, sockfd);
    if (SSL_connect(ssl) <= 0) {
        ErrorHandler::logError("[ReplicationProtocol] SSL_connect failed.");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }

    // Compute the SHA256 hash of the message.
    std::string hash = computeSHA256(message);

    // Build the final packet:
    // Packet format: [4-byte network order message length][message][32-byte SHA256 hash]
    uint32_t netMsgLength = htonl(message.size());
    std::string packet(reinterpret_cast<char*>(&netMsgLength), sizeof(netMsgLength));
    packet.append(message);
    packet.append(hash);

    // Send the complete packet using SSL_write.
    int totalSent = 0;
    int packetSize = packet.size();
    while (totalSent < packetSize) {
        int sent = SSL_write(ssl, packet.c_str() + totalSent, packetSize - totalSent);
        if (sent <= 0) {
            ErrorHandler::logError("[ReplicationProtocol] SSL_write failed during packet transmission.");
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sockfd);
            return false;
        }
        totalSent += sent;
    }

    ErrorHandler::logInfo("[ReplicationProtocol] Secure replication message sent successfully to " + nodeAddress);

    // Shutdown the SSL connection and free resources.
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    return true;
}

void ReplicationProtocol::replicateData() {
    ErrorHandler::logInfo("[ReplicationProtocol] Replicating data using the established protocol...");

    // Create a replication message using Protocol Buffers.
    replication::ReplicationData repMsg;
    repMsg.set_key("sample_key");
    repMsg.set_value("sample_value");
    repMsg.set_timestamp(static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()));

    // Update the local vector clock.
    vectorClock[localNodeID] += 1;

    // Clear previous vector clock data and insert current vector clock values.
    repMsg.clear_vector_clock();
    for (const auto &entry : vectorClock) {
        repMsg.add_vector_clock(entry.second);
    }

    // Serialize the message to a string.
    std::string serializedMessage;
    if (!repMsg.SerializeToString(&serializedMessage)) {
        ErrorHandler::logError("[ReplicationProtocol] Failed to serialize replication message.");
        return;
    }

    // Choose the replica node based on the replication key (using consistent hashing).
    std::string replicaNode = chooseReplicaNode(repMsg.key());

    // Send the replication message to the selected node.
    if (!sendReplicationMessage(replicaNode, serializedMessage)) {
        ErrorHandler::logError("[ReplicationProtocol] Failed to send replication message to " + replicaNode);
    } else {
        ErrorHandler::logInfo("[ReplicationProtocol] Replication message sent to " + replicaNode);
    }
}
