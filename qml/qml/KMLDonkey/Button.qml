import QtQuick 1.1

Rectangle {
    id: root

    property alias text: label.text
    property alias textVisible: label.visible
    property bool checkable: false
    property bool checked: false
    property alias font: label.font
    property bool enabled: true
    property alias iconSource: icon.source

    signal clicked
    signal toggled

    height: contentColumn.height + 18
    width: contentColumn.width + 4
    border.width: 1
    radius: 4
    smooth: true
    clip: true

    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: root.enabled && ((root.checkable && root.checked) || (/*!root.checkable &&*/ mouseArea.pressed)) ? "#303030" : "#606060"
        }
        GradientStop {
            position: 1.0
            color: root.enabled && ((root.checkable && root.checked) || (/*!root.checkable &&*/ mouseArea.pressed)) ? "#606060" : "#303030"
        }
    }

//    SystemPalette { id: activePalette }
//    activePalette.buttonText
//    activePalette.button

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            if (!root.enabled)
                return
            if (root.checkable) {
//                root.checked = !root.checked
                root.toggled()
            } else {
                root.clicked()
            }
        }
    }

    Column {
        id: contentColumn
        anchors {
            centerIn: parent
            margins: 5
        }

        Image {
            id: icon
            anchors.horizontalCenter: parent.horizontalCenter
            visible: source != null && source != undefined && source.toString().length > 0
            clip: true
        }

        Text {
            id: label
            anchors.horizontalCenter: parent.horizontalCenter
            font.pointSize: 11
            color: root.enabled ? "#ffffff" : "#939393"
            clip: true
        }
    }
}
