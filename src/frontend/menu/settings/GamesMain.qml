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
import "qrc:/qmlutils" as PegasusUtils
import QtQuick 2.0
import QtQuick.Window 2.2


FocusScope {
    id: root

    signal close
    signal openBiosChecking_Settings
    signal openAdvancedEmulator_Settings
//    signal openKeySettings
//    signal openGamepadSettings
//    signal openGameDirSettings

    width: parent.width
    height: parent.height
    visible: 0 < (x + width) && x < Window.window.width

    enabled: focus

    Keys.onPressed: {
        if (api.keys.isCancel(event) && !event.isAutoRepeat) {
            event.accepted = true;
            root.close();
            api.internal.recalbox.saveParameters();
        }
    }


    PegasusUtils.HorizontalSwipeArea {
        anchors.fill: parent
        onSwipeRight: root.close()
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: root.close()
    }

    ScreenHeader {
        id: header
        text: qsTr("Games") + api.tr
        z: 2
    }

    Flickable {
        id: container

        width: content.width
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        contentWidth: content.width
        contentHeight: content.height

        Behavior on contentY { PropertyAnimation { duration: 100 } }

        readonly property int yBreakpoint: height * 0.7
        readonly property int maxContentY: contentHeight - height

        function onFocus(item) {
            if (item.focus)
                contentY = Math.min(Math.max(0, item.y - yBreakpoint), maxContentY);
        }

        FocusScope {
            id: content

            focus: true
            enabled: focus

            width: contentColumn.width
            height: contentColumn.height

            Column {
                id: contentColumn
                spacing: vpx(5)

                width: root.width * 0.7
                height: implicitHeight

                Item {
                    width: parent.width
                    height: header.height + vpx(25)
                }

                SectionTitle {
                    text: qsTr("Game Screen") + api.tr
                    first: true
                }

                MultivalueOption {
                    id: optGameRatio
                    // set focus only on first item
                    focus: true

                    label: qsTr("Game Ratio") + api.tr                    
                    note: qsTr("Set ratio for all emulators (auto,4/3,16/9,16/10,custom)") + api.tr

                    onActivate: {
                        focus = true;
                        localeBox.focus = true;
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.up: optAdvancedEmulator
                    KeyNavigation.down: optPixelPerfect
                }

                ToggleOption {
                    id: optPixelPerfect

                    label: qsTr("Pixel Perfect") + api.tr
                    note: qsTr("Once enabled, your screen will be cropped, and you will have a pixel perfect image") + api.tr

                    checked: api.internal.recalbox.getBoolParameter("global.integerscale")
                    onCheckedChanged: {
                        focus = true;
                        api.internal.recalbox.setBoolParameter("global.integerscale",checked);
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.down: optSmoothGame
                }

                ToggleOption {
                    id: optSmoothGame

                    label: qsTr("Smooth Games") + api.tr
                    note: qsTr("Set smooth for all emulators") + api.tr

                    checked: api.internal.recalbox.getBoolParameter("global.smooth")
                    onCheckedChanged: {
                        focus = true;
                        api.internal.recalbox.setBoolParameter("global.smooth",checked);
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.down: optShaders
                }

                MultivalueOption {
                    id: optShaders

                    label: qsTr("Shaders") + api.tr
                    note: qsTr("Set prefered Shader effect") + api.tr

//                    value: api.internal.settings.locales.currentName

                    onActivate: {
                        focus = true;
                        localeBox.focus = true;
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.down: optShowFramerate
                }

                ToggleOption {
                    id: optShowFramerate

                    label: qsTr("Show Framerate") + api.tr
                    note: qsTr("Show FPS in game") + api.tr

                    checked: api.internal.recalbox.getBoolParameter("global.framerate")
                    onCheckedChanged: {
                        focus = true;
                        api.internal.recalbox.setBoolParameter("global.framerate",checked);
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.down: optGameRewind
                }

                SectionTitle {
                    text: qsTr("Gameplay Option") + api.tr
                    first: true
                }

                ToggleOption {
                    id: optGameRewind

                    label: qsTr("Game Rewind") + api.tr
                    note: qsTr("Set rewind for all emulators 'Only work with Retroarch' ") + api.tr

                    checked: api.internal.recalbox.getBoolParameter("global.rewind")
                    onCheckedChanged: {
                        focus = true;
                        api.internal.recalbox.setBoolParameter("global.rewind",checked);
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.down: optAutoSave
                }

                ToggleOption {
                    id: optAutoSave

                    label: qsTr("Auto Save/load") + api.tr
                    note: qsTr("Set autosave/load savestate for all emulators") + api.tr

                    checked: api.internal.recalbox.getBoolParameter("global.autosave")
                    onCheckedChanged: {
                        focus = true;
                        api.internal.recalbox.setBoolParameter("global.autosave",checked);
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.down: optBiosChecking
                }

                SectionTitle {
                    text: qsTr("Other Option") + api.tr
                    first: true
                }

                SimpleButton {
                    id: optBiosChecking

                    label: qsTr("Bios Checking") + api.tr
                    note: qsTr("Check all necessary bios !") + api.tr
                    onActivate: {
                        focus = true;
                        root.openBiosChecking_Settings();
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.down: optAdvancedEmulator
                }

                SimpleButton {
                    id: optAdvancedEmulator

                    label: qsTr("Advandced Emulator Settings") + api.tr
                    note: qsTr("choose emulator, ratio and more by system") + api.tr

                    onActivate: {
                        focus = true;
                        root.openAdvancedEmulator_Settings();
                    }
                    onFocusChanged: container.onFocus(this)
                    KeyNavigation.down: optGameRatio
                }

                Item {
                    width: parent.width
                    height: vpx(30)
                }
            }
        }
    }


    MultivalueBox {
        id: localeBox
        z: 3

        model: api.internal.settings.locales
        index: api.internal.settings.locales.currentIndex

        onClose: content.focus = true
        onSelect: api.internal.settings.locales.currentIndex = index
    }
    MultivalueBox {
        id: themeBox
        z: 3

        model: api.internal.settings.themes
        index: api.internal.settings.themes.currentIndex

        onClose: content.focus = true
        onSelect: api.internal.settings.themes.currentIndex = index
    }
}
