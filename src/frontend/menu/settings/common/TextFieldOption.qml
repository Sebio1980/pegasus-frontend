import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQml 2.15

TextField {
    id: control
    focus: true
    color: themeColor.textLabel
    selectionColor: Qt.rgba(0.0, 0.0, 0.0, 0.15)
    selectedTextColor: color
    selectByMouse: false
    font.pixelSize: vpx(22)
    font.family: globalFonts.sans
    readOnly: true

    background: Rectangle {
        radius: vpx(20)
        color: control.activeFocus ? themeColor.secondary : themeColor.main
        border.color: control.activeFocus ? themeColor.screenHeader : themeColor.main
    }

    Keys.onReleased:{
        if(visible) event.accepted = virtualKeyboardOnReleased(event);
    }
    property bool active : false //set to false by default
    Keys.onPressed: {
        /*console.log("-----Before update-----");
        console.log("event.accepted : ", event.accepted);
		console.log("event.key : ", event.key);
        console.log("control.focus : ", control.focus);
        console.log("control.readOnly : ", control.readOnly);
        console.log("active : ",active);*/
        if(visible) event.accepted, control.focus, active = virtualKeyboardOnPressed(event,control,active);
        /*console.log("-----After update-----");
        console.log("event.accepted : ", event.accepted);
		console.log("event.key : ", event.key);
        console.log("control.focus : ", control.focus);
        console.log("control.readOnly : ", control.readOnly);
        console.log("active : ",active);*/
    }
}
