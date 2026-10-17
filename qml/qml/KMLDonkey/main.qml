import QtQuick 1.1
import "qrc:///"
import "qrc:///images/"

Window {
    id: appWindow

    property int connectingState: 1
    property int connectedState: 2
    property int disconnectedState: 3
    property int errorState: 4
    property int currentState: disconnectedState

    property alias appPageStack: pageStack

    function updateState(newState) {
        appWindow.currentState = newState;
        switch(appWindow.currentState) {
            case appWindow.connectingState:
                statusText.color = '#ffff66'
                break;
            case appWindow.connectedState:
                statusText.color = '#66ff66'
                break;
            case appWindow.disconnectedState:
                statusText.color = '#c0c0c0'
                break;
            case appWindow.errorState:
                statusText.color = '#ff6666'
                break;
        }
    }

    property bool showProgressbar: false
    property bool showNotifications: true
    property bool initialized: false

    onWidthChanged: {
        if (initialized)
            captionRect.updateButtonWidth();
    }

    Rectangle {
        id: captionRect
        width: parent.width
        height: Math.max(buttonRow.height, statusText.height)
        color: parent.color

        function buttons() {
            return [hostButton, filesButton, searchButton, consoleButton /*, toolsButton*/]
        }

        function updateButtonWidth() {
            var buttons = captionRect.buttons()
            var buttonWidth = 0.0
            var button = Qt.createQmlObject('import QtQuick 1.1; Button { text:"" }', appWindow);
            var code = 'import QtQuick 1.1; Text { font.pointSize:' + button.font.pointSize + '; font.bold:' + button.font.bold;
            if (button.font.weight == Font.Light) code += ';font.weight:"Light"'
            else if (button.font.weight == Font.Normal) code += ';font.weight:"Normal"'
            else if (button.font.weight == Font.DemiBold) code += ';font.weight:"DemiBold"'
            else if (button.font.weight == Font.Bold) code += ';font.weight:"Bold"'
            else if (button.font.weight == Font.Black) code += ';font.weight:"Black"'
            var spacing = 10
            for(var i = 0; i < buttons.length; i++) {
                var txt = Qt.createQmlObject(code + '; text:"' + buttons[i].text + '"}', appWindow, "calcTextWidth")
                buttonWidth = Math.max(buttonWidth, txt.width + spacing)
                txt.destroy()
            }
            button.destroy()
            var maxWidth = (appWindow.width - statusText.implicitWidth - statusText.anchors.rightMargin) / Math.max(1, buttons.length)
            var textVisible = true
            if (maxWidth < buttonWidth) {
                textVisible = false
                buttonWidth = maxWidth
            }
            for(var k = 0; k < buttons.length; k++) {
                var btn = buttons[k]
                btn.textVisible = textVisible
                btn.width = buttonWidth
            }
        }

        Row {
            id: buttonRow

            function setPage(button) {
                var page = button.page
                pageStack.replace(page, true)
                var buttons = captionRect.buttons()
                for(var k = 0; k < buttons.length; k++) {
                    var btn = buttons[k]
                    btn.checked = (btn.page === page)
                }
            }

            Button {
                id: hostButton
                iconSource: "qrc:///images/hosts.png"
                text: "Host"
                font.pointSize: 9
                checkable: true
                property alias page: hostTab
                onToggled: buttonRow.setPage(hostButton)
            }
            Button {
                id: filesButton
                iconSource: "qrc:///images/files.png"
                text: "Files"
                font.pointSize: 9
                checkable: true
                property alias page: filesTab
                onToggled: buttonRow.setPage(filesButton)
            }
            Button {
                id: searchButton
                iconSource: "qrc:///images/search.png"
                text: "Search"
                font.pointSize: 9
                checkable: true
                property alias page: searchTab
                onToggled: buttonRow.setPage(searchButton)
            }
            Button {
                id: consoleButton
                iconSource: "qrc:///images/console.png"
                text: "Console"
                font.pointSize: 9
                checkable: true
                property alias page: consoleTab
                onToggled: buttonRow.setPage(consoleButton)
            }
            //Button {
            //    id: toolsButton
            //    iconSource: "qrc:///images/tools.png"
            //    text: "Tools"
            //    font.pointSize: 9
            //    checkable: true
            //    property alias page: toolsTab
            //    onToggled: buttonRow.setPage(toolsButton)
            //}
        }

        Label {
            id: statusText
            text: qsTr("Disconnected")
            anchors {
                right: parent.right
                rightMargin: 6
                verticalCenter: parent.verticalCenter
            }
            color: '#c0c0c0'
            font.weight: Font.Light
            font.pointSize: 9
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
        }
    }

    function pageStackContains(page) {
        return pageStack.contains(page);
    }

    PageStack {
        id: pageStack
        anchors {
            left: parent.left
            right: parent.right
            top: captionRect.bottom
            bottom: parent.bottom
        }

        HostPage {
            id: hostTab
        }
        FilesPage {
            id: filesTab
        }
        SearchPage {
            id: searchTab
        }
        ConsolePage {
            id: consoleTab
        }
        //ToolsPage {
        //    id: toolsTab
        //}
    }

    Timer {
        id: notificationTimer
        interval: 5000
        running: false
        repeat: false
        onTriggered: notificationLabel.text = ''
    }
    Label {
        id: notificationLabel
        anchors {
            left: parent.left
            right: parent.right
            //top: pageStack.bottom
            bottom: parent.bottom
        }
        color: "#ff6666"
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        visible: text.length > 0
        text: ''
        onTextChanged: {
            if (text.length > 0)
                notificationTimer.start()
        }
    }

    function connecting() {
        statusText.text = qsTr('Connecting...');
        notificationLabel.text = ''
        updateState(appWindow.connectingState);
    }

    function connected() {
        statusText.text = qsTr('Connected');
        notificationLabel.text = ''
        updateState(appWindow.connectedState);
    }

    function disconnected(err) {
        console.log("appWindow.disconnected error=" + err);

        var newState = null;
        var errName = Manager.errorName(err);
        var textSummary = Manager.errorMessage(err, true);
        var textDetailed = '';
        if (errName == 'NoError') {
            newState = appWindow.disconnectedState;
        } else {
            newState = appWindow.errorState;
            if (appWindow.showNotifications)
                textDetailed = '<b>Error:</b> ' + Manager.errorMessage(err, false);
        }
        statusText.text = textSummary;
        updateState(newState);

        if (textDetailed.length > 0)
            notificationLabel.text = textDetailed
    }

    function clientStats(files, transfer, rate) {
        statusText.text = rate;
    }

    Component.onCompleted: {
        Manager.connectingToCore.connect(connecting)
        Manager.connectedToCore.connect(connected)
        Manager.disconnectedFromCore.connect(disconnected)
        Manager.clientStats.connect(clientStats)

        var showNotificationsProperty = Manager.setting("notifications");
        if (showNotificationsProperty != null && showNotificationsProperty != undefined)
            showNotifications = showNotificationsProperty;

        var showProgressbarProperty = Manager.setting("progressbar");
        if (showProgressbarProperty != null && showProgressbarProperty != undefined)
            showProgressbar = showProgressbarProperty;

        buttonRow.setPage(hostButton)
        captionRect.updateButtonWidth()
        appWindow.initialized = true
    }
}
