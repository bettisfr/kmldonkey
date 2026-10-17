import QtQuick 1.1

Page {
    Sheet {
        id: settingsSheet
        anchors.fill: parent
        acceptButtonText: "Save"
        rejectButtonText: "Cancel"

        content: Item {
            anchors.fill: parent
            Flickable {
                id: settingsFlickable
                contentHeight: settingsColumn.implicitHeight
                anchors {
                    fill: parent
                    leftMargin: 10
                    rightMargin: 10
                    topMargin: 10
                    bottomMargin: 10
                }
                Column {
                    id: settingsColumn
                    spacing:10
                    width: parent.width
                    SectionHeader {
                        sectionText: qsTr("Connectivity")
                    }
                    CheckBox {
                        id: autoConnect
                        checked: true
                        text: qsTr("Connect to default host on startup")
                    }
                    CheckBox {
                        id: autoReconnect
                        checked: false
                        text: qsTr("Reconnect to host on error")
                    }
                    CheckBox {
                        id: autoOnline
                        checked: true
                        text: qsTr("Connect with internet on demand")
                    }
                    CheckBox {
                        id: savePassword
                        checked: true
                        text: qsTr("Save password")
                    }
                    SectionHeader {
                        sectionText: qsTr("Presentation")
                    }
                    CheckBox {
                        id: showNotifications
                        checked: false
                        text: qsTr("Show notification on error")
                    }
                    CheckBox {
                        id: showProgressbar
                        checked: false
                        text: qsTr("Show progressbar")
                    }
                }
            }
        }

        onAccepted: {
            if (showNotifications.checked != appWindow.showNotifications) {
                appWindow.showNotifications = showNotifications.checked;
                Manager.setSetting("notifications", showNotifications.checked)
            }

            if (showProgressbar.checked != appWindow.showProgressbar) {
                appWindow.showProgressbar = showProgressbar.checked;
                Manager.setSetting("progressbar", showProgressbar.checked)
            }

            Manager.setAutoConnect(autoConnect.checked)
            Manager.setAutoReconnect(autoReconnect.checked)
            Manager.setAutoOnline(autoOnline.checked)
            Manager.setSavePassword(savePassword.checked)
        }
    }

    function open() {
        autoConnect.checked = Manager.autoConnect();
        autoReconnect.checked = Manager.autoReconnect();
        autoOnline.checked = Manager.autoOnline();
        savePassword.checked = Manager.savePassword();
        showNotifications.checked = appWindow.showNotifications
        showProgressbar.checked = appWindow.showProgressbar

        settingsSheet.open();
    }
}
