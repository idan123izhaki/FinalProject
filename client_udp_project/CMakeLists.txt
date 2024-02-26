cmake_minimum_required(VERSION 3.26)
project(client_udp_project)

set(CMAKE_CXX_STANDARD 17)

add_executable(client_udp_project
#       main.cpp
#       fileHandler.hpp
#       fileHandler.cpp
#       fecAlgorithm.cpp
#       fecAlgorithm.hpp
#      example.cpp

        clientExample.cpp

        #   example3Final.cpp
    #   raptorQfecEncoder.cpp
    #   RaptorQExample.cpp
)

# Add the protobuf library
add_library(protobuf_lib ../fileStructure.pb.cc ../fileStructure.pb.h)

# Set the path to the libRaptorQ directory
set(RAPTOR_Q_PATH "/home/idan/Desktop/CLION_projects/UDP_NETWORKING/libRaptorQ-master")
include_directories(${RAPTOR_Q_PATH}/src) # headers
link_directories(${RAPTOR_Q_PATH}/build/lib) # cpp
include_directories(${RAPTOR_Q_PATH}/external/eigen3)

#add_definitions(-DRQ_LITTLE_ENDIAN)

target_link_libraries(client_udp_project protobuf_lib protobuf RaptorQ)

# target_compile_definitions(client_udp_project PRIVATE -DRQ_LITTLE_ENDIAN)
# target_compile_definitions(client_udp_project PRIVATE -DRQ_BIG_ENDIAN)
