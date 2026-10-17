import QtQuick 1.1

Page {
    Sheet {
        id: submitSheet
        anchors.fill: parent

        acceptButtonText: "Submit"
        rejectButtonText: "Cancel"

        content: Item {
            anchors.fill: parent
            Flickable {
                anchors {
                    fill: parent
                }
                clip: true

                TextArea {
                    id: urlText
                    anchors.fill: parent
                    //text: paste()
                }
            }
        }

        onAccepted: {
            Manager.submitUrl(urlText)
        }
    }

    function open() {
        submitSheet.open()
        //urlText.paste()
    }
}
