cmake_minimum_required(VERSION 3.24.0)
# find_package(Qt6 COMPONENTS Core Widgets Quick Test REQUIRED)
# find_package(Boost COMPONENTS Headers REQUIRED)

add_library(core STATIC picture_collection.cpp tree_model.cpp)
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(core PRIVATE Boost::Headers)
target_link_libraries(core PRIVATE Qt6::Quick)
