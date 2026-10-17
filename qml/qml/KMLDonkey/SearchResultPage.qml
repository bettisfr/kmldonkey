import QtQuick 1.1

Page {
    id: searchResultPage

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

    TextField {
        id: searchPageFilter
        anchors {
            left: parent.left
            right: parent.right
            top: backButton.bottom
            topMargin: 10
        }
        onTextChanged: {
            console.log("SearchResultPage.searchPageFilter text=" + text)
            var resultsModel = SearchesModel.resultsModel();
            var resultModel = resultsModel.resultModel();
            resultModel.setFilterFixedString(text)
        }
    }

    Item {
        anchors {
            left: parent.left
            right: parent.right
            top: searchPageFilter.bottom
            bottom: parent.bottom
            topMargin: 10
        }
        InfoItemView {
            id: searchResultListView
            anchors.fill: parent

            delegate: InfoItemDelegate {
                id: listItem

                Column {
                    id: listItemContent
                    Label {
                        text: name
                        font.weight: Font.Bold
                        color: (alreadyDone ? '#c0c0c0' : availability >= 5 ? '#ccffcc' : availability >= 2 ? '#ffffff' : '#ffcccc')
                    }
                    Row {
                        spacing: 10
                        Label {
                            font.weight: Font.Light
                            text: availability
                        }
                        Label {
                            font.weight: Font.Light
                            text: size
                        }
                        Label {
                            font.weight: Font.Light
                            text: network
                        }
                    }
                }

                onItemClicked: {
                    var filepage = searchResultListView.openPage("SearchResultFilePage.qml", searchPageStack)
                    console.log("objectInstance=" + filepage + " identifier=" + identifier);
                    filepage.setFileIdentifier(identifier)
                }
            }
        }
    }

    property variant searchIdentifier: null;

    function setSearchIdentifier(identifier) {
        searchResultPage.searchIdentifier = identifier;
        var resultsModel = SearchesModel.resultsModel();
        var resultModel = resultsModel.resultModel();
        searchResultListView.model = resultModel;
        resultModel.setSearchResult(identifier);
    }

    onDeactivated: {
        if (!searchPageStack.contains(searchQueryPage)) {
            console.log("SeatchResultPage.onDeactivated Destroy")
            searchResultPage.searchIdentifier = null;
            searchResultPage.destroy();
        }
    }

}
