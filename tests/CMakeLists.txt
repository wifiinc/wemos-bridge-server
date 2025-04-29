include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(test_wemosserver test_wemosserver.cpp)
target_link_libraries(test_wemosserver gtest_main wemosserver_lib)

gtest_discover_tests(test_wemosserver)
