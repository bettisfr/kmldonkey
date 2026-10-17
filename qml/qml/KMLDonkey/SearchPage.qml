import QtQuick 1.1

Page {
    id: listPage
    anchors.margins: 10

    property alias searchPageStack: searchpagestack

    PageStack {
        id: searchpagestack
        anchors.fill: parent

        Page {
            id: mainSearchPage

            InfoItemView {
                id: searchListView
                anchors.fill: parent
                model: SearchesModel
                section.property: "type"
                delegate: InfoItemDelegate {
                    id: listItem
                    width: searchListView.width
                    Row {
                        Column {
                            id: listItemContent
                            width: listItem.width - deleteButton.width
                            Label {
                                text: name
                                font.weight: Font.Bold
                            }
                            Row {
                                spacing:10
                                Label {
                                    text: type == qsTr("Query") ? fullQuery : (fetched ? hits : "?")
                                    font.weight: Font.Light
                                }
                            }
                        }
                        Button {
                            id: deleteButton
                            visible: type != qsTr("Query")
                            text: qsTr("delete")
                            //width: height
                            onClicked: {
                                var queriesModel = SearchesModel.queriesModel();
                                if (queriesModel.stopSearch(identifier))
                                    console.log("Deleting search with identifier=" + identifier);
                                else
                                    console.log("Failed to stop search with identifier=" + identifier);
                            }
                        }
                    }

                    onItemClicked: {
                        if (type == qsTr("Query")) {
                            var page = searchListView.openPage("SearchQueryPage.qml", searchPageStack);
                            if (page != null)
                                page.setSearchIdentifier(identifier);
                        } else {
                            var page = searchListView.openPage("SearchResultPage.qml", searchPageStack);
                            if (page != null)
                                page.setSearchIdentifier(identifier);
                        }
                    }
                }
            }
        }

        Component.onCompleted: {
            searchPageStack.push(mainSearchPage, true)
        }
    }

}
