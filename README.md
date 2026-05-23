# Clicky (Qt6)

Linux-native push-to-talk Claude companion. Currently a UI/UX prototype:
overlay + cursor follow + state choreography. AI plumbing (audio capture,
streaming transcription, Claude vision, ElevenLabs TTS) is not wired up yet.

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
- [QHotkey](https://github.com/Skycoder42/QHotkey) — vendored under
  `third_party/QHotkey/`. On a fresh clone:

  ```bash
  git clone --depth 1 https://github.com/Skycoder42/QHotkey third_party/QHotkey
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

## Tunables

| What                            | Where                                          |
| ------------------------------- | ---------------------------------------------- |
| Cursor poll rate                | `src/cursor_position_tracker.cpp`              |
| Task lifetime, flash hold, HSV  | `src/companion_state.cpp` (top-of-file consts) |
| Always-on-top re-stack interval | `src/overlay_window.cpp`                       |
| Tray icon size / colors         | `src/tray_icon_manager.cpp`                    |
| Hotkey strings                  | `src/global_hotkey_manager.cpp`                |
| Orbit radius / rotation period  | `qml/OverlayContent.qml` (header tunables)     |
| Glow / blur / pulse / ring      | `qml/AgentDot.qml` (header tunables + inline)  |

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
