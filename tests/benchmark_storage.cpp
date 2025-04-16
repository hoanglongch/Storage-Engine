#include <iostream>
#include <chrono>
#include "storage/storage_engine.h"

int main() {
    StorageEngine storage;
    if (!storage.initialize()) {
        std::cerr << "StorageEngine initialization failed" << std::endl;
        return -1;
    }
    const int iterations = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        storage.performIO();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Performed " << iterations << " I/O operations in " << duration.count() << " microseconds\n";
    std::cout << "Average latency: " << (duration.count() / static_cast<double>(iterations)) << " microseconds\n";
    return 0;
}
