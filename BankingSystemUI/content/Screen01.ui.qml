import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: loginScreen
    width: 1920
    height: 1080
    color: "#0F1C2E" // dark blue background

    // ---------- App Title ----------
    Text {
        text: "Banking System"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 120
        font.pixelSize: 48
        color: "white"
        font.bold: true
    }

    // ---------- Login Form ----------
    Rectangle {
        id: formContainer
        width: 500
        height: 400
        radius: 20
        color: "#162B45"
        anchors.centerIn: parent
        border.color: "#22486A"
        border.width: 2

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 30

            // Username Field
            TextField {
                id: usernameField
                placeholderText: "Username"
                width: 350
                height: 50
                font.pixelSize: 20
                color: "white"
                background: Rectangle {
                    radius: 10
                    color: "#243B5A"
                }
            }

            // Password Field
            TextField {
                id: passwordField
                placeholderText: "Password"
                echoMode: TextInput.Password
                width: 350
                height: 50
                font.pixelSize: 20
                color: "white"
                background: Rectangle {
                    radius: 10
                    color: "#243B5A"
                }
            }

            // Login Button
            Button {
                id: loginButton
                text: "Login"
                width: 200
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter
                background: Rectangle {
                    radius: 10
                    color: loginButton.down ? "#2D6AE3" : "#3E7DFB"
                }
                font.pixelSize: 20
                onClicked: {
                    // For now, just placeholder — later connect C++ Customer/Account
                }
            }
        }
    }

    // ---------- Footer ----------
    Text {
        text: "© 2025 Your Bank"
        color: "#7FA1C3"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        font.pixelSize: 16
    }
}
