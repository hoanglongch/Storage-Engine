#include "io_monitor.h"
#include <iostream>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sys/resource.h>
#include <cstring>
#include <errno.h>

// Global pointer to eBPF object (for simplicity)
static struct bpf_object *obj = nullptr;
static int bpf_map_fd = -1;

IOMonitor::IOMonitor() {
    // Constructor logic if needed.
}

IOMonitor::~IOMonitor() {
    // Clean up eBPF resources.
    if (obj) {
        bpf_object__close(obj);
    }
    std::cout << "[IOMonitor] eBPF resources cleaned up." << std::endl;
}

bool IOMonitor::initialize() {
    std::cout << "[IOMonitor] Initializing I/O monitoring system using eBPF..." << std::endl;
    if (!setupProbes()) {
        std::cerr << "[IOMonitor] Failed to set up eBPF probes." << std::endl;
        return false;
    }
    std::cout << "[IOMonitor] eBPF monitoring probes set up successfully." << std::endl;
    return true;
}

bool IOMonitor::setupProbes() {
    // Increase RLIMIT_MEMLOCK to allow loading of eBPF programs
    struct rlimit rlim = {RLIM_INFINITY, RLIM_INFINITY};
    if (setrlimit(RLIMIT_MEMLOCK, &rlim)) {
        std::cerr << "[IOMonitor] Failed to increase RLIMIT_MEMLOCK: " << strerror(errno) << std::endl;
        return false;
    }

    // Load the eBPF object file (assumes the file is located at "src/monitoring/iomonitor.bpf.o")
    obj = bpf_object__open_file("src/monitoring/iomonitor.bpf.o", nullptr);
    if (!obj) {
        std::cerr << "[IOMonitor] Failed to open eBPF object file." << std::endl;
        return false;
    }

    // Load all programs within the eBPF object
    int err = bpf_object__load(obj);
    if (err) {
        std::cerr << "[IOMonitor] Failed to load eBPF object: " << strerror(-err) << std::endl;
        return false;
    }

    // Find and attach the eBPF program by title (assumes program is titled "kprobe/sys_read")
    struct bpf_program *prog = bpf_object__find_program_by_title(obj, "kprobe/sys_read");
    if (!prog) {
        std::cerr << "[IOMonitor] Failed to find eBPF program 'kprobe/sys_read'." << std::endl;
        return false;
    }

    // Attach the kprobe to "sys_read"
    struct bpf_link *link = bpf_program__attach_kprobe(prog, false, "sys_read");
    if (!link) {
        std::cerr << "[IOMonitor] Failed to attach kprobe to sys_read." << std::endl;
        return false;
    }

    // Obtain file descriptor for the BPF map (assumes the map is named "io_metrics")
    bpf_map_fd = bpf_object__find_map_fd_by_name(obj, "io_metrics");
    if (bpf_map_fd < 0) {
        std::cerr << "[IOMonitor] Failed to find eBPF map 'io_metrics'." << std::endl;
        return false;
    }

    std::cout << "[IOMonitor] eBPF program attached to sys_read successfully, and map 'io_metrics' loaded." << std::endl;
    return true;
}

void IOMonitor::checkStatus() {
    // Look up performance metrics from the BPF map.
    // In this example, our map is an array with one element (key=0) that counts sys_read invocations.
    uint32_t key = 0;
    uint64_t value = 0;
    int err = bpf_map_lookup_elem(bpf_map_fd, &key, &value);
    if (err == 0) {
        std::cout << "[IOMonitor] sys_read call count: " << value << std::endl;
        // Simple real-time analysis: if the count exceeds a threshold, output an alert.
        if (value > 1000) {
            std::cerr << "[IOMonitor] ALERT: High sys_read call volume detected: " << value << std::endl;
        }
    } else {
        std::cerr << "[IOMonitor] Failed to read from eBPF map: " << strerror(errno) << std::endl;
    }
    
    // Sleep briefly before the next check.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

std::string IOMonitor::getMetricsJSON() {
    uint32_t key = 0;
    uint64_t value = 0;
    int err = bpf_map_lookup_elem(bpf_map_fd, &key, &value);
    if (err == 0) {
        return "{\"sys_read\": " + std::to_string(value) + "}";
    } else {
        return "{\"error\": \"Unable to retrieve metrics\"}";
    }
}
