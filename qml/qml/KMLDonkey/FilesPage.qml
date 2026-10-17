import QtQuick 1.1

Page {
    id: filesPage
    anchors.margins: 10

    property alias filesPageStack: filespagestack

    PageStack {
        id: filespagestack
        anchors.fill: parent

        Page {
            id: mainFilesPage

            InfoItemView {
                id: filesListView
                anchors.fill: parent
                model: FilesModel
                section.property: "stateText"

                delegate: InfoItemDelegate {
                    id: listItem
                    Column {
                        id: listItemContent
                        //anchors.verticalCenter: parent.verticalCenter
                        Label {
                            text: name
                            font.weight: Font.Bold
                            //color: stateValue == "Downloading" ? (speed == "" ? "#660000" : "#006600") : "#000000"
                        }
                        Row {
                            id: columnRow
                            spacing:10
                            ProgressBar {
                                id: progressBar
                                width: 180
                                minimumValue: 0
                                maximumValue: 100
                                value: progressValue
                                anchors.verticalCenter: parent.verticalCenter
                                visible: appWindow.showProgressbar
                            }
                            Label {
                                id: progressLabel
                                text: progressText
                                font.weight: Font.Light
                                horizontalAlignment: Text.AlignRight
                                visible: !appWindow.showProgressbar
                            }
                            Label {
                                id: sizeLabel
                                text: size
                                font.weight: Font.Light
                            }
                            Label {
                                id: speedLabel
                                text: speed
                                font.weight: Font.Light
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }

                    onItemClicked: {
                        filesPage.switchToFilePage = true;
                        var filepage = filesListView.openPage("FilePage.qml", filesPageStack)
                        console.log("objectInstance=", filepage, " identifier=", identifier);
                        if (filepage == null)
                            filesPage.switchToFilePage = false;
                        else
                            filepage.setFileIdentifier(identifier)
                    }
                }
            }
        }

        Component.onCompleted: {
            filesPageStack.push(mainFilesPage, true)
        }
    }

    property bool switchToFilePage: false
    onActivated: {
        console.log("FilesPage.onActivated")
        filesPage.switchToFilePage = false;
        FilesModel.setEnableUpdates(true)

    }
    onDeactivating: {
        console.log("FilesPage.onDeactivating")
        FilesModel.setEnableUpdates(false)
    }
//    onVisibleChanged: {
//        var updatesEnabled = visible || filesPage.switchToFilePage;
//        if (updatesEnabled)
//            filesPage.switchToFilePage = false;
//        FilesModel.setEnableUpdates(updatesEnabled)
//    }

}
