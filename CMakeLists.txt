cmake_minimum_required(VERSION 3.21)
project(clicky LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    Widgets
)

# QHotkey lives in-tree under third_party/QHotkey. See README "Dependencies"
# for how to refresh it on a fresh clone.
add_subdirectory(third_party/QHotkey)

set(CLICKY_SOURCES
    src/main.cpp
    src/companion_state.cpp
    src/companion_state.h
    src/cursor_position_tracker.cpp
    src/cursor_position_tracker.h
    src/global_hotkey_manager.cpp
    src/global_hotkey_manager.h
    src/overlay_window.cpp
    src/overlay_window.h
    src/task_item.cpp
    src/task_item.h
    src/tray_icon_manager.cpp
    src/tray_icon_manager.h
)

qt_add_executable(clicky ${CLICKY_SOURCES})

qt_add_qml_module(clicky
    URI Clicky
    VERSION 1.0
    QML_FILES
        qml/AgentDot.qml
        qml/OverlayContent.qml
    DEPENDENCIES
        QtQuick
        QtQuick.Effects
)

target_link_libraries(clicky PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Widgets
    QHotkey::QHotkey
)

set_target_properties(clicky PROPERTIES
    WIN32_EXECUTABLE OFF
    MACOSX_BUNDLE OFF
)
