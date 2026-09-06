cmake_minimum_required(VERSION 3.21)
project(Angelina LANGUAGES C CXX)

add_subdirectory(third_party/slint/api/cpp)

include_directories(third_party)

if(ANDROID)
    add_library(Angelina SHARED src/main.cpp
            src/Angelina.cpp
            src/Angelina.h
            third_party/miniaudio/miniaudio.c)
else()
    add_executable(Angelina src/main.cpp
            src/Angelina.cpp
            src/Angelina.h
            third_party/miniaudio/miniaudio.c)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(Angelina PRIVATE Slint::Slint X11)
else()
    target_link_libraries(Angelina PRIVATE Slint::Slint)
endif ()

slint_target_sources(Angelina ui/app-window.slint)
# On Windows, copy the Slint DLL next to the application binary so that it's found.
if (WIN32)
    add_custom_command(TARGET Angelina POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Angelina> $<TARGET_FILE_DIR:Angelina> COMMAND_EXPAND_LISTS)
endif()
