find_package(Qt6 COMPONENTS Widgets Quick REQUIRED)
add_executable(app
    main.cpp
)
set(main_qml_file main.qml)
target_sources(app PRIVATE ${main_qml_file})
target_compile_features(app PRIVATE cxx_std_20)
cmake_path(RELATIVE_PATH CMAKE_CURRENT_SOURCE_DIR
    BASE_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE cur_dir_rel_path)
set(main_qml_path "${cur_dir_rel_path}/${main_qml_file}")
message(${main_qml_path})
target_compile_definitions(app PRIVATE MAIN_QML_PATH=${main_qml_path})
target_include_directories(app PRIVATE Qt6::Widgets Qt6::Quick)
target_link_libraries(app Qt6::Widgets Qt6::Quick)
