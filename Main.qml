// Angelina desk pet.
// Copyright (C) 2026 CorvusCinereus
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Dialogs

ApplicationWindow {
    id: window
    visible: true
    title: qsTr("安洁莉娜")
    flags: Qt.FramelessWindowHint | Qt.Window | Qt.WindowStaysOnTopHint
    color: "transparent"

    property color reallyDark: "#1f1f1f"
    property color dark: "#262626"

    property bool isPlaying: false

    signal show_message(string title, string content)
    signal play_music(string music)

    onShow_message: (title, content) => {
        message_dialog.title = title
        message_dialog.text = content
        message_dialog.open()
    }

    function choose_file() {
        file_dialog.open()
    }

    function choose_dir() {
        folder_dialog.open()
    }

    Item {
        id: key_listener
        anchors.fill: parent
        focus: true
        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_Escape) {
                window.close()
            }

            if (event.key === Qt.Key_Equal || event.key === Qt.Key_Plus) {
                window.width += 40
                window.height += 40
            }

            if (event.key === Qt.Key_Minus) {
                window.width -= 40
                window.height -= 40
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
                window.startSystemMove()
            }
        }

        onDoubleClicked: {
            // TODO
            window.choose_file()
        }

        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton) {
                menu.open()
            }
        }
    }

    AnimatedImage {
        objectName: "gif"
        id: gif
        anchors.fill: parent
        source: "art/images/坐坐.gif"
    }

    MessageDialog {
        id: message_dialog

        buttons: MessageDialog.Ok

        onAccepted: {
            key_listener.focus = true
        }
    }

    FileDialog {
        id: file_dialog
        title: qsTr("选择文件")
        fileMode: FileDialog.OpenFiles
        nameFilters: ["Music files (*.mp3 *.flac *.wav *.ogg)", "All files (*.*)"]

        onAccepted: {
            window.play_music(selectedFiles[0])
        }
    }

    FolderDialog {
        id: folder_dialog
        title: qsTr("选择目录")

        onAccepted: {
            console.log(selectedFolder)
        }
    }

    Popup {
        id: menu
        anchors.centerIn: parent
        width: parent.width / 2
        height: Math.min(scroll1.contentHeight + 40, parent.height * 0.8)
        background: Rectangle {
            anchors.fill: parent
            color: window.dark
            radius: 15
            opacity: 0.9
        }

        contentItem: ScrollView {
            id: scroll1
            anchors.fill: parent
            anchors.margins: 20
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                spacing: 10
                width: scroll1.availableWidth

                Button {
                    text: qsTr("听歌")
                    Layout.fillWidth: true
                    palette.buttonText: "white"
                    background: Rectangle {
                        anchors.fill: parent
                        color: window.reallyDark
                        radius: 15
                    }

                    onClicked: {
                        menu.close()
                        music_menu.open()
                    }
                }

                Button {
                    text: qsTr("退出")
                    Layout.fillWidth: true
                    palette.buttonText: "white"
                    background: Rectangle {
                        anchors.fill: parent
                        color: window.reallyDark
                        radius: 15
                    }

                    onClicked: {
                        window.close()
                    }
                }
            }
        }
    }

    Popup {
        id: music_menu
        anchors.centerIn: parent
        width: parent.width / 3 * 2
        height: Math.min(parent.height * 0.8, 420)
        padding: 20

        background: Rectangle {
            anchors.fill: parent
            color: window.dark
            radius: 15
            opacity: 0.9
        }

        contentItem: ColumnLayout {
            spacing: 10

            // 用 Item 包住 ListView，方便叠一个"空列表"提示
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 120

                ListView {
                    id: music_list_view
                    anchors.fill: parent
                    clip: true
                    spacing: 5

                    model: ListModel {
                        id: music_list
                    }

                    delegate: Rectangle {
                        required property int index
                        required property string title

                        width: music_list_view.width
                        height: 40
                        radius: 10
                        color: ListView.isCurrentItem ? "#3f3f3f" : window.reallyDark

                        Text {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: title
                            color: "white"
                            elide: Text.ElideRight
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: music_list_view.currentIndex = index
                        }
                    }

                    // 当前项高亮
                    highlight: Rectangle {
                        color: "transparent"
                        border.color: "#7aa2f7"
                        border.width: 1
                        radius: 10
                    }
                    highlightFollowsCurrentItem: true
                }

                // 空状态提示
                Text {
                    anchors.centerIn: parent
                    visible: music_list.count === 0
                    text: qsTr("暂无歌曲")
                    color: "#888888"
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 5

                Button {
                    text: "添加文件"
                    Layout.fillWidth: true
                    palette.buttonText: "white"
                    background: Rectangle {
                        anchors.fill: parent
                        color: window.reallyDark
                        radius: 15
                    }
                    onClicked: {
                        window.choose_file()
                    }
                }

                Button {
                    text: "添加目录"
                    Layout.fillWidth: true
                    palette.buttonText: "white"
                    background: Rectangle {
                        anchors.fill: parent
                        color: window.reallyDark
                        radius: 15
                    }
                    onClicked: {
                        window.choose_dir()
                    }
                }

                // Button {
                //     text: "-"
                //     Layout.fillWidth: true
                //     palette.buttonText: "white"
                //     background: Rectangle {
                //         anchors.fill: parent
                //         color: window.reallyDark
                //         radius: 15
                //     }
                //     onClicked: {
                //         if (music_list_view.currentIndex >= 0) {
                //             music_list.remove(music_list_view.currentIndex)
                //         }
                //     }
                // }
            }

            Button {
                text: qsTr("清空")
                Layout.fillWidth: true
                palette.buttonText: "white"
                background: Rectangle {
                    anchors.fill: parent
                    color: window.reallyDark
                    radius: 15
                }
                onClicked: {
                    music_list.clear()
                }
            }

            Button {
                text: qsTr("关闭")
                Layout.fillWidth: true
                palette.buttonText: "white"
                background: Rectangle {
                    anchors.fill: parent
                    color: window.reallyDark
                    radius: 15
                }
                onClicked: {
                    music_menu.close()
                }
            }
        }
    }
}
