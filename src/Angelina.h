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
#include "app-window.h"

extern "C" {
#include <cfgpath.h>
}

class Angelina {
public:
    explicit Angelina();
    ~Angelina();

    void run();

protected:
    struct WindowState {
        int pos_x;
        int pos_y;
        float size;
    };

private:
    slint::ComponentHandle<AppWindow> _ui;
    char _config_file_path[MAX_PATH];

    int _mouse_x, _mouse_y;
    int _drag_offset_x, _drag_offset_y;

    bool get_global_mouse_position();
    void load_config();
    void save_config();
};


#endif //ANGELINA_ANGELINA_H
