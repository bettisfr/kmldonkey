import QtQuick 1.1

Rectangle {
    id: root
    width: Math.max(edit.width, 200) + 8
    height: edit.height + 8
    color: "#000000"
    radius: 4
    border.color : "#000063"
    border.width : 2

    property alias text: edit.text
    signal accepted

//    Rectangle {
//        anchors {
//            left: root.left
//            right: root.right
//            top: root.top
//            bottom: root.bottom
//            margins: 2
//        }
//        color: "#000063"
//    }

    TextInput {
        id: edit
        anchors {
            left: root.left
            right: root.right
            top: root.top
            bottom: root.bottom
            margins: 4
        }
        font.pointSize: 11
        color: "#ffffff"
        onAccepted: root.accepted()
        inputMethodHints: Qt.ImhNoPredictiveText
    }
}
