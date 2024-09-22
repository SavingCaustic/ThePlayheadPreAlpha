cmake_minimum_required(VERSION 3.10)

# Check for ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(STATUS "Found ccache, enabling...")
    set(CMAKE_C_COMPILER "ccache")
    set(CMAKE_CXX_COMPILER "ccache")
else()
    message(STATUS "ccache not found, proceeding without it.")
endif()

# needed packages
# sudo apt-get install libasound2-dev librtmidi-dev libportaudio2 libportaudiocpp0

# Project name and version
project(MyDAW2 VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add a custom target to run the PHP script
add_custom_target(
    generate_parameters
    COMMAND php ${CMAKE_CURRENT_SOURCE_DIR}/preParseParameters.php
    COMMENT "Generating parameter headers from JSON files using PHP"
)

# Automatically collect all .cpp files in src/ and subdirectories
file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Add executable
add_executable(MyDAW2 ${SOURCES})


# Add all necessary source files
# add_executable(MyDAW2
#    src/main.cpp
#    src/constants.cpp
#    src/core/timing/Rotator.cpp
#    src/core/PlayerEngine.cpp
#    src/synths/Dummy/DummyModel.cpp
#    src/core/audio/AudioMath.cpp
#    # Add other source files here
# )

# Ensure that the header generation is done before building the project
add_dependencies(MyDAW2 generate_parameters)

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)        # avoid .. of inclusions
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/ext)    # inlcude libraries that are included in source.

# Link directories (for library files)
link_directories(${PORTAUDIO_LIBRARY_DIRS})

# Link libraries (proper linking order)
target_link_libraries(MyDAW2 ${PORTAUDIO_LIBRARIES} ${RTMIDI_LIBRARIES} ${ALSA_LIBRARIES} portaudio pthread asound atomic)

# Release vs Debug
set(CMAKE_BUILD_TYPE Release)
