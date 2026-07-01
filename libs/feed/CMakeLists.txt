#==== Project Settings ====
add_library(exch_feed STATIC
    src/Sequencer.cpp
    src/QueueProducer.cpp
    src/FeedPump.cpp
)
target_include_directories(exch_feed PUBLIC include)

#==== Dependencies ====
target_link_libraries(exch_feed PUBLIC exch::core)

#==== Helpers ====
add_library(exch::feed ALIAS exch_feed)

#==== Build Flags ====
if(EXCH_BUILD_TESTS)
    add_subdirectory(tests)
endif()
