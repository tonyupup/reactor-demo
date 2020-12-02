project(server)
cmake_minimum_required(VERSION 2.8)


include_directories(include)
aux_source_directory(src SRC_DIR)
if (APPLE)
    set(SRC_DIR ${SRC_DIR} platform/kepollApi.cpp)
else(APPLE)
    set(SRC_DIR ${SRC_DIR} platform/epollApi.cpp)
endif(APPLE)

message(STATUS ${SRC_DIR})
add_compile_options(-std=c++11)
# set(CMAKE_CXX_FLAGS "-Wall std=c++11")

add_executable(main main.cpp ${SRC_DIR})