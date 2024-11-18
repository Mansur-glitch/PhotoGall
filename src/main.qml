import QtQuick 2.15
import localhost.PictureModel 1.0

Window {
    visible: true
    title: qsTr("HelloWorld")

    PictureCollection {
        id: "mainCollection"
    }

    PictureProvider {
        collection: mainCollection
        id: "pp"
        directory:  "../../Изображения"
    }
    

    ListView {
        anchors.fill: parent
        model: GroupedPictureModel {
            collection: pp.collection
        }
        delegate: Text {
            text: model.display
        }
    }
}

