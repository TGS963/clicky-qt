#pragma once

#include <QQuickView>
#include <QTimer>

class CompanionState;

// Full-screen transparent QQuickView that hosts the QML overlay (cursor-
// following primary dot + orbiting satellite stars + focus-mode bubble and
// task list). Configured for:
//   - alpha-blended surface (desktop visible behind transparent regions)
//   - click-through in Passive mode (Qt::WindowTransparentForInput); becomes
//     interactive while the user holds Right Ctrl (focus mode)
//   - always-on-top with periodic re-raise for X11 WMs that re-stack on
//     focus changes
class OverlayWindow : public QQuickView {
    Q_OBJECT
public:
    explicit OverlayWindow(CompanionState* companionState, QQuickView* parent = nullptr);

    void showFullScreenOnPrimaryDisplay();

private slots:
    void applyInteractionModeFlags();

private:
    CompanionState* companionStateValue = nullptr;
    QTimer keepAboveTimer;

    void raiseAboveOtherWindows();
};
