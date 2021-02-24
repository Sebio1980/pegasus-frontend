// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


import "common"
import "gamedireditor"
import QtQuick 2.12
import QtQuick.Layouts 1.0
import QtQuick.VirtualKeyboard 2.15


FocusScope {
    id: root

    property bool mSettingsChanged: false

    signal close

    anchors.fill: parent

    enabled: focus
    visible: opacity > 0.001
    opacity: focus ? 1.0 : 0.0
    Behavior on opacity { PropertyAnimation { duration: 150 } }

    function closeMaybe() {
        if (mSettingsChanged)
            reloadDialog.focus = true;
        else
            root.close();
            inputPanel.close();
    }

    Keys.onPressed: {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.closeMaybe();
            inputPanel.closeMaybe();
        }
    }
    Rectangle {
        id: shade

        anchors.fill: parent
        color: "#000"
        opacity: 0.75

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: root.closeMaybe()
        }
    }
    Rectangle {
        id: boxMenu
        height: parent.height * 0.5
        width: height * 1.5
        color: "#333"
        radius: vpx(8)

        anchors.centerIn: parent

        // TODO: proper gamepad button mapping
        Keys.onPressed: {
            if (event.isAutoRepeat)
                return;

            var do_remove = event.key === Qt.Key_Delete || api.keys.isDetails(event);
            var do_add = api.keys.isFilters(event);
            if (!do_add && !do_remove)
                return;

            event.accepted = true;

            if (do_remove) {
                if (list.focus && !root.isSelected(list.currentIndex))
                    root.toggleIndex(list.currentIndex);

                root.startDeletion();
            }
            if (do_add)
                filePicker.focus = true;
        }
        Keys.onReleased: {
            if (event.isAutoRepeat)
                return;
            if (event.key !== Qt.Key_Delete && !api.keys.isDetails(event))
                return;

            event.accepted = true;
            root.stopDeletion();
        }
        MouseArea {
            anchors.fill: parent
        }
        Text {
            id: info

            text: qsTr("bla bla bla") + api.tr
            color: "#999"
            font.family: globalFonts.sans
            font.pixelSize: vpx(18)
            lineHeight: 0.7

            anchors.top: parent.top
            width: parent.width
            padding: font.pixelSize * lineHeight

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }
        Rectangle {
            anchors.top: info.bottom
            anchors.bottom: footer.top
            width: parent.width - vpx(20)
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#222"
            radius: vpx(8)

            ListView {
                id: list
                anchors.fill: parent
                clip: true

                model: api.internal.settings.gameDirs
                delegate: listEntry

                focus: true
                highlightRangeMode: ListView.ApplyRange
                preferredHighlightBegin: height * 0.5 - vpx(18) * 1.25
                preferredHighlightEnd: height * 0.5 + vpx(18) * 1.25
                highlightMoveDuration: 0

                KeyNavigation.down: buttonAdd

                onModelChanged: {
                    if (isComplete)
                        root.mSettingsChanged = true;
                }

                property bool isComplete: false
                Component.onCompleted: isComplete = true

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var new_idx = list.indexAt(mouse.x, list.contentY + mouse.y);
                        if (new_idx < 0)
                            return;

                        list.currentIndex = new_idx;
                        root.toggleIndex(new_idx);
                    }
                }
            }
        }
        Item {
            id: footer

            width: parent.width
            height: buttonRow.height
//                    * 1.75
            anchors.bottom: parent.bottom

            Row {
                id: buttonRow

                anchors.centerIn: parent
//                spacing: height * 0.75

                GameDirEditorButton {
                    id: buttonAdd

                    image1: "qrc:/buttons/xb_y.png"
                    image2: "qrc:/buttons/ps_triangle.png"
                    text: qsTr("Add new") + api.tr

//                    onPress: filePicker.focus = true
                    KeyNavigation.right: buttonDel
                }
                GameDirEditorButton {
                    id: buttonDel

                    image1: "qrc:/buttons/xb_x.png"
                    image2: "qrc:/buttons/ps_square.png"
                    text: qsTr("Remove selected") + api.tr

                    onPress: root.startDeletion();
                    onRelease: root.stopDeletion();

                    KeyNavigation.up: list
                }
            }
        }
    }
//  keyboard input
    InputPanel {
        id: inputPanel
        width: footer.width // + vpx(200)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: boxMenu.bottom
    }
    Component {
        id: listEntry

        Rectangle {
            readonly property bool highlighted: ListView.view.focus
                                                && (ListView.isCurrentItem || mouseArea.containsMouse)
            readonly property bool selected: root.isSelected(index)

            width: parent.width
            height: label.height
            color: highlighted ? "#585858" : "transparent"

            Keys.onPressed: {
                if (api.keys.isAccept(event) && !event.isAutoRepeat) {
                    event.accepted = true;
                    root.toggleIndex(index);
                }
            }
//            Rectangle {
//                anchors.fill: parent
//                color: "#d55"
//                visible: parent.selected
//            }
//            Rectangle {
//                id: deleteFill
//                height: parent.height
//                width: parent.width * deletionPercent
//                color: "#924"
//                visible: parent.selected && deleteTimer.running && width > 0
//            }
//            Text {
//                id: label
//                text: modelData
//                verticalAlignment: Text.AlignVCenter
//                lineHeight: 2

//                color: "#eee"
//                font.family: globalFonts.sans
//                font.pixelSize: vpx(18)

//                width: parent.width
//                leftPadding: parent.height * 0.5
//                rightPadding: leftPadding
//                elide: Text.ElideRight
//            }
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }
    }
    ReloadQuestion {
        id: reloadDialog
        onAccept: {
            list.focus = true;
            root.mSettingsChanged = false;
            api.internal.settings.reloadProviders();
        }
        onCancel: {
            list.focus = true;
            root.mSettingsChanged = false;
            root.close();
        }
    }
}
