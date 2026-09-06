#include "Angelina.h"

#if defined(__linux__) && !defined(__ANDROID__)
    #include <X11/Xlib.h>
    #include <cstdlib>
#endif

#ifdef __ANDROID__
extern "C" void slint_main()
#else
int main(int argc, char **argv)
#endif
{
#if defined(__linux__) && !defined(__ANDROID__)
    // 无边框窗口的拖动依赖 X11 的全局指针查询(XQueryPointer)与 set_position,
    // 而 Wayland 安全模型禁止客户端轮询全局指针或自主定位窗口,
    // 因此 get_global_mouse_position() 在纯 Wayland 下无法工作。
    // 若存在可用的 X(X11 桌面或 Wayland 会话的 XWayland),则屏蔽 WAYLAND_DISPLAY,
    // 使 winit(Slint 后端)改用 X11;否则保持原生 Wayland 运行。
    if (Display* display = XOpenDisplay(nullptr)) {
        XCloseDisplay(display);
        unsetenv("WAYLAND_DISPLAY");
        unsetenv("WAYLAND_SOCKET");
    }
#endif

    Angelina angelina;

    angelina.run();
}