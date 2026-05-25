#pragma once

#include <QObject>
#include <QTimer>

class CompanionState;

// Detects trigger key held vs released and drives the task menu open/close.
// Linux/X11: polls XQueryKeymap for Right Ctrl.
// macOS:     NSEvent global monitor for Right ⌥ (no polling, event-driven).
// Windows:   polls GetAsyncKeyState for VK_RCONTROL.
class ModifierKeyMonitor : public QObject {
    Q_OBJECT
public:
    explicit ModifierKeyMonitor(CompanionState* companionState, QObject* parent = nullptr);
    ~ModifierKeyMonitor() override;

    void start();
    void stop();

    // Called from the macOS NSEvent callback (main thread).
    void handleTriggerKey(bool pressed);

private slots:
    void pollKeymap();

private:
    CompanionState* companionStateValue = nullptr;
    bool triggerKeyWasPressed = false;

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    QTimer pollingTimer;
#endif

#ifdef Q_OS_LINUX
    void* x11DisplayHandle = nullptr;
    unsigned char rightControlKeycode = 0;
#endif

#ifdef Q_OS_MACOS
    void* macMonitorHandle = nullptr;
#endif
};
