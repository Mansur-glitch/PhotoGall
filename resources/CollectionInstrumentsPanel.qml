import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import localhost.Utility 1.0
import localhost.PictureModel 1.0

Rectangle {
    id: "panel"
    height: instrumentButtons.height + 20
    color: "transparent"
    border.color: Constants.defaultBorderColor
    border.width: Constants.defaultBorderWidth
    required property PictureProvider pictureProvider
    required property PictureCollection pictureCollection

    Flow {
        id: "instrumentButtons"
        width: parent.width - 20
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
                width: lookupDepthTextMetrics.width * 2 
                                            + up.implicitIndicatorWidth
                onFocusChanged: {
                    if (activeFocus) {
                        LoseFocusDetector.focusedObject = lookupDepthSpinBox
                    }
                }
                ToolTip.visible: focus
                ToolTip.text: qsTr("Subfolders lookup depth")

                onValueChanged: {
                    panel.pictureProvider.lookupDepth = value                    
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
                closePolicy: Popup.CloseOnEscape 
                             | Popup.CloseOnPressOutsideParent

                CheckBox {
                    text: qsTr("By directory")
                    checkable: true
                    checked: false
                    onCheckedChanged: {
                        panel.pictureCollection.setGrouping(
                            PictureCollection.GroupingType.directory, checked)
                    }
                }
                CheckBox {
                    text: qsTr("By day")
                    checkable: true
                    checked: false
                    onCheckedChanged: {
                        panel.pictureCollection.setGrouping(
                                PictureCollection.GroupingType.day, checked)
                    }
                }                        
                CheckBox {
                    text: qsTr("By month")
                    checkable: true
                    checked: false
                    onCheckedChanged: {
                        panel.pictureCollection.setGrouping(
                                PictureCollection.GroupingType.month, checked)
                    }
                }                        
            }
        }
        Row {
            Button {
                id: sortingMenuButton
                text: qsTr("Sorting")
                onClicked: sortingMenu.visible = !sortingMenu.visible
                Menu {
                    id: sortingMenu
                    y: sortingMenuButton.height
                    closePolicy: Popup.CloseOnEscape
                                 | Popup.CloseOnPressOutsideParent

                    RadioButton {
                        text: qsTr("By name")
                        checked: true
                        onCheckedChanged: {
                            if (checked) {
                                panel.pictureCollection.setSorting(
                                            PictureCollection.SortingType.name)
                            }
                        }
                    }
                    RadioButton {
                        text: qsTr("By date")
                        onCheckedChanged: {
                            if (checked) {
                                panel.pictureCollection.setSorting(
                                            PictureCollection.SortingType.date)
                            }
                        }
                    }                        
                    RadioButton {
                        text: qsTr("By size")
                        onCheckedChanged: {
                            if (checked) {
                                panel.pictureCollection.setSorting(
                                            PictureCollection.SortingType.size)
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
                    panel.pictureCollection.setSortingInverseFlag(
                                                        inverseSortingFlag)
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
                closePolicy: Popup.CloseOnEscape 
                             | Popup.CloseOnPressOutsideParent
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
                                    panel.pictureCollection.setFilteringByDate(
                                      byDateSinceFilterDatePicker.selectedDate,
                                      false)       
                                } else {
                                    panel.pictureCollection.
                                                resetFilteringByDate(false)
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
                                    panel.pictureCollection.setFilteringByDate(
                                     byDateUntilFilterDatePicker.selectedDate,
                                                                          true)
                                } else {
                                    panel.pictureCollection.
                                                    resetFilteringByDate(true)
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
            Rectangle {
                id: "imageSearchPatternBorder"
                width: 100 
                height: childrenRect.height + border.width * 2
                border.color: Constants.defaultBorderColor
                border.width: Constants.defaultBorderWidth
                clip: true

                TextInput {
                    id: "imageSearchPattern"
                    width: parent.width - parent.border.width * 2
                    y: parent.border.width
                    x: parent.border.width
                    font.pixelSize: Constants.defaultFont.pixelSize * 1.5
                    onEditingFinished: {
                        panel.pictureCollection.setSearchPattern(text)
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
                        color: Constants.defaultDarkColor
                        font: imageSearchPattern.font
                        visible: !imageSearchPattern.text && 
                                 !imageSearchPattern.activeFocus
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
                        panel.pictureCollection.setSearchPattern("")
                    }
                }
            }
        }
    }                       
}
