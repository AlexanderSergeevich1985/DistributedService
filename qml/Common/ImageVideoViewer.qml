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

import QtQuick.Controls 1.1
import QtQuick.Controls 1.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.1

import QtMultimedia 5.8

Rectangle {
    property double ratioX: screen_width/width
    property double ratioY: screen_height/height
    property double screen_width: 2560
    property double screen_height: 1240
    id: imageVideoViewerId
    focus: true
    anchors.left: parent.left;
    anchors.right: parent.right;

    function setSize(width_, height_) {
        screen_width = width_;
        screen_height = height_;
        ratioX = screen_width/width
        ratioY = screen_height/height
    }

    Keys.onPressed: {
        var k, m, shift, ctrl;
        k = event.key;
        m = event.modifiers;
        shift = event.modifiers & Qt.ShiftModifier;
        ctrl  = event.modifiers & Qt.ControlModifier;
        event.accepted = true;
        videoSource.keyPressed(k, m)
    }

    MouseArea {
        Keys.onPressed: {
            console.log("key pressed")
            var k, m, shift, ctrl;
            k = event.key;
            m = event.modifiers;
            shift = event.modifiers & Qt.ShiftModifier;
            ctrl  = event.modifiers & Qt.ControlModifier;
            event.accepted = true;
            videoSource.keyPressed(k, m)
        }
        id: mouseAreaId
        anchors.fill: parent
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.columnSpan: 1
        Layout.rowSpan: 1
        Layout.row: 1
        Layout.column: 1
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        states: [
            State {
                name: "BASE"
                PropertyChanges { target: mouseAreaId; drag.target: undefined}
            },
            State {
                name: "DRAGGABLE"
                PropertyChanges { target: mouseAreaId; drag.target: parent}
            }
        ]
        drag {
            axis: Drag.XandYAxis
            minimumX: 0
            minimumY: 0
            maximumX: parent.parent.width - parent.width
            maximumY: parent.parent.height - parent.height
            smoothed: true
        }
        onClicked: {
            videoSource.mouseEvents(mouse.button, ratioX * mouse.x, ratioY * mouse.y)
        }
        onPressAndHold: {
            if(mouse.button == 1) {
                mouseAreaId.state = "DRAGGABLE"
                mouse.accepted = false //mouse event is USED but NOT CONSUMED...
            }
        }
        onPositionChanged: {
            if(mouseAreaId.containsMouse) {
                if(mouseAreaId.state == "DRAGGABLE") {
                    videoSource.mouseEvents(ratioX * mouse.x, ratioY * mouse.y)
                }
            }
        }
        onReleased: {
            mouseAreaId.state = "BASE"
            videoSource.mouseEvents(mouse.button, ratioX * mouse.x, ratioY * mouse.y, false)
        }
        onWidthChanged: {
            ratioX = screen_width/width
        }
        onHeightChanged: {
            ratioY = screen_height/height
        }
    }
    Image {
        id: image
        anchors.fill: parent
        source: "qrc:///icons/background.jpg"
        fillMode: "Stretch"
    }
    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        source: videoSource
        fillMode: "Stretch"
    }
}
