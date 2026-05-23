#pragma once

#include <QObject>
#include <QPointF>

class CompanionState : public QObject {
    Q_OBJECT
    Q_PROPERTY(VoiceState voiceState READ voiceState NOTIFY voiceStateChanged)
    Q_PROPERTY(OverlayMode overlayMode READ overlayMode NOTIFY overlayModeChanged)
    Q_PROPERTY(QPointF cursorScreenPosition READ cursorScreenPosition NOTIFY cursorScreenPositionChanged)

public:
    enum VoiceState {
        Idle,
        Listening,
        Processing,
        Responding,
    };
    Q_ENUM(VoiceState)

    enum OverlayMode {
        FollowingCursor,
        Docked,
    };
    Q_ENUM(OverlayMode)

    explicit CompanionState(QObject* parent = nullptr);

    VoiceState voiceState() const { return voiceStateValue; }
    OverlayMode overlayMode() const { return overlayModeValue; }
    QPointF cursorScreenPosition() const { return cursorScreenPositionValue; }

public slots:
    void setVoiceState(VoiceState newVoiceState);
    void setOverlayMode(OverlayMode newOverlayMode);
    void toggleOverlayMode();
    void togglePushToTalkListening();
    void setCursorScreenPosition(const QPointF& newCursorScreenPosition);

signals:
    void voiceStateChanged();
    void overlayModeChanged();
    void cursorScreenPositionChanged();

private:
    VoiceState voiceStateValue = Idle;
    OverlayMode overlayModeValue = FollowingCursor;
    QPointF cursorScreenPositionValue;
};
