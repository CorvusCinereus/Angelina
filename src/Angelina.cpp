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
#include <random>
#include <ctime>
#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__)
extern "C" {
    #include <X11/Xlib.h>
}
#endif

Angelina::Angelina()
    : _ui(AppWindow::create())
    , _musics(std::make_shared<slint::VectorModel<std::tuple<int, slint::SharedString, slint::SharedString>>>())
    , _sound_active(false)
    , _loop(false)
    , _exe_path(get_exe_path())
    , _is_music(false)
{
    get_user_config_folder(_config_file_path, MAX_PATH, "angelina");
    std::filesystem::create_directory(_config_file_path);

    ma_engine_init(nullptr, &_engine);

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

    { // 音乐播放相关回调
        _ui->set_musics(_musics);
        _ui->on_add_dir([&] {
            std::thread([&] {
                const std::filesystem::path path(pfd::select_folder("选择目录", pfd::path::home()).result());
                if (path.empty()) return;

                slint::invoke_from_event_loop([&, path] {
                    for (const auto& entry: std::filesystem::directory_iterator(path)) {
                        if (const std::string& file_name = entry.path().filename(); file_name.find("flac") != std::string::npos || file_name.find("ogg") != std::string::npos || file_name.find("wav") != std::string::npos || file_name.find("mp3") != std::string::npos) {
                            _musics->push_back({
                                _musics->row_count(),
                                slint::SharedString(entry.path().filename().u8string()),
                                slint::SharedString(entry.path().u8string()),
                            });
                        }
                    }
                });
            }).detach();
        });

        _ui->on_add_file([&] {
            std::thread([&] {
                const std::vector<std::string> files = pfd::open_file("选择文件", pfd::path::home(), {"Music", "*.flac *.mp3 *.wav *.ogg"}, pfd::opt::multiselect).result();
                slint::invoke_from_event_loop([&, files] {
                    for (const auto& file: files) {
                        auto path = std::filesystem::path(file);
                        _musics->push_back({
                            _musics->row_count(),
                            slint::SharedString(path.filename().u8string()),
                            slint::SharedString(path.u8string())
                        });
                    }
                });
            }).detach();
        });

        _ui->on_play_music([&](const int index, const slint::SharedString& path) {
            _current_index = index;
            _is_music = true;
            play_music(path.data());
        });

        _ui->on_stop_music([&] {
            stop_music();
            _is_music = false;
        });

        _ui->on_double_click([&] {
            _is_music = false;
            std::string voice = _exe_path + "/res/voices";
            switch (get_random_int(0, 1)) {
                case 0:
                    voice += "/click.mp3";
                    break;
                case 1:
                    voice += "/outdoor.mp3";
                    break;
                default:
                    break;
            }
            play_music(voice);
        });

        _ui->on_toggle_loop([&] {_loop = !_loop;});

        _ui->on_random_gif([&] (const auto& gifs) {
            int result = get_random_int(0, gifs->row_count() - 1);
            _ui->invoke_change_gif(gifs->row_data(result).value());
        });
    }
}

Angelina::~Angelina() {
    stop_music();
    ma_engine_uninit(&_engine);
}

void Angelina::run() {
    load_config();

    const std::time_t now = std::time(nullptr);
    const std::tm* tm = std::localtime(&now);
    if (const int hour = tm->tm_hour; hour > 6 && hour < 9) {
        _ui->invoke_change_gif(Fly);
        play_music(std::format("{}/res/voices/greet.mp3", _exe_path));
    } else {
        _ui->invoke_change_gif(get_random_int(0, 3));
        play_music(std::format("{}/res/voices/hirarido.mp3", _exe_path));
    }

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
    const bool ok = XQueryPointer(display, DefaultRootWindow(display), &root, &child,
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

void on_sound_end(void* pUserData, ma_sound *) {
    auto angelina = static_cast<Angelina*>(pUserData);

    if (angelina->_is_music) {
        slint::invoke_from_event_loop([angelina] {
            angelina->play_next();
        });
    } else {
        slint::invoke_from_event_loop([angelina] {
            angelina->_ui->set_playing(false);
        });
    }
}

void Angelina::play_next() {
    if (_loop) {
        play_music(_current_index);
        return;
    }

    ++_current_index;
    if (_current_index == _musics->row_count()) {
        _current_index = 0;
    }
    _ui->set_playing_index(_current_index);
    play_music(_current_index);
}

void Angelina::play_music(const std::string& music_name) {
    stop_music();

    if (ma_sound_init_from_file(&_engine, music_name.c_str(), 0, nullptr, nullptr, &_sound) != MA_SUCCESS) {
        auto m = pfd::message("Error", std::format("无法播放{}", music_name), pfd::choice::ok, pfd::icon::error);
        _ui->set_playing(false);
        return;
    }
    _sound_active = true;
    ma_sound_set_end_callback(&_sound, on_sound_end, this);
    ma_sound_start(&_sound);
}

void Angelina::play_music(const int index) {
    play_music(std::get<2>(_musics->row_data(index).value()).data());
}

void Angelina::stop_music() {
    if (!_sound_active) {
        return;
    }

    ma_sound_stop(&_sound);
    ma_sound_uninit(&_sound);
    _sound_active = false;
}

int Angelina::get_random_int(const int min, const int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

std::string Angelina::get_exe_path() {
#ifdef _WIN32
    std::string path(MAX_PATH, '\0');
    DWORD len = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (len == 0) return {};
    // 若缓冲区不足,GetModuleFileNameA 返回 nSize(即结果被截断),需扩大缓冲区重试。
    while (len >= path.size()) {
        path.resize(path.size() * 2);
        len = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (len == 0) return {};
    }
    path.resize(len);
    return std::filesystem::path(path).parent_path().string();
#elif defined(__linux__)
    std::error_code ec;
    const auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
    return ec ? std::string{} : exe.parent_path().string();
#else
#error "Unsupported platform"
#endif
}