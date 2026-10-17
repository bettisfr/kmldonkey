import QtQuick 1.1

Rectangle {
    id: sectionHeader
    width: parent.width
    height: childrenRect.height
    color: "#000000"
    property string sectionText: ""
    Rectangle {
        width: parent.width - sectionLabel.implicitWidth - 10;
        height: 2
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: sectionLabel.height / 2
        color: "#969696"
    }
    Label {
        id: sectionLabel
        anchors.topMargin: 2
        anchors.bottomMargin: 2
        text: sectionText
        font.bold: true
        color: "#969696"
        width: parent.width
        horizontalAlignment: Text.AlignRight
    }
}
