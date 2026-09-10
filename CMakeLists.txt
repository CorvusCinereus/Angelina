cmake_minimum_required(VERSION 3.16)

project(Angelina VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(third_party/miniaudio)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appAngelina
        src/main.cpp
        src/Angelina.cpp
        src/Angelina.h
        third_party/miniaudio/miniaudio.c
)

file(GLOB GIF_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "art/images/*.gif")
file(GLOB VOICES_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "art/voices/*.mp3")

qt_add_qml_module(appAngelina
    URI Angelina
    QML_FILES
        Main.qml
        RESOURCES ${GIF_FILES}
        RESOURCES ${VOICES_FILES}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appAngelina PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appAngelina
    OUTPUT_NAME angelina
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appAngelina
    PRIVATE Qt6::Quick
)

install(TARGETS appAngelina
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
