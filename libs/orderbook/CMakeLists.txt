#==== Project Settings ==== 
add_library(exch_orderbook STATIC src/OrderBook.cpp)
target_include_directories(exch_orderbook PUBLIC include)

#==== Dependencies ====
target_link_libraries(exch_orderbook PUBLIC exch::core)   # ← references Core

#==== Helpers ====
add_library(exch::orderbook ALIAS exch_orderbook)         # nice namespaced name

if(EXCH_BUILD_TESTS)
  add_subdirectory(tests)
endif()
