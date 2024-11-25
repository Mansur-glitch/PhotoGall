cmake_minimum_required(VERSION 3.24.0)

qt_add_qml_module(mainqml URI mainqml
    RESOURCE_PREFIX /
    QML_FILES
        main.qml
        ItemSavedSize.qml
    RESOURCES
        images/double_left.png
    STATIC
)

# set(images_dir "resources/images")
# add_library(attach_resources INTERFACE)
# qt_add_resources(attach_resources "images" PREFIX "images"
#    BASE "${images_dir}" FILES "${images_dir}/double_left.png")

