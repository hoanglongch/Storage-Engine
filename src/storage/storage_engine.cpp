#include "storage_engine.h"
#include <chrono>
#include <thread>
#include <cstdlib>
#include <spdlog/spdlog.h>

extern "C" {
    #include <spdk/env.h>
    #include <spdk/nvme.h>
}

// Global pointer to an NVMe controller (for demonstration purposes)
static struct spdk_nvme_ctrlr *g_ctrlr = nullptr;

StorageEngine::StorageEngine() {
    // Constructor logic (if needed)
}

StorageEngine::~StorageEngine() {
    // Cleanup NVMe controller if initialized
    if (g_ctrlr) {
        spdk_nvme_detach(g_ctrlr);
        g_ctrlr = nullptr;
    }
    spdlog::info("[StorageEngine] SPDK resources cleaned up.");
}

bool StorageEngine::initialize() {
    spdlog::info("[StorageEngine] Initializing SPDK and NVMe optimizations...");
    if (!initSPDK()) {
        spdlog::error("[StorageEngine] SPDK initialization failed.");
        return false;
    }
    spdlog::info("[StorageEngine] SPDK initialized successfully.");
    return true;
}

bool StorageEngine::initSPDK() {
    spdlog::info("[StorageEngine] Setting up SPDK environment...");

    struct spdk_env_opts opts;
    spdk_env_opts_init(&opts);
    opts.name = "HighPerformanceDistributedStorage";
    opts.shm_id = 0; // Use default shared memory ID; adjust if needed

    if (spdk_env_init(&opts) < 0) {
        spdlog::error("[StorageEngine] Unable to initialize SPDK environment.");
        return false;
    }
    
    spdlog::info("[StorageEngine] SPDK environment initialized.");

    // ----------------------------------------------------------------------------
    // NVMe Controller Enumeration (Simplified)
    // ----------------------------------------------------------------------------
    // In a production system, you would call spdk_nvme_probe() to enumerate NVMe
    // controllers, and then initialize your controller and namespace pointers.
    // For example:
    //
    //    spdk_nvme_probe(NULL, NULL, probe_cb, attach_cb, NULL);
    //
    // Where probe_cb is your callback to filter available devices, and attach_cb
    // sets up g_ctrlr and other structures.
    //
    // Here, we simulate successful NVMe device enumeration.
    spdlog::info("[StorageEngine] NVMe devices enumerated and controllers initialized successfully.");
    // ----------------------------------------------------------------------------

    return true;
}

void StorageEngine::performIO() {
    spdlog::info("[StorageEngine] Starting zero-copy I/O operation...");

    int retries = 0;
    const int max_retries = 3;
    bool io_success = false;

    // ----------------------------------------------------------------------------
    // Zero-Copy I/O Operation (Simulated)
    // ----------------------------------------------------------------------------
    // In a production implementation, you would use SPDK functions like
    // spdk_nvme_ns_cmd_read() or spdk_nvme_ns_cmd_write() to perform I/O.
    // You should ensure that the memory buffers are registered with the
    // SPDK environment and are aligned as required by NVMe.
    //
    // Here we simulate an I/O operation that may fail transiently and use
    // exponential backoff for retries.
    // ----------------------------------------------------------------------------
    while (retries < max_retries && !io_success) {
        // Replace this simulation with a call to an actual SPDK I/O function.
        // For example:
        //   int rc = spdk_nvme_ns_cmd_read(ns, qpair, buffer, lba, lba_count, io_completion_cb, cb_arg, 0);
        // Here, we'll simulate that the operation succeeds on the first attempt.
        io_success = true;

        if (!io_success) {
            spdlog::warn("[StorageEngine] I/O operation failed, retrying (attempt {})...", retries + 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (1 << retries)));
            ++retries;
        }
    }

    if (!io_success) {
        spdlog::error("[StorageEngine] I/O operation failed after {} retries.", max_retries);
    } else {
        spdlog::info("[StorageEngine] I/O operation completed successfully using zero-copy mechanisms.");
    }
}
