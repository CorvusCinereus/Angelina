//
// Created by corvuscinereus on 2026/9/6.
//

#ifndef ANGELINA_ANGELINA_H
#define ANGELINA_ANGELINA_H
#include "app-window.h"
#include <SDL3/SDL.h>

class Angelina {
public:
    explicit Angelina();
    ~Angelina();

    void run();

private:
    slint::ComponentHandle<AppWindow> _ui;

    int _mouse_x, _mouse_y;
    int _drag_offset_x, _drag_offset_y;

    bool get_global_mouse_position();
};


#endif //ANGELINA_ANGELINA_H
