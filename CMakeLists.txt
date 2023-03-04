cmake_minimum_required(VERSION 3.13)
project (tgb-t)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra")

add_executable (tgb-t main.cpp)
target_link_libraries (tgb-t)