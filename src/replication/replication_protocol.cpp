#include "replication_protocol.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <functional>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Include the generated Protocol Buffers header.
// Make sure to compile replication.proto with protoc to generate replication.pb.h and replication.pb.cc.
#include "replication.pb.h"

#define REPLICA_DEFAULT_PORT 6000
#define MAX_RETRY 3

ReplicationProtocol::ReplicationProtocol() {
    // In a real-world scenario, you might derive this from configuration.
    localNodeID = "node_local";
}

ReplicationProtocol::~ReplicationProtocol() {
    // Cleanup if needed.
}

bool ReplicationProtocol::initialize() {
    std::cout << "[ReplicationProtocol] Initializing replication protocol..." << std::endl;
    if (!setupReplication()) {
        std::cerr << "[ReplicationProtocol] Failed to set up replication protocol." << std::endl;
        return false;
    }
    std::cout << "[ReplicationProtocol] Replication protocol initialized successfully." << std::endl;
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

    std::cout << "[ReplicationProtocol] Consistent hash ring and vector clocks set up." << std::endl;
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
    int port = REPLICA_DEFAULT_PORT; // default port value

    size_t colonPos = nodeAddress.find(':');
    if (colonPos != std::string::npos) {
        ip = nodeAddress.substr(0, colonPos);
        port = std::stoi(nodeAddress.substr(colonPos + 1));
    } else {
        ip = nodeAddress;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "[ReplicationProtocol] Socket creation failed." << std::endl;
        return false;
    }

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "[ReplicationProtocol] Invalid IP address: " << ip << std::endl;
        close(sockfd);
        return false;
    }

    int retry = 0;
    bool connected = false;
    while (retry < MAX_RETRY && !connected) {
        if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == 0) {
            connected = true;
        } else {
            std::cerr << "[ReplicationProtocol] Connection attempt " << retry + 1 << " to " << nodeAddress << " failed." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (1 << retry)));
            retry++;
        }
    }

    if (!connected) {
        std::cerr << "[ReplicationProtocol] Failed to connect to " << nodeAddress << " after " << MAX_RETRY << " attempts." << std::endl;
        close(sockfd);
        return false;
    }

    // Send the length of the message (as 4-byte network order integer).
    uint32_t msgLength = htonl(message.size());
    if (send(sockfd, &msgLength, sizeof(msgLength), 0) != sizeof(msgLength)) {
        std::cerr << "[ReplicationProtocol] Failed to send message length." << std::endl;
        close(sockfd);
        return false;
    }

    // Send the serialized message.
    ssize_t sentBytes = send(sockfd, message.c_str(), message.size(), 0);
    if (sentBytes != (ssize_t)message.size()) {
        std::cerr << "[ReplicationProtocol] Failed to send the entire replication message." << std::endl;
        close(sockfd);
        return false;
    }

    std::cout << "[ReplicationProtocol] Replication message sent successfully to " << nodeAddress << std::endl;
    close(sockfd);
    return true;
}

void ReplicationProtocol::replicateData() {
    std::cout << "[ReplicationProtocol] Replicating data using the established protocol..." << std::endl;

    // Create a replication message using Protocol Buffers.
    replication::ReplicationData repMsg;
    repMsg.set_key("sample_key");
    repMsg.set_value("sample_value");
    repMsg.set_timestamp(static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()));

    // Update local vector clock.
    vectorClock[localNodeID] += 1;

    // Clear previous vector clock data and insert current vector clock values.
    repMsg.clear_vector_clock();
    for (const auto &entry : vectorClock) {
        repMsg.add_vector_clock(entry.second);
    }

    // Serialize the message to a string.
    std::string serializedMessage;
    if (!repMsg.SerializeToString(&serializedMessage)) {
        std::cerr << "[ReplicationProtocol] Failed to serialize replication message." << std::endl;
        return;
    }

    // Choose the replica node based on the replication key (using consistent hashing).
    std::string replicaNode = chooseReplicaNode(repMsg.key());

    // Send the replication message to the selected node.
    if (!sendReplicationMessage(replicaNode, serializedMessage)) {
        std::cerr << "[ReplicationProtocol] Failed to send replication message to " << replicaNode << std::endl;
    } else {
        std::cout << "[ReplicationProtocol] Replication message sent to " << replicaNode << std::endl;
    }
}
