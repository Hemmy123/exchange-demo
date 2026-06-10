add_executable(exchange_sim src/main.cpp)

#==== Dependencies ===== 
target_link_libraries(exchange_sim PRIVATE exch::orderbook)


