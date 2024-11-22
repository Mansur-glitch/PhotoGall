cmake_minimum_required(VERSION 3.24.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(utils)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME project_name)
project($project_name VERSION 0.0.1 LANGUAGES CXX)
set(project_root ${CMAKE_CURRENT_SOURCE_DIR})
set(project_build_root "${project_root}/build")
set(project_build_specific ${CMAKE_CURRENT_BINARY_DIR})
set(project_resources "${project_root}/resources")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Quick Test)
qt_standard_project_setup()

# App exe target
qt_add_executable(app)
target_sources(app PRIVATE src/main.cpp src/picture_model.cpp)

# Add core lib target
add_subdirectory(src "${CMAKE_CURRENT_BINARY_DIR}/corelib")

# Add mainqmlplugin lib target
add_subdirectory(resources "${CMAKE_CURRENT_BINARY_DIR}/resources")

target_link_libraries(app PRIVATE Qt6::Quick core mainqmlplugin)

# Add test targets
add_subdirectory(test "${CMAKE_CURRENT_BINARY_DIR}/test")

# Copy compile_commands.json to /build directory if needed
if(NOT "${project_build_specific}" STREQUAL "${project_build_root}")
  get_copy_script_cmd(${project_build_specific}/compile_commands.json
                      ${project_build_root} copy_cmd)
  add_custom_target(cp_cmp_cmd ALL ${copy_cmd})
endif()

# set_target_properties(app PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
# target_compile_definitions(app PRIVATE MAIN_QML_PATH=${main_qml_rel_path})
