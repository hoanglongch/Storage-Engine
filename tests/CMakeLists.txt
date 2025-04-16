cmake_minimum_required(VERSION 3.10)
project(Tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Bring in GoogleTest using FetchContent.
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include the project's source directory.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Collect all test source files.
file(GLOB TEST_SOURCES *.cpp)

# Create test executable.
add_executable(runTests ${TEST_SOURCES})
target_link_libraries(runTests gtest gtest_main pthread)

include(GoogleTest)
gtest_discover_tests(runTests)
