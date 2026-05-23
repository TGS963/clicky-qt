# Clicky (Qt6)

Push-to-talk Claude companion — Linux native port. Qt6/QML, C++17.

UI/UX prototype. AI integration TBD.

## Build

```bash
mkdir -p build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build . -j
./clicky
```

## Hotkeys

- `Ctrl+Alt+Space` — toggle listening (visual only for now)
- `Ctrl+Alt+D` — toggle dock (top-right) vs follow-cursor

## Status

Overlay + cursor follow + state animations + tray + hotkeys. No AI plumbing yet.
