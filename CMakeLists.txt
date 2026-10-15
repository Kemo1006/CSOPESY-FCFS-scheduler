cmake_minimum_required(VERSION 3.15)
project(fcfs_scheduler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(emulator src/main.cpp src/scheduler.cpp)
target_link_libraries(emulator PRIVATE Threads::Threads)
