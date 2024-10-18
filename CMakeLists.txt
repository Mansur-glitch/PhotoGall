cmake_minimum_required(VERSION 3.24.0)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils.cmake")

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME project_name)
project($project_name VERSION 0.0.1 LANGUAGES CXX)
set(project_root ${CMAKE_CURRENT_SOURCE_DIR})
set(project_build_root "${project_root}/build")
set(project_build_specific ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt specific
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add app target
add_subdirectory(src "${CMAKE_CURRENT_BINARY_DIR}/app")

