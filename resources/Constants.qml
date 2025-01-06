// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
pragma Singleton
import QtQuick
Item {
    Text {
        id: "emptyText"
        visible: false
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
}
