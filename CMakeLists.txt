cmake_minimum_required(VERSION 3.21)
project(clicky LANGUAGES CXX)
if(APPLE)
    enable_language(OBJCXX)
endif()

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

# ModifierKeyMonitor distinguishes trigger key per platform (Qt collapses L/R modifiers).
# Linux/X11: XQueryKeymap for Right Ctrl. macOS: NSEvent global monitor for Right ⌥.
# Windows: GetAsyncKeyState for VK_RCONTROL.
if(APPLE)
elseif(UNIX)
    find_package(X11 REQUIRED)
elseif(WIN32)
    # GetAsyncKeyState is in user32 — Qt links it already on Windows.
else()
    message(FATAL_ERROR "clicky only builds on Linux, macOS, or Windows.")
endif()

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
    src/modifier_key_monitor.cpp
    src/modifier_key_monitor.h
    src/overlay_window.cpp
    src/overlay_window.h
    src/task_item.cpp
    src/task_item.h
    src/task_list_model.cpp
    src/task_list_model.h
    src/tray_icon_manager.cpp
    src/tray_icon_manager.h
)

qt_add_executable(clicky ${CLICKY_SOURCES})

if(APPLE)
    set_target_properties(clicky PROPERTIES MACOSX_BUNDLE TRUE)
endif()

qt_add_qml_module(clicky
    URI Clicky
    VERSION 1.0
    QML_FILES
        qml/AgentDot.qml
        qml/MorphingCompanion.qml
        qml/OverlayContent.qml
        qml/TaskListContent.qml
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

if(APPLE)
    target_sources(clicky PRIVATE
        src/mac_window_config.mm
        src/mac_modifier_monitor.mm
    )
    set_source_files_properties(
        src/mac_window_config.mm
        src/mac_modifier_monitor.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc"
    )
    target_link_libraries(clicky PRIVATE "-framework AppKit")
elseif(UNIX)
    target_link_libraries(clicky PRIVATE X11::X11)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(clicky PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor)
endif()
