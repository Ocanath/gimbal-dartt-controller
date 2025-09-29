cmake_minimum_required(VERSION 3.14)
project(gimbal-dartt-controller)

set(CMAKE_CXX_STANDARD 20)
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

FetchContent_Declare(
    byte_stuffing
    GIT_REPOSITORY https://github.com/Ocanath/byte-stuffing.git
    GIT_TAG        master
)

# Fetch SDL2 library
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.10  # Use a stable release tag
)

# Make the libraries available
FetchContent_MakeAvailable(dartt_protocol serial byte_stuffing SDL2)

# Create the main executable
add_executable(gimbal-dartt-controller
    src/main.cpp
)

# Link the libraries to the executable
target_link_libraries(gimbal-dartt-controller
    PRIVATE
    dartt_protocol
    dartt_checksum
    serial
    cobs
    PPP
    SDL2::SDL2
    SDL2::SDL2main
)

# Include directories (if needed, depending on how the libraries expose their headers)
target_include_directories(gimbal-dartt-controller
    PRIVATE
    ${dartt_protocol_SOURCE_DIR}
    ${serial_SOURCE_DIR}
)