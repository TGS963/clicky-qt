#pragma once

#include <QString>

// Captures a screenshot of whichever screen currently holds the cursor,
// draws a ring marking the area around the cursor, and writes the result to
// a PNG in the system temp directory.
//
// Self-contained: it reads the live cursor position via QCursor::pos() and
// knows nothing about the overlay windows, the task list, or CompanionState.
// Hiding the application's own overlays before a grab is the caller's job
// (see ScreenshotCaptureController).
class CursorMarkedScreenshotCapturer {
public:
    // Returns the absolute path of the saved PNG, or an empty string if the
    // screen grab produced a null pixmap (e.g. some Wayland configs) or the
    // file could not be written. The caller should treat an empty return as
    // "capture failed" and not spawn a task.
    QString captureScreenWithCursorMarker();
};
