#include "companion_state.h"

CompanionState::CompanionState(QObject* parent)
    : QObject(parent) {}

void CompanionState::setVoiceState(VoiceState newVoiceState) {
    if (voiceStateValue == newVoiceState) {
        return;
    }
    voiceStateValue = newVoiceState;
    emit voiceStateChanged();
}

void CompanionState::setOverlayMode(OverlayMode newOverlayMode) {
    if (overlayModeValue == newOverlayMode) {
        return;
    }
    overlayModeValue = newOverlayMode;
    emit overlayModeChanged();
}

void CompanionState::toggleOverlayMode() {
    setOverlayMode(overlayModeValue == FollowingCursor ? Docked : FollowingCursor);
}

void CompanionState::togglePushToTalkListening() {
    // No AI wiring yet — flips Listening on, then auto returns to Idle on next call.
    // Once audio capture is plugged in, this becomes press/release-driven.
    if (voiceStateValue == Listening) {
        setVoiceState(Idle);
    } else {
        setVoiceState(Listening);
    }
}

void CompanionState::setCursorScreenPosition(const QPointF& newCursorScreenPosition) {
    if (cursorScreenPositionValue == newCursorScreenPosition) {
        return;
    }
    cursorScreenPositionValue = newCursorScreenPosition;
    emit cursorScreenPositionChanged();
}
