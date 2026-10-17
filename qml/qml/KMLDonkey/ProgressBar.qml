import QtQuick 1.1

Rectangle {
    id: root

    property double minimumValue: 0.0
    property double maximumValue: 100.0
    property double value: 0.0

    Rectangle {
        color: '#939393'
        width: parent.width
    }
    Rectangle {
        color: '#00ffff'
        width: parent.width / 100.0 * ( (maximumValue - minimumValue) / 100.0 * value )
    }

}
