import QtQuick.Controls
import QtQuick.Layouts
import QtQuick
import WayCrateLock

Page {
    id: root
    anchors.fill: parent
    property bool isIn: false

    property int year
    property int month
    property int day

    property int hours
    property int minutes

    function leftPad(number) {
        var output = number + '';
        while (output.length < 2) {
            output = '0' + output;
        }
        return output;
    }

    background: Image {
        anchors.fill: root
        fillMode: Image.PreserveAspectCrop
        source: CommandLine.background
        opacity: CommandLine.opacity
    }
    Item {
        Timer {
            interval: 100
            running: true
            repeat: true
            onTriggered: root.timeChanged()
        }
    }

    function timeChanged() {
        var date = new Date;
        year = date.getFullYear();
        month = date.getMonth() + 1;
        day = date.getDate();
        hours = date.getHours();
        minutes = date.getMinutes();
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            root.isIn = true;
            input.focus = true;
            root.forceActiveFocus();
        }
        onExited: {
            root.isIn = false;
            input.focus = false;
        }

        ColumnLayout {
            anchors.fill: parent

            Item {
                Layout.preferredHeight: 30
            }

            Label {
                text: CommandLine.currentDate
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: root.isIn ? 30 : 35
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

            TimePanel {
                Layout.alignment: Qt.AlignHCenter
            }

            Item {
                Layout.preferredHeight: 30
            }

            TextField {
                objectName: "input"
                id: input
                visible: root.isIn && CommandLine.usePam
                Layout.alignment: Qt.AlignHCenter
                text: CommandLine.password
                placeholderText: "Password"
                onEditingFinished: {
                    CommandLine.password = input.text;
                }
                echoMode: TextInput.Password
                Layout.preferredWidth: 250
                onAccepted: {
                    CommandLine.RequestUnlock();
                }
            }

            Label {
                visible: CommandLine.errorMessage !== "" && root.isIn
                text: CommandLine.errorMessage
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 15
                color: "red"
            }

            RoundButton {
                visible: root.isIn
                implicitWidth: 60
                implicitHeight: 60
                Layout.alignment: Qt.AlignHCenter
                icon.name: "unlock"
                onClicked: {
                    CommandLine.RequestUnlock();
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
                Layout.preferredHeight: 40
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: root.leftPad(root.hours)
                    font.pointSize: root.isIn ? 30 : 35
                    font.bold: true
                    Layout.preferredWidth: 80
                    Behavior on font.pointSize {
                        enabled: true
                        SmoothedAnimation {
                            velocity: 70
                        }
                    }
                }
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: ":"
                    font.pointSize: root.isIn ? 30 : 35
                    font.bold: true
                    Behavior on font.pointSize {
                        enabled: true
                        SmoothedAnimation {
                            velocity: 70
                        }
                    }
                }
                Label {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: root.leftPad(root.minutes)
                    font.pointSize: root.isIn ? 30 : 35
                    font.bold: true
                    Layout.preferredWidth: 80
                    Behavior on font.pointSize {
                        enabled: true
                        SmoothedAnimation {
                            velocity: 70
                        }
                    }
                }
            }

            Item {
                Layout.preferredHeight: 30
            }
        }
    }
}
