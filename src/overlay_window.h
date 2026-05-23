#pragma once

#include <QQuickView>

class CompanionState;

class OverlayWindow : public QQuickView {
    Q_OBJECT
public:
    explicit OverlayWindow(CompanionState* companionState, QQuickView* parent = nullptr);

    void showFullScreenOnPrimaryDisplay();

private:
    CompanionState* companionStateValue = nullptr;

    void applyClickThroughAndAlwaysAboveAttributes();
};
