//
// Created by corvuscinereus on 2026/9/6.
//

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

        _ui->window().set_position(slint::LogicalPosition(slint::Point(static_cast<float>(_mouse_x - _drag_offset_x), static_cast<float>(_mouse_y - _drag_offset_y))));
        std::cout << _mouse_x << std::endl;
    });

    _ui->on_start_drag([&] {
        get_global_mouse_position();

        const slint::PhysicalPosition pos = _ui->window().position();
        _drag_offset_x = _mouse_x - pos.x;
        _drag_offset_y = _mouse_y - pos.y;

        std::cout << _drag_offset_x << " " << _drag_offset_y << std::endl;
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
