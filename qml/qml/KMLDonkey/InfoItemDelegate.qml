import QtQuick 1.1

Item {
    id: listItem
    width: parent.width
    height: listItemContent.implicitHeight

    signal itemClicked(variant item)

    Rectangle {
        anchors.fill: parent
        visible: mouseArea.pressed
        gradient: Gradient {
//            GradientStop {
//                position: 0.0
//                color: mouseArea.pressed ? "#303030" : "#202020"
//            }
//            GradientStop {
//                position: 1.0
//                color: mouseArea.pressed ? "#404040" : "#101010"
//            }

            GradientStop {
                position: 0.0
                color: "#303030"
            }
            GradientStop {
                position: 1.0
                color: "#404040"
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: listItem.itemClicked(listItem)
    }
}
