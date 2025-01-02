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
    Text {
        id: "emptyText"
    }
    readonly property font defaultFont: emptyText.font 
    readonly property color defaultDarkColor: palette.dark
    palette.disabled.button: defaultDarkColor
    readonly property color defaultMidDarkColor: palette.mid
    readonly property color defaultMidLightColor: palette.midlight
    readonly property color defaultBackgroundColor: palette.base
    readonly property color defaultAccentColor: palette.accent
    readonly property real defaultBorderWidth: 2
    readonly property color defaultBorderColor: defaultAccentColor 

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
            anchors.centerIn: parent
            anchors.right: parent.right
            width: parent.width
            height: parent.height

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
                                width: parent.height
                                height: parent.height
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
                                width: parent.height
                                height: parent.height
                            }
                            onClicked: {
                                directoryTree.downRootToSelected();
                            }
                        }
                    }

                    Rectangle {
                        id: "currentPathFieldBorder"
                        anchors.top: treeControlButtons.bottom
                        width: parent.width
                        height: childrenRect.height + border.width * 2
                        color: wnd.defaultMidLightColor
                        border.color: wnd.defaultBorderColor
                        border.width: wnd.defaultBorderWidth

                        TextInput {
                            id: "currentPathField"
                            width: parent.width - parent.border.width * 2
                            y: parent.border.width
                            x: parent.border.width
                            property string acceptedText: directoryTree.rootDirectory 
                            text: acceptedText
                            Binding on text {
                                        when: !currentPathField.activeFocus
                                        value: currentPathField.acceptedText
                                        restoreMode: Binding.RestoreNone
                                    }
                            onAccepted: {
                                directoryTree.rootDirectory = text
                            }
                            Keys.onEscapePressed: currentPathField.focus = false
                            validator: DirectoryValidator {}
                            color: acceptableInput ? "green": "red"
                            font.pixelSize: defaultFont.pixelSize * 1.5
                            selectByMouse: true
                            onFocusChanged: {
                                if (activeFocus) {
                                    LoseFocusDetector.focusedObject = currentPathField
                                }
                            }
                        }
                    }

                    DirectoryTree {
                        id: "directoryTree"
                        anchors.top: currentPathFieldBorder.bottom
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
                        border.color: wnd.defaultBorderColor
                        border.width: wnd.defaultBorderWidth
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
                        if (heightUnder * previewImage.aspectRatio < widthUnder) {
                            relSpaceUnder = heightUnder * previewImage.aspectRatio
                        }
                        let relSpaceNextTo = widthNextTo
                        if (heightNextTo * previewImage.aspectRatio < widthNextTo) {
                            relSpaceNextTo = heightNextTo * previewImage.aspectRatio
                        }
                        return relSpaceUnder > relSpaceNextTo
                    }
                    Rectangle {
                        id: "previewBox"
                        width: parent.placePreviewImageUnder? parent.width: parent.spaceNextToInfo 
                        height: parent.placePreviewImageUnder? parent.spaceUnderInfo: parent.height
                        border.color: wnd.defaultBorderColor
                        border.width: wnd.defaultBorderWidth
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
                                property real aspectRatio: sourceSize.height != 0 ? sourceSize.width / sourceSize.height: 1
                            }
                        }
                    }
                }

                Flow {
                    id: "settingsTab"
                    spacing: 10
                    Rectangle {
                        width: childrenRect.width + 20
                        height: childrenRect.height + 20
                        color: "transparent"
                        border.color: wnd.defaultBorderColor
                        border.width: wnd.defaultBorderWidth

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
                                    checked: ! Settings.getToolPanelPosition()
                                    onToggled: {
                                        mainSplit.changeSplitLayoutOrder()
                                        Settings.setToolPanelPosition(mainSplit.toolsFirst)
                                    }
                                }
                                Label {
                                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                    text: mainSplit.orientation == Qt.Horizontal ? qsTr("Right") : qsTr("Bottom")
                                }
                            }
                            Label {
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                text: qsTr("Orientation")
                            }
                            CheckBox {
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                id: "autoChangeOrientationSetting"
                                text: qsTr("Change automatically")
                                checkable: true
                                checked: Settings.getLayoutOrientationAutoChange()
                                onCheckedChanged: {
                                    Settings.setLayoutOrientationAutoChange(checked)
                                }
                            }
                            RowLayout {
                                enabled: !autoChangeOrientationSetting.checked
                                Label {
                                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                    text: qsTr("Horizontal")
                                }
                                Switch {
                                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                    id: "orientationSwitch"
                                    checked: Settings.getLayoutOrientation() === Qt.Vertical
                                    onCheckedChanged: {
                                        Settings.setLayoutOrientation(checked? Qt.Vertical: Qt.Horizontal)
                                    }
                                }
                                Label {
                                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                    text: qsTr("Vertical")
                                }
                            }
                        }
                    }
            
                    Rectangle {
                        width: childrenRect.width + 20
                        height: childrenRect.height + 20
                        color: "transparent"
                        border.color: wnd.defaultBorderColor
                        border.width: wnd.defaultBorderWidth

                        ColumnLayout {
                            x: 10
                            y: 10
                            Label {
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                text: qsTr("Language")
                            }
                            ComboBox {
                                id: "languageChangeList"
                                currentIndex: Settings.getLanguageNum()
                                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                                signal languageChanged(newLang: string)
                                onActivated: {
                                    languageChangeList.languageChanged(currentText);
                                    Settings.setLanguageNum(currentIndex)
                                }
                                Component.onCompleted: {
                                    languageChangeList.languageChanged.connect(Settings.handleChangeLanguage)
                                }
                                textRole: "text"
                                model: ListModel {
                                    id: model
                                    ListElement {
                                        text: "en"
                                        img: "images/uk.png"    
                                    }
                                    ListElement {
                                        text: "ru"
                                        img: "images/ru.png"    
                                    }
                                }
                                delegate: ItemDelegate {
                                    text: model.text
                                    icon.source: model.img
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: "mainPanel"
            anchors.fill: parent

            Flow {
                id: "collectionInstrumentsPanel"
                width: parent.width - 10
                x: 10
                y: 10
                spacing: 5
                Row {
                    Image {
                        source: "images/tree.png"
                        height: groupingMenuButton.height
                        width: height
                    }
                    SpinBox {
                        id: "lookupDepthSpinBox"
                        from: 0
                        to: 2
                        value: 0

                        TextMetrics {
                            id: "lookupDepthTextMetrics"
                            text: "0"
                        }
                        width: lookupDepthTextMetrics.width * 2 + up.implicitIndicatorWidth
                        onFocusChanged: {
                            if (activeFocus) {
                                LoseFocusDetector.focusedObject = lookupDepthSpinBox
                            }
                        }
                        ToolTip.visible: focus
                        ToolTip.text: qsTr("Subfolders lookup depth")

                        onValueChanged: {
                            pictureProvider.lookupDepth = value                    
                        }
                    }
                }
                Button {
                    id: groupingMenuButton
                    text: qsTr("Grouping")
                    onClicked: groupingMenu.visible = !groupingMenu.visible

                    Menu {
                        id: groupingMenu
                        y: groupingMenuButton.height
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                        CheckBox {
                            text: qsTr("By directory")
                            checkable: true
                            checked: false
                            onCheckedChanged: {
                                mainCollection.setGrouping(PictureCollection.GroupingType.directory, checked)
                            }
                        }
                        CheckBox {
                            text: qsTr("By day")
                            checkable: true
                            checked: false
                            onCheckedChanged: {
                                mainCollection.setGrouping(PictureCollection.GroupingType.day, checked)
                            }
                        }                        
                        CheckBox {
                            text: qsTr("By month")
                            checkable: true
                            checked: false
                            onCheckedChanged: {
                                mainCollection.setGrouping(PictureCollection.GroupingType.month, checked)
                            }
                        }                        
                    }
                }
                Row {
                    // width: sortingMenuButton.width + inverseSortingMenuButton.width
                    // height: sortingMenuButton.height
                    Button {
                        id: sortingMenuButton
                        text: qsTr("Sorting")
                        onClicked: sortingMenu.visible = !sortingMenu.visible

                        Menu {
                            id: sortingMenu
                            y: sortingMenuButton.height
                            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                            RadioButton {
                                text: qsTr("By name")
                                checked: true
                                onCheckedChanged: {
                                    if (checked) {
                                        mainCollection.setSorting(PictureCollection.SortingType.name)
                                    }
                                }
                            }
                            RadioButton {
                                text: qsTr("By date")
                                onCheckedChanged: {
                                    if (checked) {
                                        mainCollection.setSorting(PictureCollection.SortingType.date)
                                    }
                                }
                            }                        
                            RadioButton {
                                text: qsTr("By size")
                                onCheckedChanged: {
                                    if (checked) {
                                        mainCollection.setSorting(PictureCollection.SortingType.size)
                                    }
                                }
                            }                        
                        }
                    }
                    Button {
                        id: inverseSortingMenuButton
                        width: sortingMenuButton.height
                        height: sortingMenuButton.height
                        x: sortingMenuButton.width
                        property bool inverseSortingFlag: false
                        Image {
                            source: "images/inverse.png"
                            height: parent.height
                            width: parent.width
                        }
                        onClicked: {
                            inverseSortingFlag = !inverseSortingFlag
                            mainCollection.setSortingInverseFlag(inverseSortingFlag)
                        }
                    }
                }

                Button {
                    id: "filteringMenuButton"
                    text: qsTr("Filtering")
                    onClicked: {
                        filteringMenuPopup.visible = !filteringMenuPopup.visible
                    }

                    Popup {
                        id: "filteringMenuPopup"
                        y: parent.height
                        focus: visible
                        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
                        ColumnLayout {
                            Row {
                                Text {
                                    text: qsTr("By date since")
                                }
                                CheckBox {
                                    id: "byDateSinceFilterEnable"
                                    checkable: true
                                    checked: false
                                    onCheckedChanged: {
                                        if (checked) {
                                            mainCollection.setFilteringByDate(byDateSinceFilterDatePicker.selectedDate, false)       
                                        } else {
                                            mainCollection.resetFilteringByDate(false)
                                        }
                                    }
                                }
                                DatePicker {
                                    id: "byDateSinceFilterDatePicker"
                                    enabled: ! byDateSinceFilterEnable.checked 
                                }
                            }
                            Row {
                                Text {
                                    text: qsTr("By date until")
                                }
                                CheckBox {
                                    id: "byDateUntilFilterEnable"
                                    checkable: true
                                    checked: false
                                    onCheckedChanged: {
                                        if (checked) {
                                            mainCollection.setFilteringByDate(byDateUntilFilterDatePicker.selectedDate, true)       
                                        } else {
                                            mainCollection.resetFilteringByDate(true)
                                        }
                                    }
                                }
                                DatePicker {
                                    id: "byDateUntilFilterDatePicker"
                                    enabled: ! byDateUntilFilterEnable.checked 
                                }
                            }
                        }
                    }
                }
                Row {
                    // width: imageSearchPatternBorder.width + resetImageSearchPatternBorder.width 
                    // height: imageSearchPatternBorder.height
                    Rectangle {
                        id: "imageSearchPatternBorder"
                        width: 100 
                        height: childrenRect.height + border.width * 2
                        border.color: wnd.defaultBorderColor
                        border.width: wnd.defaultBorderWidth
                        clip: true

                        TextInput {
                            id: "imageSearchPattern"
                            width: parent.width - parent.border.width * 2
                            y: parent.border.width
                            x: parent.border.width
                            font.pixelSize: defaultFont.pixelSize * 1.5
                            onEditingFinished: {
                                mainCollection.setSearchPattern(text)
                            }

                            Keys.onEscapePressed: focus = false
                            onFocusChanged: {
                                if (activeFocus) {
                                    LoseFocusDetector.focusedObject = imageSearchPattern
                                }
                            }
                            property string placeholderText: qsTr("Search...")

                            Text {
                                text: imageSearchPattern.placeholderText
                                color: wnd.defaultDarkColor
                                font: imageSearchPattern.font
                                visible: !imageSearchPattern.text && !imageSearchPattern.activeFocus
                            }                
                        }
                    }
                    Rectangle {
                        id: "resetImageSearchPatternBorder"
                        height: imageSearchPatternBorder.height 
                        width: height
                        x: imageSearchPatternBorder.width - border.width
                        border.color: imageSearchPatternBorder.border.color
                        border.width: imageSearchPatternBorder.border.width
                        Image {
                            width: parent.width - parent.border.width * 2
                            height: parent.height - parent.border.width * 2
                            x: parent.border.width
                            y: parent.border.width
                            source: "images/cross.png"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                imageSearchPattern.text = ""
                                mainCollection.setSearchPattern("")
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: "collectionInstrumentsPanelBorders"
                anchors.top: parent.top
                width: parent.width
                height: collectionInstrumentsPanel.height + 20
                color: "transparent"
                border.color: wnd.defaultBorderColor
                border.width: wnd.defaultBorderWidth
            }

            ListView {
                id: "fileList"
                anchors.top: collectionInstrumentsPanelBorders.bottom
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
                                resolutionLabel.text = previewImage.sourceSize.width+"x"+previewImage.sourceSize.height 
                            }
                        }
                    }
                    Rectangle {
                        anchors.fill: parent
                        color: isGroupLabel? wnd.defaultDarkColor:
                               listItem.ListView.isCurrentItem ? wnd.defaultAccentColor: wnd.defaultBackgroundColor
                    }
                    Image {
                        id: listItemIcon
                        source: isGroupLabel? "images/category.png": "images/image.png"
                        width: listItemLabel.font.pixelSize * 1.5
                        height: width
                        anchors.verticalCenter: parent.verticalCenter
                        x: isGroupLabel? 5: 15
                    }
                    Label {
                        id: listItemLabel
                        x: listItemIcon.x + listItemIcon.width + 5
                        text: model.display
                        font.pixelSize: wnd.defaultFont.pixelSize
                    }
                }
            }
        }
    }

    SplitView {
        id: "mainSplit"
        anchors.fill: parent
        orientation: autoChangeOrientationSetting.checked ?
                     (wnd.aspectRatio > 1.0 ? Qt.Horizontal: Qt.Vertical) :
                     (orientationSwitch.checked ? Qt.Vertical: Qt.Horizontal)

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
        readonly property real handleTouchableThickness: 64
        property color handleColor 
        handle: Rectangle {
            id: "handleDelegate"
            implicitWidth: mainSplit.handleVisualThickness
            implicitHeight: mainSplit.handleVisualThickness 
            color: SplitHandle.pressed ? wnd.defaultAccentColor: wnd.defaultDarkColor
            onColorChanged: {
                mainSplit.handleColor = color
            }

            containmentMask: Item {
                x: mainSplit.orientation == Qt.Horizontal ? (handleDelegate.width - width) / 2: 0
                y: mainSplit.orientation == Qt.Horizontal ? 0: (handleDelegate.height - height) / 2
                width: mainSplit.orientation == Qt.Horizontal ? mainSplit.handleTouchableThickness: mainSplit.width 
                height: mainSplit.orientation == Qt.Horizontal ? mainSplit.height: mainSplit.handleTouchableThickness
            }
        }

        ItemSavedSize {
            id: "splitFirst"
            SplitView.minimumWidth: {SplitView.minimumWidth = 50}
            SplitView.minimumHeight: {SplitView.minimumHeight = 50}
            SplitView.preferredWidth: wnd.width * (mainSplit.toolsFirst? 0.3: 0.7)
            SplitView.preferredHeight: wnd.height * (mainSplit.toolsFirst? 0.3: 0.7)
            // Component.onCompleted: {
            //     console.log(Window.width + " " + wnd.width + " " + parent.width)
            //     SplitView.preferredWidth = wnd.width * (mainSplit.toolsFirst? 0.3: 0.7)
            //     SplitView.preferredHeight = wnd.height * (mainSplit.toolsFirst? 0.3: 0.7)
            // }
            // SplitView.preferredWidth: {console.log(mainSplit.width); SplitView.preferredWidth = mainSplit.width * (mainSplit.toolsFirst? 0.3: 0.7);}
            // SplitView.preferredHeight: {SplitView.preferredHeight = mainSplit.height * (mainSplit.toolsFirst? 0.3: 0.7);}
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
        width: mainSplit.orientation == Qt.Horizontal? mainSplit.handleTouchableThickness / 2: expandImage.width
        height: mainSplit.orientation == Qt.Horizontal? expandImage.height: mainSplit.handleTouchableThickness / 2
        
        x: {
            var fullWidth = mainSplit.width
            var splitPos = splitFirst.width + (mainSplit.toolsFirst? mainSplit.handleVisualThickness: 0)
            if (mainSplit.orientation == Qt.Vertical) {
                return fullWidth / 2 - width / 2
            }
            var base = mainSplit.toolsExpanded ? splitPos: (mainSplit.toolsFirst ? 0: fullWidth)
            var offset = mainSplit.toolsFirst ^ mainSplit.toolsExpanded  ? 0: -width
            return base + offset
        }
        y: {
            var fullHeight = mainSplit.height
            var splitPos = splitFirst.height + (mainSplit.toolsFirst? mainSplit.handleVisualThickness: 0)
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
                source: mainSplit.orientation == Qt.Horizontal? "images/double_left.png": "images/double_top.png"
                readonly property real smallSide: mainSplit.handleVisualThickness * 3
                readonly property real bigSide: smallSide * 2
                width: mainSplit.orientation == Qt.Horizontal? smallSide: bigSide
                height: mainSplit.orientation == Qt.Horizontal? bigSide: smallSide
                // transformOrigin: Item.Center
                // rotation: mainSplit.orientation == Qt.Horizontal? 0: 90
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
}

