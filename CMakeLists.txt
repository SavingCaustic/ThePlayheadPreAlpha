cmake_minimum_required(VERSION 3.10)

# Needed packages
# sudo apt-get install libasound2-dev librtmidi-dev libportaudio2 libportaudiocpp0

# Project name and version
project(MyDAW2 VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Automatically collect all .cpp files in src/ and subdirectories
file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Add executable
add_executable(MyDAW2 ${SOURCES})

# Ensure that the header generation is done before building the project
# add_dependencies(MyDAW2 generate_parameters)

# Find PortAudio, ALSA, and RtMidi
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(RTMIDI REQUIRED rtmidi)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# SIMD optimization flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")
endif()

# Include directories for target
target_include_directories(MyDAW2 PRIVATE
    ${PORTAUDIO_INCLUDE_DIRS}
    ${RTMIDI_INCLUDE_DIR}
    ${ALSA_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src        # Avoid .. in inclusions
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ext    # Include libraries that are in the source
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

# Release vs Debug
set(CMAKE_BUILD_TYPE Release)
