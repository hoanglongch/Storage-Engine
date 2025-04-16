# High-Level Architecture Diagram

Below is a simplified ASCII diagram that outlines the primary components and their interactions:

                          +------------------+
                          |  I/O Monitoring  |
                          |      (eBPF)      |
                          +--------+---------+
                                   |
                                   v
        +------------+     +--------------+     +-----------------------+
        |  Storage   |     | Distributed  |     |     Replication       |
        |  Engine    |     |    Cache     |     |     Protocol          |
        |  (SPDK)    |     | (RDMA, Custom|     | (ProtoBuf, Consistent |
        |            |     |  Allocator)  |     |   Hash, Vector Clocks)|
        +------------+     +--------------+     +-----------------------+
               \                 |                        /
                \                |                       /
                 \               |                      /
                  \              v                     /
                   +------------+------------+--------+
                                | Middleware & Logging |
                                | (Message Queue)      |
                                +----------------------+



### Explanation of the Diagram
- **Storage Engine (SPDK):**  
  Handles the low-level I/O operations using SPDK and NVMe optimizations.
  
- **Distributed Cache (RDMA, Custom Allocator):**  
  Manages caching with RDMA for low latency and a custom memory allocator for optimal performance.
  
- **Replication Protocol:**  
  Ensures data durability and consistency through Protocol Buffers, consistent hashing to distribute load, and vector clocks for conflict resolution.
  
- **I/O Monitoring (eBPF):**  
  Monitors system performance in real time via eBPF probes.
  
- **Middleware (Message Queue & Logging):**  
  Acts as a bridge between components, handling asynchronous communication and centralized logging.
