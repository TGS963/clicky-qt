#import <AppKit/AppKit.h>

void configureMacOverlayWindow(unsigned long long winId) {
    NSView *nsView = (__bridge NSView *)reinterpret_cast<void *>(winId);
    NSWindow *nsWindow = [nsView window];
    if (!nsWindow) return;

    // Match the original clicky Swift implementation exactly:
    // https://github.com/farzaa/clicky

    // Stay above all apps (same level as original .screenSaver)
    [nsWindow setLevel:NSScreenSaverWindowLevel];

    // Stay visible when our Qt app loses focus to another app
    [nsWindow setHidesOnDeactivate:NO];

    // Show on every Space; don't move with Space transitions; appear over fullscreen apps
    [nsWindow setCollectionBehavior:
        NSWindowCollectionBehaviorCanJoinAllSpaces |
        NSWindowCollectionBehaviorStationary |
        NSWindowCollectionBehaviorFullScreenAuxiliary];

    // Bring to front without activating/focusing our app
    [nsWindow orderFrontRegardless];
}
