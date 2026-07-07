
add_executable(feed_tests FeedTests.cpp
SequencerTests.cpp)

target_link_libraries(feed_tests
	PRIVATE
	exch::feed
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(feed_tests)

