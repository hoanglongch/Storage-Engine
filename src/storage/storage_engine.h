#ifndef STORAGE_ENGINE_H
#define STORAGE_ENGINE_H

class StorageEngine {
public:
    StorageEngine();
    ~StorageEngine();

    // Initialize SPDK, NVMe, and setup zero-copy I/O path
    bool initialize();

    // Simulated I/O operation using zero-copy optimizations
    void performIO();

private:
    // Internal function to initialize the SPDK environment
    bool initSPDK();
};

#endif // STORAGE_ENGINE_H
