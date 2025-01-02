cmake_minimum_required(VERSION 3.24.0)

qt_add_qml_module(mainqml URI mainqml
    RESOURCE_PREFIX /
    QML_FILES
        main.qml
        ItemSavedSize.qml
        DirectoryTree.qml
        ToolsTabButton.qml
        DatePicker.qml
    RESOURCES
        images/double_left.png
        images/double_top.png
        images/up.png
        images/right_angle.png
        images/category.png
        images/files.png
        images/info.png
        images/image.png
        images/down_to.png
        images/inverse.png
        images/gears.png
        images/cross.png
        images/tree.png
        images/transparency.jpg
        images/ru.png
        images/uk.png
    STATIC
)

# set(images_dir "resources/images")
# add_library(attach_resources INTERFACE)
# qt_add_resources(attach_resources "images" PREFIX "images"
#    BASE "${images_dir}" FILES "${images_dir}/double_left.png")

