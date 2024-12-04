cmake_minimum_required(VERSION 3.10)

# Project name and version
project(MyDAW2 VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
# set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "include-what-you-use")

# Default to Release if no build type is specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Select main file based on build type and set flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MAIN_FILE "${PROJECT_SOURCE_DIR}/src/mainDebug.cpp") # Use mainDebug.cpp in Debug
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0") # Enable debug symbols, disable optimizations
else()
    set(MAIN_FILE "${PROJECT_SOURCE_DIR}/src/main.cpp") # Use main.cpp in Release
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=x86-64 -mtune=native -ftree-vectorize -flto") # Optimize for Release
endif()

# Automatically collect all .cpp files in src/ except the main file
file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)
list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp" "${PROJECT_SOURCE_DIR}/src/mainDebug.cpp")
list(APPEND SOURCES ${MAIN_FILE})

# Add executable
add_executable(MyDAW2 ${SOURCES})

# Find PortAudio, ALSA, and RtMidi
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(RTMIDI REQUIRED rtmidi)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# SIMD optimization flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Check for AVX2 and FMA for x86/x86_64
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|x86")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")
    endif()

    # Check for ARM architecture and add NEON flags
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon")
    endif()
endif()

# Include directories for target
target_include_directories(MyDAW2 PRIVATE
    ${PORTAUDIO_INCLUDE_DIRS}
    ${RTMIDI_INCLUDE_DIR}
    ${ALSA_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ext
)

# Link directories (for library files)
link_directories(${PORTAUDIO_LIBRARY_DIRS})

# Link libraries (proper linking order)
target_link_libraries(MyDAW2
    ${PORTAUDIO_LIBRARIES}
    ${RTMIDI_LIBRARIES}
    ${ALSA_LIBRARIES}
    portaudio
    pthread
    asound
    atomic
)
