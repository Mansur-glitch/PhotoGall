import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import localhost.Utility 1.0

Flow {
    id: "settingsTab"
    spacing: 10
    readonly property alias autoChangeOrientation: autoChangeOrientation.checked 
    readonly property int appOrientation: orientationSwitch.checked?
                                                 Qt.Vertical: Qt.Horizontal
    required property int actualOrientation
    property bool appToolPanelFirst
    Rectangle {
        width: childrenRect.width + 20
        height: childrenRect.height + 20
        color: "transparent"
        border.color: Constants.defaultBorderColor
        border.width: Constants.defaultBorderWidth

        ColumnLayout {
            x: 10
            y: 10
            Label {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Tool panel position")
            }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                Label {
                    text: settingsTab.actualOrientation == Qt.Horizontal ?
                                              qsTr("Left") : qsTr("Top")
                }
                Switch {
                    id: "splitLayoutSwitch"
                    checked: ! Settings.getToolPanelPosition()
                    onToggled: {
                        settingsTab.appToolPanelFirst = 
                                !settingsTab.appToolPanelFirst 
                        Settings.setToolPanelPosition(
                                settingsTab.appToolPanelFirst)
                    }
                }
                Label {
                    text: settingsTab.actualOrientation == Qt.Horizontal ?
                                          qsTr("Right") : qsTr("Bottom")
                }
            }
            Label {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Orientation")
            }
            CheckBox {
                Layout.alignment: Qt.AlignHCenter
                id: "autoChangeOrientation"
                text: qsTr("Change automatically")
                checkable: true
                checked: Settings.getLayoutOrientationAutoChange()
                onCheckedChanged: {
                    Settings.setLayoutOrientationAutoChange(checked)
                }
            }
            RowLayout {
                enabled: !autoChangeOrientation.checked
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Horizontal")
                }
                Switch {
                    Layout.alignment: Qt.AlignHCenter
                    id: "orientationSwitch"
                    checked: Settings.getLayoutOrientation() === Qt.Vertical
                    onCheckedChanged: {
                        Settings.setLayoutOrientation(
                                checked? Qt.Vertical: Qt.Horizontal)
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
        border.color: Constants.defaultBorderColor
        border.width: Constants.defaultBorderWidth

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
                    languageChangeList.languageChanged.
                        connect(Settings.handleChangeLanguage)
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
