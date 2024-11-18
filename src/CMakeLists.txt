find_package(Qt6 COMPONENTS Core Widgets Quick Test REQUIRED)
find_package(Boost COMPONENTS Headers REQUIRED)

add_library(core STATIC picture_collection.cpp)
target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC Qt6::Core ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(core PRIVATE Boost::Headers)
target_link_libraries(core PRIVATE Qt6::Core)

add_executable(app
    main.cpp picture_model.cpp
)
set(main_qml_file main.qml)
target_sources(app PRIVATE ${main_qml_file})

cmake_path(RELATIVE_PATH CMAKE_CURRENT_SOURCE_DIR
    BASE_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE cur_dir_rel_path)
set(main_qml_path "${cur_dir_rel_path}/${main_qml_file}")
target_compile_definitions(app PRIVATE MAIN_QML_PATH=${main_qml_path})

target_include_directories(app PRIVATE Qt6::Widgets Qt6::Quick)
target_link_libraries(app core Qt6::Widgets Qt6::Quick)
