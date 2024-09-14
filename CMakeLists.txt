cmake_minimum_required(VERSION 3.10)

# needed packages
# sudo apt-get install libasound2-dev librtmidi-dev libportaudio2 libportaudiocpp0

# Project name and version
project(MyDAW2 VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Automatically add all source files in src directory
# file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add executable target
#add_executable(myDaw2 src/main.cpp)

# Add all necessary source files
add_executable(myDaw2
    src/main.cpp
    src/constants.cpp
    src/core/Rotator.cpp
    src/core/PlayerEngine.cpp
    src/synths/Dummy/DummyModel.cpp
    # Add other source files here
)

# Find PortAudio, ALSA, and RtMidi
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(RTMIDI REQUIRED rtmidi)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# SIMD optimization flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")
endif()

# Include directories
include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(${RTMIDI_INCLUDE_DIR})
include_directories(${ALSA_INCLUDE_DIRS})

# Link directories (for library files)
link_directories(${PORTAUDIO_LIBRARY_DIRS})

# Link libraries (proper linking order)
target_link_libraries(myDaw2 ${PORTAUDIO_LIBRARIES} ${RTMIDI_LIBRARIES} ${ALSA_LIBRARIES} portaudio pthread asound)

# Release vs Debug
set(CMAKE_BUILD_TYPE Release)
