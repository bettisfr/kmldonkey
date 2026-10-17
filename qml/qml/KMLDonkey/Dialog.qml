import QtQuick 1.1

Rectangle {
    id: root
    width: parent.width
    height: parent.height
    color: "#000000"

    property string title: ""
    property string content: ""

    Column {
        spacing: 10
        Label {
            id: titleLabel
            text: title
            anchors.horizontalCenter: parent.horizontalCenter
            font.bold: true
        }
        Label {
            id: contentLabel
            text: content
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 10
        }
        Row {
            Button {
                id: okButtom
                onClicked: {
                    //TODO
                    //appWindow.pageStack.pop()
                }
            }
        }
    }
}
