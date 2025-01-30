cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ThePlayhead VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Default to Release if no build type is specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Select architecture-specific flags
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|x86")
    set(ARCH_FLAGS "-march=x86-64 -mtune=native")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    message(STATUS "Configuring for ARM (Raspberry Pi or similar)")
    set(ARCH_FLAGS "-march=armv8-a -mtune=cortex-a72")
else()
    message(WARNING "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    set(ARCH_FLAGS "")
endif()

# Select main file and build-specific flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MAIN_FILE "${PROJECT_SOURCE_DIR}/src/mainDebug.cpp")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 ${ARCH_FLAGS}") # Debug symbols, no optimizations
else()
    set(MAIN_FILE "${PROJECT_SOURCE_DIR}/src/main.cpp")
    # set(CMAKE_CXX_FLAGS_RELEASE "-O3..."
    set(CMAKE_CXX_FLAGS_RELEASE "-O0 ${ARCH_FLAGS} -ftree-vectorize -flto -ffast-math") # Fast build Release
    endif()

# Automatically collect all .cpp files in src/ except the main file
file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Exclude main files from SOURCES and add the selected main file
file(GLOB MAIN_FILES "${PROJECT_SOURCE_DIR}/src/main*.cpp")
list(REMOVE_ITEM SOURCES ${MAIN_FILES})
list(APPEND SOURCES ${MAIN_FILE})

# Add executable
add_executable(ThePlayhead ${SOURCES})

# Find PortAudio, ALSA, and RtMidi
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(RTMIDI REQUIRED rtmidi)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# Include directories for target
target_include_directories(ThePlayhead PRIVATE
    ${PORTAUDIO_INCLUDE_DIRS}
    ${RTMIDI_INCLUDE_DIR}
    ${ALSA_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ext
)

# Link directories (for library files)
link_directories(${PORTAUDIO_LIBRARY_DIRS})

# Link libraries (proper linking order)
target_link_libraries(ThePlayhead
    ${PORTAUDIO_LIBRARIES}
    ${RTMIDI_LIBRARIES}
    ${ALSA_LIBRARIES}
    portaudio
    pthread
    asound
    atomic
)
