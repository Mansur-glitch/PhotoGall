import QtQuick 2.15
import localhost.PictureModel 1.0

Window {
    visible: true
    title: qsTr("HelloWorld")


    PictureProvider {
        id: "pp"
        directory:  "../../Изображения"
    }

    PictureCollection {
        id: "mainCollection"
        provider: pp
    }

    ListView {
        anchors.fill: parent
        model: GroupedPictureModel {
            collection: mainCollection
        }
        delegate: Text {
            text: model.display
        }
    }
}

