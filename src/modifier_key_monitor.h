#pragma once

#include <QObject>
#include <QTimer>

class CompanionState;

// Polls the X11 keymap at ~60 Hz to detect when Right Ctrl is held vs
// released. On press it asks CompanionState to enter focus mode; on release
// it asks it to exit. The distinction between left and right Ctrl is
// determined via X11's keycode tables — Qt's queryKeyboardModifiers()
// collapses them into a single Ctrl bit, which is insufficient here.
//
// X11-only. Wayland would need a different mechanism (e.g. a portal-based
// global shortcut or compositor-specific protocol); not implemented in this
// phase.
class ModifierKeyMonitor : public QObject {
    Q_OBJECT
public:
    explicit ModifierKeyMonitor(CompanionState* companionState, QObject* parent = nullptr);
    ~ModifierKeyMonitor() override;

    void start();
    void stop();

private slots:
    void pollKeymap();

private:
    CompanionState* companionStateValue = nullptr;
    QTimer pollingTimer;

    void* x11DisplayHandle = nullptr;   // opaque ::Display*
    unsigned char rightControlKeycode = 0;
    bool rightControlWasPressed = false;
};
