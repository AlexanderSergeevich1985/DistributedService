import QtQuick 2.0

Rectangle {
    id: statusBar
    property int font_size: 12
    property int size: 1.7 * font_size
    property string message: "Loaded"

    width: parent.width
    height: size
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    Style { id: style }
    border.color: style.borderColor
    border.width: 2
    color: style.backgroundColor2

    Text {
        id: statusCount
        color: style.textColor
        font.family: "Courier New"
        font.bold: true
        font.pointSize: font_size
        text: message
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 5
        anchors.centerIn: parent
    }
}
