project(server)
cmake_minimum_required(VERSION 2.8)

aux_source_directory(src SRC_DIR)
message(STATUS ${SRC_DIR})
add_compile_options(-std=c++11)
# set(CMAKE_CXX_FLAGS "-Wall std=c++11")

add_executable(main main.cpp ${SRC_DIR})
add_executable(client Mclient.cpp ${SRC_DIR})