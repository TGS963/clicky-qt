#pragma once

#ifdef __APPLE__
// Configures an NSWindow for a fullscreen transparent overlay:
//   - NSScreenSaverWindowLevel  (stays above all apps)
//   - hidesOnDeactivate = NO    (stays visible when our app loses focus)
//   - collectionBehavior: canJoinAllSpaces + stationary + fullScreenAuxiliary
//   - orderFrontRegardless      (show without activating our app)
void configureMacOverlayWindow(unsigned long long winId);

// Restore NSScreenSaverWindowLevel after Qt's setWindowFlags() resets it.
void restoreMacWindowLevel(unsigned long long winId);
#endif
