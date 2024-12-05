import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import localhost.DirectoryTreeModel 1.0

TreeView {
    id: "tree"
    clip: true
    selectionModel: ItemSelectionModel {
        id: "itemSelection"
        onCurrentIndexChanged: {
            tree.selectedDirectory = treeModel.getPath(itemSelection.currentIndex)
        }
    }
    model: DirectoryTreeModel {
        id: "treeModel"
        rootDirectory: "/home/mansur"
    }

    property alias rootDirectory: treeModel.rootDirectory
    property string selectedDirectory: treeModel.getPath(itemSelection.currentIndex)
    function setUpperRoot(): void {
        treeModel.setUpperRoot();
    }
    function downRootToSelected(): void {
        if (itemSelection.currentIndex.valid) {
            treeModel.downRootTo(itemSelection.currentIndex);
        }
    }

    delegate: Item {
        id: "dirDelegate"
        implicitWidth: tree.width
        implicitHeight: label.implicitHeight * 1.5

        readonly property real indentation: 20
        readonly property real padding: 5

        // Assigned to by TreeView:
        required property TreeView treeView
        required property bool isTreeNode
        required property bool expanded
        required property int hasChildren
        required property int depth
        required property int row
        required property int column
        required property bool current

        // Rotate indicator when expanded by the user
        // (requires TreeView to have a selectionModel)
        property Animation indicatorAnimation: NumberAnimation {
            target: indicator
            property: "rotation"
            from: expanded ? 0 : 90
            to: expanded ? 90 : 0
            duration: 100
            easing.type: Easing.OutQuart
        }
        TableView.onPooled: indicatorAnimation.complete()
        TableView.onReused: if (current) indicatorAnimation.start()
        onExpandedChanged: {
            indicator.rotation = expanded ? 90 : 0;
        }

        Rectangle {
            id: background
            anchors.fill: parent
            color: "green"
            opacity: current ? 0.8: 0.0
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                var index = treeView.index(row, column)
                selectionModel.setCurrentIndex(index, ItemSelectionModel.NoUpdate)
                console.log(current + " " + treeModel.getPath(index))
            }
        }

        Label {
            id: indicator
            x: padding + (depth * indentation)
            anchors.verticalCenter: parent.verticalCenter
            visible: isTreeNode && model.mayExpandFlag
            text: "▶"

            TapHandler {
                onSingleTapped: {
                    var index = treeView.index(row, column)
                    treeView.selectionModel.setCurrentIndex(index, ItemSelectionModel.NoUpdate)
                    if (! expanded) {
                        treeModel.expandChildAtIndex(index);
                        treeView.expand(row)
                    } else {
                        treeView.collapse(row)                        
                    }
                }
            }
        }

        Label {
            id: label
            x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - padding - x
            clip: true
            text: model.display
        }
    }
}
