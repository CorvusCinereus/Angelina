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
import Qt.labs.folderlistmodel

ApplicationWindow {
    id: window
    visible: true
    title: qsTr("安洁莉娜")
    flags: Qt.FramelessWindowHint | Qt.Window | Qt.WindowStaysOnTopHint
    color: "transparent"

    property color reallyDark: "#1f1f1f"
    property color dark: "#262626"

    property bool isPlaying: false
    property bool loop: false

    // 正在播放的歌曲路径（用于列表高亮）
    property string playing_music: ""

    // 统一的音乐扩展名过滤（供 FileDialog 与 FolderListModel 共用）
    readonly property var music_extensions: ["*.mp3", "*.flac", "*.wav", "*.ogg"]

    signal show_message(string title, string content)
    signal play_music(string music)
    signal stop_music()
    signal random()
    signal random_on_play()

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

    // 找到某首歌在列表里的下标，找不到返回 -1
    function find_music_index(path) {
        for (let i = 0; i < music_list.count; ++i) {
            if (music_list.get(i).path === path)
                return i
        }
        return -1
    }

    // 播放列表里的第 index 首
    function play_music_at(index) {
        if (index < 0 || index >= music_list.count)
            return

        let item = music_list.get(index)
        music_list_view.currentIndex = index
        window.playing_music = item.path
        // 必须先 play_music 再置 isPlaying：C++ 的 play_music 会先 stop_music()，
        // 而 stop_music() 在 isPlaying 为真时会 uninit 掉 _sound，提前置位会二次 uninit
        window.play_music(item.path)
        window.isPlaying = true
    }

    // 由 C++ 在音乐自然播完时调用（此时 isPlaying 已被置为 false，_sound 已释放）
    function at_music_end() {
        if (music_list.count === 0)
            return

        let index = find_music_index(window.playing_music)
        if (index < 0)
            return

        // 单曲循环：重放当前这首；列表循环：下一首，到底了就回到第一首
        play_music_at(window.loop ? index : (index + 1) % music_list.count)
    }

    // 把一首歌加入列表（自动去重）
    function add_music_file(url) {
        if (url === undefined || url === null)
            return
        let s = String(url)
        if (s.length === 0 || s === "undefined")
            return

        for (let i = 0; i < music_list.count; ++i) {
            if (music_list.get(i).path === s)
                return
        }

        let name = s.substring(s.lastIndexOf('/') + 1)
        try {
            name = decodeURIComponent(name)
        } catch (e) {
            // 解码失败就保留原样
        }
        music_list.append({title: name, path: s})
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
            if (!isPlaying) {
                window.random()
            }
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
    }

    FolderListModel {
        id: folder_model
        folder: ""
        showDirs: false
        showFiles: true
        showDotAndDotDot: false
        showOnlyReadable: true
        nameFilters: window.music_extensions
        sortField: FolderListModel.Name

        property bool scanning: false

        function scanInto() {
            for (let i = 0; i < count; ++i) {
                // 角色名在不同 Qt 版本里大小写不一致：Qt 5.15+ 为 "fileUrl"
                let url = get(i, "fileUrl")
                if (url === undefined)
                    url = get(i, "fileURL")
                if (url === undefined || url === null || String(url).length === 0)
                    continue
                window.add_music_file(url)
            }
        }

        onStatusChanged: {
            if (scanning && status === FolderListModel.Ready) {
                scanning = false
                scanInto()
            }
        }
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
            for (let i = 0; i < selectedFiles.length; ++i) {
                window.add_music_file(selectedFiles[i])
            }
        }
    }

    FolderDialog {
        id: folder_dialog
        title: qsTr("选择目录")

        onAccepted: {
            // 选的是同一个目录时 folder 不会变化，statusChanged 不会触发，手动扫一次
            if (String(folder_model.folder) === String(selectedFolder)
                && folder_model.status === FolderListModel.Ready) {
                folder_model.scanInto()
            } else {
                folder_model.scanning = true
                folder_model.folder = selectedFolder
            }
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
                    text: qsTr("停止")
                    visible: window.isPlaying
                    enabled: window.isPlaying
                    Layout.fillWidth: true
                    palette.buttonText: "white"
                    background: Rectangle {
                        anchors.fill: parent
                        color: window.reallyDark
                        radius: 15
                    }

                    onClicked: {
                        window.random()
                        window.stop_music()
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

        background: Rectangle {
            anchors.fill: parent
            color: window.dark
            radius: 15
            opacity: 0.9
        }

        // 窗口太矮时整个面板可以滚动，避免按钮被压扁
        contentItem: ScrollView {
            id: music_scroll
            anchors.fill: parent
            anchors.margins: 20
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                spacing: 10
                width: music_scroll.availableWidth - (music_scroll.ScrollBar.vertical.visible
                                                      ? music_scroll.ScrollBar.vertical.width : 0)
                // 内容不满一屏时撑满面板，超过一屏时才由 ScrollView 滚动
                height: Math.max(implicitHeight, music_scroll.availableHeight)

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
                            id: item_rect
                            required property int index
                            required property string title
                            required property string path

                            width: music_list_view.width
                            height: 40
                            radius: 10
                            color: is_playing ? "#3a4a6b"
                                              : (ListView.isCurrentItem ? "#3f3f3f" : window.reallyDark)

                            readonly property bool is_playing: window.playing_music === item_rect.path

                            Text {
                                anchors.left: parent.left
                                anchors.right: del_button.left
                                anchors.leftMargin: 12
                                anchors.rightMargin: 8
                                anchors.verticalCenter: parent.verticalCenter
                                text: item_rect.title
                                color: "white"
                                elide: Text.ElideRight
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: music_list_view.currentIndex = item_rect.index

                                onDoubleClicked: {
                                    music_list_view.currentIndex = item_rect.index
                                    window.playing_music = item_rect.path
                                    window.play_music(item_rect.path)
                                    window.isPlaying = true

                                    window.random_on_play()
                                }
                            }

                            Button {
                                id: del_button
                                text: "✕"
                                width: 26
                                height: 26
                                padding: 0
                                anchors.right: parent.right
                                anchors.rightMargin: 7
                                anchors.verticalCenter: parent.verticalCenter
                                palette.buttonText: "white"

                                background: Rectangle {
                                    anchors.fill: parent
                                    radius: width / 2
                                    color: del_button.hovered ? "#8c3a3a" : "#3a2a2a"
                                    opacity: 0.9
                                }

                                onClicked: {
                                    music_list.remove(item_rect.index)
                                }
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
                }

                Button {
                    text: window.loop ? qsTr("单曲循环")
                                      : qsTr("列表循环")
                    Layout.fillWidth: true
                    palette.buttonText: "white"
                    background: Rectangle {
                        anchors.fill: parent
                        color: window.reallyDark
                        radius: 15
                    }
                    onClicked: {
                        window.loop = !window.loop
                    }
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
}