cmake_minimum_required(VERSION 3.10)
project(HighPerformanceDistributedStorage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages - assuming SPDK, RDMA, Protocol Buffers, and eBPF libs are installed
find_package(Protobuf REQUIRED)
# find_package(SPDK)  # Custom find modules may be required
# find_package(RDMA REQUIRED)
# find_package(libbpf REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${Protobuf_INCLUDE_DIRS})

file(GLOB STORAGE_SOURCES "src/storage/*.cpp")
file(GLOB CACHE_SOURCES "src/cache/*.cpp")
file(GLOB REPLICATION_SOURCES "src/replication/*.cpp")
file(GLOB MONITORING_SOURCES "src/monitoring/*.cpp")
file(GLOB MAIN_SOURCES "src/main.cpp")

add_executable(HighPerformanceDistributedStorage 
    ${MAIN_SOURCES} 
    ${STORAGE_SOURCES} 
    ${CACHE_SOURCES} 
    ${REPLICATION_SOURCES} 
    ${MONITORING_SOURCES}
)

target_link_libraries(HighPerformanceDistributedStorage ${Protobuf_LIBRARIES})
# target_link_libraries(HighPerformanceDistributedStorage spdk rdma libbpf)
