project(server)
cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_STANDARD 17)
# cmake_policy(SET CMP0057 NEW)

include_directories(/usr/local/opt/boost/include/)
include_directories(include)
set(BOOST_ROOT /usr/local/opt/boost)
set(Boost_INCLUDE_DIR /usr/local/opt/boost/include)
set(Boost_LIBRARY_DIR /usr/local/opt/boost/bin)
find_package(Boost REQUIRED COMPONENTS
    filesystem
    log
    PATHS /usr/local/opt/boost/bin/lib/cmake/
)
if(${Boost_FOUND})
    message(STATUS ${Boost_LIBRARIES})
endif(${Boost_FOUND})


aux_source_directory(src SRC_DIR)
if (APPLE)
    set(SRC_DIR ${SRC_DIR} platform/kepollApi.cpp)
else(APPLE)
    set(SRC_DIR ${SRC_DIR} platform/epollApi.cpp)
endif(APPLE)

message(STATUS ${SRC_DIR})

# set(CMAKE_CXX_FLAGS "-Wall std=c++11")
add_executable(main main.cpp ${SRC_DIR})