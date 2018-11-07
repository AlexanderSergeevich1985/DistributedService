/**The MIT License (MIT)
 Copyright (c) 2018 by AleksanderSergeevich
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */
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
