import QtQuick 1.1

Page {
    id: consolePage
    anchors.margins: 10

    TextField {
        id: consoleInput
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        //placeholderText: qsTr("Command, '?' for help")
        onAccepted: {
            var command = text.trim()
            if (command.length < 1)
                return
            if (Manager.isConnected()) {
                Manager.sendConsoleMessage(command);
                text = "";
            } else {
                consoleOutput.text = qsTr("Disconnected");
            }
        }
    }

    Flickable {
        id: consoleOutputFlickable
        anchors {
            left: parent.left
            right: parent.right
            top: consoleInput.bottom
            bottom: parent.bottom
            topMargin: 2
            bottomMargin: 2
        }
        clip: true
        height: parent.height - consoleInput.height - 20
        //width: parent.width
        contentHeight: Math.max(height, consoleOutput.implicitHeight)

        TextArea {
            id: consoleOutput
            width: parent.width
            height: Math.max(implicitHeight, parent.height)
            wrapMode: TextEdit.Wrap
            readOnly: true
            //onTextChanged: {
            //    var pos = Math.max(0, text.length - 1)
            //    var rect = positionToRectangle(pos)
            //    consoleOutputFlickable.contentX = 0
            //    consoleOutputFlickable.contentY = rect.y + rect.height - consoleOutputFlickable.height;
            //}
        }
        onContentHeightChanged: {
            contentX = 0
            contentY = consoleOutput.implicitHeight - height;
        }
    }

    function connected() {
        consoleOutput.text =
            "LICENSE\nThis program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA\n\n" +
            "WARNING\nPeer-to-peer networks, particularly those accessible via KMLDonkey, may contain material which you may be legally prohibited from downloading or redistributing. It is your responsibility to verify that any material you download from any peer-to-peer network is being made available to you lawfully. It is also your responsibility to verify that you have the legal right to redistribute any material which you make available to others via any peer-to-peer networks. In short, it is your responsibility to use this software in accordance with applicable laws where you live, including copyright law.\n\n" +
            "RESTRICTIONS\nThe authors of this software intend it to be used for the lawful exchange of data, art, software and other documents or works which may be lawfully distributed to third parties. The authors will not support, and strongly discourage, any use of this software to violate copyright or other intellectual property laws. In particular, the authors discourage any transmission of any material in violation of any applicable intellectual property law or regulation. Each user is assumed to comply with such rules and restrictions as well as the rules and restrictions applicable to the P2P networks or services connected directly or indirectly to this software.\n\n" +
            "DISCLAIMER OF LIABILITY\nThe authors of this software do not and cannot exercise any control whatsoever over the content of the information exchanged using this software. The authors make no warranties of any kind, whether expressed or implied, for the service this software is providing or for the data exchanged with the assistance of this software. The authors cannot be held responsible for any claims resulting from the user's conduct and/or use of the software which is in any manner unlawful or which damages such user or any other party.\n\n";
    }

    function disconnected() {
        consoleOutput.text = "";
        consoleOutputFlickable.contentX = 0
        consoleOutputFlickable.contentY = 0
    }

    function receivedConsoleMessage(message) {
        consoleOutput.text += message + "\n";
    }

    Component.onCompleted: {
        Manager.connectedToCore.connect(connected)
        Manager.disconnectedFromCore.connect(disconnected)
        Manager.receivedConsoleMessage.connect(receivedConsoleMessage)
    }
}
