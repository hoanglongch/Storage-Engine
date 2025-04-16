#include <iostream>
#include <thread>
#include "storage/storage_engine.h"
#include "cache/distributed_cache.h"
#include "replication/replication_protocol.h"
#include "monitoring/io_monitor.h"
#include "monitoring/rest_server.h"

int main() {
    // Initialize Storage Engine
    StorageEngine storage;
    if (!storage.initialize()) {
        std::cerr << "Failed to initialize storage engine." << std::endl;
        return -1;
    }
    
    // Initialize Distributed Cache
    DistributedCache cache;
    if (!cache.initialize()) {
        std::cerr << "Failed to initialize distributed cache." << std::endl;
        return -1;
    }
    
    // Initialize Replication Protocol
    ReplicationProtocol replication;
    if (!replication.initialize()) {
        std::cerr << "Failed to initialize replication protocol." << std::endl;
        return -1;
    }
    
    // Initialize I/O Monitoring System
    IOMonitor monitor;
    if (!monitor.initialize()) {
        std::cerr << "Failed to initialize I/O monitoring system." << std::endl;
        return -1;
    }
    
    // Start REST server for exporting metrics.
    RestServer restServer(8080, [&monitor]() { return monitor.getMetricsJSON(); });
    if (!restServer.start()) {
        std::cerr << "Failed to start REST server." << std::endl;
        return -1;
    }
    
    // Simulate main processing loop
    std::cout << "System initialized. Running main loop..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        storage.performIO();
        cache.processRequests();
        replication.replicateData();
        monitor.checkStatus();
    }
    
    std::cout << "System shutting down." << std::endl;
    restServer.stop();
    return 0;
}
