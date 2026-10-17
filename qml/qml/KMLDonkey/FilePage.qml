import QtQuick 1.1

Page {
    id: filePage

    Button {
        id: backButton
        text: "< Back"
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        onClicked: filesPageStack.pop()
    }

    Flickable {
        id: fileFlickable
        anchors {
            left: parent.left
            right: parent.right
            top: backButton.bottom
            bottom: parent.bottom
            topMargin: 10
        }
        clip: true
        contentWidth: parent.width;
        contentHeight: columnGrid.height;

        function updateButtonWidth() {
            var buttons = [pauseButton, resumeButton, retryButton, saveButton, deleteButton];
            var visibleButtons = [];
            for(var i in buttons)
                if (buttons[i].visible)
                    visibleButtons.push(buttons[i]);
            for(var i in visibleButtons)
                visibleButtons[i].width = fileFlickable.width / visibleButtons.length;
        }
        onWidthChanged: updateButtonWidth()

        Column {
            id: columnGrid
            spacing:10

            Row {
                id: buttonRow

                Button {
                    id: pauseButton
                    text: qsTr("Pause")
                    visible: false
                    onClicked: {
                        console.log("Pause download with identifier=" + filePage.fileIdentifier);
                        FilesModel.pauseFile(filePage.fileIdentifier);
                    }
                }
                Button {
                    id: resumeButton
                    text: qsTr("Resume")
                    visible: false
                    onClicked: {
                        console.log("Resume download with identifier=" + filePage.fileIdentifier);
                        FilesModel.resumeFile(filePage.fileIdentifier);
                    }
                }
                Button {
                    id: retryButton
                    text: qsTr("Retry")
                    visible: false
                    onClicked: {
                        console.log("Retry download with identifier=" + filePage.fileIdentifier);
                        FilesModel.retryFile(filePage.fileIdentifier);
                    }
                }
                Button {
                    id: saveButton
                    text: qsTr("Save")
                    visible: false
                    onClicked: {
                        console.log("Save download with identifier=" + filePage.fileIdentifier);
                        FilesModel.saveFile(filePage.fileIdentifier);
                    }
                }
                Button {
                    id: deleteButton
                    text: qsTr("Delete")
                    visible: false
                    onClicked: {
                        console.log("Cancel download with identifier=" + filePage.fileIdentifier);
                        FilesModel.cancelFile(filePage.fileIdentifier);
                    }
                }
            }

            Grid {
                id: grid
                spacing: 2
                columns: 2
                rows: 12
                property variant editWidth: fileFlickable.width - fileFlickable.anchors.leftMargin - fileFlickable.anchors.rightMargin - columnGrid.spacing - grid.spacing - nameLabel.width;

                Label {
                    id: nameLabel
                    anchors.top: nameText.top
                    anchors.bottom: nameText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Name:"
                }
                TextArea {
                    id: nameText
                    readOnly: true
                    wrapMode: TextEdit.WrapAnywhere
                    width: grid.editWidth
                }

                Label {
                    anchors.top: sizeText.top
                    anchors.bottom: sizeText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Size:"
                }
                TextArea {
                    id: sizeText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: receivedText.top
                    anchors.bottom: receivedText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Received:"
                }
                TextArea {
                    id: receivedText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: progressText.top
                    anchors.bottom: progressText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Progress:"
                }
                TextArea {
                    id: progressText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: stateText.top
                    anchors.bottom: stateText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "State:"
                }
                TextArea {
                    id: stateText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: speedText.top
                    anchors.bottom: speedText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Speed:"
                }
                TextArea {
                    id: speedText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: etaText.top
                    anchors.bottom: etaText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "ETA:"
                }
                TextArea {
                    id: etaText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: lastSeenText.top
                    anchors.bottom: lastSeenText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Last Seen:"
                }
                TextArea {
                    id: lastSeenText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: ageText.top
                    anchors.bottom: ageText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Age:"
                }
                TextArea {
                    id: ageText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: priorityText.top
                    anchors.bottom: priorityText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Priority:"
                }
                TextArea {
                    id: priorityText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: networkText.top
                    anchors.bottom: networkText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Network:"
                }
                TextArea {
                    id: networkText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: hashText.top
                    anchors.bottom: hashText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: "Hash:"
                }
                TextArea {
                    id: hashText
                    readOnly: true
                    width: grid.editWidth
                }
            }
        }
    }

    property variant fileIdentifier: null;
    property string fileState: "";

    function setFileIdentifier(identifier) {
        filePage.fileIdentifier = identifier;

        fileState = FilesModel.data(identifier, "stateValue");
        updateFileState();

        nameText.text = FilesModel.data(identifier, "name")
        receivedText.text = FilesModel.data(identifier, "received")
        sizeText.text = FilesModel.data(identifier, "size")
        progressText.text = FilesModel.data(identifier, "progressText")
        speedText.text = FilesModel.data(identifier, "speed")
        etaText.text = FilesModel.data(identifier, "eta")
        priorityText.text = FilesModel.data(identifier, "priority")
        networkText.text = FilesModel.data(identifier, "network")
        hashText.text = FilesModel.data(identifier, "hash")
        lastSeenText.text = FilesModel.data(identifier, "lastSeen")
        ageText.text = FilesModel.data(identifier, "age")
        stateText.text = FilesModel.data(identifier, "stateText")
    }

    function updateFileState() {
        saveButton.visible = filePage.fileIdentifier != null && fileState == "Complete";
        deleteButton.visible = filePage.fileIdentifier != null;

        if (fileState == "Paused") {
            pauseButton.visible = false;
            resumeButton.visible = true;
        } else if (fileState == "Downloading" || fileState == "Looking" || fileState == "New" || fileState == "Queued") {
            pauseButton.visible = true;
            resumeButton.visible = false;
        } else {
            pauseButton.visible = false;
            resumeButton.visible = false;
        }

        retryButton.visible = fileState != "Complete" && fileState != "Downloading" && fileState != "Paused";

        fileFlickable.updateButtonWidth()
    }

    function updateFileValues(id) {
        if (filePage.fileIdentifier == null || (id != null && id != filePage.fileIdentifier))
            return;

        var state = FilesModel.data(filePage.fileIdentifier, "stateValue");
        if (state == undefined) {
            filesPageStack.pop();
            return;
        }

        if (state != fileState) {
            fileState = state;
            updateFileState()
        }

        //nameText.text = FilesModel.data(filePage.fileIdentifier, "name")
        receivedText.text = FilesModel.data(filePage.fileIdentifier, "received")
        //sizeText.text = FilesModel.data(filePage.fileIdentifier, "size")
        progressText.text = FilesModel.data(filePage.fileIdentifier, "progressText")
        speedText.text = FilesModel.data(filePage.fileIdentifier, "speed")
        etaText.text = FilesModel.data(filePage.fileIdentifier, "eta")
        //priorityText.text = FilesModel.data(filePage.fileIdentifier, "priority")
        //networkText.text = FilesModel.data(filePage.fileIdentifier, "network")
        //hashText.text = FilesModel.data(filePage.fileIdentifier, "hash")
        lastSeenText.text = FilesModel.data(filePage.fileIdentifier, "lastSeen")
        //ageText.text = FilesModel.data(filePage.fileIdentifier, "age")
        stateText.text = FilesModel.data(filePage.fileIdentifier, "stateText")
    }

    onActivated: {
        FilesModel.fileChanged.connect(updateFileValues)
    }
    onDeactivating: {
        FilesModel.fileChanged.disconnect(updateFileValues)
    }
    onDeactivated: {
        if (!filesPageStack.contains(filePage)) {
            console.log("FilePage.onDeactivated Destroy")
            filePage.fileIdentifier = null;
            fileState = "";
            filePage.destroy();
        }
    }
}
