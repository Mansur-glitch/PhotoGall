import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TabButton {
    height: tabButtonText.height + 10
    anchors.verticalCenter: parent.verticalCenter
    property alias buttonText: tabButtonText.text
    property alias imageSource: image.source
    Text {
        id: "tabButtonText"
        y: 5
        x: parent.height + 5
        font.pixelSize: wnd.defaultFont.pixelSize * 1.5
    }
    Image {
        id: "image"
        y: 5
        width: tabButtonText.height
        height: tabButtonText.height
    }
}
