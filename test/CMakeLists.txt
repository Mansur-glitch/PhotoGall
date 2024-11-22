cmake_minimum_required(VERSION 3.24.0)

add_executable(test_collection test_collection.cpp)
target_link_libraries(test_collection PRIVATE core Qt6::Test)
target_include_directories(test_collection PRIVATE Qt6::Test)

enable_testing(true)
add_test(NAME test_collection COMMAND test_collection)
