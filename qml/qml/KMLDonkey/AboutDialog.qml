import QtQuick 1.1

Dialog {
    title: Text {
        text: "About"
    }

    content: Column {
        id: aboutContent
        width: parent.width
        spacing: 10
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            font.pointSize: 14
            color: "#c0c0ff"
            text: "KMLDonkey 2.2.1"
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            color: "#ffffff"
            wrapMode: Text.WordWrap
            text: "<center>A MLNet client for controlling centralized downloads.</center>"
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#ffffff"
            text: "<i>http://extragear.kde.org/apps/kmldonkey/</i>"
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#c0c0c0"
            text: "<center>(C) 2003-2012 The KMLDonkey Authors<br/>" +
                  "Sebastian Sauer (mail@dipe.org)<br/>" +
                  "Gioacchino Mazzurco (gmazzurco89@gmail.com)<br/>" +
                  "Christian Muehlhaeuser (chris@chris.de)<br/>" +
                  "Eduard Huguet (eduardhc@gmail.com)<br/>" +
                  "Petter Stokke (ummo@hellokitty.com)</center>" +
                  "<br/><br/>"
        }
    }

    buttons: ButtonRow {
        style: ButtonStyle {}
        Button {
            text: "Close";
            onClicked: accept()
        }
    }
}
