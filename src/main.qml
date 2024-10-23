import QtQuick 2.15
import localhost.PictureListModel 1.0

Window {
    visible: true
    title: qsTr("HelloWorld")

    ListView {
        anchors.fill: parent
        model: PictureListModel {
            directory: "../../Изображения"
        }
        delegate: Text {
            text: model.display
        }
    }
}

