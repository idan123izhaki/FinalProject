cmake_minimum_required(VERSION 3.26)
project(server_udp_project)

set(CMAKE_CXX_STANDARD 17)

add_executable(server_udp_project
#        main.cpp
#        serverHandling.hpp
#        serverHandling.cpp
        serverExample.cpp
)

add_library(protobuf_lib ../fileStructure.pb.cc ../fileStructure.pb.h)

# Set the path to the libRaptorQ directory
set(RAPTOR_Q_PATH "/home/idan/Desktop/CLION_projects/UDP_NETWORKING/libRaptorQ-master")
include_directories(${RAPTOR_Q_PATH}/src) # headers
link_directories(${RAPTOR_Q_PATH}/build/lib) # cpp
include_directories(${RAPTOR_Q_PATH}/external/eigen3)

target_link_libraries(server_udp_project protobuf_lib protobuf RaptorQ)


# This line adds a compilation definition (-DRQ_LITTLE_ENDIAN) to the target server_udp_project.
#The definition is set as private, which means it only applies to the target specified (server_udp_project).
#In this case, -DRQ_LITTLE_ENDIAN is a preprocessor definition that indicates the target system is little-endian.
target_compile_definitions(server_udp_project PRIVATE -DRQ_LITTLE_ENDIAN)
