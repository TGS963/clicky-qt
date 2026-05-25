#include "modifier_key_monitor.h"

#include "companion_state.h"

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif

#ifdef Q_OS_MACOS
#include "mac_modifier_monitor.h"

static void macTriggerCb(bool pressed, void* ctx) {
    static_cast<ModifierKeyMonitor*>(ctx)->handleTriggerKey(pressed);
}
#endif

#ifdef Q_OS_WIN
#include <windows.h>
namespace {
constexpr int MODIFIER_POLL_INTERVAL_MS = 16;
}
#endif

#ifdef Q_OS_LINUX
namespace {
constexpr int MODIFIER_POLL_INTERVAL_MS = 16;
}
#endif

ModifierKeyMonitor::ModifierKeyMonitor(CompanionState* companionState, QObject* parent)
    : QObject(parent), companionStateValue(companionState) {

#ifdef Q_OS_LINUX
    Display* display = XOpenDisplay(nullptr);
    if (display) {
        x11DisplayHandle = display;
        rightControlKeycode = static_cast<unsigned char>(
            XKeysymToKeycode(display, XK_Control_R));
    }
    pollingTimer.setInterval(MODIFIER_POLL_INTERVAL_MS);
    connect(&pollingTimer, &QTimer::timeout, this, &ModifierKeyMonitor::pollKeymap);
#elif defined(Q_OS_WIN)
    pollingTimer.setInterval(MODIFIER_POLL_INTERVAL_MS);
    connect(&pollingTimer, &QTimer::timeout, this, &ModifierKeyMonitor::pollKeymap);
#endif
}

ModifierKeyMonitor::~ModifierKeyMonitor() {
    stop();
#ifdef Q_OS_LINUX
    if (x11DisplayHandle) {
        XCloseDisplay(static_cast<Display*>(x11DisplayHandle));
        x11DisplayHandle = nullptr;
    }
#endif
}

void ModifierKeyMonitor::start() {
#ifdef Q_OS_LINUX
    if (x11DisplayHandle && rightControlKeycode != 0) {
        pollingTimer.start();
    }
#elif defined(Q_OS_MACOS)
    if (!macMonitorHandle) {
        macMonitorHandle = startMacModifierMonitor(macTriggerCb, this);
    }
#elif defined(Q_OS_WIN)
    pollingTimer.start();
#endif
}

void ModifierKeyMonitor::stop() {
#ifdef Q_OS_MACOS
    stopMacModifierMonitor(macMonitorHandle);
    macMonitorHandle = nullptr;
#elif defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    pollingTimer.stop();
#endif
}

void ModifierKeyMonitor::handleTriggerKey(bool pressed) {
    if (pressed == triggerKeyWasPressed) return;
    triggerKeyWasPressed = pressed;
    if (pressed) {
        companionStateValue->openTaskMenu();
    } else {
        companionStateValue->closeTaskMenu();
    }
}

void ModifierKeyMonitor::pollKeymap() {
#ifdef Q_OS_LINUX
    Display* display = static_cast<Display*>(x11DisplayHandle);
    char keymapBitset[32] = {0};
    XQueryKeymap(display, keymapBitset);
    const unsigned char keycode = rightControlKeycode;
    const bool pressed = (keymapBitset[keycode / 8] & (1 << (keycode % 8))) != 0;
#elif defined(Q_OS_WIN)
    const bool pressed = (GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0;
#else
    const bool pressed = false;
#endif
    handleTriggerKey(pressed);
}
