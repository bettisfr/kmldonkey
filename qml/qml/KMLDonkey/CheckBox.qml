import QtQuick 1.1

Item {
    width: boxNo.width + boxYes.width + label.width + label.anchors.leftMargin
    height: label.height

    property alias text: label.text
    property bool checked: false

    Rectangle {
        id: boxNo
        color: checked ? "#330000" : "#cc0000"
        border.color : "#ff0000"
        border.width : checked ? 0 : 2
        radius: 2
        width: label.height
        height: label.height
        anchors {
            left: parent.left
            top: parent.top
        }
    }

    Rectangle {
        id: boxYes
        color: checked ? "#00cc00" : "#003300"
        border.color : "#00ff00"
        border.width : checked ? 2 : 0
        radius: 2
        width: label.height
        height: label.height
        anchors {
            left: boxNo.right
            leftMargin: 2
            top: parent.top
        }
    }

    Label {
        id: label
        text: ""
        anchors {
            left: boxYes.right
            leftMargin: 6
            top: parent.top
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: checked = !checked
    }
}
