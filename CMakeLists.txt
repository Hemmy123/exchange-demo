### --- High Level Config --- ###
cmake_minimum_required(VERSION 3.28)
project(Exchange CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-fcontracts)   # experimental opt-id
add_compile_options(-freflection)
add_link_options(-fcontracts)      # pulls in the default handler

### --- Unit Tests --- ### 
option(EXCH_BUILD_TESTS "Build unit tests" ON)
enable_testing()

include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG        v1.17.0
)

FetchContent_MakeAvailable(googletest)

### --- Projects --- ### 
add_subdirectory(libs/core)
add_subdirectory(libs/matching)
add_subdirectory(libs/orderbook)
add_subdirectory(libs/marketdata)
add_subdirectory(libs/feed)
add_subdirectory(apps/exchange_sim)

### --- Warning Config --- ### 
add_library(exch_warnings INTERFACE)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(exch_warnings INTERFACE -Wall -Wextra -Wconversion -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(exch_warnings INTERFACE /W4 /WX)
endif()

add_library(exch::warnings ALIAS exch_warnings)

foreach(lib exch_core exch_feed exch_marketdata exch_matching exch_orderbook)
  get_target_property(_type ${lib} TYPE)
  if(NOT _type STREQUAL "INTERFACE_LIBRARY")
    target_link_libraries(${lib} PRIVATE exch::warnings)
  endif()
endforeach()
