import QtQuick.Controls
import QtQuick.Layouts
import QtQuick
import WayCrateLock

Page {
    id: root
    objectName: "root"
    anchors.fill: parent
    property bool isIn: false
    property date currentTime: new Date()
    property bool busy: CommandLine.busy

    function leftPad(number) {
        var output = number + '';
        while (output.length < 2) {
            output = '0' + output;
        }
        return output;
    }

    function checkPassword() {
        CommandLine.RequestUnlock();
    }

    background: Image {
        anchors.fill: root
        fillMode: Image.PreserveAspectCrop
        source: CommandLine.background
        opacity: CommandLine.opacity
    }

    Item {
        Timer {
            interval: 900
            running: true
            repeat: true
            onTriggered: root.timeChanged()
        }
    }

    function timeChanged() {
        var date = new Date;
        currentTime = date;
    }

    Keys.onPressed: {
        root.isIn = true;
        root.forceActiveFocus();
        input.focus = true;
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            root.isIn = true;
            root.forceActiveFocus();
            input.focus = true;
        }
        onExited: {
            root.isIn = false;
            input.focus = false;
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.horizontalCenter: parent.horizontalCenter

            Item {
                Layout.preferredHeight: 50
            }

            Label {
                font.family: "SFMono Nerd Font Bold"
                color: "#FEFDF7"
                opacity: 0.9
                text: Qt.formatDateTime(root.currentTime, "dddd, d MMMM")
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: root.isIn ? 35 : 30
                font.bold: true
                Behavior on font.pointSize {
                    enabled: true
                    SmoothedAnimation {
                        velocity: 70
                    }
                }
            }

            Label {
                font.family: "SFMono Nerd Font Bold"
                color: "#FEFDF7"
                opacity: 0.7
                Layout.alignment: Qt.AlignHCenter
                text: Qt.formatTime(root.currentTime, "hh:mm")
                font.pointSize: root.isIn ? 85 : 80
                font.bold: true
                Behavior on font.pointSize {
                    enabled: true
                    SmoothedAnimation {
                        velocity: 70
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            Label {
                visible: MediaPlayerBackend.hasMedia
                text: MediaPlayerBackend.currentDisplayName
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 13
                font.bold: true
            }

            RowLayout {
                visible: MediaPlayerBackend.hasMedia
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                RoundButton {
                    enabled: MediaPlayerBackend.canGoNext
                    implicitWidth: 60
                    implicitHeight: 60
                    Layout.alignment: Qt.AlignHCenter
                    icon.name: "go-previous"
                    onClicked: {
                        MediaPlayerBackend.goPre();
                    }
                }
                RoundButton {
                    enabled: MediaPlayerBackend.canPlay
                    implicitWidth: 60
                    implicitHeight: 60
                    Layout.alignment: Qt.AlignHCenter
                    icon.name: MediaPlayerBackend.playbackStatus === "Playing" ? "media-playback-pause" : "media-playback-start"

                    onClicked: {
                        if (MediaPlayerBackend.playbackStatus === "Playing") {
                            MediaPlayerBackend.pause();
                            return;
                        }
                        MediaPlayerBackend.play();
                    }
                }
                RoundButton {
                    enabled: MediaPlayerBackend.canGoNext
                    implicitWidth: 60
                    implicitHeight: 60
                    Layout.alignment: Qt.AlignHCenter
                    icon.name: "go-next"
                    onClicked: {
                        MediaPlayerBackend.goNext();
                    }
                }
            }

            Item {
                Layout.preferredHeight: 30
            }

            TextField {
                id: input
                property bool showText: false
                property bool stateVisible: root.isIn

                objectName: "input"

                Layout.preferredWidth: 250
                Layout.alignment: Qt.AlignHCenter

                enabled: !root.busy
                text: CommandLine.password
                placeholderText: "Password"
                onEditingFinished: {
                    CommandLine.password = input.text;
                }
                onAccepted: checkPassword()

                Keys.onEscapePressed: {
                    focus = false;
                }

                states: [
                    State {
                        name: "Visible"
                        when: root.isIn
                        PropertyChanges {
                            target: input
                            opacity: 1.0
                        }
                        PropertyChanges {
                            target: input
                            visible: true
                        }
                    },
                    State {
                        name: "Invisible"
                        when: !root.isIn
                        PropertyChanges {
                            target: input
                            opacity: 0.0
                        }
                        PropertyChanges {
                            target: input
                            visible: false
                        }
                    }
                ]

                transitions: [
                    Transition {
                        from: "Visible"
                        to: "Invisible"

                        SequentialAnimation {
                            NumberAnimation {
                                property: "opacity"
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                property: "visible"
                                duration: 0
                            }
                        }
                    },
                    Transition {
                        from: "Invisible"
                        to: "Visible"

                        SequentialAnimation {
                            NumberAnimation {
                                property: "visible"
                                duration: 0
                            }
                            NumberAnimation {
                                property: "opacity"
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                ]

                rightPadding: 40
                echoMode: showText ? TextField.Normal : TextField.Password
                RoundButton {
                    font.family: "Ubuntu Nerd Font"
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: input.showText ? "" : ""
                    onClicked: input.showText = !input.showText
                    background: Rectangle {
                        color: Qt.alpha(Material.accent, 0.2)
                        radius: 25
                    }
                }
            }

            Label {
                id: errorLabel
                text: CommandLine.errorMessage
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 15
                color: "red"
                font.bold: true

                states: [
                    State {
                        when: CommandLine.errorMessageVisible && root.isIn
                        name: "Visible"
                        PropertyChanges {
                            target: errorLabel
                            opacity: 1.0
                        }
                        PropertyChanges {
                            target: errorLabel
                            visible: true
                        }
                    },
                    State {
                        name: "Invisible"
                        when: !(CommandLine.errorMessageVisible && root.isIn)
                        PropertyChanges {
                            target: errorLabel
                            opacity: 0.0
                        }
                        PropertyChanges {
                            target: errorLabel
                            visible: false
                        }
                    }
                ]

                transitions: [
                    Transition {
                        from: "Visible"
                        to: "Invisible"

                        SequentialAnimation {
                            NumberAnimation {
                                property: "opacity"
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                property: "visible"
                                duration: 0
                            }
                        }
                    },
                    Transition {
                        from: "Invisible"
                        to: "Visible"
                        SequentialAnimation {
                            NumberAnimation {
                                property: "visible"
                                duration: 0
                            }
                            NumberAnimation {
                                property: "opacity"
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                ]
            }

            BusyIndicator {
                visible: root.busy && root.isIn
                Layout.alignment: Qt.AlignHCenter
            }

            Item {
                Layout.preferredHeight: 40
            }

            Item {
                Layout.preferredHeight: 30
            }
        }
    }
}
