import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15

import localhost.PictureModel 1.0

Window {
    id: mainWnd
    visible: true
    title: qsTr("PhotoGall")
    property real aspectRatio: width / height

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
            color: "green"
            anchors.right: parent.right

            Flow {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 10
                Rectangle {
                    width: childrenRect.width + 20
                    height: childrenRect.height + 20
                    color: "transparent"
                    border.color: "black"
                    ColumnLayout {
                        x: 10
                        y: 10
                        Label {
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            text: qsTr("Tool panel position")
                        }
                        RowLayout {
                            Label {
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                text: mainSplit.orientation == Qt.Horizontal ? qsTr("Left") : qsTr("Top")
                            }
                            Switch {
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                id: "splitLayoutSwitch"
                                checked: false
                                onToggled: {
                                    mainSplit.toolsFirst = ! mainSplit.toolsFirst;
                                }
                            }
                            Label {
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                text: mainSplit.orientation == Qt.Horizontal ? qsTr("Right") : qsTr("Bottom")
                            }
                        }
                    }
                }
            
                Rectangle {
                    width: childrenRect.width + 20
                    height: childrenRect.height + 20
                    color: "transparent"
                    border.color: "black"
                    ColumnLayout {
                        x: 10
                        y: 10
                        Label {
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            text: qsTr("Language")
                        }
                        ComboBox {
                            id: "languageChangeList"
                            objectName: "languageChangeList"
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            signal languageChanged(newLang: string)
                            onActivated: {
                                languageChangeList.languageChanged(currentText);
                            }
                            model: ListModel {
                                id: model
                                ListElement { text: "en" }
                                ListElement { text: "ru" }
                            }
                        }
                    }
                }
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
        orientation: mainWnd.aspectRatio > 1.0 ? Qt.Horizontal: Qt.Vertical 
        property bool toolsFirst: true
        property bool toolsExpanded: true
        property list<QtObject> splitItems
        property var toolsItem
        property var galleryItem
        onToolsFirstChanged: {
            var it0 = mainSplit.splitItems[0];
            var it1 = mainSplit.splitItems[1];
            var temp = it0.width; it0.SplitView.preferredWidth = it1.width; it1.SplitView.preferredWidth = temp;
            temp = it0.height; it0.SplitView.preferredHeight = it1.height; it1.SplitView.preferredHeight = temp;
            temp = it0.SplitView.fillWidth; it0.SplitView.fillWidth = it1.SplitView.fillWidth; it1.SplitView.fillWidth = temp;
            temp = it0.SplitView.fillHeight; it0.SplitView.fillHeight = it1.SplitView.fillHeight; it1.SplitView.fillHeight = temp;
            temp = it0.SplitView.minimumWidth; it0.SplitView.minimumWidth = it1.SplitView.minimumWidth; it1.SplitView.minimumWidth = temp;
            temp = it0.SplitView.minimumHeight; it0.SplitView.minimumHeight = it1.SplitView.minimumHeight; it1.SplitView.minimumHeight = temp;

            if (mainSplit.toolsFirst) {
                toolPanel.parent = it0;
                toolsItem = it0;
                mainPanel.parent = it1;
                galleryItem = it1;
            } else {
                toolPanel.parent = it1;
                toolsItem = it1;
                mainPanel.parent = it0;
                galleryItem = it0;
            }
        }

        onToolsExpandedChanged: {
            if (! toolsExpanded) {
                toolsItem.savedMinWidth = toolsItem.SplitView.minimumWidth 
                toolsItem.savedMinHeight = toolsItem.SplitView.minimumHeight 
                toolsItem.SplitView.minimumWidth = 0
                toolsItem.SplitView.minimumHeight = 0
                if (orientation == Qt.Horizontal) {
                    toolsItem.savedWidth = toolsItem.width
                    toolsItem.SplitView.preferredWidth = 0
                } else {
                    toolsItem.savedHeight = toolsItem.height
                    toolsItem.SplitView.preferredHeight = 0
                }
                toolsItem.visible = false;
            } else {
                if (orientation == Qt.Horizontal) {
                    toolsItem.SplitView.preferredWidth = toolsItem.savedWidth
                } else {
                    toolsItem.SplitView.preferredHeight = toolsItem.savedHeight
                }
                toolsItem.SplitView.minimumWidth = toolsItem.savedMinWidth
                toolsItem.SplitView.minimumHeight = toolsItem.savedMinHeight
                toolsItem.visible = true;
            }
        }
        handle: Rectangle {
            id: "handleDelegate"
            implicitWidth: mainSplit.orientation == Qt.Horizontal ? 8: 4
            height: mainSplit.orientation == Qt.Horizontal ? 4: 8
            color: SplitHandle.pressed ? "grey": "blue"

            containmentMask: Item {
                x: mainSplit.orientation == Qt.Horizontal ? (handleDelegate.width - width) / 2: 0
                y: mainSplit.orientation == Qt.Horizontal ? 0: (handleDelegate.height - height) / 2
                width: mainSplit.orientation == Qt.Horizontal ? 64: mainSplit.width 
                height: mainSplit.orientation == Qt.Horizontal ? mainSplit.height: 64
            }
        }

        ItemSavedSize {
            id: "splitFirst"
            SplitView.minimumWidth: {SplitView.minimumWidth = 200}
            SplitView.minimumHeight: {SplitView.minimumWidth = 200}
        }

        ItemSavedSize {
            id: "splitSecond"
            SplitView.minimumWidth: {SplitView.minimumWidth = 400}
            SplitView.minimumHeight: {SplitView.minimumWidth = 400}
        }

        Component.onCompleted: {
            splitItems.length = 2
            splitItems[0] = splitFirst
            splitItems[1] = splitSecond
            toolPanel.parent = splitFirst
            mainPanel.parent = splitSecond
            toolsItem = splitFirst
            galleryItem = splitSecond
        }
    }

    Image {
        id: "splitButton"
        source: "images/double_left.png"
        width: 50
        height: 50
        x: {
            var fullWidth = mainSplit.width
            var splitPos = splitFirst.width
            if (mainSplit.orientation == Qt.Vertical) {
                return fullWidth / 2 - width / 2
            }
            var base = mainSplit.toolsExpanded ? splitPos: (mainSplit.toolsFirst ? 0: mainSplit.width)
            var offset = mainSplit.toolsFirst ^ mainSplit.toolsExpanded  ? 0: -width
            return base + offset
        }
        y: {
            var fullHeight = mainSplit.height
            var splitPos = splitFirst.height
            if (mainSplit.orientation == Qt.Horizontal) {
                return fullHeight / 2 - height / 2
            }
            var base = mainSplit.toolsExpanded ? splitPos: (mainSplit.toolsFirst ? 0: mainSplit.height)
            var offset = mainSplit.toolsFirst ^ mainSplit.toolsExpanded  ? 0: -height
            return base + offset
        }
        z: mainSplit.z + 1

        MouseArea {
            anchors.fill: parent
            onClicked: {
                mainSplit.toolsExpanded = ! mainSplit.toolsExpanded 
            }
        }
    }
}

