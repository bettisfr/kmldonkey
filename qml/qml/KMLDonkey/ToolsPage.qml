import QtQuick 1.1

Page {
    id: hostPage
    anchors.margins: 10

    InfoItemView {
        id: hostListView
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        delegate: InfoItemDelegate {
            id: listItem
            Column {
                id: listItemContent
                width: listItem.width
                spacing: 10

//                Label {
//                    text: name
//                    font.weight: Font.Bold
//                    elide: Text.ElideRight
//                    color: (
//                        connectionState == "disconnected" ? "#ffffff" :
//                        connectionState == "connected" ? "#66ff66" :
//                        connectionState == "connecting" ? "#ffff66" :
//                        connectionState == "error" ? "#ff6666" :
//                        "#ffffff")
//                }
//                Label {
//                    width: listItemColumn.width
//                    font.weight: Font.Light
//                    elide: Text.ElideRight
//                    text: (username.length > 0 && address.length ? username + '@' + address + ':' + port : address.length > 0 ? address + ':' + port : username)
//                }
            }

            onItemClicked: {
//                var hostitempage = hostListView.openPage("HostItemPage.qml", hostPageStack)
//                console.log("objectInstance=" + hostitempage + " identifier=" + identifier);
//                if (hostitempage != null)
//                    hostitempage.setHostIdentifier(identifier)
            }
        }
    }

    Component.onCompleted: {
    }

}
