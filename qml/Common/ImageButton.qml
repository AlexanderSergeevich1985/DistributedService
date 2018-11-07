import QtQuick 2.0

Rectangle {
    id: buttonId

    property url source: ""
    property int size: 40
    property alias mirror: imageId.mirror

    signal clicked

    property bool checkable: false
    property bool checked: false
    property url sourceChecked: ""

    width: size
    height: size
    color: "transparent"

    Image {
        id: imageId
        anchors.fill: parent;
        fillMode: Image.PreserveAspectFit
        source: buttonId.source
    }

    MouseArea {
        id: mousearea
        hoverEnabled: true
        anchors.fill: parent
        onClicked: {
            if ( buttonId.checkable ) {
                if ( buttonId.state === 'checked' ) {
                    buttonId.state = 'unchecked';
                } else {
                    buttonId.state = 'checked';
                }
            }
            buttonId.clicked()
        }
    }

    states: [
        State {
            name: "unchecked"
            PropertyChanges {target: imageId; source: buttonId.source}
            PropertyChanges {target: buttonId; checked: false}
        },
        State {
            name: "checked"
            PropertyChanges {target: imageId; source: buttonId.sourceChecked}
            PropertyChanges {target: buttonId; checked: true}
        }
    ]
}
