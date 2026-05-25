#pragma once

#include <QQuickView>
#include <QTimer>

class CompanionState;
class QScreen;

// Full-screen transparent QQuickView covering one physical screen.
// One instance is created per QScreen in main.cpp.
//
// Configured for:
//   - alpha-blended surface (desktop visible behind transparent regions)
//   - click-through in Passive mode (Qt::WindowTransparentForInput); becomes
//     interactive while the user holds the trigger key (focus mode)
//   - always-on-top with periodic re-raise for X11 WMs that re-stack on
//     focus changes (skipped on macOS/Wayland via platformName check)
class OverlayWindow : public QQuickView {
    Q_OBJECT
public:
    explicit OverlayWindow(CompanionState* companionState, QScreen* screen,
                           QQuickView* parent = nullptr);

    void showOnScreen();
    QScreen* overlayScreen() const { return screenValue; }

private slots:
    void applyInteractionModeFlags();

private:
    CompanionState* companionStateValue = nullptr;
    QScreen* screenValue = nullptr;
    QTimer keepAboveTimer;

    void raiseAboveOtherWindows();
};
