import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: mainWindow
    width: 1920
    height: 1080
    visible: true
    title: "Banking System"
    color: "#0F1C2E" // Тёмно-синий фон

    // Загружаем экран логина
    Screen01 {
        anchors.fill: parent
    }
}
