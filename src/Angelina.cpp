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
#include <QFile>
#include <QQmlProperty>
#include <QUrl>
#include <random>

static int get_random_int(const int min, const int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

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
        QObject::connect(_root, SIGNAL(random_on_play()), this, SLOT(random_on_play()));
    }

    random();
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
    if (!_sound_active || !ma_sound_at_end(&_sound))
        return;

    const bool was_playing = QQmlProperty(_root, "isPlaying").read().toBool();
    stop_music();
    if (was_playing)
        QMetaObject::invokeMethod(_root, "at_music_end");
}

void Angelina::play_music(const QString& file) {
    stop_music();

    const QString path = file.startsWith("file:", Qt::CaseInsensitive)
                             ? QUrl(file).toLocalFile()
                             : file;

    QFile resource(path);
    if (!resource.open(QIODevice::ReadOnly)) {
        QMetaObject::invokeMethod(_root, "show_message", Qt::AutoConnection, Q_ARG(QString, "Error"), Q_ARG(QString, "无法播放"));
        return;
    }
    _sound_data = resource.readAll();
    resource.close();

    const ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    if (ma_decoder_init_memory(_sound_data.constData(), _sound_data.size(), &config, &_decoder) != MA_SUCCESS) {
        _sound_data.clear();
        QMetaObject::invokeMethod(_root, "show_message", Qt::AutoConnection, Q_ARG(QString, "Error"), Q_ARG(QString, "无法播放"));
        return;
    }
    if (ma_sound_init_from_data_source(&_engine, &_decoder, 0, nullptr, &_sound) != MA_SUCCESS) {
        ma_decoder_uninit(&_decoder);
        _sound_data.clear();
        QMetaObject::invokeMethod(_root, "show_message", Qt::AutoConnection, Q_ARG(QString, "Error"), Q_ARG(QString, "无法播放"));
        return;
    }

    _sound_active = true;
    ma_sound_set_end_callback(&_sound, at_music_end, this);
    ma_sound_start(&_sound);

    QQmlProperty(_root, "isPlaying").write(true);
}

void Angelina::stop_music() {
    if (!_sound_active)
        return;

    ma_sound_stop(&_sound);
    ma_sound_uninit(&_sound);
    ma_decoder_uninit(&_decoder);
    _sound_data.clear();
    _sound_active = false;
    QQmlProperty(_root, "isPlaying").write(false);
}

void Angelina::random() {
    char buffer[64];
    std::sprintf(buffer, "art/images/%d.gif", get_random_int(1, 8));
    QQmlProperty(_gif, "source").write(QString(buffer));

    static bool first = true;
    if (first) {
        first = false;
        std::sprintf(buffer, ":/qt/qml/Angelina/art/voices/greet%d.mp3", get_random_int(1, 3));
        play_music(QString(buffer));
    } else if (!QQmlProperty(_root, "isPlaying").read().toBool()) {
        std::sprintf(buffer, ":/qt/qml/Angelina/art/voices/%d.mp3", get_random_int(1, 6));
        play_music(QString(buffer));
    }
}

void Angelina::random_on_play() {
    QQmlProperty(_gif, "source").write(get_random_int(0, 1) ? "art/images/购物.gif" : "art/images/看书.gif");
}