# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2025 Mansur Mukhametzyanov
cmake_minimum_required(VERSION 3.24.0)

add_executable(test_collection test_collection.cpp)
target_link_libraries(test_collection PRIVATE collection Qt6::Test)

add_executable(tree_model_test tree_model_test.cpp)
target_link_libraries(tree_model_test PRIVATE tree_model Qt6::Quick Qt6::Test)

enable_testing(true)
add_test(NAME test_collection COMMAND test_collection)
add_test(NAME tree_model_test COMMAND tree_model_test)
