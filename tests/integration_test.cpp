#include <gtest/gtest.h>
#include "storage/storage_engine.h"
#include "cache/distributed_cache.h"
#include "replication/replication_protocol.h"
#include "monitoring/io_monitor.h"

TEST(IntegrationTest, AllModulesInitialize) {
    StorageEngine storage;
    EXPECT_TRUE(storage.initialize());
    
    DistributedCache cache;
    EXPECT_TRUE(cache.initialize());
    
    ReplicationProtocol replication;
    EXPECT_TRUE(replication.initialize());
    
    // Optionally, initialize the monitoring subsystem.
    IOMonitor monitor;
    EXPECT_TRUE(monitor.initialize());
}
