#include <iostream>
#include "storage/storage_engine.h"
#include "cache/distributed_cache.h"
#include "replication/replication_protocol.h"
#include "monitoring/io_monitor.h"
#include "monitoring/rest_server.h"
#include "error_handler.h"
#include "thread_pool.h"

int main() {
    // Initialize Storage Engine.
    StorageEngine storage;
    if (!storage.initialize()) {
        ErrorHandler::logError("Failed to initialize storage engine.");
        return -1;
    }
    
    // Initialize Distributed Cache.
    DistributedCache cache;
    if (!cache.initialize()) {
        ErrorHandler::logError("Failed to initialize distributed cache.");
        return -1;
    }
    
    // Initialize Replication Protocol.
    ReplicationProtocol replication;
    if (!replication.initialize()) {
        ErrorHandler::logError("Failed to initialize replication protocol.");
        return -1;
    }
    
    // Initialize I/O Monitoring.
    IOMonitor monitor;
    if (!monitor.initialize()) {
        ErrorHandler::logError("Failed to initialize I/O monitoring system.");
        return -1;
    }
    
    // Start REST server for exporting metrics.
    RestServer restServer(8080, [&monitor]() { return monitor.getMetricsJSON(); });
    if (!restServer.start()) {
        ErrorHandler::logError("Failed to start REST server.");
        return -1;
    }
    
    ErrorHandler::logInfo("System initialized. Running main loop with concurrency...");

    // Create a thread pool with 4 worker threads.
    ThreadPool pool(4);
    for (int i = 0; i < 10; ++i) {
        // Enqueue tasks concurrently.
        auto ioFuture = pool.enqueue([&storage]() { storage.performIO(); });
        auto cacheFuture = pool.enqueue([&cache]() { cache.processRequests(); });
        auto replFuture = pool.enqueue([&replication]() { replication.replicateData(); });
        auto monFuture = pool.enqueue([&monitor]() { monitor.checkStatus(); });

        // Wait for all tasks of this iteration to complete.
        ioFuture.get();
        cacheFuture.get();
        replFuture.get();
        monFuture.get();
    }
    
    ErrorHandler::logInfo("System shutting down.");
    restServer.stop();
    pool.shutdown();
    return 0;
}
