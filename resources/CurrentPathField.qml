import QtQuick
import QtQuick.Controls

import localhost.Utility 1.0

Rectangle {
    id: "fieldBorder"
    height: childrenRect.height + border.width * 2
    color: Constants.defaultMidLightColor
    border.color: Constants.defaultBorderColor
    border.width: Constants.defaultBorderWidth
    property alias acceptedText: pathField.acceptedText

    TextInput {
        id: "pathField"
        width: parent.width - parent.border.width * 2
        y: parent.border.width
        x: parent.border.width
        property string acceptedText: ""
        text: acceptedText
        Binding on text {
                    when: ! pathField.activeFocus
                    value: pathField.acceptedText
                    restoreMode: Binding.RestoreNone
                }
        onAccepted: {
            acceptedText = text
        }
        Keys.onEscapePressed: focus = false
        validator: DirectoryValidator {}
        color: acceptableInput ? "green": "red"
        font.pixelSize: Constants.defaultFont.pixelSize * 1.5
        selectByMouse: true
        onFocusChanged: {
            if (activeFocus) {
                LoseFocusDetector.focusedObject = pathField
            }
        }
    }
}
