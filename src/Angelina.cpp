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
#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
#endif

Angelina::Angelina(): _ui(AppWindow::create()) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

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

Angelina::~Angelina() {
    SDL_Quit();
}

void Angelina::run() {
    _ui->run();
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
