# HighPerformanceDistributedStorage

This project implements a high-performance distributed storage system in C++ with these core features:

- **Custom Storage Engine:** Uses SPDK to implement a zero-copy I/O path and NVMe optimizations, reducing read/write latency.
- **Distributed Cache:** Implements RDMA-based communication, a custom memory allocator, and lock-free data structures to scale and handle 10K+ concurrent requests.
- **Replication Protocol:** Uses Protocol Buffers, consistent hashing, and vector clocks to achieve high data durability even under fault scenarios.
- **I/O Monitoring:** Integrates an I/O monitoring system via eBPF with kernel probes to detect bottlenecks in real-time.

## Prerequisites

- **C++:** C++17 or later
- **Build:** CMake
- **Libraries:** 
  - SPDK (Storage Performance Development Kit)
  - RDMA (e.g., libibverbs)
  - Protocol Buffers compiler and libraries
  - eBPF libraries (e.g., libbpf, bpftool)

## Building the Project

From the project root:

```bash
mkdir build
cd build
cmake ..
make
```

## Running the Project

Execute the compiled binary:

```bash
./Storage-Engine
```