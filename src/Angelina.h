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

#ifndef ANGELINA_ANGELINA_H
#define ANGELINA_ANGELINA_H
#include <QByteArray>
#include <QSettings>
extern "C" {
#include <miniaudio.h>
}

class Angelina : public QObject {
    Q_OBJECT
public:
    Angelina(QObject* root);
    ~Angelina();

public slots:
    void play_music(const QString& file);
    void stop_music();
    void random();
    void random_on_play();
    void handle_music_end();

private:
    QObject* _root;
    QObject* _gif;
    QSettings _settings;

    ma_engine _engine;
    ma_sound _sound;
    ma_decoder _decoder;
    QByteArray _sound_data;
    bool _sound_active = false;

    friend void at_music_end(void* pUserData, ma_sound* sound);
};

#endif //ANGELINA_ANGELINA_H
