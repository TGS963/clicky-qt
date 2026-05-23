import QtQuick
import Clicky

Item {
    id: overlayRoot
    anchors.fill: parent

    property real followOffsetX: 22
    property real followOffsetY: 22
    property real dockOffsetFromRight: 60
    property real dockOffsetFromTop: 60

    property string currentVoiceState: voiceStateNameFor(companionState.voiceState)
    property bool isDocked: companionState.overlayMode === 1

    function voiceStateNameFor(stateEnumValue) {
        // Match CompanionState::VoiceState ordering: 0=Idle 1=Listening 2=Processing 3=Responding
        if (stateEnumValue === 1) return "Listening";
        if (stateEnumValue === 2) return "Processing";
        if (stateEnumValue === 3) return "Responding";
        return "Idle";
    }

    property real cursorPosX: companionState.cursorScreenPosition.x
    property real cursorPosY: companionState.cursorScreenPosition.y

    property real targetX: isDocked
        ? overlayRoot.width - dockOffsetFromRight
        : cursorPosX + followOffsetX
    property real targetY: isDocked
        ? dockOffsetFromTop
        : cursorPosY + followOffsetY

    AgentDot {
        id: agentDot
        currentVoiceState: overlayRoot.currentVoiceState
        x: overlayRoot.targetX - width / 2
        y: overlayRoot.targetY - height / 2

        Behavior on x {
            NumberAnimation {
                duration: overlayRoot.isDocked ? 360 : 80
                easing.type: overlayRoot.isDocked ? Easing.OutCubic : Easing.OutQuad
            }
        }
        Behavior on y {
            NumberAnimation {
                duration: overlayRoot.isDocked ? 360 : 80
                easing.type: overlayRoot.isDocked ? Easing.OutCubic : Easing.OutQuad
            }
        }
    }
}
