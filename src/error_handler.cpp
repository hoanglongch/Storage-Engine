#include "error_handler.h"
#include <spdlog/spdlog.h>

StorageException::StorageException(ErrorCode code, const std::string &message)
    : errorCode(code), errorMessage(message) {}

const char* StorageException::what() const noexcept {
    return errorMessage.c_str();
}

ErrorCode StorageException::code() const noexcept {
    return errorCode;
}

namespace ErrorHandler {
    void logInfo(const std::string &message) {
        spdlog::info(message);
    }
    void logWarning(const std::string &message) {
        spdlog::warn(message);
    }
    void logError(const std::string &message) {
        spdlog::error(message);
    }
    void handleError(ErrorCode code, const std::string &message) {
        spdlog::error("Error {}: {}", static_cast<int>(code), message);
        throw StorageException(code, message);
    }
}
