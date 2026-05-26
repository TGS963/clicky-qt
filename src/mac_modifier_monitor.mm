#import <AppKit/AppKit.h>

// kVK_RightOption from HIToolbox — 0x3D. Avoid importing Carbon.h (deprecated).
static const unsigned short kRightOptionKeyCode = 0x3D;

void* startMacModifierMonitor(void (*cb)(bool pressed, void* ctx), void* ctx) {
    void (^handler)(NSEvent*) = ^(NSEvent* event) {
        if (event.keyCode != kRightOptionKeyCode) return;
        bool pressed = (event.modifierFlags & NSEventModifierFlagOption) != 0;
        cb(pressed, ctx);
    };

    id globalMonitor = [NSEvent addGlobalMonitorForEventsMatchingMask:NSEventMaskFlagsChanged
                                                              handler:handler];

    // Covers the case where our window becomes key (loses NSNonactivatingPanelMask);
    // global monitor only fires for events going to other apps.
    // Must return the event — nil would swallow it.
    id localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskFlagsChanged
                                                            handler:^NSEvent*(NSEvent* event) {
        handler(event);
        return event;
    }];

    NSMutableArray* monitors = [NSMutableArray array];
    if (globalMonitor) [monitors addObject:globalMonitor];
    if (localMonitor)  [monitors addObject:localMonitor];
    return (__bridge_retained void*)monitors;
}

void stopMacModifierMonitor(void* handle) {
    if (!handle) return;
    NSMutableArray* monitors = (__bridge_transfer NSMutableArray*)handle;
    for (id m in monitors)
        [NSEvent removeMonitor:m];
}
