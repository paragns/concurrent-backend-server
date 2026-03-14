cmake_minimum_required(VERSION 3.10)
project(concurrent-backend-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main server executable
add_executable(server
    src/main.cpp
    src/server.cpp
)

# Link pthread (for future chapters)
target_link_libraries(server pthread)

# Include directories
target_include_directories(server PRIVATE src/)
