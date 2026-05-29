cmake_minimum_required(VERSION 3.28)
project(OrderBook)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(OrderBook
        src/main.cpp
        src/OrderBook.cpp
)
