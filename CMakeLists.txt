cmake_minimum_required(VERSION 3.20)

# Allow LLVM_PATH to be set via command line: cmake -DLLVM_PATH=/path/to/llvm ..
# Default to the depot_tools bundled LLVM if not specified
set(LLVM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../webrtc_checkout/src/third_party/llvm-build/Release+Asserts"
        CACHE PATH "Path to LLVM toolchain (depot_tools bundled version)")

# Verify LLVM path exists
if(NOT EXISTS "${LLVM_PATH}/bin/clang++")
    message(FATAL_ERROR "LLVM toolchain not found at: ${LLVM_PATH}\n"
            "Please set LLVM_PATH to the correct location:\n"
            "  cmake -DLLVM_PATH=/path/to/llvm ..")
endif()

message(STATUS "Using LLVM toolchain: ${LLVM_PATH}")

# Set compilers before project() call
set(CMAKE_CXX_COMPILER "${LLVM_PATH}/bin/clang++")
set(CMAKE_C_COMPILER "${LLVM_PATH}/bin/clang")

# Force use of LLVM linker (lld) instead of system ld
# This is critical to avoid architecture mismatch errors
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld")

# Use LLVM archiver and ranlib
set(CMAKE_AR "${LLVM_PATH}/bin/llvm-ar")
set(CMAKE_RANLIB "${LLVM_PATH}/bin/llvm-ranlib")

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(webrtcexample)

# WebRTC preprocessor definitions
add_definitions(-DWEBRTC_POSIX=1)
add_definitions(-D__STDC_CONSTANT_MACROS=1)

# WebRTC paths - allow override via command line
set(WEBRTC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../webrtc_checkout/src"
        CACHE PATH "WebRTC source root directory")
set(WEBRTC_BUILD_DIR "${WEBRTC_ROOT}/out/Default"
        CACHE PATH "WebRTC build output directory")

# Verify WebRTC paths exist
if(NOT EXISTS "${WEBRTC_ROOT}")
    message(FATAL_ERROR "WebRTC source not found at: ${WEBRTC_ROOT}")
endif()
if(NOT EXISTS "${WEBRTC_BUILD_DIR}/obj")
    message(FATAL_ERROR "WebRTC build output not found at: ${WEBRTC_BUILD_DIR}\n"
            "Please build WebRTC first: ninja -C ${WEBRTC_BUILD_DIR}")
endif()

message(STATUS "WebRTC root: ${WEBRTC_ROOT}")
message(STATUS "WebRTC build: ${WEBRTC_BUILD_DIR}")

# Include directories
include_directories(
        ${WEBRTC_ROOT}
        ${WEBRTC_ROOT}/third_party/abseil-cpp
        ${WEBRTC_ROOT}/third_party/libyuv/include
)

# Source files
set(SOURCES
        main.cpp
        data_channel_observer.cpp
        data_channel_observer.h
        simple_peer_connection_observer.cpp
        simple_peer_connection_observer.h
        sdp_observer.cpp
        sdp_observer.h
        local_signaling.cpp
        local_signaling.h
)

# Create executable
add_executable(webrtcexample ${SOURCES})

# Link libraries
target_link_libraries(webrtcexample
        "${WEBRTC_BUILD_DIR}/obj/libwebrtc.a"
        pthread
        dl
        rt
        X11
        GL
        GLU
        Xext
        Xfixes
        Xdamage
        Xrandr
        Xcomposite
)

# Compiler flags matching WebRTC build configuration
target_compile_options(webrtcexample PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -Wno-unused-variable
        -Wno-nullability-completeness
        -Wno-nullability-extension
        -fno-rtti                           # WebRTC disables RTTI
        -fno-exceptions                     # WebRTC disables exceptions
        -fvisibility=hidden
        -fvisibility-inlines-hidden
)

# Print build configuration summary
message(STATUS "")
message(STATUS "=== Build Configuration ===")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Linker: lld (${LLVM_PATH}/bin/ld.lld)")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "===========================")
message(STATUS "")