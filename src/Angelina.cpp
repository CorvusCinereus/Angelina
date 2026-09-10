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

#include "Angelina.h"
#include <QQmlProperty>
#include <QUrl>

Angelina::Angelina(QObject *root): _root(root) {
    _gif = _root->findChild<QObject*>("gif");

    const int size = _settings.value("window/size", 500).toInt();
    QQmlProperty(_root, "width").write(size);
    QQmlProperty(_root, "height").write(size);

    if (ma_engine_init(nullptr, &_engine) != MA_SUCCESS) {
        QMetaObject::invokeMethod(_root, "show_message", Qt::AutoConnection, Q_ARG(QString, "Error"), Q_ARG(QString, "Cannot init ma_engine!"));
        std::exit(-1);
    }

    {
        QObject::connect(_root, SIGNAL(play_music(QString)), this, SLOT(play_music(QString)));
        QObject::connect(_root, SIGNAL(stop_music()), this, SLOT(stop_music()));
        QObject::connect(_root, SIGNAL(random()), this, SLOT(random()));
    }
}

Angelina::~Angelina() {
    stop_music();
    ma_engine_uninit(&_engine);

    _settings.setValue("window/size", QQmlProperty(_root, "width").read().toInt());
}

void at_music_end(void* pUserData, ma_sound* sound) {
    auto angelina = static_cast<Angelina*>(pUserData);

    QMetaObject::invokeMethod(angelina, "handle_music_end", Qt::QueuedConnection);
}

void Angelina::handle_music_end() {
    if (!QQmlProperty(_root, "isPlaying").read().toBool() || !ma_sound_at_end(&_sound))
        return;

    stop_music();
    QQmlProperty(_root, "isPlaying").write(false);
    QMetaObject::invokeMethod(_root, "at_music_end");
}

void Angelina::play_music(const QString& file) {
    stop_music();

    auto music = QUrl(file).toLocalFile();
    if (ma_sound_init_from_file(&_engine, music.toStdString().c_str(), 0, nullptr, nullptr, &_sound) != MA_SUCCESS) {
        QMetaObject::invokeMethod(_root, "show_message", Qt::AutoConnection, Q_ARG(QString, "Error"), Q_ARG(QString, "无法播放"));
        return;
    }
    ma_sound_start(&_sound);
    ma_sound_set_end_callback(&_sound, at_music_end, this);
}

void Angelina::stop_music() {
    if (QQmlProperty(_root, "isPlaying").read().toBool()) {
        ma_sound_stop(&_sound);
        ma_sound_uninit(&_sound);
        QQmlProperty(_root, "isPlaying").write(false);
    }
}

void Angelina::random() {
    // TODO
}