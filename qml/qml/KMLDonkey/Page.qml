import QtQuick 1.1

Rectangle {
    id: root
    color: "#000000"
    //visible: false

    anchors {
        fill: parent
    }

    signal activating()
    signal activated()
    signal deactivating()
    signal deactivated()
}
