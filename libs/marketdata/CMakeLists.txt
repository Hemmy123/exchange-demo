#==== Project Settings ==== 
add_library(exch_marketdata STATIC 
  src/MarketDataPublisher.cpp
)
target_include_directories(exch_marketdata PUBLIC include)

#==== Dependencies ====
target_link_libraries(exch_marketdata PUBLIC exch::orderbook)
target_link_libraries(exch_marketdata PUBLIC exch::core)   # ← references Core

#==== Helpers ====
add_library(exch::marketdata ALIAS exch_marketdata)


#==== Build Flags ====
if(EXCH_BUILD_TESTS)
  add_subdirectory(tests)
endif()

