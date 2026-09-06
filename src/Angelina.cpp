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
#include <SDL3/SDL.h>
#include <cfgpath.h>
#include <filesystem>
#include <format>
#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
#endif

Angelina::Angelina(): _ui(AppWindow::create()) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

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

    {
        _ui->on_save_config([&] {save_config();});
    }
}

Angelina::~Angelina() {
    SDL_Quit();

    sqlite3_close(_db);
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

void Angelina::save_config() {
    // const slint::PhysicalSize size = _ui->window().size();
    const float size = _ui->get_size();
    const slint::PhysicalPosition pos = _ui->window().position();

    sqlite3_exec(_db, "DELETE FROM window;", nullptr, nullptr, nullptr);
    sqlite3_stmt *stmt = nullptr;
    sqlite3_prepare_v2(_db, "INSERT INTO window VALUES (?, ?, ?);", -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, static_cast<int>(size));
    sqlite3_bind_int(stmt, 2, pos.x);
    sqlite3_bind_int(stmt, 3, pos.y);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void Angelina::load_config() {
    char path[MAX_PATH];
    get_user_config_folder(path, MAX_PATH, "angelina");
    std::filesystem::create_directory(path);

    if (sqlite3_open(std::format("{}/config.db", path).c_str(), &_db) == SQLITE_OK) {
        char *err = nullptr;
        sqlite3_exec(_db, "CREATE TABLE IF NOT EXISTS window(size INT, pos_x INT, pos_y INT);", nullptr, nullptr, &err);
        if (err)
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", err, nullptr);

        // 查表
        sqlite3_stmt *stmt = nullptr;
        sqlite3_prepare_v2(_db, "SELECT * FROM window;", -1, &stmt, nullptr);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const int size = sqlite3_column_int(stmt, 0);
            const int pos_x = sqlite3_column_int(stmt, 1);
            const int pos_y = sqlite3_column_int(stmt, 2);

            // _ui->window().set_size(slint::PhysicalSize(slint::Size<uint32_t>(size, size)));
            _ui->set_size(static_cast<float>(size));
            _ui->window().set_position(slint::PhysicalPosition(slint::Point<int>(pos_x, pos_y)));
        }
        sqlite3_finalize(stmt);
    } else
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "无法打开SQLite3数据库", sqlite3_errmsg(_db), nullptr);
}