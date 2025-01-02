import QtQuick
import QtQuick.Controls
import localhost.Utility 1.0

Row {
    property date currentDate: new Date()
    property alias year: yearSpin.value
    property alias month: monthSpin.value
    property alias day: daySpin.value
    property date selectedDate: new Date(year, month, day)

    SpinBox {
        id: "yearSpin"
        from: 2000
        to: 3000
        value: parent.currentDate.getFullYear()
        editable: true

        textFromValue: function(value, locale) {
            return value;
        }
        validator: StrictIntValidator {
            minVal: yearSpin.from
            maxVal: yearSpin.to
        }
        TextMetrics {
            id: "yearTextMetrics"
            text: "0000"
        }
        width: yearTextMetrics.width * 1.5 + up.implicitIndicatorWidth

        onFocusChanged: {
            if (activeFocus) {
                LoseFocusDetector.focusedObject = yearSpin
            }
        }
    }
    SpinBox {
        id: "monthSpin"
        from: 0
        to: 11
        value: parent.currentDate.getMonth()

        property var items: [qsTr("Jan."), qsTr("Feb."), qsTr("Mar."), qsTr("Apr."),qsTr("May"), qsTr("June"),
                             qsTr("July"), qsTr("Aug."), qsTr("Sept."), qsTr("Oct."), qsTr("Nov."), qsTr("Dec.")]

        // validator: RegularExpressionValidator {
        //     regularExpression: new RegExp("(Small|Medium|Large)", "i")
        // }

        textFromValue: function(value) {
            return items[value];
        }

        valueFromText: function(text) {
            for (var i = 0; i < items.length; ++i) {
                if (items[i].toLowerCase().indexOf(text.toLowerCase()) === 0)
                    return i
            }
            return spinBox.value
        }
        TextMetrics {
            id: "monthTextMetrics"
            text: monthSpin.displayText
        }
        width: monthTextMetrics.width * 1.5 + up.implicitIndicatorWidth

        onFocusChanged: {
            if (activeFocus) {
                LoseFocusDetector.focusedObject = monthSpin
            }
        }
    }
    SpinBox {
        id: "daySpin"
        from: 1
        to: new Date(yearSpin.value, monthSpin.value + 1, 0).getDate();
        value: parent.currentDate.getDate()

        editable: true

        textFromValue: function(value, locale) {
            return value;
        }
        validator: StrictIntValidator {
            minVal: daySpin.from
            maxVal: daySpin.to
        }
        TextMetrics {
            id: "dayTextMetrics"
            text: "00"
        }
        width: dayTextMetrics.width * 1.5 + up.implicitIndicatorWidth

        onFocusChanged: {
            if (activeFocus) {
                LoseFocusDetector.focusedObject = daySpin
            }
        }
    }
}

