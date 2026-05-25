#import <AppKit/AppKit.h>

// kVK_RightOption from HIToolbox — 0x3D. Avoid importing Carbon.h (deprecated).
static const unsigned short kRightOptionKeyCode = 0x3D;

void* startMacModifierMonitor(void (*cb)(bool pressed, void* ctx), void* ctx) {
    id monitor = [NSEvent addGlobalMonitorForEventsMatchingMask:NSEventMaskFlagsChanged
                                                        handler:^(NSEvent* event) {
        if (event.keyCode != kRightOptionKeyCode) return;
        // modifierFlags reflects state AFTER the change: option bit set = key down.
        bool pressed = (event.modifierFlags & NSEventModifierFlagOption) != 0;
        cb(pressed, ctx);
    }];
    return (__bridge_retained void*)monitor;
}

void stopMacModifierMonitor(void* handle) {
    if (!handle) return;
    id monitor = (__bridge_transfer id)handle;
    [NSEvent removeMonitor:monitor];
}
