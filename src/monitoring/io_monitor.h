#ifndef IO_MONITOR_H
#define IO_MONITOR_H

#include <string>

class IOMonitor {
public:
    IOMonitor();
    ~IOMonitor();

    // Initialize the I/O monitoring system via eBPF.
    bool initialize();

    // Check the current I/O performance and detect potential bottlenecks.
    void checkStatus();

    // Get metrics as a JSON string.
    std::string getMetricsJSON();

private:
    // Internal function to set up eBPF probes for kernel-level monitoring.
    bool setupProbes();
};

#endif // IO_MONITOR_H
