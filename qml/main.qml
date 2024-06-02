import QtQuick
import QtQuick.Controls.Material
import WayCrateLock

Window {
    id: w
    Material.theme: Material.Dark
    Material.accent: Material.Indigo
    // opacity: 0
    //
    // NumberAnimation on opacity {
    //     from: 0
    //     to: 1
    //     duration: 1000
    //     running: true
    // }

    MainView {
        anchors.fill: parent
    }
}
