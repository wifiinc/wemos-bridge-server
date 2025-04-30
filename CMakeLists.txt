cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")
include_directories(include)

add_library(wemosserver_lib src/wemosserver.cpp)
add_library(i2cclient_lib src/i2cclient.cpp)
add_library(slavemanager_lib src/slavemanager.cpp)

add_executable(server src/main.cpp)
target_link_libraries(server wemosserver_lib i2cclient_lib slavemanager_lib pthread)

if(NOT CMAKE_CROSSCOMPILING)
  enable_testing()
  add_subdirectory(tests)
endif()
