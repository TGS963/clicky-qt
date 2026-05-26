#import <AppKit/AppKit.h>

void configureMacOverlayWindow(unsigned long long winId) {
    NSView *nsView = (__bridge NSView *)reinterpret_cast<void *>(winId);
    NSWindow *nsWindow = [nsView window];
    if (!nsWindow) return;

    // https://github.com/farzaa/clicky — match original Swift implementation
    [nsWindow setLevel:NSScreenSaverWindowLevel];
    [nsWindow setHidesOnDeactivate:NO];
    [nsWindow setCollectionBehavior:
        NSWindowCollectionBehaviorCanJoinAllSpaces |
        NSWindowCollectionBehaviorStationary |
        NSWindowCollectionBehaviorFullScreenAuxiliary];
    [nsWindow orderFrontRegardless];
}

void restoreMacWindowLevel(unsigned long long winId) {
    NSView *nsView = (__bridge NSView *)reinterpret_cast<void *>(winId);
    NSWindow *nsWindow = [nsView window];
    if (!nsWindow) return;
    [nsWindow setLevel:NSScreenSaverWindowLevel];
}
