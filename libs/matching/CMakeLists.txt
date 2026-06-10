
#==== Project Settings ==== 
add_library(exch_matching STATIC src/MatchingEngine.cpp)
target_include_directories(exch_matching PUBLIC include)

#==== Dependencies ====
target_link_libraries(exch_matching PUBLIC exch::orderbook)

#==== Helpers ====
add_library(exch::matching ALIAS exch_matching)
