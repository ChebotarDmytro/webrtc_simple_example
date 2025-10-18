cmake_minimum_required(VERSION 3.20)

# Set compilers before project() call
set(CMAKE_CXX_COMPILER /usr/bin/clang++-21)
set(CMAKE_C_COMPILER /usr/bin/clang-21)

# Force use of LLVM linker (lld) instead of system ld
# This is critical to avoid architecture mismatch errors
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld")

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(webrtcexample)

# Validate build type
set(ALLOWED_BUILD_TYPES "Debug" "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

list(FIND ALLOWED_BUILD_TYPES ${CMAKE_BUILD_TYPE} INDEX)
if(INDEX EQUAL -1)
    message(FATAL_ERROR
            "Invalid build type: ${CMAKE_BUILD_TYPE}. "
            "Only Debug and RelWithDebInfo are supported.")
endif()

# WebRTC preprocessor definitions
add_definitions(-DWEBRTC_POSIX=1)
add_definitions(-D__STDC_CONSTANT_MACROS=1)

# Find WebRTC package (provided by Conan)
find_package(webrtc REQUIRED)

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

# Link WebRTC package (Conan handles all dependencies automatically)
target_link_libraries(webrtcexample
        PRIVATE
        webrtc::webrtc
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
        -Wno-deprecated-builtins
        -fno-rtti                           # WebRTC disables RTTI
        -fno-exceptions                     # WebRTC disables exceptions
        -fvisibility=hidden
        -fvisibility-inlines-hidden
)

# Build type specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(webrtcexample PRIVATE _DEBUG)
    target_compile_options(webrtcexample PRIVATE -O0 -g3)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_definitions(webrtcexample PRIVATE NDEBUG)
    target_compile_options(webrtcexample PRIVATE -O2 -g1)
endif()

# Print build configuration summary
message(STATUS "")
message(STATUS "=== Build Configuration ===")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Linker: lld")
message(STATUS "WebRTC package: FOUND")
message(STATUS "===========================")
message(STATUS "")