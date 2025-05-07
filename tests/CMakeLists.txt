include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(test_wemosserver test_wemosserver.cpp)
target_link_libraries(test_wemosserver gtest_main wemosserver_lib i2cclient_lib slavemanager_lib)
gtest_discover_tests(test_wemosserver)

add_executable(test_i2cclient test_i2cclient.cpp)
target_link_libraries(test_i2cclient gtest_main i2cclient_lib)
gtest_discover_tests(test_i2cclient)

add_executable(test_slavemanager test_slavemanager.cpp)
target_link_libraries(test_slavemanager gtest_main slavemanager_lib)
gtest_discover_tests(test_slavemanager)