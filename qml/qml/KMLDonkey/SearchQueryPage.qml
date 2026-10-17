import QtQuick 1.1

Page {
    id: searchQueryPage

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
        id: newSearchFlickable
        anchors {
            left: parent.left
            right: parent.right
            top: backButton.bottom
            bottom: parent.bottom
            topMargin: 10
        }
        clip: true
        contentHeight: mainSearchGrid.implicitHeight

        Column {
            id: mainSearchGrid
            Column {
                id: searchGrid
                spacing: 10
            }

            Button {
                id: searchButton
                anchors {
                    top: searchGrid.bottom
                    left: parent.left
                    right: parent.right
                    topMargin: 10
                }
                enabled: false
                text: qsTr("Start Search")
                //onClicked: {}
            }
        }
    }

    property variant searchQuery: null;

    function setSearchIdentifier(identifier) {
        searchQueryPage.searchQuery = null;

        var itemCounter = 0
        var labelItems = []
        var code = 'import QtQuick 1.1\n';

        function parseList(query, parentItem, hidden) {
            console.log("QueryList count=" + query.queryList().length);
            var queryList = query.queryList()
            for(var i in queryList) {
                var q = queryList[i];
                parse(q, parentItem)
            }
        }

        function parseModule(query, parentItem, hidden) {
            var n = query.name();
            var q = query.query();

            var code1 = 'Rectangle { color: "#000000"; border.color: "#606060"; border.width:2; radius:8; smooth:true; }\n';
            var item1 = Qt.createQmlObject(code + code1, parentItem);

            var code2 = 'Column { Label { color:"#a0a0a0"; text:"' + n + '" } }\n';
            var item2 = Qt.createQmlObject(code + code2, item1);

            parse(q, item2);

            item1.width = newSearchFlickable.width;
            item1.height = item2.height;
        }

        function parseTwoStrings(query, parentItem, hidden) {
            var id1 = 'id' + (++itemCounter);
            var id2 = 'id' + (++itemCounter);

            var code0 = 'Row {}\n';
            var item0 = Qt.createQmlObject(code + code0, parentItem);

            var code1 = 'Label { id:' + id1 + '; text:"' + query.key() + '"; verticalAlignment:Text.AlignVCenter }\n';
            var item1 = Qt.createQmlObject(code + code1, item0);

            var code2edit = 'console.log("SearchQueryPage textChange query=" + query + " text=" + text); query.setValue(text);';
            var code2 = 'TextField { id:' + id2 + '; text:"' + query.value() + '"; property variant query: null; onTextChanged: {' + code2edit + '} }\n';
            var item2 = Qt.createQmlObject(code + code2, item0);
            item2.query = query;

            item1.anchors.top = item2.top
            item1.anchors.bottom = item2.bottom

            function textAccepted() {
                searchButton.focus = true;
                searchButton.clicked();
            }
            item2.accepted.connect(textAccepted)

            if (hidden) {
                item1.enabled = false
                item2.enabled = false
            }

            labelItems.push(item1);
        }

        function parse(query, parentItem, hidden) {
            if (query == null || query == undefined) {
                console.log("Parse undefined SearchQuery=" + query);
                return;
            }
            console.log("Parse query=" + query + " operator=" + query.queryOperation());
            if (query.queryOperation() == "And") {
                parseList(query, parentItem, hidden);
            } else if (query.queryOperation() == "Or") {
                parseList(query, parentItem, hidden);
            } else if (query.queryOperation() == "AndNot") {
                console.log("SearchQueryPage 'AndNot' query not handled!");
                //TODO
            } else if (query.queryOperation() == "Module") {
                parseModule(query, parentItem, hidden);
            } else if (query.queryOperation() == "Keywords") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "MinSize") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "MaxSize") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "Format") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "Media") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "Mp3Artist") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "Mp3Title") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "Mp3Album") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "Mp3Bitrate") {
                parseTwoStrings(query, parentItem, hidden);
            } else if (query.queryOperation() == "Hidden") {
                parseList(query, parentItem, true);
            } else {
                console.log("SearchQuery has unknown operator=" + query.queryOperation());
            }
        }

        var queriesModel = SearchesModel.queriesModel();
        var queryTemplate = queriesModel.query(identifier)
        if (queryTemplate == null) {
            console.log("No such SearchQuery with identifier=" + identifier);
            return;
        }

        searchQueryPage.searchQuery = queryTemplate.clone();

        var queryString = queriesModel.data(identifier, "simpleQuery");
        console.log("SearchQuery identifier=" + identifier + " searchQuery=" + searchQueryPage.searchQuery + " queryString=" + queryString);

        parse(searchQueryPage.searchQuery, searchGrid, false);

        var maxWidth = 0;
        for(var i in labelItems) {
            var w = labelItems[i].implicitWidth;
            if (w > maxWidth)
                maxWidth = w;
        }
        if (maxWidth > 0) {
            for(var i in labelItems) {
                labelItems[i].width = maxWidth;
            }
        }

        searchButton.enabled = maxWidth > 0;
        searchButton.clicked.connect(startSearch);
    }

    function startSearch() {
        var query = searchQueryPage.searchQuery;
        if (query == null || query == undefined) {
            console.log("Undefined SearchQuery=" + query);
            return;
        }
        console.log("Start search: " + query.queryString());
        var queriesModel = SearchesModel.queriesModel();
        var searchIdentifier = queriesModel.startSearchQuery(query);
        console.log("searchIdentifier: " + searchIdentifier);
        var page = searchListView.openPage("SearchResultPage.qml", searchPageStack);
        if (page != null)
            page.setSearchIdentifier(searchIdentifier);
    }

    onDeactivated: {
        if (!searchPageStack.contains(searchQueryPage)) {
            console.log("SearchQueryPage.onDeactivated Destroy")
            searchQueryPage.searchQuery = null;
            searchQueryPage.destroy();
        }
    }
}
