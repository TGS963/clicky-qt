#pragma once

#include <QQuickView>
#include <QTimer>

class CompanionState;

// Full-screen transparent QQuickView that hosts the QML overlay (cursor-
// following primary dot + orbiting satellite stars). Configured for:
//   - alpha-blended surface (desktop visible behind transparent regions)
//   - click-through (Qt::WindowTransparentForInput)
//   - always-on-top with periodic re-raise for X11 WMs that re-stack on
//     focus changes
class OverlayWindow : public QQuickView {
    Q_OBJECT
public:
    explicit OverlayWindow(CompanionState* companionState, QQuickView* parent = nullptr);

    void showFullScreenOnPrimaryDisplay();

private:
    CompanionState* companionStateValue = nullptr;
    QTimer keepAboveTimer;

    void applyClickThroughAndAlwaysAboveAttributes();
    void raiseAboveOtherWindows();
};
