cmake_minimum_required(VERSION 3.28)
project(Exchange CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libs/core)
add_subdirectory(libs/matching)
add_subdirectory(libs/orderbook)
# add_subdirectory(libs/marketdata)
# add_subdirectory(libs/feedhandler)


add_subdirectory(apps/exchange_sim)
