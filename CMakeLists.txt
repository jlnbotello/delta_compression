cmake_minimum_required(VERSION 3.0)
project(Compressor)

# Set the path to your header files
include_directories(inc)

# Add all the source files from the src and test directories
file(GLOB SOURCES src/*.c test/*.c)

# Create an executable from the sources
add_executable(tests ${SOURCES})
