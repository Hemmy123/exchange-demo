add_executable(marketdata_tests MarketDataTests.cpp)

target_link_libraries(marketdata_tests
	PRIVATE
	exch::marketdata 
	exch::matching
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(marketdata_tests)

