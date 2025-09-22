cmake_minimum_required(VERSION 3.14)
project(gimbal-dartt-controller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent module
include(FetchContent)

# Fetch dartt-protocol library
FetchContent_Declare(
    dartt_protocol
    GIT_REPOSITORY https://github.com/Ocanath/dartt-protocol.git
    GIT_TAG master  # or specify a specific tag/commit
)

# Fetch serial library
FetchContent_Declare(
    serial
    GIT_REPOSITORY https://github.com/Ocanath/serial-cross-platform.git
    GIT_TAG master  # or specify a specific tag/commit
)

# Make the libraries available
FetchContent_MakeAvailable(dartt_protocol serial)

# Create the main executable
add_executable(gimbal-dartt-controller
    src/main.cpp
)

# Link the libraries to the executable
target_link_libraries(gimbal-dartt-controller
    PRIVATE
    dartt_protocol
    serial
)

# Include directories (if needed, depending on how the libraries expose their headers)
target_include_directories(gimbal-dartt-controller
    PRIVATE
    ${dartt_protocol_SOURCE_DIR}
    ${serial_SOURCE_DIR}
)