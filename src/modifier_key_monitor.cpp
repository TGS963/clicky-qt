#include "modifier_key_monitor.h"

#include "companion_state.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace {
constexpr int MODIFIER_POLL_INTERVAL_MS = 16;
}  // namespace

ModifierKeyMonitor::ModifierKeyMonitor(CompanionState* companionState, QObject* parent)
    : QObject(parent), companionStateValue(companionState) {

    Display* display = XOpenDisplay(nullptr);
    if (display) {
        x11DisplayHandle = display;
        rightControlKeycode = static_cast<unsigned char>(
            XKeysymToKeycode(display, XK_Control_R));
    }

    pollingTimer.setInterval(MODIFIER_POLL_INTERVAL_MS);
    connect(&pollingTimer, &QTimer::timeout, this, &ModifierKeyMonitor::pollKeymap);
}

ModifierKeyMonitor::~ModifierKeyMonitor() {
    if (x11DisplayHandle) {
        XCloseDisplay(static_cast<Display*>(x11DisplayHandle));
        x11DisplayHandle = nullptr;
    }
}

void ModifierKeyMonitor::start() {
    if (x11DisplayHandle && rightControlKeycode != 0) {
        pollingTimer.start();
    }
}

void ModifierKeyMonitor::stop() {
    pollingTimer.stop();
}

void ModifierKeyMonitor::pollKeymap() {
    Display* display = static_cast<Display*>(x11DisplayHandle);
    char keymapBitset[32] = {0};
    XQueryKeymap(display, keymapBitset);

    // XQueryKeymap returns a 256-bit bitmap; bit N corresponds to keycode N.
    const unsigned char keycode = rightControlKeycode;
    const bool rightControlIsPressedNow =
        (keymapBitset[keycode / 8] & (1 << (keycode % 8))) != 0;

    if (rightControlIsPressedNow == rightControlWasPressed) {
        return;
    }
    rightControlWasPressed = rightControlIsPressedNow;

    if (rightControlIsPressedNow) {
        companionStateValue->openTaskMenu();
    } else {
        companionStateValue->closeTaskMenu();
    }
}
