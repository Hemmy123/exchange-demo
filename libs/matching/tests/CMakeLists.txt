add_executable(matchingengine_tests MatchingEngineTests.cpp)

target_link_libraries(matchingengine_tests
	PRIVATE
	exch::matching
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(matchingengine_tests)

