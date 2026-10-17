import QtQuick 1.1

ListView {
    id: itemListView
    //anchors.fill: parent
//    anchors.topMargin: 10
//    anchors.bottomMargin: 10
//    anchors.leftMargin: 10
//    anchors.rightMargin: 10
    clip: true
    spacing: 5

    section.criteria: ViewSection.FullString

    section.delegate: Component {
        id: sectionHeading
        SectionHeader {
            sectionText: section
        }
    }

    function openPage(file, pageStack) {
        var component = Qt.createComponent(file)
        if (component.status != Component.Ready) {
            console.log("Error loading component:", component.errorString());
            return null;
        }
        var obj = component.createObject(parent, {});
        if (pageStack != null && pageStack != undefined) {
            pageStack.push(obj)
        } else {
            appPageStack.push(obj);
        }
        console.log("Loaded component:", component, " and created an instance of object:", obj);
        return obj
    }
}
