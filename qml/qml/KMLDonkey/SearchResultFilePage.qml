import QtQuick 1.1

Page {
    id: searchResultFilePage

    Button {
        id: backButton
        text: "< Back"
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        onClicked: searchPageStack.pop()
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

        Column {
            id: columnGrid
            spacing:10

            Button {
                id: downloadButton
                text: qsTr("Download")
                width: fileFlickable.width
                visible: false
                onClicked: {
                    var resultsModel = SearchesModel.resultsModel();
                    var resultModel = resultsModel.resultModel();
                    console.log("Start download fileIdentifier=" + searchResultFilePage.fileIdentifier);
                    if (!resultModel.startDownload(searchResultFilePage.fileIdentifier))
                        console.log("Failed to start download of fileIdentifier=" + searchResultFilePage.fileIdentifier);
                }
            }

            Grid {
                id: grid
                spacing: 2
                columns: 2
                rows: 6
                property variant editWidth: fileFlickable.width - fileFlickable.anchors.leftMargin - fileFlickable.anchors.rightMargin - columnGrid.spacing - grid.spacing - nameLabel.width;

                Label {
                    id: nameLabel
                    anchors.top: nameText.top
                    anchors.bottom: nameText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Name:")
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
                    text: qsTr("Size:")
                }
                TextArea {
                    id: sizeText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: availabilityText.top
                    anchors.bottom: availabilityText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Availability:")
                }
                TextArea {
                    id: availabilityText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: formatText.top
                    anchors.bottom: formatText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Format:")
                }
                TextArea {
                    id: formatText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: alreadyDoneText.top
                    anchors.bottom: alreadyDoneText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Already Done:")
                }
                TextArea {
                    id: alreadyDoneText
                    readOnly: true
                    width: grid.editWidth
                }

                Label {
                    anchors.top: networkText.top
                    anchors.bottom: networkText.bottom
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Network:")
                }
                TextArea {
                    id: networkText
                    readOnly: true
                    width: grid.editWidth
                }
            }
        }
    }

    property variant fileIdentifier: null;

    function setFileIdentifier(identifier) {
        searchResultFilePage.fileIdentifier = identifier;

        downloadButton.visible = identifier != null;

        var resultsModel = SearchesModel.resultsModel();
        var resultModel = resultsModel.resultModel();

        nameText.text = resultModel.data(identifier, "name");
        sizeText.text = resultModel.data(identifier, "size");
        networkText.text = resultModel.data(identifier, "network");
        availabilityText.text = resultModel.data(identifier, "availability");
        formatText.text = resultModel.data(identifier, "format");
        alreadyDoneText.text = resultModel.data(identifier, "alreadyDone");
    }

    onDeactivated: {
        if (!searchPageStack.contains(searchQueryPage)) {
            console.log("SeatchResultFilePage.onDeactivated Destroy")
            searchResultFilePage.fileIdentifier = null;
            searchResultFilePage.destroy();
        }
    }

}
