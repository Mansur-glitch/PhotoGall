import QtQuick 2.15
import QtQuick.Controls 2.15
import localhost.PictureModel 1.0

Window {
    id: mainWnd
    visible: true
    title: qsTr("PhotoGall")
    property real aspectRatio: width / height
    property int toolPanelOrientation: aspectRatio > 1.0 ? Qt.Horizontal: Qt.Vertical
    property bool toolPanelAfterGallery: true

    PictureProvider {
        id: "pp"
        directory: currentPathField.text
    }

    PictureCollection {
        id: "mainCollection"
        provider: pp
    }

    Item {
        id: "hiddenWidgets"
        visible: false
        Rectangle {
            id: "toolPanel"
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            color: "black"
            anchors.right: parent.right

            Text {
                anchors.centerIn: toolPanel
                text: "T: " + mainWnd.aspectRatio + " " + mainWnd.toolPanelOrientation
                color: "white"
            }
        }

        Rectangle {
            id: "mainPanel"
            anchors.centerIn: parent
            width: parent.width
            height: parent.height

            TextInput {
                id: "currentPathField"
                anchors.top: parent.top
                width: parent.width
                text: "/home/mansur/Изображения"
            }

            ListView {
                id: "fileList"
                anchors.top: currentPathField.bottom
                anchors.bottom: parent.bottom
                width: parent.width
                model: GroupedPictureModel {
                    collection: mainCollection
                }
                delegate: Text {
                    text: model.display
                }
            }
        }
    }

    SplitView {
        id: "mainSplit"
        anchors.fill: parent
        orientation: mainWnd.toolPanelOrientation

        handle: Rectangle {
            id: "handleDelegate"
            implicitWidth: 8
            height: 4
            color: SplitHandle.pressed ? "grey": "blue"

            containmentMask: Item {
                x: (handleDelegate.width - width) / 2
                width: 64
                height: mainSplit.height
            }
        }

        Rectangle {
            id: "splitFirst"
            SplitView.minimumWidth:  200
            SplitView.minimumHeight:  200
        }

        Rectangle {
            id: "splitSecond"
            SplitView.minimumWidth:  200
            SplitView.minimumHeight:  200
        }

        states: [
            State {
                name: "toolsAfterGallery"
                when: mainWnd.toolPanelAfterGallery 
                ParentChange { target: mainPanel; parent: splitFirst;}
                ParentChange { target: toolPanel; parent: splitSecond;}
                PropertyChanges {splitSecond {
                        SplitView.fillWidth: false  
                        SplitView.fillHeight: false
                        implicitHeight: 200;
                        implicitWidth: 200;
                    }
                }
                PropertyChanges {splitFirst {
                        SplitView.fillWidth: true  
                        SplitView.fillHeight: true
                        implicitHeight: 400;
                        implicitWidth: 400;
                    }
                }
            },
            State {
                name: "galleryAfterTools"
                when: ! mainWnd.toolPanelAfterGallery 
                ParentChange { target: mainPanel; parent: splitSecond;}
                ParentChange { target: toolPanel; parent: splitFirst;}
                PropertyChanges {splitFirst {
                        SplitView.fillWidth: false  
                        SplitView.fillHeight: false
                        implicitHeight: 200;
                        implicitWidth: 200;
                    }
                }
                PropertyChanges {splitSecond {
                        SplitView.fillWidth: true  
                        SplitView.fillHeight: true
                        implicitHeight: 400;
                        implicitWidth: 400;
                    }
                }
            }
        ]
    }

    Image {
        id: "splitButton"
        source: "images/double_left.png"
        x: splitFirst.x + splitFirst.width
        y: splitFirst.y + splitFirst.height / 2
        // color: "red"
        width: 50
        height: 50
        z: mainSplit.z + 1

        MouseArea {
            anchors.fill: parent
            property bool galleryFirst: false
            onClicked: {
                galleryFirst = ! galleryFirst
                if (galleryFirst) {
                    splitButton.mirror = false
                    // splitButton.color = "yellow"
                    mainWnd.toolPanelAfterGallery = true
                } else {
                    // splitButton.color = "magenta"
                    splitButton.mirror = true
                    mainWnd.toolPanelAfterGallery = false
                }
            }
        }
    }
}

