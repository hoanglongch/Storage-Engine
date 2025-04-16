#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <exception>

// Standardized error codes for the system.
enum class ErrorCode {
    SUCCESS = 0,
    SPDK_INIT_FAILURE,
    RDMA_NO_DEVICES,
    RDMA_DEVICE_OPEN_FAILURE,
    RDMA_PD_ALLOC_FAILURE,
    RDMA_CQ_CREATE_FAILURE,
    RDMA_QP_CREATE_FAILURE,
    MEMORY_ALLOC_FAILURE,
    REPLICATION_SERIALIZE_FAILURE,
    SOCKET_CREATION_FAILURE,
    CONNECTION_FAILURE,
    EBPF_PROBE_FAILURE,
    REST_SERVER_FAILURE,
    // Add additional error codes as necessary.
};

// Custom exception to be thrown for unrecoverable errors.
class StorageException : public std::exception {
public:
    StorageException(ErrorCode code, const std::string &message);
    virtual const char* what() const noexcept override;
    ErrorCode code() const noexcept;
private:
    ErrorCode errorCode;
    std::string errorMessage;
};

namespace ErrorHandler {
    // Log informational messages.
    void logInfo(const std::string &message);
    // Log warnings.
    void logWarning(const std::string &message);
    // Log errors.
    void logError(const std::string &message);
    // Handle an unrecoverable error by logging and throwing an exception.
    void handleError(ErrorCode code, const std::string &message);
}

#endif // ERROR_HANDLER_H
