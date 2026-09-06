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
#include <filesystem>
#include <format>
#include <fstream>
#include <portable-file-dialogs.h>
#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
#endif

Angelina::Angelina(): _ui(AppWindow::create()) {
    get_user_config_folder(_config_file_path, MAX_PATH, "angelina");
    std::filesystem::create_directory(_config_file_path);

    { // 实现鼠标拖拽
        _ui->on_drag([&] {
            get_global_mouse_position();
            _ui->window().set_position(slint::PhysicalPosition(slint::Point<int32_t>{ _mouse_x - _drag_offset_x, _mouse_y - _drag_offset_y }));
        });

        _ui->on_start_drag([&] {
            get_global_mouse_position();

            const slint::PhysicalPosition pos = _ui->window().position();
            _drag_offset_x = _mouse_x - pos.x;
            _drag_offset_y = _mouse_y - pos.y;
        });
    }
}

void Angelina::run() {
    load_config();

    _ui->run();

    save_config();
}

bool Angelina::get_global_mouse_position() {
#ifdef _WIN32
    POINT pt;
    if (GetCursorPos(&pt)) {
        _mouse_x = pt.x;
        _mouse_y = pt.y;
        return true;
    }
    return false;
#elif defined(__linux__)
    Display* display = XOpenDisplay(nullptr);
    if (!display) return false;
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    bool ok = XQueryPointer(display, DefaultRootWindow(display), &root, &child,
                            &root_x, &root_y, &win_x, &win_y, &mask);
    if (ok) { _mouse_x = root_x; _mouse_y = root_y; }
    XCloseDisplay(display);
    return ok;
#else
#error "Unsupported platform"
#endif
}

void Angelina::load_config() {
    WindowState state = {};

    if (std::ifstream file(std::format("{}/window_state.bin", _config_file_path), std::ios::binary); file.is_open()) {
        file.read(reinterpret_cast<char*>(&state), sizeof(state));

        _ui->set_size(state.size);
        _ui->window().set_position(slint::PhysicalPosition(slint::Point<int32_t>(state.pos_x, state.pos_y)));
    }
}

void Angelina::save_config() {
    WindowState state = {};
    const slint::PhysicalPosition pos = _ui->window().position();
    state.pos_x = pos.x;
    state.pos_y = pos.y;
    state.size = _ui->get_size();

    if (std::ofstream file(std::format("{}/window_state.bin", _config_file_path), std::ios::binary); file.is_open())
        file.write(reinterpret_cast<char*>(&state), sizeof(state));
    else
        auto m = pfd::message("Error", "Could not save window_state.bin", pfd::choice::ok, pfd::icon::error);
}