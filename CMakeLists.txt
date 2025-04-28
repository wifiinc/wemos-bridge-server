cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")
include_directories(include)

add_library(math_lib src/math.cpp)

add_executable(server src/main.cpp)
target_link_libraries(server math_lib)

if(NOT CMAKE_CROSSCOMPILING)
  enable_testing()
  add_subdirectory(tests)
endif()
