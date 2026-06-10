add_library(exch_core INTERFACE)
target_include_directories(exch_core INTERFACE include)
add_library(exch::core ALIAS exch_core)
