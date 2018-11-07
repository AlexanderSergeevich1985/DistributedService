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
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4

Rectangle {
    id: dateInputId
    anchors.margins: 10
    height: 200
    color: "lightskyblue"
    clip: true
    anchors.centerIn: parent
    property date currentDate: new Date(2018, 11, 5)

    signal dateChanged(date value)

    Component.onCompleted: {
        dayId.value = currentDate.getDate();
        monthId.value = currentDate.getMonth();
        yearId.value = currentDate.getFullYear();
    }

    Timer {
        id: timerId
    }

    function delay(delayTime, cb) {
        timerId.interval = delayTime;
        timerId.repeat = false;
        timerId.triggered.connect(cb);
        timerId.start();
    }

    ColumnLayout {
        id: columnId
        width: parent ? parent.width : 100
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
            Label {
                text: localizationId.dialogCaption
                font.family: "Helvetica"
                font.bold: true
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignBaseline | Qt.AlignLeft
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 20
            Layout.bottomMargin: 20
            Label {
                text: localizationId.dayCaption
                font.family: "Helvetica"
                font.bold: true
                Layout.alignment: Qt.AlignBaseline | Qt.AlignLeft
            }
            SpinBox {
                id: dayId
                onEditingFinished: {
                    currentDate.setDate(value)
                }
                minimumValue: 1
                maximumValue: 31
            }
            Label {
                text: localizationId.monthCaption
                font.family: "Helvetica"
                font.bold: true
                Layout.alignment: Qt.AlignBaseline | Qt.AlignLeft
            }
            SpinBox {
                id: monthId
                onEditingFinished: {
                    currentDate.setMonth(value)
                }
                minimumValue: 1
                maximumValue: 12
                onValueChanged: {
                    dayId.maximumValue = checkLeapYear(value, yearId.value);
                }
                function checkLeapYear(month, year) {
                    if(month == 4 || month == 6 || month == 9 || month == 11) {
                        return 30;
                    }
                    else if(month == 2) {
                        if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {//check for leap year
                            return 29;
                        }
                        else {
                            return 28;
                        }
                    }
                    else {
                        return 31;
                    }
                }
            }
            Label {
                text: localizationId.yearCaption
                font.family: "Helvetica"
                font.bold: true
                Layout.alignment: Qt.AlignBaseline | Qt.AlignLeft
            }
            SpinBox {
                property int cashedValue: 1;
                id: yearId
                onEditingFinished: {
                    currentDate.setFullYear(value)
                }
                minimumValue: -40000
                maximumValue: 40000
                onValueChanged: {
                    delay(0.00001, function() {
                        if(value == 0) {
                            if(cashedValue > 0) {
                                value = -1;
                                cashedValue = -1;
                            }
                            else {
                                value = 1;
                                cashedValue = 1;
                            }
                        }
                    });
                }
            }
        }
        RowLayout {
            id: buttonLayoutId
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            property int fontSize: 14
            property int buttonWidth: fontSize * Math.max(localizationId.buttonSaveCaption.length, localizationId.buttonCancelCaption.length)

            Button {
                id: okButtonId
                implicitWidth: buttonLayoutId.buttonWidth
                Text {
                    id: okButtonTextId2
                    color: "gray"
                    font.family: "Courier New"
                    font.bold: true
                    font.pointSize: buttonLayoutId.fontSize
                    text: localizationId.buttonSaveCaption
                    anchors.centerIn: okButtonId
                }
                style: ButtonStyle {
                    padding {
                        left: 0.5 * buttonLayoutId.fontSize
                        right: 0.5 * buttonLayoutId.fontSize
                        top: 0.5 * buttonLayoutId.fontSize
                        bottom: 0.5 * buttonLayoutId.fontSize
                    }
                }
                onClicked: {
                    //gridId.date = new Date(yearId.value, monthId.value, dayId.value);
                    //dateInputId.dateChanged(new Date(yearId.value, monthId.value, dayId.value));
                    gridId.cashedDate = new Date(yearId.value, monthId.value, dayId.value);
                }
            }

            Item {
                Layout.fillHeight: true
                implicitWidth: 0.5 * buttonLayoutId.buttonWidth
            }

            Button {
                id: cancelButtonId
                implicitWidth: buttonLayoutId.buttonWidth
                Text {
                    id: cancelButtonTextId
                    color: "gray"
                    font.family: "Courier New"
                    font.bold: true
                    font.pointSize: buttonLayoutId.fontSize
                    text: localizationId.buttonCancelCaption
                    anchors.centerIn: cancelButtonId
                }
                style: ButtonStyle {
                    padding {
                        left: 0.5 * buttonLayoutId.fontSize
                        right: 0.5 * buttonLayoutId.fontSize
                        top: 0.5 * buttonLayoutId.fontSize
                        bottom: 0.5 * buttonLayoutId.fontSize
                    }
                }
                onClicked: {
                    dateDialog.close();
                }
            }
        }
    }
}
