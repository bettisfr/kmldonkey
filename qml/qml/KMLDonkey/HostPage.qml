import QtQuick 1.1

Page {
    id: hostPage
    anchors.margins: 10

    property alias hostPageStack: hostpagestack

    PageStack {
        id: hostpagestack
        anchors.fill: parent

//        currentPage: mainHostPage
        onCurrentPageChanged: {
            console.log("HostItem currentPageChanged = " + currentPage)
        }

        Page {
            id: mainHostPage

            Button {
                id: addHostButton
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                }
                text: qsTr("Add Host")
                onClicked: {
                    var hostitempage = hostListView.openPage("HostItemPage.qml", hostPageStack)
                    console.log("objectInstance=" + hostitempage)
                    if (hostitempage != null)
                        hostitempage.setHostIdentifier(undefined)
                }
            }

            InfoItemView {
                id: hostListView
                anchors {
                    left: parent.left
                    right: parent.right
                    top: addHostButton.bottom
                    bottom: parent.bottom
                    topMargin: 10
                }
                delegate: InfoItemDelegate {
                    id: listItem
                    Row {
                        id: listItemContent
                        width: listItem.width
                        height: listItemColumn.height
                        spacing: 10
                        Column {
                            id: listItemColumn
                            width: listItem.width - hostPage.buttonWidth - 10
                            Label {
                                text: name
                                font.weight: Font.Bold
                                elide: Text.ElideRight
                                color: (
                                    connectionState == "disconnected" ? "#ffffff" :
                                    connectionState == "connected" ? "#66ff66" :
                                    connectionState == "connecting" ? "#ffff66" :
                                    connectionState == "error" ? "#ff6666" :
                                    "#ffffff")
                            }
                            Label {
                                width: listItemColumn.width
                                font.weight: Font.Light
                                elide: Text.ElideRight
                                color: isDefault ? "#aaffff" : "#ffffff"
                                text: (username.length > 0 && address.length ? username + '@' + address + ':' + port : address.length > 0 ? address + ':' + port : username)
                            }
                        }
                        Column {
                            height: parent.height
                            Button {
                                id: connectButton
                                width: hostPage.buttonWidth
                                anchors.verticalCenter: parent.verticalCenter
                                visible: isDisconnected
                                text: qsTr("Connect")
                                onClicked: {
                                    Manager.connectToCore(identifier)
                                }
                            }
                            Button {
                                id: disconnectButton
                                width: hostPage.buttonWidth
                                anchors.verticalCenter: parent.verticalCenter
                                visible: !isDisconnected
                                text: qsTr("Disconnect")
                                onClicked: {
                                    Manager.disconnectFromCore()
                                }
                            }
                        }
                    }

                    onItemClicked: {
                        var hostitempage = hostListView.openPage("HostItemPage.qml", hostPageStack)
                        console.log("objectInstance=" + hostitempage + " identifier=" + identifier);
                        if (hostitempage != null)
                            hostitempage.setHostIdentifier(identifier)
                    }
                }
            }
        }
    }

    property int buttonWidth: 0

    Component.onCompleted: {
        hostPageStack.push(mainHostPage, true)

        var button = Qt.createQmlObject('import QtQuick 1.1; Button { text:"" }', hostListView)
        var code = 'import QtQuick 1.1; Text { font.pointSize:' + button.font.pointSize + '; font.bold:' + button.font.bold
        if (button.font.weight == Font.Light) code += ';font.weight:"Light"'
        else if (button.font.weight == Font.Normal) code += ';font.weight:"Normal"'
        else if (button.font.weight == Font.DemiBold) code += ';font.weight:"DemiBold"'
        else if (button.font.weight == Font.Bold) code += ';font.weight:"Bold"'
        else if (button.font.weight == Font.Black) code += ';font.weight:"Black"'
        var text1 = Qt.createQmlObject(code + '; text:"' + qsTr("Connect") + '"}', hostListView, "calcTextWidth")
        var text2 = Qt.createQmlObject(code + '; text:"' + qsTr("Disconnect") + '"}', hostListView, "calcTextWidth")
        var spacing = 10
        hostPage.buttonWidth = Math.max(text1.width, text2.width) + spacing*2
        button.destroy()
        text1.destroy()
        text2.destroy()

        hostListView.model = Manager.hostModel()
    }

}
