import QtQuick 1.1

Page {
    id: hostItemPage

    Flickable {
        id: hostItemFlickable
        anchors.fill: parent
        contentWidth: columnGrid.width
        contentHeight: columnGrid.height

        Item {
            id: columnGrid
            width: parent.width

            Column {
                //id: columnGrid
                width: hostItemPage.width
                spacing: 10

                Button {
                    id: backButton
                    text: "< Back"
                    width: parent.width
                    onClicked: hostPageStack.pop()
                }

                Grid {
                    id: grid
                    anchors {
                        top: backButton.bottom
                        topMargin: 10
                    }
                    width: parent.width
                    spacing: 10
                    columns: 2
                    rows: 5
                    Label {
                        id: nameLabel
    //                    anchors.top: nameText.top
    //                    anchors.bottom: nameText.bottom
    //                    verticalAlignment: Text.AlignVCenter
                        text: qsTr("Hostname:")
                    }
                    TextField {
                        id: nameText
                        text: ""
                        onTextChanged: validateInput();
                    }
                    Label {
                        id: addressLabel
    //                    anchors.top: addressText.top
    //                    anchors.bottom: addressText.bottom
    //                    verticalAlignment: Text.AlignVCenter
                        text: qsTr("Address:")
                    }
                    TextField {
                        id: addressText
                        text: ""
                        onTextChanged: validateInput();
                    }
                    Label {
                        id: portLabel
    //                    anchors.top: portText.top
    //                    anchors.bottom: portText.bottom
    //                    verticalAlignment: Text.AlignVCenter
                        text: qsTr("Port:")
                    }
                    TextField {
                        id: portText
                        text: ""
                        //validator: IntValidator { bottom: 0; top: 32767 }
                    }
                    Label {
                        id: userLabel
    //                    anchors.top: userText.top
    //                    anchors.bottom: userText.bottom
    //                    verticalAlignment: Text.AlignVCenter
                        text: qsTr("Username:")
                    }
                    TextField {
                        id: userText
                        text: ""
                    }
                    Label {
                        id: passLabel
    //                    anchors.top: passText.top
    //                    anchors.bottom: passText.bottom
    //                    verticalAlignment: Text.AlignVCenter
                        text: qsTr("Password:")
                    }
                    TextField {
                        id: passText
                        //echoMode: TextInput.Password
                        //echoMode: TextInput.PasswordEchoOnEdit
                        text: ""
                    }
                }
                CheckBox {
                    id: defaultCheckbox
                    anchors {
                        top: grid.bottom
                        topMargin: 10
                    }
                    text: "Default Host"
                }
                Row {
                    id: buttonGrid
                    anchors {
                        top: defaultCheckbox.bottom
                        topMargin: 10
                    }
                    Button {
                        id: deleteButton
                        width: hostItemPage.width / 2 - grid.spacing
                        text: qsTr("Delete")
                        onClicked: {
                            var identifier = hostItemPage.hostIdentifier;
                            var m = Manager.hostModel();
                            console.log("HostItemPage Delete identifier=" + identifier + " hostModel=" + m);
                            if (!m.removeHost(identifier)) {
                                console.log("HostItemPage Failed to delete identifier=" + identifier);
                                return;
                            }
                            hostPageStack.pop()
                        }
                    }
                    Button {
                        id: saveButton
                        width: hostItemPage.width / 2 - grid.spacing
                        text: qsTr("Save")
                        onClicked: {
                            var identifier = hostItemPage.hostIdentifier;
                            var m = Manager.hostModel();
                            console.log("HostItemPage Save identifier=" + identifier + " hostModel=" + m);
                            if (identifier == undefined) {
                                identifier = nameText.text.trim();
                                m.addHost(identifier);
                            } else {
                                m.setData(identifier, "name", nameText.text)
                            }
                            m.setData(identifier, "address", addressText.text)
                            m.setData(identifier, "port", portText.text)
                            m.setData(identifier, "username", userText.text)
                            m.setData(identifier, "userpass", passText.text)
                            if (defaultCheckbox.checked) {
                                m.setSelectedHost(identifier);
                            } else if (identifier == m.selectedHost()) {
                                m.setSelectedHost(null);
                            }
                            hostPageStack.pop();
                        }
                    }
                }
            }
        }
    }

    property variant hostIdentifier: null;

    function validateInput() {
        var m = Manager.hostModel();
        var name = nameText.text.trim();
        saveButton.enabled =
                name.length > 0 &&
                addressText.text.trim().length > 0 &&
                portText.text.trim().length > 0 &&
                (hostItemPage.hostIdentifier == name || m.data(name, "name") == undefined);
        deleteButton.enabled =
                hostItemPage.hostIdentifier != undefined;
    }

    function setHostIdentifier(identifier) {
        hostItemPage.hostIdentifier = identifier;
        var m = Manager.hostModel()
        console.log("HostItemPage.setHostIdentifier identifier=" + identifier + " hostModel=" + m);
        if (identifier == undefined) { // it's a new host item that doesn't have settings yet
            var copyIdentifier = m.selectedHost();
            if (copyIdentifier == null || copyIdentifier == undefined)
                copyIdentifier = ''
            copyIdentifier = copyIdentifier.toString().trim()
            if (copyIdentifier.length < 1) {
                var hosts = m.hostList();
                if (hosts.length > 0) {
                    copyIdentifier = hosts[0]
                    if (copyIdentifier == null || copyIdentifier == undefined)
                        copyIdentifier = ''
                    else
                        copyIdentifier = copyIdentifier.toString().trim()
                }
            }

            if (copyIdentifier.length < 1) { // it's the very first item
                nameText.text = 'default'
                addressText.text = '192.168.1.1'
                portText.text = '4001'
                userText.text = 'admin'
                passText.text = ''
                defaultCheckbox.checked = true;
            } else { // set at least some defaults
                var num = 2;
                var hosts = m.hostList();
                for(var i = 0; i < hosts.length;) {
                    var h = hosts[i];
                    if (h == 'default' + num) {
                        num += 1;
                        i = 0;
                    } else {
                        i += 1;
                    }
                }
                nameText.text = 'default' + num;
                addressText.text = m.data(copyIdentifier, "address")
                portText.text = m.data(copyIdentifier, "port")
                userText.text = m.data(copyIdentifier, "username")
                passText.text = m.data(copyIdentifier, "userpass")
                defaultCheckbox.checked = false;
            }
        } else {
            nameText.text = m.data(identifier, "name")
            addressText.text = m.data(identifier, "address")
            portText.text = m.data(identifier, "port")
            userText.text = m.data(identifier, "username")
            passText.text = m.data(identifier, "userpass")
            defaultCheckbox.checked = identifier == m.selectedHost();
        }
        validateInput();
    }

    onDeactivated: {
        if (!hostPageStack.contains(hostItemPage)) {
            console.log("HostItemPage.onDeactivated Destroy")
            hostItemPage.hostIdentifier = null
            hostItemPage.destroy()
        }
    }

}
