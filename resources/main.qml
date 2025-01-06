// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import localhost.Utility 1.0
import localhost.PictureModel 1.0
import localhost.DirectoryTreeModel 1.0

Window {
    id: wnd
    visible: true
    title: qsTr("PhotoGall")

    property real aspectRatio: width / height
    PictureProvider {
        id: "pictureProvider"
        directory: directoryTree.selectedDirectory
    }

    PictureCollection {
        id: "mainCollection"
        provider: pictureProvider
    }

    Item {
        id: "hiddenWidgets"
        visible: false
        Rectangle {
            id: "toolPanel"
            anchors.fill: parent
            TabBar {
                id: "toolTabs"
                anchors.top: parent.top
                width: parent.width
                height: filesystemTabButton.height

                ToolsTabButton {
                    id: "filesystemTabButton"
                    buttonText: qsTr("Filesystem")
                    imageSource: "images/files.png"
                    clip: true
                }
                ToolsTabButton {
                    id: "infoTabButton"
                    buttonText: qsTr("Info")
                    imageSource: "images/info.png"
                    clip: true
                }
                ToolsTabButton {
                    id: "settingsTabButton"
                    buttonText: qsTr("Settings")
                    imageSource: "images/gears.png"
                    clip: true
                }
            }

            StackLayout {
                id: "toolStack"
                anchors.top: toolTabs.bottom
                anchors.bottom: parent.bottom
                width: parent.width
                currentIndex: toolTabs.currentIndex

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    RowLayout {
                        id: "treeControlButtons"
                        anchors.top: parent.top
                        width: parent.width
                        height: filesystemTabButton.height
                        spacing: 0
                        Button {
                            id: "upTreeButton"
                            height: filesystemTabButton.height
                            enabled: ! directoryTree.isFilesystemRoot
                            Layout.fillWidth: true
                            Image {
                                source: "images/up.png"
                                anchors.centerIn: parent
                                height: parent.height
                                width: height
                            }
                            onClicked: {
                                directoryTree.setUpperRoot();
                            }
                        }
                        Button {
                            height: filesystemTabButton.height
                            enabled: directoryTree.hasSelection
                            Layout.fillWidth: true
                            Image {
                                source: "images/down_to.png"
                                anchors.centerIn: parent
                                height: parent.height
                                width: height
                            }
                            onClicked: {
                                directoryTree.downRootToSelected();
                            }
                        }
                    }

                    CurrentPathField {
                        id: "currentPathField"
                        anchors.top: treeControlButtons.bottom
                        width: parent.width
                        acceptedText: directoryTree.rootDirectory
                        onAcceptedTextChanged: {
                            // Avoid loop binding
                            if (directoryTree.rootDirectory != acceptedText) {
                                directoryTree.rootDirectory = acceptedText
                            }
                        }
                    }

                    DirectoryTree {
                        id: "directoryTree"
                        anchors.top: currentPathField.bottom
                        anchors.bottom: parent.bottom
                        width: parent.width
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Rectangle {
                        id: "imageInfoBox"
                        width: childrenRect.width + border.width * 2
                        height: childrenRect.height + border.width * 2
                        border.color: Constants.defaultBorderColor
                        border.width: Constants.defaultBorderWidth
                        Column {
                            x: parent.border.width
                            y: parent.border.width
                            Row {
                                Label {
                                    text: qsTr("File name: ")
                                    font.bold: true
                                }
                                Label {
                                    id: "fileNameLabel"
                                }
                            }
                            Row {
                                Label {
                                    text: qsTr("Directory: ")
                                    font.bold: true
                                }
                                Label {
                                    id: "directoryLabel"
                                }
                            }
                            Row {
                                Label {
                                    text: qsTr("Last modified: ")
                                    font.bold: true
                                }
                                Label {
                                    id: "lastModifiedLabel"
                                }
                            }
                            Row {
                                Label {
                                    text: qsTr("Size: ")
                                    font.bold: true
                                }
                                Label {
                                    id: "sizeLabel"
                                }
                            }
                            Row {
                                Label {
                                    text: qsTr("Resolution: ")
                                    font.bold: true
                                }
                                Label {
                                    id: "resolutionLabel"
                                }
                            }
                            Button {
                                id: "openImageButton"
                                text: qsTr("Open")
                                enabled: false
                                onClicked: {
                                    fullWindowImage.source = previewImage.source
                                    fullWindowViewer.visible = true
                                }
                            }
                        }                       
                    }
                    property real spaceUnderInfo: height - imageInfoBox.height
                    property real spaceNextToInfo: width - imageInfoBox.width
                    property bool placePreviewImageUnder: {
                        let widthUnder = width                    
                        let heightUnder = spaceUnderInfo
                        let widthNextTo = spaceNextToInfo
                        let heightNextTo = height

                        let relSpaceUnder = widthUnder
                        if (heightUnder * previewImage.aspectRatio
                                                     < widthUnder) {
                            relSpaceUnder = heightUnder 
                                            * previewImage.aspectRatio
                        }
                        let relSpaceNextTo = widthNextTo
                        if (heightNextTo * previewImage.aspectRatio
                                                         < widthNextTo) {
                            relSpaceNextTo = heightNextTo
                                             * previewImage.aspectRatio
                        }
                        return relSpaceUnder > relSpaceNextTo
                    }
                    Rectangle {
                        id: "previewBox"
                        width: parent.placePreviewImageUnder?
                                     parent.width: parent.spaceNextToInfo 
                        height: parent.placePreviewImageUnder?
                                     parent.spaceUnderInfo: parent.height
                        border.color: Constants.defaultBorderColor
                        border.width: Constants.defaultBorderWidth
                        Image {
                            width: parent.width - parent.border.width * 2 
                            height: parent.height - parent.border.width * 2 
                            x: parent.border.width
                            y: parent.border.width
                            source: "images/transparency.jpg"
                            fillMode: Image.Tile
                            horizontalAlignment: Image.AlignLeft
                            verticalAlignment: Image.AlignTop
                            smooth: false
                            Image {
                                id: "previewImage"
                                width:  parent.width 
                                height: parent.height
                                fillMode: Image.PreserveAspectFit
                                visible: false
                                property real aspectRatio: 
                                                sourceSize.height != 0 ?
                                                sourceSize.width 
                                                / sourceSize.height: 1
                            }
                        }
                    }
                }

                SettingsTab {
                    id: "settingsTab"
                    onAppToolPanelFirstChanged: {
                        mainSplit.changeSplitLayoutOrder()
                    }
                    actualOrientation: mainSplit.orientation
                    aboutProgPopupWindow: aboutProgrammPopup
                }
            }
        }

        Rectangle {
            id: "mainPanel"
            anchors.fill: parent

            CollectionInstrumentsPanel {
                id: "collectionInstrumentsPanel"
                anchors.top: parent.top
                width: parent.width
                pictureProvider: pictureProvider
                pictureCollection: mainCollection
            }

            ListView {
                id: "fileList"
                anchors.top: collectionInstrumentsPanel.bottom
                anchors.bottom: parent.bottom
                width: parent.width
                focus: true
                clip: true
                model: PictureListModel {
                    collection: mainCollection
                }
                delegate: Item {
                    id: listItem
                    implicitWidth: fileList.width
                    implicitHeight: listItemLabel.implicitHeight * 1.5
                    property bool isGroupLabel: model.groupFlag
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            fileList.currentIndex = index
                            if (model.groupFlag) {
                                fileNameLabel.text = ""
                                directoryLabel.text = ""
                                lastModifiedLabel.text = ""
                                sizeLabel.text = ""
                                resolutionLabel.text = ""
                                openImageButton.enabled = false

                                previewImage.visible = false
                            } else {
                                fileNameLabel.text = model.fileName
                                directoryLabel.text = model.directory
                                lastModifiedLabel.text = model.lastModified
                                sizeLabel.text = model.size

                                toolTabs.currentIndex = 1
                                openImageButton.enabled = true

                                previewImage.source = "file:" + model.filePath
                                previewImage.visible = true
                                resolutionLabel.text = 
                                    previewImage.sourceSize.width
                                    + "x" +previewImage.sourceSize.height 
                            }
                        }
                    }
                    Rectangle {
                        anchors.fill: parent
                        color: isGroupLabel? Constants.defaultDarkColor:
                               listItem.ListView.isCurrentItem ?
                               Constants.defaultAccentColor :
                               Constants.defaultBackgroundColor
                    }
                    Image {
                        id: listItemIcon
                        source: isGroupLabel? "images/category.png":
                                                     "images/image.png"
                        width: listItemLabel.font.pixelSize * 1.5
                        height: width
                        anchors.verticalCenter: parent.verticalCenter
                        x: isGroupLabel? 5: 15
                    }
                    Label {
                        id: listItemLabel
                        x: listItemIcon.x + listItemIcon.width + 5
                        text: model.display
                        font.pixelSize: Constants.defaultFont.pixelSize
                    }
                }
            }
        }
    }

    SplitView {
        id: "mainSplit"
        anchors.fill: parent
        orientation: settingsTab.autoChangeOrientation ?
                     (wnd.aspectRatio > 1.0 ? Qt.Horizontal: Qt.Vertical) :
                     settingsTab.appOrientation

        property bool toolsFirst: Settings.getToolPanelPosition()
        property bool toolsExpanded: true
        property list<QtObject> splitItems
        property var toolsItem
        property var galleryItem
        function changeSplitLayoutOrder() {
            toolsFirst = !toolsFirst;
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
        readonly property real handleVisualThickness: 8
        readonly property real handleTouchableThickness: 32
        property color handleColor 
        handle: Rectangle {
            id: "handleDelegate"
            implicitWidth: mainSplit.handleVisualThickness
            implicitHeight: mainSplit.handleVisualThickness 
            color: SplitHandle.pressed ? Constants.defaultAccentColor:
                                             Constants.defaultDarkColor
            onColorChanged: {
                mainSplit.handleColor = color
            }

            containmentMask: Item {
                x: mainSplit.orientation == Qt.Horizontal ?
                         (handleDelegate.width - width) / 2: 0
                y: mainSplit.orientation == Qt.Horizontal ?
                         0: (handleDelegate.height - height) / 2
                width: mainSplit.orientation == Qt.Horizontal ?
                         mainSplit.handleTouchableThickness: mainSplit.width
                height: mainSplit.orientation == Qt.Horizontal ?
                         mainSplit.height: mainSplit.handleTouchableThickness
            }
        }

        ItemSavedSize {
            id: "splitFirst"
            SplitView.minimumWidth: {SplitView.minimumWidth = 50}
            SplitView.minimumHeight: {SplitView.minimumHeight = 50}
            SplitView.preferredWidth: wnd.width * 
                                        (mainSplit.toolsFirst? 0.3: 0.7)
            SplitView.preferredHeight: wnd.height * 
                                        (mainSplit.toolsFirst? 0.3: 0.7)
        }

        ItemSavedSize {
            id: "splitSecond"
            SplitView.minimumWidth: {SplitView.minimumWidth = 50}
            SplitView.minimumHeight: {SplitView.minimumHeight = 50}
        }

        Component.onCompleted: {
            splitItems.length = 2
            splitItems[0] = splitFirst
            splitItems[1] = splitSecond
            toolPanel.parent = toolsFirst? splitFirst: splitSecond
            mainPanel.parent = toolsFirst? splitSecond: splitFirst
            toolsItem = toolsFirst? splitFirst: splitSecond
            galleryItem = toolsFirst? splitSecond: splitFirst
        }
    }

    Item {
        id: "splitButton"
        width: mainSplit.orientation == Qt.Horizontal? 
                                    mainSplit.handleTouchableThickness / 2
                                    : expandImage.width
        height: mainSplit.orientation == Qt.Horizontal?
                                    expandImage.height
                                    : mainSplit.handleTouchableThickness / 2
        
        x: {
            var fullWidth = mainSplit.width
            var splitPos = splitFirst.width + 
                (mainSplit.toolsFirst? mainSplit.handleVisualThickness: 0)
            if (mainSplit.orientation == Qt.Vertical) {
                return fullWidth / 2 - width / 2
            }
            var base = mainSplit.toolsExpanded ?
                         splitPos: (mainSplit.toolsFirst ? 0: fullWidth)
            var offset = mainSplit.toolsFirst ^ mainSplit.toolsExpanded ?
                                                                 0: -width
            return base + offset
        }
        y: {
            var fullHeight = mainSplit.height
            var splitPos = splitFirst.height +
                     (mainSplit.toolsFirst? mainSplit.handleVisualThickness: 0)
            if (mainSplit.orientation == Qt.Horizontal) {
                return fullHeight / 2 - height / 2
            }
            var base = mainSplit.toolsExpanded ?
                     splitPos: (mainSplit.toolsFirst ? 0: mainSplit.height)
            var offset = mainSplit.toolsFirst ^ mainSplit.toolsExpanded ?
                                                                 0: -height
            return base + offset
        }
        z: mainSplit.z + 1

        MouseArea {
            anchors.fill: parent
            onClicked: {
                mainSplit.toolsExpanded = ! mainSplit.toolsExpanded 
            }
        }
        Rectangle {
            color: mainSplit.handleColor
            radius: 5
            width: childrenRect.width
            height: childrenRect.height
            x: {
                if (mainSplit.orientation == Qt.Vertical) {
                    return 0;
                }
                if (! mainSplit.toolsFirst ^ mainSplit.toolsExpanded) {
                    return parent.width - width;
                } else {
                    return 0;
                }
            }
            y: {
                if (mainSplit.orientation == Qt.Horizontal) {
                    return 0;
                }
                if (! mainSplit.toolsFirst ^ mainSplit.toolsExpanded) {
                    return parent.height - height;
                } else {
                    return 0;
                }
            }
            Image {
                id: "expandImage"
                source: mainSplit.orientation == Qt.Horizontal?
                         "images/double_left.png": "images/double_top.png"
                readonly property real smallSide: 
                                    mainSplit.handleVisualThickness * 3
                readonly property real bigSide: smallSide * 2
                width: mainSplit.orientation == Qt.Horizontal?
                                                 smallSide: bigSide
                height: mainSplit.orientation == Qt.Horizontal?
                                                 bigSide: smallSide
                mirror: mainSplit.toolsFirst ^ mainSplit.toolsExpanded
                mirrorVertically: mainSplit.toolsFirst ^ mainSplit.toolsExpanded
            }
        }
    }
    Item {
        id: "fullWindowViewer"
        anchors.fill: parent
        z: 2
        visible: false
        focus: visible
        onFocusChanged: {
            if (activeFocus) {
                LoseFocusDetector.focusedObject = fullWindowViewer
            }
        }
        Keys.onEscapePressed: {
            visible = false
        }

        MouseArea {
            anchors.fill: parent
        }
        Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.6
        }
        Button {
            anchors.top: parent.top
            anchors.right: parent.right
            width: 50
            height: 50
            icon.source: "images/cross.png"
            icon.width: 50
            icon.height: 50
            hoverEnabled: true
            opacity: hovered? 1.0: 0.6
            onClicked: {
                fullWindowViewer.visible = false       
            }
        }
        Image {
            id: "fullWindowImage"
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            opacity: 1.0
        }
    }
    
    Popup {
        id: "aboutProgrammPopup"
        width: wnd.width * 2 / 3
        height: wnd.height * 2 / 3
        anchors.centerIn: parent
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        ScrollView {
            width: parent.width
            height: parent.height
            contentWidth: availableWidth
            Text {
                x: 10
                width: aboutProgrammPopup.width - 30
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                onLinkActivated: function(link) {
                    Qt.openUrlExternally(link)
                } 
                text: qsTr("
PhotoGall v0.0.1. Ultra minimal photo gallery<br>
Copyright (C) 2025 Mansur Mukhametzyanov<br>
<br>
Github: <a href=\"https://github.com/Mansur-glitch/PhotoGall\">https://github.com/Mansur-glitch/PhotoGall</a><br>
Icons: <a href=\"https://uxwing.com/\">https://uxwing.com</a><br>
<br>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.<br>
<br>
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.<br>
<br>
You should have received a copy of the GNU General Public License along with this program. If not, see <a href=\"https://www.gnu.org/licenses/\">&lt;https://www.gnu.org/licenses/&gt;</a>")
            }
        }
        Button {
            anchors.top: parent.top
            anchors.right: parent.right
            width: 50
            height: 50
            icon.source: "images/cross.png"
            icon.width: 50
            icon.height: 50
            hoverEnabled: true
            opacity: hovered? 1.0: 0.6
            onClicked: {
                aboutProgrammPopup.visible = false       
            }
        }
    }
}

