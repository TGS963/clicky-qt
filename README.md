# Clicky (Qt6)

Linux-native push-to-talk voice companion, built around local models —
local LLM, local speech-to-text, local text-to-speech. No cloud, no API
keys. Currently a UI/UX prototype: overlay + cursor follow + state
choreography. The AI plumbing (audio capture, local STT, local LLM
inference, local TTS) is not wired up yet.

## Architecture

The application is a single Qt6 process:

- `OverlayWindow` — full-screen transparent, frameless, click-through
  `QQuickView` that hosts the QML scene above all other windows.
- `CompanionState` — central observable model; exposes voice state, overlay
  mode (follow / dock), cursor position, the list of active satellite
  tasks, and a transient flash color used by the listening choreography.
- `CursorPositionTracker` — polls `QCursor::pos()` at ~60 Hz and pushes it
  into `CompanionState`. Drives the QML cursor-following binding.
- `GlobalHotkeyManager` — registers the system-wide shortcuts via
  [QHotkey](https://github.com/Skycoder42/QHotkey) and forwards activations
  into `CompanionState`.
- `TrayIconManager` — system tray icon + context menu (toggle dock,
  toggle listening, quit). Silently no-ops on WMs without a tray host.
- QML side: `AgentDot.qml` is the visual primitive used for both the
  primary cursor companion and the orbiting satellite stars;
  `OverlayContent.qml` composes them.

## Dependencies

- Qt6 (Core, Gui, Qml, Quick, Widgets, QtQuick.Effects)
- A C++17 compiler
- CMake ≥ 3.21
- [QHotkey](https://github.com/Skycoder42/QHotkey) — a git submodule under
  `third_party/QHotkey/`. Clone recursively:

  ```bash
  git clone --recurse-submodules https://github.com/TGS963/clicky-qt
  ```

  Already cloned without it? Fetch the submodule:

  ```bash
  git submodule update --init
  ```

## Build

```bash
mkdir -p build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build . -j
./clicky
```

## Hotkeys

| Shortcut          | Action                                                |
| ----------------- | ----------------------------------------------------- |
| `Ctrl+Alt+Space`  | Toggle listening. First press picks a random task     |
|                   | color and the primary dot fades into it. Second press |
|                   | spawns a satellite star in that color and the primary |
|                   | dot fades back to blue.                               |
| `Ctrl+Alt+D`      | Dock the primary dot to the top-right corner / back   |
|                   | to following the cursor.                              |

## Focus mode

Hold **Right Ctrl** and the blue companion smoothly morphs into a task
menu card anchored at the cursor. Each row shows color + title +
description + lifetime progress; hovering a row reveals an `✕` that
immediately force-closes that task. Satellites fade out while the menu is
open — rows are the canonical view in this mode.

- **Release Right Ctrl** → card morphs back into the blue companion.
- **Click anywhere outside the card** → menu closes (overlay stays
  click-through afterwards).

Right Ctrl detection uses X11's `XQueryKeymap` so the left and right Ctrl
keys can be distinguished; Wayland is not supported for this gesture yet.

## Tunables

| What                            | Where                                          |
| ------------------------------- | ---------------------------------------------- |
| Cursor poll rate                | `src/cursor_position_tracker.cpp`              |
| Modifier poll rate              | `src/modifier_key_monitor.cpp`                 |
| Task lifetime, flash hold, HSV  | `src/companion_state.cpp` (top-of-file consts) |
| Always-on-top re-stack interval | `src/overlay_window.cpp`                       |
| Tray icon size / colors         | `src/tray_icon_manager.cpp`                    |
| Hotkey strings                  | `src/global_hotkey_manager.cpp`                |
| Orbit radius / rotation period  | `qml/OverlayContent.qml` (header tunables)     |
| Focus bubble / list layout      | `qml/OverlayContent.qml` (focus tunables)      |
| Glow / blur / pulse / ring      | `qml/AgentDot.qml` (header tunables + inline)  |
| Task-list panel styling         | `qml/TaskListPanel.qml`                        |

## Known limitations

- Tray icon requires a StatusNotifier / AppIndicator host (KDE, GNOME with
  the extension, XFCE, etc.). Plain dwm has no tray host; everything else
  still works.
- The transparent overlay relies on a compositor to alpha-blend. dwm without
  a compositor will show the overlay as a black background.
- Minimal X11 window managers re-stack floating windows above
  override-redirect overlays on focus changes;
  `OverlayWindow::raiseAboveOtherWindows()` ticks every
  `WINDOW_RESTACK_INTERVAL_MS` to compensate.
