include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(test_math test_math.cpp)
target_link_libraries(test_math gtest_main math_lib)

gtest_discover_tests(test_math)
