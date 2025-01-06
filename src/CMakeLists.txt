# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2025 Mansur Mukhametzyanov
cmake_minimum_required(VERSION 3.24.0)
find_package(Boost COMPONENTS Headers REQUIRED)

add_library(collection STATIC collection.cpp)
target_compile_features(collection PUBLIC cxx_std_20)
target_include_directories(collection PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(collection PRIVATE Qt6::Core)

add_library(tree_model STATIC tree_model.cpp)
target_compile_features(tree_model PUBLIC cxx_std_20)
target_include_directories(tree_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(tree_model PRIVATE Boost::Headers)
target_link_libraries(tree_model PRIVATE Qt6::Quick)

add_library(core STATIC utility.cpp picture_provider.cpp
            collection_manager.cpp picture_list_model.cpp)
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core PRIVATE collection Qt6::Quick)
